/* target specific code goes her */

#include <stdio.h>
#include <string.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>

#include "typedefs.h"
#include "util.h"
#include "engine-err.h"
#include "target.h"
#include "constants.h"

static pid_t inferior_pid;
static bool is_target_running = false;

#define MAX_NR_BKPTS	32
static struct
{
	bool	is_used;
	unsigned char	prev_insn;
	ARM_CORE_WORD	addr;
}
bkpt_data[MAX_NR_BKPTS];

static enum GEAR_ENGINE_ERR_ENUM core_open(struct target_ctl_context * ctx)
{
/*! \todo	remove this */
struct user_regs_struct regs;
int status;

	/* clear breakpoint data */
	memset(bkpt_data, 0, sizeof bkpt_data);
	if ((inferior_pid = fork()) == -1)
	{
		/* error */
		perror("fork");
		panic("");
	}
	else if (!inferior_pid)
	{
		/* child */
		if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1)
			panic("");
		execve("test.elf", (char * []) { 0 }, (char * []) { 0 });
		perror("execve");
		panic("");
	}
	else
	{
		/* parent */
		if (waitpid(inferior_pid, &status, 0) == -1)
		{
			perror("waitpid");
			panic("");
		}
		if (!WIFSTOPPED(status))
		{
			printf("unknown child stop reason!\n");
			panic("");
		}
		if (WSTOPSIG(status) != SIGTRAP)
			panic("");
		printf("ok, child stopped on signal %i\n", WSTOPSIG(status));
		printf("dumping registers:\n");
		if (ptrace(PTRACE_GETREGS, inferior_pid, 0, &regs) == -1)
		{
			perror("ptrace");
			panic("");
		}
		printf("%%eip: 0x%08x\n", regs.eip);
	}
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_close(struct target_ctl_context * ctx)
{
	printf("slaying child\n");
	if (ptrace(PTRACE_KILL, inferior_pid, 0, 0) == -1)
		panic("");
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_read(struct target_ctl_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes)
{
int i;
long target_word;
char * dest_buf;

	dest_buf = (char *) dest;

	if (target_is_core_running(ctx))
		panic("");
	for (i = 0; i < *nbytes;)
	{
		errno = 0;
		target_word = ptrace(PTRACE_PEEKDATA, inferior_pid, source + i, 0);
		if (errno)
		{
			perror("ptrace()");
			printf("error reading core word at address %i\n", source + i);
			panic("");
		}
		dest_buf[i++] = target_word;
		target_word >>= 8;
		if (i == *nbytes)
			break;
		dest_buf[i++] = target_word;
		target_word >>= 8;
		if (i == *nbytes)
			break;
		dest_buf[i++] = target_word;
		target_word >>= 8;
		if (i == *nbytes)
			break;
		dest_buf[i++] = target_word;
	}
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_write(struct target_ctl_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes)
{
int i;
long target_word;
unsigned char * src_buf;

	src_buf = (char *) source;

	if (target_is_core_running(ctx))
		panic("");
	for (i = 0; i < *nbytes;)
	{
		if (*nbytes - i < sizeof(ARM_CORE_WORD))
		{
			/* write a partial word - must first read the word
			 * at the address of interest and merge it with
			 * the new data */
			int t;
			t = i;
			errno = 0;
			target_word = ptrace(PTRACE_PEEKDATA, inferior_pid, dest + t, 0);
			if (errno)
			{
				perror("ptrace()");
				panic("");
			}
			target_word &=~ 0xff;
			target_word |= src_buf[i++];
			if (i != *nbytes)
				target_word &=~ 0xff00, target_word |= src_buf[i++];
			if (i != *nbytes)
				target_word &=~ 0xff0000, target_word |= src_buf[i++];
			errno = 0;
			ptrace(PTRACE_POKEDATA, inferior_pid, dest + t, target_word);
			if (errno)
				panic("");
		}
		else
		{
			/* write a whole word */
			target_word = src_buf[i++];
			target_word |= src_buf[i++] << 8;
			target_word |= src_buf[i++] << 16;
			target_word |= src_buf[i++] << 24;

			errno = 0;
			ptrace(PTRACE_POKEDATA, inferior_pid, dest + i - 4, target_word);
			if (errno)
				panic("");
		}
	}
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_read(struct target_ctl_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i, j;
long int regs[17];

	if (target_is_core_running(ctx))
		panic("");
	if (!mask)
		panic("");
	if (ptrace(PTRACE_GETREGS, inferior_pid, 0, &regs) == -1)
	{
		perror("ptrace");
		panic("");
	}
	for (i = j = 0; i < 17 && mask; i++, mask >>= 1)
		if (mask & 1)
			buffer[j++] = regs[i];
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_write(struct target_ctl_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i, j;
long int regs[17];

	if (target_is_core_running(ctx))
		panic("");
	if (!mask)
		panic("");
	if (ptrace(PTRACE_GETREGS, inferior_pid, 0, &regs) == -1)
	{
		perror("ptrace");
		panic("");
	}
	for (i = j = 0; i < 17 && mask; i++, mask >>= 1)
		if (mask & 1)
			regs[i] = buffer[j++];
	if (ptrace(PTRACE_SETREGS, inferior_pid, 0, &regs) == -1)
	{
		perror("ptrace");
		panic("");
	}
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
long target_word;
int i;
	for (i = 0; i < MAX_NR_BKPTS; i++)
		if (bkpt_data[i].is_used && bkpt_data[i].addr == address)
			panic("");
	for (i = 0; i < MAX_NR_BKPTS; i++)
		if (!bkpt_data[i].is_used)
			break;
	if (i == MAX_NR_BKPTS)
		panic("");
	bkpt_data[i].is_used = true;
	bkpt_data[i].addr = address;

	errno = 0;
	target_word = ptrace(PTRACE_PEEKDATA, inferior_pid, address, 0);
	if (errno)
	{
		perror("ptrace()");
		printf("error reading memory address 0x%08x\n", address);
		panic("");
	}
	bkpt_data[i].prev_insn = target_word;
	target_word &=~ 0xff;
	target_word |= 0xcc;
	errno = 0;
	ptrace(PTRACE_POKEDATA, inferior_pid, address, target_word);
	if (errno)
		panic("");
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_clear_break(struct target_ctl_context * ctx, ARM_CORE_WORD address)
{
long target_word;
int i;
	for (i = 0; i < MAX_NR_BKPTS; i++)
		if (bkpt_data[i].is_used && bkpt_data[i].addr == address)
			break;
	if (i == MAX_NR_BKPTS)
		panic("");

	errno = 0;
	target_word = ptrace(PTRACE_PEEKDATA, inferior_pid, address, 0);
	if (errno)
		panic("");
	target_word &=~ 0xff;
	target_word |= bkpt_data[i].prev_insn;
	errno = 0;
	ptrace(PTRACE_POKEDATA, inferior_pid, address, target_word);
	if (errno)
		panic("");
	bkpt_data[i].is_used = false;
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_step(struct target_ctl_context * ctx)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_run(struct target_ctl_context * ctx/*ARM_CORE_WORD * halt_addr*/)
{
int status;
long target_word;
struct user_regs_struct regs;

	if (ptrace(PTRACE_GETREGS, inferior_pid, 0, &regs) == -1)
	{
		perror("ptrace");
		panic("");
	}
	printf("resuming at %%eip: 0x%08x\n", regs.eip);

	if (ptrace(PTRACE_CONT, inferior_pid, 0, 0) == -1)
	{
		perror("ptrace");
		printf("pid: %i\n", (int) inferior_pid);
		panic("");
	}
	is_target_running = true;
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_ioctl(struct target_ctl_context * ctx, int request_len, ARM_CORE_WORD * request, int * response_len, ARM_CORE_WORD ** response)
{
	panic("");
}

static enum GEAR_ENGINE_ERR_ENUM core_get_status(struct target_ctl_context * ctx, enum TARGET_CORE_STATE_ENUM * status)
{
	if (target_is_core_running(ctx))
		*status = TARGET_CORE_STATE_RUNNING;
	else
		*status = TARGET_CORE_STATE_HALTED;
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
int status;
struct user_regs_struct regs;

	if (!is_target_running)
		return false;
	if (waitpid(inferior_pid, &status, WNOHANG) == -1)
	{
		perror("waitpid");
		panic("");
	}
	if (!WIFSTOPPED(status))
	{
		return true;
	}
	if (WSTOPSIG(status) != SIGTRAP)
	{
		unsigned char buf[16];
		int i = sizeof buf;
		printf("inferior stopped at signal %i\n", WSTOPSIG(status));
		/* undo the program counter by 1 instruction */
		if (ptrace(PTRACE_GETREGS, inferior_pid, 0, &regs) == -1)
		{
			perror("ptrace");
			panic("");
		}
		printf("%%eip: 0x%08x\n", regs.eip);
		printf("opcode bytes:\n");
		is_target_running = false;
		if (core_mem_read(ctx, buf, regs.eip, &i)
				!= GEAR_ERR_NO_ERROR
				|| i != sizeof buf)
			panic("");
		for (i = 0; i < sizeof buf; i++)
			printf("%02x ", buf[i]);
		panic("");
	}
	is_target_running = false;
	/* undo the program counter by 1 instruction */
	if (ptrace(PTRACE_GETREGS, inferior_pid, 0, &regs) == -1)
	{
		perror("ptrace");
		panic("");
	}
	printf("%%eip: 0x%08x\n", regs.eip);
	regs.eip--;
	if (ptrace(PTRACE_SETREGS, inferior_pid, 0, &regs) == -1)
	{
		perror("ptrace");
		panic("");
	}
	return false;
}

enum GEAR_ENGINE_ERR_ENUM target_get_core_access(struct target_ctl_context * ctx)
{
	ctx->cc = &core_funcs;
	return GEAR_ERR_NO_ERROR;
}

