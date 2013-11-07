/* target specific code goes her */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __LINUX__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#else
#include <winsock2.h>
#endif
#include <stdbool.h>

#include "typedefs.h"
#include "util.h"
#include "engine-err.h"
#include "target.h"
#include "constants.h"

/*! \todo	move this into the target_ctl_private_data data structure below */
static int vortex_fd;

enum
{
	NR_MAX_HW_BKPTS		= 32,
};
struct target_ctl_private_data
{
	struct
	{
		int nr_hw_bkpts;
		struct
		{
			uint32_t	addr;
			struct
			{
				int	is_active	: 1;
			};
		} bkpts[NR_MAX_HW_BKPTS];
	}
	bkpt_data;
	bool is_connected;
};

static uint32_t get_vortex_hex_nr(void)
{
uint32_t x;
char c;
int res;

	x = 0;
	while(1)
	{
		res = recv(vortex_fd, & c, 1, 0);
		if (res != 1)
			panic("recv()");
		if (isalnum(c))
			break;
	}

	while (1)
	{
		c = tolower(c);
		if (isalpha(c))
			c -= 'a' - 10;
		else
			c -= '0';
		x <<= 4;
		x |= c;

		res = recv(vortex_fd, & c, 1, 0);
		if (res != 1)
			panic("recv()");
		if (!isalnum(c))
			break;
	}
	printf("%s(): returning 0x%08x\n", __func__, x);
	return x;

}

static void bkpt_sync(struct target_ctl_context * ctx)
{
int i;
struct target_ctl_private_data * p;

	if (target_is_core_running(ctx))
		panic("");
	p = ctx->core_data;
	xprintf(vortex_fd, "[hex] ");
	for (i = 0; i < p->bkpt_data.nr_hw_bkpts; i ++)
		if (p->bkpt_data.bkpts[i].is_active)
			xprintf(vortex_fd, "%x %x armv7m-insert-bkpt\n", i, p->bkpt_data.bkpts[i].addr);
		else
			xprintf(vortex_fd, "%x armv7m-remove-bkpt\n", i);
}

static void step_over_bkpt(void)
{
uint32_t x;	

	if (target_is_core_running(0))
		panic("");
	xprintf(vortex_fd, "[hex] ");
	xprintf(vortex_fd, "FP_CTRL t@ .\n");
	x = get_vortex_hex_nr();
	xprintf(vortex_fd, "FP_CTRL_KEY FP_CTRL t!\n");
	xprintf(vortex_fd, "armv7m-step-no-ints\n");
	if (target_is_core_running(0))
		panic("");
	xprintf(vortex_fd, "%x FP_CTRL_KEY or FP_CTRL t!\n", x);

}

static enum GEAR_ENGINE_ERR_ENUM core_open(struct target_ctl_context * ctx, const char * executable_fname, int argc, const char ** argv, const char ** errmsg_hint)
{
struct sockaddr_in addr;
struct target_ctl_private_data * p;
uint32_t x;
int i;

	p = ctx->core_data;

	if (p->is_connected)
	{
		printf("already connected to a vortex server...\n");
		return GEAR_ERR_NO_ERROR;
	}
	printf("trying to connect to a vortex server...\n");
	if ((vortex_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		panic("socket()");
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1122);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(vortex_fd, (struct sockaddr *) & addr, sizeof addr) == -1)
	{
		printf("connection to a vortex failed\n");
		if (errmsg_hint)
			* errmsg_hint = strdup("connection to the vortex server failed; "
				"check vortex server settings - either the vortex server is "
				"dead, or the vortex probe is dysfunctional"
				);
		return GEAR_ERR_GENERIC_ERROR;
	}
	p->is_connected = true;
	printf("connection to a vortex established\n");

	printf("target is %s\n", target_is_core_running(ctx) ? "running" : "halted");

	/* retrieve the number of hardware breakpoints */
	xprintf(vortex_fd, "FP_CTRL t@ .\n");
	x = get_vortex_hex_nr();
	x = p->bkpt_data.nr_hw_bkpts = ((x >> 4) & 0xf) | ((x >> 8) & 0xf0);
	printf("number of hardware breakpoints: %i\n", x);
	/* disable all hardware breakpoints in the target */
	for (i = 0; i < x; i ++)
		xprintf(vortex_fd, "0 FP_COMP0 %i cells + t!\n", i);

	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_close(struct target_ctl_context * ctx)
{
struct target_ctl_private_data * p;

	p = ctx->core_data;
	if (!p->is_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;
	if (shutdown(vortex_fd, SD_BOTH))
		panic("shutdown()");
	p->is_connected = false;
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_read(struct target_ctl_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes)
{
uint32_t * m, i;

	if (!ctx->core_data->is_connected) return GEAR_ERR_TARGET_CORE_DEAD; if (target_is_core_running(ctx)) return GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING;

	m = (uint32_t *) dest;
	i = * nbytes >> 2;
	xprintf(vortex_fd, "%x %x armv7m-mem-dump\n", source, i);
	while (i --)
		* m ++ = get_vortex_hex_nr();
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_write(struct target_ctl_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes)
{
	if (!ctx->core_data->is_connected) return GEAR_ERR_TARGET_CORE_DEAD; if (target_is_core_running(ctx)) return GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING;

	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_read(struct target_ctl_context * ctx, unsigned mode, unsigned thread_id, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i, j;

	if (!ctx->core_data->is_connected) return GEAR_ERR_TARGET_CORE_DEAD; if (target_is_core_running(ctx)) return GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING;

	if (!mask)
		panic("");
	for (i = j = 0; i < NR_ARM_REGS && mask; i++, mask >>= 1)
		if (mask & 1)
		{
			xprintf(vortex_fd, "%x armv7m-reg-read .\n", i);
			buffer[j++] = get_vortex_hex_nr();
		}
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_write(struct target_ctl_context * ctx, unsigned mode, unsigned thread_id, unsigned long mask, ARM_CORE_WORD buffer[])
{
	if (!ctx->core_data->is_connected) return GEAR_ERR_TARGET_CORE_DEAD; if (target_is_core_running(ctx)) return GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING;

	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_cop_read(struct target_ctl_context * ctx, unsigned CPnum, unsigned long mask, ARM_CORE_WORD buffer[])
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_cop_write(struct target_ctl_context * ctx, unsigned CPnum, unsigned long mask, ARM_CORE_WORD buffer[])
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_set_break(struct target_ctl_context * ctx, ARM_CORE_WORD address)
{
int i, j;
struct target_ctl_private_data * p;

	p = ctx->core_data; if (!p->is_connected) return GEAR_ERR_TARGET_CORE_DEAD; if (target_is_core_running(ctx)) return GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING;
	/* see if a breakpoint is already set on the given address */
	for (j = -1, i = 0; i < p->bkpt_data.nr_hw_bkpts; i ++)
		if (!p->bkpt_data.bkpts[i].is_active)
		{
			if (j == -1)
				j = i;
		}
		else if (p->bkpt_data.bkpts[i].addr == address)
			return GEAR_ERR_BKPT_ALREADY_SET_AT_ADDR;
	if (j == -1)
		return GEAR_ERR_CANT_SET_HW_BKPT;
	p->bkpt_data.bkpts[j].is_active = 1;
	p->bkpt_data.bkpts[j].addr = address;
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM core_clear_break(struct target_ctl_context * ctx, ARM_CORE_WORD address)
{
int i;
struct target_ctl_private_data * p;

	if (target_is_core_running(ctx))
		panic("");
	p = ctx->core_data;
	/* see if a breakpoint is already set on the given address */
	for (i = 0; i < p->bkpt_data.nr_hw_bkpts; i ++)
		if (p->bkpt_data.bkpts[i].is_active
			&& p->bkpt_data.bkpts[i].addr == address)
		{
			break;
		}
	if (i == p->bkpt_data.nr_hw_bkpts)
		panic("");
	p->bkpt_data.bkpts[i].is_active = 0;
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_run(struct target_ctl_context * ctx, unsigned thread_id/*ARM_CORE_WORD * halt_addr*/)
{
	if (!ctx->core_data->is_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;
	if (target_is_core_running(ctx))
		return GEAR_ERR_NO_ERROR;

	bkpt_sync(ctx);
	xprintf(vortex_fd, "armv7m-run\n");
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_halt(struct target_ctl_context * ctx, unsigned thread_id/*ARM_CORE_WORD * halt_addr*/)
{
	if (!ctx->core_data->is_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;
	if (!target_is_core_running(ctx))
		return GEAR_ERR_NO_ERROR;

	xprintf(vortex_fd, "armv7m-halt\n");
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_insn_step(struct target_ctl_context * ctx, unsigned thread_id/*ARM_CORE_WORD * halt_addr*/)
{
	if (!ctx->core_data->is_connected) return GEAR_ERR_TARGET_CORE_DEAD; if (target_is_core_running(ctx)) return GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING;
	bkpt_sync(ctx);
	xprintf(vortex_fd, "armv7m-step-no-ints\n");
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_ioctl(struct target_ctl_context * ctx, int request_len, ARM_CORE_WORD * request, int * response_len, ARM_CORE_WORD ** response)
{
	panic("");
}

static enum GEAR_ENGINE_ERR_ENUM core_get_status(struct target_ctl_context * ctx, enum TARGET_CORE_STATE_ENUM * status)
{
struct target_ctl_private_data * p;

	p = ctx->core_data;
	if (!p->is_connected)
		* status = TARGET_CORE_STATE_DEAD;
	else if (target_is_core_running(ctx))
		* status = TARGET_CORE_STATE_RUNNING;
	else
		* status = TARGET_CORE_STATE_HALTED;
	return GEAR_ERR_NO_ERROR;
}

static struct core_access_struct core_funcs =
{
	/* core_open */
	core_open,
	/* core_close */
	core_close,
	/* core_mem_read */
	core_mem_read,
	/* core_mem_write */
	core_mem_write,
	/* core_reg_read */
	core_reg_read,
	/* core_reg_write */
	core_reg_write,
	/* core_cop_read */
	core_cop_read,
	/* core_cop_write */
	core_cop_write,
	/* core_set_break */
	core_set_break,
	/* core_clear_break */
	core_clear_break,
	/* core_run */
	core_run,
	/* core_halt */
	core_halt,
	/* core_insn_step */
	core_insn_step,
	/* io_ctl */
	core_ioctl,
	/* core_get_status */
	core_get_status,
};

int target_is_core_running(struct target_ctl_context * ctx)
{
uint32_t x;	
	xprintf(vortex_fd, "DHCSR t@ .\n");
	x = get_vortex_hex_nr();
	if (x & (1 << 17))
		return 0;
	else
		return 1;
}

enum GEAR_ENGINE_ERR_ENUM target_get_core_access(struct target_ctl_context * ctx)
{
	ctx->cc = &core_funcs;
	ctx->core_data = calloc(1, sizeof * ctx->core_data);
	ctx->core_data->is_connected = false;
	return GEAR_ERR_NO_ERROR;
}

