
#ifdef CXX
extern "C" {
#endif
#include <unistd.h>
#if __LINUX__
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#else
#define _WIN32_WINNT	0x0501
#include <windows.h>
#include <wincon.h>
#include <winsock2.h>
#include <winuser.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int gprintf(const char * format, ...);

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-constants.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "gear.h"
#include "arm-dis.h"
#include "symtab.h"
#include "dobj-access.h"
#include "type-access.h"
#include "aranges-access.h"
#include "util.h"
#include "srcfile.h"
#include "exec.h"
#include "breakpoint.h"
#include "dbx-support.h"
#include "target-comm.h"
#include "target-img-load.h"
#include "frame-reg-cache.h"
#include "target-dump-cstring.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*! \todo	remove this */
void target_hacks_init(struct gear_engine_context * ctx);

#define MAX_FRONTEND_CONNECTIONS	16
/* total number of open connections to frontend sockets */
static int	nr_open_connections;
struct
{
	/* frontend communication socket file descriptor */
	int	comm_fd;
	bool	is_used;
	char	cmd_buf[1024];
	int	idx;
	/* the remote socket address for this connection */
	struct	sockaddr_in	addr;
}
frontend_comm_data[MAX_FRONTEND_CONNECTIONS];

ssize_t write_to_frontends(const void *buf, size_t count)
{
int i;
	for (i = 0; i < MAX_FRONTEND_CONNECTIONS; i++)
		if (frontend_comm_data[i].is_used)
		{
#if __LINUX__
			if (write(frontend_comm_data[i].comm_fd, buf, count) == -1)
				panic("");
#else
			{
			int err;
				while (1)
					if ((err = send(frontend_comm_data[i].comm_fd, (const char *) buf, count, 0)) != count)
					{
						gprintf("send(): error %i\n", err);
						gprintf("WSAGetLastError(): %i\n", err = WSAGetLastError());
						if (err == WSAEWOULDBLOCK)
							continue;
						panic("");
					}
					else
						break;
			}
#endif
		}
	/*! \todo	this is not really correct, but who bothers anyway... */
	return count;
}


/*! \todo	clean this up */
int target_comm_get_core_access(struct core_control * cc);
/*! \todo	clean this up */
void init_target_comm(struct gear_engine_context * ctx);

void dwarf_hacked_init(struct gear_engine_context * ctx);

#ifdef TARGET_i386
void init_i386_target_desc(struct gear_engine_context * ctx);
#elif defined TARGET_ARMV7M
void init_armv7m_target_desc(struct gear_engine_context * ctx);
#else
#error architecture not defined
#endif

#ifdef CXX
}
#endif

static struct gear_engine_context * ctx;

#define GEAR_BANNER "gear, v0.0.1, the arm debugging gear; copyright 2006-2010 sgs\n\n"


struct format_data def_exam_format = { 'x', 1, 1, };
ARM_CORE_WORD def_exam_addr;

#ifdef CXX
extern "C"
{
#endif
	void init_jtagdrv(void);
	void arm7tdmis_core_access_init(void);
	void arm7tdmis_request_target_halt(void);
	int arm7tdmis_is_debug_halted(void);
#ifdef CXX
}
#endif

static void process_cmdline(struct gear_engine_context * ctx, int argc, char ** argv)
{
/*! \todo	move this to some parameter file */
static const int GEAR_ENGINE_SERVER_SOCKET_PORT_NR = 0x1234;
/*! \todo	move this to some parameter file */
static const int TARGET_CORE_CONTROLLER_SERVER_SOCKET_PORT_NR = 0x1111;
int i;
bool is_exec_specified;

	/* initialize default settings values */
	ctx->settings.server_port_nr = GEAR_ENGINE_SERVER_SOCKET_PORT_NR;
	ctx->settings.target_ctl_port_nr = TARGET_CORE_CONTROLLER_SERVER_SOCKET_PORT_NR;

	is_exec_specified = false;

	for (i = 1; i < argc; i++)
	{
		/* handle long options (starting with '--') */
		if (argv[i][0] == '-' && argv[i][1] == '-')
		{
			if (!strcmp(argv[i] + 2, "help"))
			{
				gprintf(
"gear, a c-language debug engine\n"
"\n"
"usage: %s [--server-port port_number] [--target-port port_number] executable-file\n"
"\n"
"executable-file is an executable file to debug\n"
"	only executables in elf file format are supported, only dwarf debug\n"
"	information is supported machine targets supported - very\n"
"	limited support for arm7 under gdb armulator(very dated),\n"
"	very limited support for i386 linux ptrace() based debugging,\n"
"	very limited support for i386 win32\n"
"\n"
"--server-port port_number\n"
"	specifies socket port number for the gear debug server to listen on\n"
"	for incoming debug connections; if not specified,\n"
"	defaults to 0x1234 (decimal 4660)\n"
"\n"
"--target-port port_number\n"
"	specifies socket port number of the target core controller to which\n"
"	to attempt connection for the purposes of debugging the specified\n"
"	executable; if not specified, defaults to 0x1111 (decimal 4369)\n"
"",
argv[0]
);
				exit(0);
			}
			else if (!strcmp(argv[i] + 2, "server-port"))
			{
				char * endptr;
				long p;
				if (i + 1 == argc)
				{
					gprintf("bad option\n");
					exit(1);
				}
				p = strtoul(argv[i++], &endptr, 0);
				if (*endptr || !p || p > 65535)
				{
					gprintf("bad option\n");
					exit(1);
				}
				ctx->settings.server_port_nr = p;
			}
			else if (!strcmp(argv[i] + 2, "target-port"))
			{
				char * endptr;
				long p;
				if (i + 1 == argc)
				{
					gprintf("bad option\n");
					exit(1);
				}
				p = strtoul(argv[i++], &endptr, 0);
				if (*endptr || !p || p > 65535)
				{
					gprintf("bad option\n");
					exit(1);
				}
				ctx->settings.target_ctl_port_nr = p;
			}
			else
			{
					gprintf("bad option; run with '--help' to get help on command line options\n");
					exit(1);
			}
		}
		else
		{
			/* assume an executable file has been specified */
			if (is_exec_specified)
			{
				gprintf("bad options - multiple executable files specified\n");
				exit(1);
			}
			is_exec_specified = true;
			ctx->settings.dbg_process_disk_file_name = strdup(argv[i]);
		}
	}
	if (!is_exec_specified)
	{
		gprintf("bad options - no executable file specified\n");
		exit(1);
	}
	/* convert the executable to an elf format (if at all necessary)
	 * so that libdwarf can be used for accessing the dwarf debug
	 * information */
#ifdef __LINUX__
	panic("");
#else
	/* run binutils objcopy to do the job... */
	switch (0) {
		wchar_t cmdline[256];
		int j;
		DWORD exit_code;
		STARTUPINFOW si;
		PROCESS_INFORMATION pi;
		struct _stat statbuf, statbuf1;

default:

		memset(&pi, 0, sizeof pi);
		memset(&si, 0, sizeof si);
		/* build the command line */
		wcsncpy(cmdline, L" --output-target elf32-little ", sizeof cmdline / sizeof * cmdline);
		for (i = wcslen(cmdline), j = 0; i < sizeof cmdline / sizeof * cmdline - 1 && ctx->settings.dbg_process_disk_file_name[j]; i ++, j ++)
			cmdline[i] = ctx->settings.dbg_process_disk_file_name[j];
		wcsncat(cmdline, L" ", sizeof cmdline / sizeof * cmdline - i);
		for (i = wcslen(cmdline), j = 0; i < sizeof cmdline / sizeof * cmdline - 1 && ctx->settings.dbg_process_disk_file_name[j]; i ++, j ++)
			cmdline[i] = ctx->settings.dbg_process_disk_file_name[j];
		wcsncat(cmdline + i, L".elf", sizeof cmdline / sizeof * cmdline - i);

		if (!(ctx->dbg_info_elf_disk_file_name = (char *) malloc(strlen(ctx->settings.dbg_process_disk_file_name) + sizeof ".elf")))
		{
			gprintf("out of core\n");
			exit(1);
		}
		strcpy(ctx->dbg_info_elf_disk_file_name, ctx->settings.dbg_process_disk_file_name);
		strcat(ctx->dbg_info_elf_disk_file_name, ".elf");
		/* see if the file already exists and if so, if it also
		 * has a modification date such that, with good probability,
		 * it could be reused, and invoking objcopy can be avoided */
		if (!_stat(ctx->settings.dbg_process_disk_file_name, &statbuf)
			       && !_stat(ctx->dbg_info_elf_disk_file_name, &statbuf1))
		{
			/* files exist, check their modification times */
			if (statbuf1.st_mtime > statbuf.st_mtime)
			{
				/* file can be reused */
				gprintf("ok, file reusable, not running objcopy\n");
				break;
			}
		}

		gprintf("running objcopy, please be patient: %ls\n", cmdline);
		if (!(CreateProcessW(L"p:/mingw32-gcc-4.6.3/bin/arm-none-eabi-objcopy.exe",
					cmdline,
					0,
					0,
					FALSE, /* do not inherit handles */
					0,
					0,
					0,
					&si,
					&pi)))
		{
			wchar_t * msgbuf;
			gprintf("fatal error: failed executing objcopy, error is:\n");
			FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					(const void *) GetLastError(),
					0,
					0,
					(wchar_t *) &msgbuf,
					0,
					0);
			gprintf("%ls\n", msgbuf);
			exit(1);
		}
		if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)
		{
			wchar_t * msgbuf;
			gprintf("fatal error: failed executing objcopy, error is:\n");
			FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					(const void *) GetLastError(),
					0,
					0,
					(wchar_t *) &msgbuf,
					0,
					0);
			gprintf("%ls\n", msgbuf);
			exit(1);
		}
		if (!GetExitCodeProcess(pi.hProcess, &exit_code))
		{
			wchar_t * msgbuf;
			gprintf("fatal error: failed executing objcopy, error is:\n");
			FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					(const void *) GetLastError(),
					0,
					0,
					(wchar_t *) &msgbuf,
					0,
					0);
			gprintf("%ls\n", msgbuf);
			exit(1);
		}
		if (exit_code)
		{
			gprintf("objcopy exit code %i, aborting\n", (int) exit_code);
			exit(1);
		}
	}
#endif

}


/*! \todo	remove this */
HWND hwnd;

struct type_access_stats type_access_stats;

int main(int argc, char ** argv)
{
int i, j;
int res;
struct core_control cc;
/* variables for use in the select() call in the main loop;
 * two file descriptors are monitored for incoming data - one for
 * the frontend and one for the target monitor */
fd_set read_fds, err_fds;
int nfds;
/* server socket file descriptor */
int socket_fd;
int comm_fd;
struct sockaddr_in addr;
int len;
struct timeval tv;
struct timezone tz;

	gprintf("!!! WARNING !!! COMPILING WITH PROFILING AND CODE COVERAGE ENABLED MAY CAUSE VERY CONFUSING AND ERRATIC BEHAVIOR !!!\n");
	gprintf("!!! YOU HAVE BEEN WARNED !!!\n");
	gprintf_switch_on(true);
	gprintf("%s", GEAR_BANNER);

	/* allocate a context */
	ctx = (struct gear_engine_context *) calloc(1, sizeof * ctx);

	process_cmdline(ctx, argc, argv);

#ifndef __LINUX__
	{
		WSADATA w;
		if (WSAStartup(0x0101, &w))
		{
			gprintf("failed to initialize winsock\n");
			exit(1);
		}
	}
#endif

#ifdef TARGET_i386
	init_i386_target_desc(ctx);
#elif defined TARGET_ARMV7M
	init_armv7m_target_desc(ctx);
#else
#error architecture not defined
#endif

	init_target_dump_cstring(ctx);
	init_target_comm(ctx);
	target_comm_get_core_access(ctx->cc = &cc);
	init_exec(ctx);
	init_types(ctx);

	gprintf("initializing symbol table...\n");
	init_symtab(ctx);
	gprintf("\nwarning: performing hacked initialization of the dwarf engine\n");

#if 0
HWND WINAPI CreateWindow(
  __in_opt  LPCTSTR lpClassName,
  __in_opt  LPCTSTR lpWindowName,
  __in      DWORD dwStyle,
  __in      int x,
  __in      int y,
  __in      int nWidth,
  __in      int nHeight,
  __in_opt  HWND hWndParent,
  __in_opt  HMENU hMenu,
  __in_opt  HINSTANCE hInstance,
  __in_opt  LPVOID lpParam
);
#endif
if (0) {
	hwnd = CreateWindow("STATIC",
			"arm-gear progress",
			WS_VISIBLE,
			100, 100, 256, 256,
			GetConsoleWindow(),
			0,
			GetModuleHandle(0),
			0);
	if (!hwnd)
	{
		exit(1);
	}
}


	gprintf_switch_on(true);
	dwarf_hacked_init(ctx);
	gprintf_switch_on(true);

	init_aranges(ctx);

	gprintf("initialization complete\n");
	gprintf("dtype_data usage counts:\n");
	gprintf("nr_base_type_nodes == %i\n", type_access_stats.nr_base_type_nodes);
	gprintf("nr_typedef_nodes == %i\n", type_access_stats.nr_typedef_nodes);
	gprintf("nr_tqual_nodes == %i\n", type_access_stats.nr_tqual_nodes);
	gprintf("nr_arr_type_nodes == %i\n", type_access_stats.nr_arr_type_nodes);
	gprintf("nr_ptr_nodes == %i\n", type_access_stats.nr_ptr_nodes);
	gprintf("nr_struct_nodes == %i\n", type_access_stats.nr_struct_nodes);
	gprintf("nr_union_nodes == %i\n", type_access_stats.nr_union_nodes);
	gprintf("nr_member_nodes == %i\n", type_access_stats.nr_member_nodes);
	gprintf("nr_enumerator_nodes == %i\n", type_access_stats.nr_enumerator_nodes);
	gprintf("nr_enumeration_nodes == %i\n", type_access_stats.nr_enumeration_nodes);
	gprintf("nr_dobj_nodes == %i\n", type_access_stats.nr_dobj_nodes);
	gprintf("nr_subprogram_nodes == %i\n", type_access_stats.nr_subprogram_nodes);
	gprintf("nr_lexblocks == %i\n", type_access_stats.nr_lexblocks);
	gprintf("nr_subprogram_prototype_nodes == %i\n", type_access_stats.nr_subprogram_prototype_nodes);
	gprintf("nr_symtab_nodes == %i\n", type_access_stats.nr_symtab_nodes);

	init_frame_reg_cache(ctx);
	target_hacks_init(ctx);


#ifdef TARGET_ARM
	gprintf("trying to load a test elf image into simulator core...\n");
	target_img_load_elf(ctx, "test.elf", LOAD_RAM | LOAD_FLASH);
#endif

	gprintf("not loading image\n");
	gprintf("initializing frame unwinder...\n");
	srcfile_build_src_cu_tab(ctx);
	
	init_bkpt(ctx);
	//init_thread_ctl();

	/* create frontend comm socket */
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		panic("socket");
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(ctx->settings.server_port_nr);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(socket_fd, (struct sockaddr *) &addr, sizeof addr) == -1)
	{
		panic("bind");
	}

	gprintf("listening on port %i...\n", ctx->settings.server_port_nr);
	if (listen(socket_fd, 0) == -1)
	{
		panic("listen");
	}

	/* change the mode of the frontend server socket to non-blocking
	 * so that it is possible to service multiple connections simultaneously */
#if 0
#if __LINUX__
	if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
		panic("");
#else
	{
		unsigned long x = 1;
	if (ioctlsocket(socket_fd, FIONBIO, &x))
		panic("");
	}
#endif
#endif

	/* frontend connection request dequeuing loop */
	while(1)
	{
		gprintf(
"\t**********************************************************\n"
"\tno connections active, waiting for connection request...\n"
"\t**********************************************************\n"
	);
	len = sizeof addr;


	FD_ZERO(&read_fds);
	FD_ZERO(&err_fds);
	nfds = socket_fd;

	FD_SET(socket_fd, &err_fds);
	FD_SET(socket_fd, &read_fds);
	res = select(nfds + 1, &read_fds, 0, &err_fds, /* block */ 0);
	if (FD_ISSET(socket_fd, &err_fds))
		panic("");

#if __LINUX__
	if ((comm_fd = accept(socket_fd, (struct sockaddr *) &addr, (socklen_t *) &len)) == -1)
#else
	if ((comm_fd = accept(socket_fd, (struct sockaddr *) &addr, (int *) &len)) == -1)
#endif
		panic("accept");

		nr_open_connections = 1;
		gprintf(
"\t****************************************************\n"
"\tconnection accepted from %s, port %i(0x%04x); total connections: %i\n"
"\t****************************************************\n",
	inet_ntoa(addr.sin_addr), addr.sin_port, addr.sin_port, nr_open_connections);

		frontend_comm_data[0].comm_fd = comm_fd;
		frontend_comm_data[0].addr = addr;
		frontend_comm_data[0].idx = 0;
		frontend_comm_data[0].is_used = true;
		nfds = MAX(socket_fd, comm_fd);

/*! \todo	move this to an appropriate place */
#define TARGET_STATE_POLL_PERIOD_MS		30

		/* issue the very first target state query request and
		 * enter the comm loop */
		if (ctx->cc->is_connected(ctx))
			target_comm_issue_core_status_request(ctx);
		if (gettimeofday(&tv, &tz) < 0)
			panic("");
		while (1)
		{
		struct timeval cur_tv, timeout;

			if (gettimeofday(&cur_tv, &tz) < 0)
				panic("");
			/* see if the time to read the target state query response
			 * has come; this doesnt have to be (and isnt) very exact -
		         * with the current code, in case that a connection
			 * has been dropped, delays of at least upto
			 * 2 * TARGET_STATE_POLL_PERIOD_MS +
			 *	net_timeout_of_core_get_status
			 * time units are possible before it is discovered
			 * that the communication has been dropped;
			 * doesnt really matter that much... */
			if (abs((cur_tv.tv_sec - tv.tv_sec) * 1000)
					+ abs((cur_tv.tv_usec - tv.tv_usec) / 1000)
					>= TARGET_STATE_POLL_PERIOD_MS
					
					&& ctx->cc->is_connected(ctx))
			{
			enum TARGET_CORE_STATE_ENUM unused;
				/* time to read the target status query response */
				/* it shouldnt really be necessary to handle errors
				 * here, because, in case of some error, if the
				 * target core state has changed, the appropriate
				 * interested party should have been notified
				 * (this could for example be the executor,
				 * function exec_update_target_state(), or the
				 * target-img-load.c module, etc.), and the error
				 * should have been handled there,
				 * which is about all that should be done in case of
				 * an error */
				if (ctx->cc->core_get_status(ctx, &unused)
						!= GEAR_ERR_NO_ERROR)
					panic("");
				target_comm_issue_core_status_request(ctx);
				if (gettimeofday(&tv, &tz) < 0)
					panic("");
			}

			/* compute file descriptor masks */
			FD_ZERO(&read_fds);
			FD_ZERO(&err_fds);
			for (i = 0; i < MAX_FRONTEND_CONNECTIONS; i++)
				if (frontend_comm_data[i].is_used)
				{
					FD_SET(frontend_comm_data[i].comm_fd, &err_fds);
					FD_SET(frontend_comm_data[i].comm_fd, &read_fds);
				}
			/* if not all connections are busy, allow more */
			if (nr_open_connections < MAX_FRONTEND_CONNECTIONS)
			{
				FD_SET(socket_fd, &err_fds);
				FD_SET(socket_fd, &read_fds);
			}
			timeout.tv_sec = 0;
			timeout.tv_usec = TARGET_STATE_POLL_PERIOD_MS * 1000;
			res = select(nfds + 1, &read_fds, 0, &err_fds, &timeout);
			if (res < 0)
			{
				perror("select");
				if (errno == EBADF)
				{
					/* some of the file descriptors is bad,
					 * maybe the remote host aborted
					 * the connection; fstat() the
					 * file descriptors of all currently
					 * active sockets and close the bad
					 * ones */
					for (i = 0; i < MAX_FRONTEND_CONNECTIONS; i++)
						if (frontend_comm_data[i].is_used)
						{
#if __LINUX__
							struct stat dummy;
							if (fstat(frontend_comm_data[i].comm_fd, &dummy))
#else
							struct _stat dummy;
							if (_fstat(frontend_comm_data[i].comm_fd, &dummy))
#endif
							{
								if (errno != EBADF)
									panic("");
								gprintf("shutting down connection from %s, port %i(0x%04x)\n",
										inet_ntoa(frontend_comm_data[i].addr.sin_addr),
										frontend_comm_data[i].addr.sin_port,
										frontend_comm_data[i].addr.sin_port);
								gprintf(
"\t****************************************************\n"
"\tconnection aborted; total connections %i\n"
"\t****************************************************\n",
								nr_open_connections - 1);
								frontend_comm_data[i].is_used = false;
								nfds = socket_fd;
								for (j = 0; j < MAX_FRONTEND_CONNECTIONS; j++)
									if (frontend_comm_data[j].is_used)
										nfds = MAX(nfds, frontend_comm_data[j].comm_fd);
								if (!--nr_open_connections)
								{
									/* all connections closed */
									/* read the target core status request response
									 * to flush the incoming target controller data
									 * buffer */
									{
									enum TARGET_CORE_STATE_ENUM unused;
									if (ctx->cc->core_get_status(ctx, &unused)
											!= GEAR_ERR_NO_ERROR)
										panic("");
									}
								}
								/* get out of the loop - the number of open
								 * connections is reinspected below, if zero,
								 * the frontend socket reading loop is aborted altogether */
								break;
							}
						}
				}
				/* check for EAGAIN here */
				else
					panic("");
			}
			else if (res == 0)
			{
				/* the timeout has elapsed */
			}
			/* first, see if there are any new connections pending */
			else if (FD_ISSET(socket_fd, &err_fds))
				panic("");
			else if (FD_ISSET(socket_fd, &read_fds))
			{
				/* accept new connection */
				for (i = 0; i < MAX_FRONTEND_CONNECTIONS; i++)
					if (!frontend_comm_data[i].is_used)
						break;
				if (i == MAX_FRONTEND_CONNECTIONS)
					/* impossible, but check anyway... */
					panic("");
#if __LINUX__
				if ((comm_fd = accept(socket_fd, (struct sockaddr *) &addr, (socklen_t *) &len)) == -1)
					panic("accept");
#else
				len = sizeof addr;
				if ((comm_fd = accept(socket_fd, (struct sockaddr *) &addr, (int *) &len)) == -1)
					panic("accept");
#endif

				nr_open_connections++;
					gprintf(
"\t****************************************************\n"
"\tconnection accepted from %s, port %i(0x%04x); total connections: %i\n"
"\t****************************************************\n",
					inet_ntoa(addr.sin_addr), addr.sin_port, addr.sin_port, nr_open_connections);
				nfds = MAX(nfds, comm_fd);
				frontend_comm_data[i].comm_fd = comm_fd;
				frontend_comm_data[i].addr = addr;
				frontend_comm_data[i].idx = 0;
				frontend_comm_data[i].is_used = true;

			}
			/* finally, see on which connections (if any) there is incoming data, and get it */
			else
			{
		for (i = 0; i < MAX_FRONTEND_CONNECTIONS; i++)
			if (frontend_comm_data[i].is_used && FD_ISSET(frontend_comm_data[i].comm_fd, &read_fds))
			{
				comm_fd = frontend_comm_data[i].comm_fd;
				if (frontend_comm_data[i].idx == sizeof(frontend_comm_data[i].cmd_buf))
					panic("");
#if __LINUX__
				res = read(comm_fd, frontend_comm_data[i].cmd_buf + frontend_comm_data[i].idx,
						sizeof(frontend_comm_data[i].cmd_buf) - frontend_comm_data[i].idx);
#else
				res = recv(comm_fd, frontend_comm_data[i].cmd_buf + frontend_comm_data[i].idx,
						sizeof(frontend_comm_data[i].cmd_buf) - frontend_comm_data[i].idx, 0);

#endif
				if (!res)
				{
close_connection:
					gprintf("shutting down connection from %s, port %i(0x%04x)\n",
							inet_ntoa(frontend_comm_data[i].addr.sin_addr),
							frontend_comm_data[i].addr.sin_port,
							frontend_comm_data[i].addr.sin_port);
					gprintf(
"\t****************************************************\n"
"\tconnection aborted; total connections %i\n"
"\t****************************************************\n",
					nr_open_connections - 1);
#if __LINUX__
					if (close(comm_fd))
						panic("");
#else
					if (shutdown(comm_fd, SD_BOTH))
						panic("");
#endif
					frontend_comm_data[i].is_used = false;

					nfds = socket_fd;
					for (j = 0; j < MAX_FRONTEND_CONNECTIONS; j++)
						if (frontend_comm_data[j].is_used)
							nfds = MAX(nfds, frontend_comm_data[j].comm_fd);
					if (!--nr_open_connections)
					{
						/* all connections closed */
						/* read the target core status request response
						 * to flush the incoming target controller data
						 * buffer */
						if (ctx->cc->is_connected(ctx))
						{
						enum TARGET_CORE_STATE_ENUM unused;
						if (ctx->cc->core_get_status(ctx, &unused)
								!= GEAR_ERR_NO_ERROR)
							panic("");
						}
					}
					/* get out of the loop - the number of open
					 * connections is reinspected below, if zero,
					 * the frontend socket reading loop is aborted altogether */
					break;
				}
				if (res == -1)
				{
#if __LINUX__
					if (errno == ECONNRESET)
#else
					if (errno == WSAECONNRESET)
#endif
					{
						perror("read");
						goto close_connection;
					}
					perror("read");
					panic("");
				}
				/* see if a whole frontend request record has been
				 * received (scan for a newline) */
				for (j = frontend_comm_data[i].idx; j < frontend_comm_data[i].idx + res; j++)
					if (frontend_comm_data[i].cmd_buf[j] == '\n')
					{
						/* a record has been found - process it */
						/* first, see if any token has been supplied */	 
						frontend_comm_data[i].cmd_buf[j] = '\0';
						/* just prior to invoking the command parser, the target
						 * status query response must be read so that it does
						 * not interfere with requests sent from the command
						 * parser to the target controller */
						/* it shouldnt really be necessary to handle errors
						 * here, because, in case of some error, if the
						 * target core state has changed, the appropriate
						 * interested party should have been notified
						 * (this could for example be the executor,
						 * function exec_update_target_state(), or the
						 * target-img-load.c module, etc.), and the error
						 * should have been handled there,
						 * which is about all that should be done in case of
						 * an error */
						if (ctx->cc->is_connected(ctx))
						{
						enum TARGET_CORE_STATE_ENUM unused;
						if (ctx->cc->core_get_status(ctx, &unused)
								!= GEAR_ERR_NO_ERROR)
							panic("");
						}
						/* invoke the parser */
						gprintf("frontend> \n\n%s\n", frontend_comm_data[i].cmd_buf);
						/*! \todo	remove this; this is here to add
						 *		a (pseudo)random delay to test
						 *		how th*/
						dbx_parse(ctx, frontend_comm_data[i].cmd_buf);
						/* this is needed for proper
						 * target core status tracking */
						if (ctx->cc->is_connected(ctx))
							target_comm_issue_core_status_request(ctx);
						/* this will cause the target core status request
						 * to be requested immediately in the loop above;
						 * this is needed to avoid lengthy delays when reporting
						 * the completion of operations which take relatively
						 * small amounts of time (such as instruction level
						 * single stepping), and so to decrease nuisance for
						 * the debuger users */
						if (gettimeofday(&tv, &tz) < 0)
							panic("");
						tv = cur_tv;
						tv.tv_sec -= (TARGET_STATE_POLL_PERIOD_MS + 1000) / 1000;
						memmove(frontend_comm_data[i].cmd_buf, frontend_comm_data[i].cmd_buf + j + 1,
								sizeof(frontend_comm_data[i].cmd_buf) - j - 1);
						/* restart loop */
						j -= frontend_comm_data[i].idx;
						res -= j + 1;
						j = frontend_comm_data[i].idx = 0;
					}
				frontend_comm_data[i].idx += res;
			}
			}
			if (!nr_open_connections)
				break;
		}
	}
	return 0;
}


