/* target specific code goes her */

#include <stdio.h>
#include <string.h>

#include "typedefs.h"
#include "util.h"
#include "engine-err.h"
#include "target.h"
#include "constants.h"

static enum GEAR_ENGINE_ERR_ENUM core_open(struct target_ctl_context * ctx, const char * executable_fname, int argc, const char ** argv)
{
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_close(struct target_ctl_context * ctx)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_read(struct target_ctl_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes)
{
	if (target_is_core_running(ctx))
		panic("");
	memset(dest, 0, *nbytes);
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_write(struct target_ctl_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes)
{
	if (target_is_core_running(ctx))
		panic("");
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_read(struct target_ctl_context * ctx, unsigned mode, unsigned thread_id, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i, j;

	if (target_is_core_running(ctx))
		panic("");
	if (!mask)
		panic("");
	for (i = j = 0; i < NR_ARM_REGS && mask; i++, mask >>= 1)
		if (mask & 1)
			buffer[j++] = 0;
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_write(struct target_ctl_context * ctx, unsigned mode, unsigned thread_id, unsigned long mask, ARM_CORE_WORD buffer[])
{
	if (target_is_core_running(ctx))
		panic("");
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
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_clear_break(struct target_ctl_context * ctx, ARM_CORE_WORD address)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_step(struct target_ctl_context * ctx)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_run(struct target_ctl_context * ctx, unsigned thread_id/*ARM_CORE_WORD * halt_addr*/)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_ioctl(struct target_ctl_context * ctx, int request_len, ARM_CORE_WORD * request, int * response_len, ARM_CORE_WORD ** response)
{
	panic("");
}

static enum GEAR_ENGINE_ERR_ENUM core_get_status(struct target_ctl_context * ctx, enum TARGET_CORE_STATE_ENUM * status)
{
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
	/* core_step */
	core_step,
	/* core_run */
	core_run,
	/* io_ctl */
	core_ioctl,
	/* core_get_status */
	core_get_status,
};

int target_is_core_running(struct target_ctl_context * ctx)
{
	return 0;
}

enum GEAR_ENGINE_ERR_ENUM target_get_core_access(struct target_ctl_context * ctx)
{
	ctx->cc = &core_funcs;
	return GEAR_ERR_NO_ERROR;
}

