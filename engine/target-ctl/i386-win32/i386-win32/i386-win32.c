/* target specific code goes her */

#include <stdio.h>
#include <string.h>

#undef PTW32_STATIC_LIB

#include <pthread.h>
#include <stdbool.h>

#include <windows.h>

#include "typedefs.h"
#include "util.h"
#include "engine-err.h"
#include "target.h"
#include "constants.h"

struct target_ctl_private_data
{
	pthread_barrier_t	barrier;
	pthread_mutex_t		run_mutex;
	pthread_cond_t		run_cond;
	pthread_t		target_monitor_thread_handle;
	/* as returned by WaitForDebugEvent() */
	unsigned long		dbg_event_code;
	bool			is_core_running;
	/* child process related data */
	STARTUPINFO		si;
	PROCESS_INFORMATION	pi;
	const char		* executable_fname;
	int			argc;
	const char		** argv;
	struct
	{
		unsigned long	addr;
		unsigned char	orig_byte;
		bool		is_used;
	}
#define MAX_NR_BKPTS		64	
	bkpt_data[MAX_NR_BKPTS];
#define MAX_THREADS		64
	/* in this array, a NULL value means an unused entry;
	 * thread ids are passed to/from the gear engine as
	 * numbers which are indexes in this array */
	HANDLE	thread_ids[MAX_THREADS];
};


static void * target_monitor_thread(void * param)
{
struct target_ctl_context * ctx = (struct target_ctl_context *) param;
struct target_ctl_private_data * p = ctx->core_data;
DEBUG_EVENT dbg_evt;
int res;
int once = 1;
char cmdline[32768], * pcmdline;
int clen;
int i, j;

	if (pthread_mutex_lock(&p->run_mutex))
		panic("");

	/* construct the child command line */
	pcmdline = cmdline;
	clen = sizeof cmdline - 1;
	/* in the loop below, the target program name
	 * will be copied as its first argument */
	/* skip the first command line argument -
	 * it is this target-controller's executable name */
	j = 1;
	do
	{
		*pcmdline++ = '"';
		clen --;
		i = strlen(p->argv[j]);
		if (i > clen)
			i = clen;
		strncpy(pcmdline, p->argv[j], i);
		pcmdline += i;
		clen -= i;
		if (clen)
		{
			*pcmdline++ = '"';
			clen --;
			if (clen)
			{
				*pcmdline++ = ' ';
				clen --;
			}
		}
		j ++;
	}
	while (j < p->argc && clen != 0);
	*pcmdline = '\0';

	/* deploy the child process */
	memset(&p->si, 0, sizeof p->si);
	p->si.cb = sizeof p->si;
	memset(&p->pi, 0, sizeof p->pi);

	if (!CreateProcess(p->executable_fname,
				cmdline /* command line */,
				0 /* process handle not inheritable */,
				0 /* thread handle not inheritable */,
				FALSE /* handle inheritance is FALSE */,
				DEBUG_PROCESS /* creation flags - debug the child */,
				0 /* use parent environment */,
				0 /* use parent starting directory */,
				&p->si,
				&p->pi))
	{
		panic("failed to create child process for debugging\n");
	}
	/* wait for debugged child stop notification */  
	if (!WaitForDebugEvent(&dbg_evt, INFINITE))
		panic("");
	if (dbg_evt.dwDebugEventCode != CREATE_PROCESS_DEBUG_EVENT)
		panic("");

	res = pthread_barrier_wait(&p->barrier);
	if (res != 0 && res != PTHREAD_BARRIER_SERIAL_THREAD)
		panic("");
	while (1)
	{
		if (pthread_cond_wait(&p->run_cond, &p->run_mutex))
			panic("");
		p->is_core_running = true;
		/* do any other run setup here */
		/* acknowledge the run request has been accepted */
		res = pthread_barrier_wait(&p->barrier);
		if (res != 0 && res != PTHREAD_BARRIER_SERIAL_THREAD)
			panic("");
		while (1)
		{
			if (!ContinueDebugEvent(dbg_evt.dwProcessId, dbg_evt.dwThreadId, DBG_CONTINUE))
				panic("");
			if (!WaitForDebugEvent(&dbg_evt, INFINITE))
				panic("");
			printf("debug event reported: 0x%08x\n", (unsigned int)(p->dbg_event_code = dbg_evt.dwDebugEventCode));
			switch (dbg_evt.dwDebugEventCode)
			{
				case CREATE_PROCESS_DEBUG_EVENT:
					printf("create process event"); panic("");
				case CREATE_THREAD_DEBUG_EVENT:
					printf("create thread event");
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					printf("new thread created - handle this properly\n");
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					break;
				case EXIT_PROCESS_DEBUG_EVENT:
					printf("exit process event"); panic("");
				case EXIT_THREAD_DEBUG_EVENT:
					printf("exit thread event");
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					printf("thread destroyed - handle this properly\n");
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					break;
				case OUTPUT_DEBUG_STRING_EVENT:
					printf("output debug string event:\n");
					printf("---------------------------------------------\n");
					{
						OUTPUT_DEBUG_STRING_INFO dbgstr;
						dbgstr = dbg_evt.u.DebugString;
						if (dbgstr.fUnicode)
							panic("");
						else
						{
							unsigned int nbytes;
							char * pstr;
							/* fetch the string from process memory */
							nbytes = dbgstr.nDebugStringLength;
							if (!(pstr = malloc(nbytes)))
								panic("");
							if (ctx->cc->core_mem_read(ctx,
										pstr,
										(ARM_CORE_WORD) dbgstr.lpDebugStringData,
										&nbytes) != GEAR_ERR_NO_ERROR
									|| nbytes != dbgstr.nDebugStringLength)
								panic("");
							printf("%s", pstr);
							free(pstr);
						}
					}
					printf("\n---------------------------------------------\n");
					continue;
					break;
				case RIP_EVENT:
					printf("rip event"); panic("");
				case UNLOAD_DLL_DEBUG_EVENT:
					printf("unload dll event");
					break;
				case EXCEPTION_DEBUG_EVENT:
					printf("exception raised in thread 0x%08x\n", (unsigned int) dbg_evt.dwThreadId);
					printf("code is 0x%08x\n", (unsigned int) dbg_evt.u.Exception.ExceptionRecord.ExceptionCode);
					printf("address is 0x%08x\n", (unsigned int) dbg_evt.u.Exception.ExceptionRecord.ExceptionAddress);
					if (dbg_evt.u.Exception.ExceptionRecord.ExceptionCode != EXCEPTION_BREAKPOINT)
						panic("");
					dbg_evt.u.Exception.ExceptionRecord.ExceptionCode;
					/*! \todo	fix this - there is some one-time breakpoint
					 *		hit in winnt.dll, at dbgUiConnectToDbg... */ 
					if (!once)
					{
						/* breakpoint hit - fix the program counter as it is now one instruction
						 * past the breakpoint instruction - get the program counter to
						 * point exactly to the breakpoint instruction as this is the
						 * value expected by the gear engine */
						CONTEXT c = { .ContextFlags = CONTEXT_FULL };

						if (!GetThreadContext(p->pi.hThread, &c))
							panic("");
						c.Eip --;
						if (!SetThreadContext(p->pi.hThread, &c))
							panic("");
						goto out;
					}
					once = 0;
					break;
				case LOAD_DLL_DEBUG_EVENT:
					{
						extern const char * GetFileNameFromHandle(HANDLE hFile);
						const char * dllname;

						LOAD_DLL_DEBUG_INFO * dll;
						dll = &dbg_evt.u.LoadDll;
						dllname = GetFileNameFromHandle(dll->hFile);
						CloseHandle(dll->hFile);
						printf("dll load debug event:\n");
						if (dllname)
						{
							printf("dll name: %s\n", dllname);
							free(dllname);
						}
						printf("dll load image base: 0x%08x\n", (unsigned int) dll->lpBaseOfDll);
						printf("continuing the process...\n");
						break;
					}
			}
			continue;
		}
out:		
		p->is_core_running = false;
	}
}

static enum GEAR_ENGINE_ERR_ENUM core_open(struct target_ctl_context * ctx, const char * executable_fname, int argc, const char ** argv)
{
	struct target_ctl_private_data * p;
	int res;

	p = ctx->core_data;
	p->executable_fname = strdup(executable_fname);
	p->argc = argc;
	p->argv = argv;
	if (pthread_create(&p->target_monitor_thread_handle, 0, target_monitor_thread, ctx))
		panic("");
	/* make sure the target monitor thread initialization is complete */
	res = pthread_barrier_wait(&p->barrier);
	if (res != 0 && res != PTHREAD_BARRIER_SERIAL_THREAD)
		panic("");
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_close(struct target_ctl_context * ctx)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_read(struct target_ctl_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes)
{
	DWORD bytes_read;

	if (!ReadProcessMemory(ctx->core_data->pi.hProcess, (LPVOID) source, dest, *nbytes, &bytes_read))
	{
		printf("!!! does not read %i bytes from address 0x%08x\n", *nbytes, source);
		return GEAR_ERR_GENERIC_ERROR;
	}
	if (bytes_read != *nbytes)
		panic("");
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_write(struct target_ctl_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes)
{
	DWORD bytes_written;

	if (target_is_core_running(ctx))
		panic("");
	if (!WriteProcessMemory(ctx->core_data->pi.hProcess, (LPVOID) dest, (LPCVOID) source, *nbytes, &bytes_written))
		panic("");
	if (bytes_written != *nbytes)
		panic("");
	if (!FlushInstructionCache(ctx->core_data->pi.hProcess, (LPVOID) dest, bytes_written))
		panic("");
	printf("flushing %i bytes at addres 0x%08x\n", bytes_written, dest);
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_read(struct target_ctl_context * ctx, unsigned mode, unsigned thread_id, unsigned long mask, ARM_CORE_WORD buffer[])
{
	int i, j;
	CONTEXT c = { .ContextFlags = CONTEXT_FULL };
	DWORD * p = &c.SegGs;
#if 0
	/* copied verbatim from the mingw winnt.h */

#ifdef _X86_
#define SIZE_OF_80387_REGISTERS	80
#define CONTEXT_i386	0x10000
#define CONTEXT_i486	0x10000
#define CONTEXT_CONTROL	(CONTEXT_i386|0x00000001L)
#define CONTEXT_INTEGER	(CONTEXT_i386|0x00000002L)
#define CONTEXT_SEGMENTS	(CONTEXT_i386|0x00000004L)
#define CONTEXT_FLOATING_POINT	(CONTEXT_i386|0x00000008L)
#define CONTEXT_DEBUG_REGISTERS	(CONTEXT_i386|0x00000010L)
#define CONTEXT_EXTENDED_REGISTERS (CONTEXT_i386|0x00000020L)
#define CONTEXT_FULL	(CONTEXT_CONTROL|CONTEXT_INTEGER|CONTEXT_SEGMENTS)
#define MAXIMUM_SUPPORTED_EXTENSION  512
	typedef struct _FLOATING_SAVE_AREA {
		DWORD	ControlWord;
		DWORD	StatusWord;
		DWORD	TagWord;
		DWORD	ErrorOffset;
		DWORD	ErrorSelector;
		DWORD	DataOffset;
		DWORD	DataSelector;
		BYTE	RegisterArea[80];
		DWORD	Cr0NpxState;
	} FLOATING_SAVE_AREA;
	typedef struct _CONTEXT {
		DWORD	ContextFlags;
		DWORD	Dr0;
		DWORD	Dr1;
		DWORD	Dr2;
		DWORD	Dr3;
		DWORD	Dr6;
		DWORD	Dr7;
		FLOATING_SAVE_AREA FloatSave;
		DWORD	SegGs;
		DWORD	SegFs;
		DWORD	SegEs;
		DWORD	SegDs;
		DWORD	Edi;
		DWORD	Esi;
		DWORD	Ebx;
		DWORD	Edx;
		DWORD	Ecx;
		DWORD	Eax;
		DWORD	Ebp;
		DWORD	Eip;
		DWORD	SegCs;
		DWORD	EFlags;
		DWORD	Esp;
		DWORD	SegSs;
		BYTE	ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
	} CONTEXT;

#endif
	/* end of verbatim copy from the mingw winnt.h */
#endif
	if (!mask)
		panic("");
	if (target_is_core_running(ctx))
		panic("");
	if (!GetThreadContext(ctx->core_data->pi.hThread, &c))
		panic("");
	for (i = j = 0; i < NR_ARM_REGS && mask; i++, mask >>= 1)
		if (mask & 1)
			buffer[j++] = p[i];
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_write(struct target_ctl_context * ctx, unsigned mode, unsigned thread_id, unsigned long mask, ARM_CORE_WORD buffer[])
{
	int i, j;
	CONTEXT c = { .ContextFlags = CONTEXT_FULL };
	DWORD * p = &c.SegGs;
	/* copied verbatim from the mingw winnt.h */
#if 0

#ifdef _X86_
#define SIZE_OF_80387_REGISTERS	80
#define CONTEXT_i386	0x10000
#define CONTEXT_i486	0x10000
#define CONTEXT_CONTROL	(CONTEXT_i386|0x00000001L)
#define CONTEXT_INTEGER	(CONTEXT_i386|0x00000002L)
#define CONTEXT_SEGMENTS	(CONTEXT_i386|0x00000004L)
#define CONTEXT_FLOATING_POINT	(CONTEXT_i386|0x00000008L)
#define CONTEXT_DEBUG_REGISTERS	(CONTEXT_i386|0x00000010L)
#define CONTEXT_EXTENDED_REGISTERS (CONTEXT_i386|0x00000020L)
#define CONTEXT_FULL	(CONTEXT_CONTROL|CONTEXT_INTEGER|CONTEXT_SEGMENTS)
#define MAXIMUM_SUPPORTED_EXTENSION  512
	typedef struct _FLOATING_SAVE_AREA {
		DWORD	ControlWord;
		DWORD	StatusWord;
		DWORD	TagWord;
		DWORD	ErrorOffset;
		DWORD	ErrorSelector;
		DWORD	DataOffset;
		DWORD	DataSelector;
		BYTE	RegisterArea[80];
		DWORD	Cr0NpxState;
	} FLOATING_SAVE_AREA;
	typedef struct _CONTEXT {
		DWORD	ContextFlags;
		DWORD	Dr0;
		DWORD	Dr1;
		DWORD	Dr2;
		DWORD	Dr3;
		DWORD	Dr6;
		DWORD	Dr7;
		FLOATING_SAVE_AREA FloatSave;
		DWORD	SegGs;
		DWORD	SegFs;
		DWORD	SegEs;
		DWORD	SegDs;
		DWORD	Edi;
		DWORD	Esi;
		DWORD	Ebx;
		DWORD	Edx;
		DWORD	Ecx;
		DWORD	Eax;
		DWORD	Ebp;
		DWORD	Eip;
		DWORD	SegCs;
		DWORD	EFlags;
		DWORD	Esp;
		DWORD	SegSs;
		BYTE	ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
	} CONTEXT;

#endif
#endif
	/* end of verbatim copy from the mingw winnt.h */


	if (!mask)
		panic("");
	if (target_is_core_running(ctx))
		panic("");
	if (!GetThreadContext(ctx->core_data->pi.hThread, &c))
		panic("");
	for (i = j = 0; i < NR_ARM_REGS && mask; i++, mask >>= 1)
		if (mask & 1)
			p[i] = buffer[j++];
	c.ContextFlags = CONTEXT_FULL;
	if (!SetThreadContext(ctx->core_data->pi.hThread, &c))
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

/* returns -1 if the breakpoint was not found; at *free_idx - returns the index of a free record, -1 if none */
static int bkpt_locate_for_addr(struct target_ctl_context * ctx, ARM_CORE_WORD addr, int * free_idx)
{
	int i;
	* free_idx = -1;
	for (i = 0; i < MAX_NR_BKPTS; i++)
	{
		if (!ctx->core_data->bkpt_data[i].is_used)
			* free_idx = i;
		if (ctx->core_data->bkpt_data[i].is_used && ctx->core_data->bkpt_data[i].addr == addr)
			return i;
	}
	return -1;
}

static enum GEAR_ENGINE_ERR_ENUM core_set_break(struct target_ctl_context * ctx, ARM_CORE_WORD address)
{
	int i, n;
	struct target_ctl_private_data * p;

	printf("setting breakpoint for address 0x%08x\n", (unsigned int) address);
	p = ctx->core_data;
	if (target_is_core_running(ctx))
		panic("");
	if (bkpt_locate_for_addr(ctx, address, &i) != -1 || i == -1)
		panic("");
	n = 1;
	if (core_mem_read(ctx, &p->bkpt_data[i].orig_byte, address, &n) != GEAR_ERR_NO_ERROR)
		panic("");
	n = 1;
	if (core_mem_write(ctx, address, (char[]){[0] = 0xcc}, &n) != GEAR_ERR_NO_ERROR)
		panic("");
	p->bkpt_data[i].addr = address;
	p->bkpt_data[i].is_used = 1;
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_clear_break(struct target_ctl_context * ctx, ARM_CORE_WORD address)
{
	int i, n;
	struct target_ctl_private_data * p;

	p = ctx->core_data;
	if (target_is_core_running(ctx))
		panic("");
	if ((i = bkpt_locate_for_addr(ctx, address, &i)) == -1)
		panic("");
	n = 1;
	if (core_mem_write(ctx, address, &p->bkpt_data[i].orig_byte, &n) != GEAR_ERR_NO_ERROR)
		panic("");
	p->bkpt_data[i].is_used = 0;

	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_step(struct target_ctl_context * ctx)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_run(struct target_ctl_context * ctx, unsigned thread_id/*ARM_CORE_WORD * halt_addr*/)
{
	struct target_ctl_private_data * p;
	int res;

	p = ctx->core_data;
	if (target_is_core_running(ctx))
		panic("");
	if (pthread_mutex_lock(&p->run_mutex))
		panic("");
	if (pthread_cond_signal(&p->run_cond))
		panic("");
	if (pthread_mutex_unlock(&p->run_mutex))
		panic("");
	/* acknowledge the run request has been accepted */
	res = pthread_barrier_wait(&p->barrier);
	if (res != 0 && res != PTHREAD_BARRIER_SERIAL_THREAD)
		panic("");

	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_ioctl(struct target_ctl_context * ctx, int request_len, ARM_CORE_WORD * request, int * response_len, ARM_CORE_WORD ** response)
{
	panic("");
}

static enum GEAR_ENGINE_ERR_ENUM core_get_dbg_event(struct target_ctl_context * ctx, enum TARGET_CORE_STATE_ENUM * status)
{
	panic("");
	if (target_is_core_running(ctx))
		* status = TARGET_CORE_STATE_RUNNING;
	else		
		* status = TARGET_CORE_STATE_HALTED;
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM core_get_status(struct target_ctl_context * ctx, enum TARGET_CORE_STATE_ENUM * status)
{
	if (target_is_core_running(ctx))
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
	return ctx->core_data->is_core_running;
}

enum GEAR_ENGINE_ERR_ENUM target_get_core_access(struct target_ctl_context * ctx)
{
	struct target_ctl_private_data * p;

	/* perform pthread data initialization and deploy the target
	 * monitoring thread */
	if (!(ctx->core_data = p = calloc(1, sizeof * p)))
		panic("");
	if (pthread_barrier_init(&p->barrier, 0, 2))
		panic("");
	p->run_mutex = PTHREAD_MUTEX_INITIALIZER;
	p->run_cond = PTHREAD_COND_INITIALIZER;
	p->is_core_running = false;
	ctx->cc = &core_funcs;
	return GEAR_ERR_NO_ERROR;
}

