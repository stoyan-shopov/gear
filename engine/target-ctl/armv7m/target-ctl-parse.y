
%error-verbose
%parse-param { struct target_ctl_context * ctx }
%debug

%{

/* this is a very simple stub template for writing target core controllers
 * to be used by the gear engine; the main routine opens a socket and
 * listens for a gear engine to connect to the socket; when a
 * connection is established, a request dispatching loop is entered,
 * which waits for incoming requests from the gear engine and processes
 * them; a trivial request parser is coded here; the syntax of the
 * requests supported and the replies should be evident from the
 * simple parser grammar; also read the comments in file target-comm.c
 * from the gear engine source code package */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef __LINUX__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#else
#include <winsock2.h>
#endif


#include <stdarg.h>
#include <stdint.h>

#include "typedefs.h"
#include "util.h"

#include "engine-err.h"

#include "target.h"


#define TARGET_CTL_DEFAULT_PORT		0x1111

static const int COMM_BUF_SIZE = 0x1000;

static int gear_comm_fd;

static struct
{
	/* register mask value used when writing to registers */
	int regmask;
	int reg_nr;
	ARM_CORE_WORD	addr;
	ARM_CORE_WORD	mem_write_len;
	ARM_CORE_WORD	* ioctl_buf;
	int ioctl_req_len;
	int ioctl_buf_idx;
	unsigned char * mem_write_buf;
	int	buf_idx;
	int	is_scanner_aborted;
	unsigned char * comm_buf;
	int lex_idx;
	int comm_buf_idx;
}
lex_vars;

#include "lex.yy.c"

void yyerror (struct target_ctl_context * ctx, char const * s)
{
	printf("%s\n", s);
	if (!lex_vars.is_scanner_aborted)
		panic("");
}

static void dump_errcode(enum GEAR_ENGINE_ERR_ENUM err)
{
static const char * errmsgs[GEAR_ERR_NR_ERR_CODES] =
{
	[GEAR_ERR_NO_ERROR] = "GEAR_ERR_NO_ERROR",
	[GEAR_ERR_GENERIC_ERROR] = "GEAR_ERR_GENERIC_ERROR",
	[GEAR_ERR_DBX_CMD_NOT_CODED] = "GEAR_ERR_DBX_CMD_NOT_CODED",
	[GEAR_ERR_DBX_CMD_NOT_RECOGNIZED] = "GEAR_ERR_DBX_CMD_NOT_RECOGNIZED",
	[GEAR_ERR_DBX_CMD_STRING_EMPTY] = "GEAR_ERR_DBX_CMD_STRING_EMPTY",
	[GEAR_ERR_EXPR_SYNTAX_ERROR] = "GEAR_ERR_EXPR_SYNTAX_ERROR",
	[GEAR_ERR_EXPR_SEMANTIC_ERROR] = "GEAR_ERR_EXPR_SEMANTIC_ERROR",
	[GEAR_ERR_TARGET_CORE_CONNECTION_FAILED] = "GEAR_ERR_TARGET_CORE_CONNECTION_FAILED",
	[GEAR_ERR_TARGET_CORE_DEAD] = "GEAR_ERR_TARGET_CORE_DEAD",
	[GEAR_ERR_TARGET_CTL_BAD_PARAMS] = "GEAR_ERR_TARGET_CTL_BAD_PARAMS",
	[GEAR_ERR_TARGET_CTL_MEM_READ_ERROR] = "GEAR_ERR_TARGET_CTL_MEM_READ_ERROR",
	[GEAR_ERR_TARGET_ACCESS_ERROR] = "GEAR_ERR_TARGET_ACCESS_ERROR",
	[GEAR_ERR_BKPT_ALREADY_SET_AT_ADDR] = "GEAR_ERR_BKPT_ALREADY_SET_AT_ADDR",
	[GEAR_ERR_CANT_SET_HW_BKPT] = "GEAR_ERR_CANT_SET_HW_BKPT",
	[GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING] = "GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING",
	[GEAR_ERR_CANT_UNWIND_STACK_FRAME] = "GEAR_ERR_CANT_UNWIND_STACK_FRAME",
	[GEAR_ERR_CANT_REWIND_STACK_FRAME] = "GEAR_ERR_CANT_REWIND_STACK_FRAME",
	[GEAR_ERR_BACKTRACE_DATA_UNAVAILABLE] = "GEAR_ERR_BACKTRACE_DATA_UNAVAILABLE",
};
	if (err >= GEAR_ERR_NR_ERR_CODES)
		panic("");

	xprintf(gear_comm_fd, "%s,", errmsgs[err]);

}

%}


%union
{
	ARM_CORE_WORD		num;
}

%token CORE_OPEN_KW
%token MEM_READ_KW
%token MEM_WRITE_KW
%token REGS_READ_KW
%token REGS_WRITE_KW
%token CORE_RUN_KW
%token CORE_HALT_KW
%token CORE_INSN_STEP_KW
%token BKPT_SET_KW
%token BKPT_CLEAR_KW
%token GET_STATE_KW
%token IOCTL_KW
%token NUM
%type <num>	NUM

%start	gear_engine_request

%%

gear_engine_request
		:	IOCTL_KW '(' ioctl_data ')'
		{
			int response_len;
			ARM_CORE_WORD * response;
			int i;
			if (ctx->cc->core_ioctl(ctx, lex_vars.ioctl_req_len, lex_vars.ioctl_buf,
						&response_len, &response) != GEAR_ERR_NO_ERROR)
				panic("");
			xprintf(gear_comm_fd, "GEAR_ERR_NO_ERROR,");
			xprintf(gear_comm_fd, "%i,", response_len);
			for (i = 0; i < response_len; i++)
				xprintf(gear_comm_fd, "0x%08x ", response[i]);
			xprintf(gear_comm_fd, "\n");
			free(lex_vars.ioctl_buf);
			if (response && response_len)
				free(response);
		}
		|	CORE_OPEN_KW
		{
			const char * errmsg_hint;
			enum GEAR_ENGINE_ERR_ENUM err;
			enum TARGET_CORE_STATE_ENUM state;

			err = ctx->cc->core_open(ctx, 0, 0, 0, &errmsg_hint);

			dump_errcode(err);
			if (err == GEAR_ERR_GENERIC_ERROR && errmsg_hint)
			{
				xprintf(gear_comm_fd, "ERRMSG_HINT=\"%s\",", errmsg_hint);
				free(errmsg_hint);
			}

			if (ctx->cc->core_get_status(ctx, &state) !=
					GEAR_ERR_NO_ERROR)
				panic("");
			switch (state)
			{
				case TARGET_CORE_STATE_DEAD:
					xprintf(gear_comm_fd, "TARGET_CORE_STATE_DEAD");
					break;
				case TARGET_CORE_STATE_HALTED:
					xprintf(gear_comm_fd, "TARGET_CORE_STATE_HALTED");
					break;
				case TARGET_CORE_STATE_RUNNING:
					xprintf(gear_comm_fd, "TARGET_CORE_STATE_RUNNING");
					break;
				default:
					panic("");
			}
			xprintf(gear_comm_fd, "\n");

		}
		|	MEM_READ_KW '(' NUM ',' NUM ')'
		{
			/* the first argument ($3) is the address
			 * to read from; the second one ($5) is
		         * the number of bytes to read */	 
			uint32_t i, j, addr;
			unsigned char * buf;
			int nbytes;
			enum GEAR_ENGINE_ERR_ENUM err;

			nbytes = $5;
			addr = $3;

			if (!nbytes)
				panic("");
			/* allocate buffer */
			/* access the target in 32 bit words */
			addr &= ~ 3;
			nbytes += $3 - addr;
			nbytes += 3;
			nbytes &= ~ 3;
			if (!(buf = malloc(nbytes)))
				panic("");

			i = nbytes;
			err = ctx->cc->core_mem_read(ctx, buf, addr, &nbytes);
			dump_errcode(err);
			if (err == GEAR_ERR_NO_ERROR)
			{
				if (ctx->cc->core_mem_read(ctx, buf, addr, &nbytes) != GEAR_ERR_NO_ERROR)
					panic("");
				if (nbytes != i)
					panic("");
				i = $5;
				j = $3 & 3;
				while (i--)
					xprintf(gear_comm_fd, "%i ", buf[j++]);
				free(buf);
			}
			xprintf(gear_comm_fd, "\n");
		}
		|	REGS_READ_KW '(' NUM ')'
		{
			int i, regmask, regcnt;
			ARM_CORE_WORD regs[19];
			enum GEAR_ENGINE_ERR_ENUM err;
			/* the argument ($3) is a bitmap of the registers
			 * to read: bits 0 - 15 encode the core registers
			 * with these number, bit16 encodes the program
			 * counter (same as r15), bit17 encodes the cpsr,
			 * bit18 encodes the spsr; this encoding is taken
			 * from the arm rdi sources and may be changed;
			 * it is an error to supply an empty (zero) bitmap */

			regmask = $3;
			if (!regmask)
				panic("");
			regcnt = 0;
			for (i = 0; i < 16; i++)
				if (regmask & (1 << i))
				{
					if ((err = ctx->cc->core_reg_read(ctx, 0, 255, 1 << i, regs + i))
							!= GEAR_ERR_NO_ERROR)
						goto reg_read_error;
					regcnt ++;
				}
			if (regmask & (1 << 16))
			{
				if ((err = ctx->cc->core_reg_read(ctx, 0, 255, 1 << 16, regs + 16))
						!= GEAR_ERR_NO_ERROR)
					goto reg_read_error;
				regcnt ++;
			}
			if (regmask & (1 << 17))
			{
				if ((err = ctx->cc->core_reg_read(ctx, 0, 255, 1 << 17, regs + 17))
						!= GEAR_ERR_NO_ERROR)
					goto reg_read_error;
				regcnt ++;
			}
			if (regmask & (1 << 18))
			{
				if ((err = ctx->cc->core_reg_read(ctx, 0, 255, 1 << 18, regs + 18))
						!= GEAR_ERR_NO_ERROR)
					goto reg_read_error;
				regcnt ++;
			}
reg_read_error:			
			dump_errcode(err);
			if (err == GEAR_ERR_NO_ERROR)
			{
				for (i = 0; regcnt; i ++, regcnt --)
					xprintf(gear_comm_fd, "0x%08x ", regs[i]);

			}
			xprintf(gear_comm_fd, "\n");
		}
		|	REGS_WRITE_KW	'(' reg_write_data ')'
		{
			int i;
			/* the syntax of the reg_write_data nonterminal is:
		         * mask, reg0 ... regn
			 * where mask is the bitmap of the registers that
			 * should be written (for details on the bitmap
			 * format, see the comments for the register
			 * reading request above; the reg0 ... regn
			 * are the register values to write to the
			 * register set indicated by the mask value;
			 * it is mandatory that the number of register
			 * values supplied equals the number of non-zero
			 * bits in the mask */

			if (lex_vars.regmask || lex_vars.reg_nr > 19)
				panic("");
			dump_errcode(GEAR_ERR_NO_ERROR);
			xprintf(gear_comm_fd, "\n");
		}
		|	MEM_WRITE_KW	'(' mem_write_data ')'
		{
			int nbytes;
			/* the syntax of the mem_write_data nonterminal is:
		         * address, len, byte0 ... byten
			 * where address is the starting address, len is
			 * the number of bytes to be written, and
			 * byte0 ... byten are the data bytes to be written */

			nbytes = lex_vars.mem_write_len;
			if (ctx->cc->core_mem_write(ctx, lex_vars.addr, lex_vars.mem_write_buf, &nbytes)
					!= GEAR_ERR_NO_ERROR)
				panic("");
			if (nbytes != lex_vars.mem_write_len)
				panic("");
			free(lex_vars.mem_write_buf);
			lex_vars.mem_write_buf = 0;
			dump_errcode(GEAR_ERR_NO_ERROR);
			xprintf(gear_comm_fd, "\n");
		}
		| CORE_RUN_KW
		{
			enum GEAR_ENGINE_ERR_ENUM err;
			/* run the target core */
			err = ctx->cc->core_run(ctx, 0);
			dump_errcode(err);
			xprintf(gear_comm_fd, "\n");
		}
		| CORE_HALT_KW
		{
			enum GEAR_ENGINE_ERR_ENUM err;
			/* run the target core */
			err = ctx->cc->core_halt(ctx, 0);
			dump_errcode(err);
			xprintf(gear_comm_fd, "\n");
		}
		| CORE_INSN_STEP_KW
		{
			enum GEAR_ENGINE_ERR_ENUM err;
			/* instruction-wise single step the target core */
			err = ctx->cc->core_insn_step(ctx, 0);
			dump_errcode(err);
			xprintf(gear_comm_fd, "\n");
		}
		| BKPT_SET_KW '(' NUM ')'
		{
			enum GEAR_ENGINE_ERR_ENUM err;
			/* set a breakpoint at the address specified
			 * by $3 */
			err = ctx->cc->core_set_break(ctx, $3);
			dump_errcode(err);
			xprintf(gear_comm_fd, "\n");

		}
		| BKPT_CLEAR_KW '(' NUM ')'
		{
			enum GEAR_ENGINE_ERR_ENUM err;
			/* clear a breakpoint at the address specified
			 * by $3 */
			err = ctx->cc->core_clear_break(ctx, $3);
			dump_errcode(err);
			xprintf(gear_comm_fd, "\n");

		}
		| GET_STATE_KW
		{
			enum TARGET_CORE_STATE_ENUM state;

			if (ctx->cc->core_get_status(ctx, &state) !=
					GEAR_ERR_NO_ERROR)
				panic("");
			dump_errcode(GEAR_ERR_NO_ERROR);
			switch (state)
			{
				case TARGET_CORE_STATE_DEAD:
					xprintf(gear_comm_fd, "TARGET_CORE_STATE_DEAD");
					break;
				case TARGET_CORE_STATE_HALTED:
					xprintf(gear_comm_fd, "TARGET_CORE_STATE_HALTED");
					break;
				case TARGET_CORE_STATE_RUNNING:
					xprintf(gear_comm_fd, "TARGET_CORE_STATE_RUNNING");
					break;
				default:
					panic("");
			}
			xprintf(gear_comm_fd, "\n");
		}
		;
reg_write_data
		:	NUM ','
			{ 
				if (lex_vars.regmask || lex_vars.reg_nr)
					panic("");
				lex_vars.regmask = $1;
				lex_vars.reg_nr = 0;
				if (!$1) panic("");
			}
		|	reg_write_data NUM
			{
				ARM_CORE_WORD reg;
				if (!lex_vars.regmask)
					panic("");
				while (!(lex_vars.regmask & 1))
					lex_vars.regmask >>= 1, lex_vars.reg_nr ++;
				/* write register contents here */
				/* write_reg(...) */
				reg = $2;
				if (ctx->cc->core_reg_write(ctx, 0, 255, 1 << lex_vars.reg_nr, &reg)
						!= GEAR_ERR_NO_ERROR)
					panic("");
				lex_vars.regmask >>= 1, lex_vars.reg_nr ++;
			}
		;

mem_write_data
		:	NUM ',' NUM ','
		{
			if (lex_vars.mem_write_len)
				panic("");
			lex_vars.addr = $1;
			lex_vars.mem_write_len = $3;
			if (!(lex_vars.mem_write_buf = malloc($3)))
				panic("");
			lex_vars.buf_idx = 0;
			if (lex_vars.regmask || lex_vars.reg_nr > 19)
				panic("");
		}
		|	mem_write_data NUM
		{
			if (lex_vars.mem_write_len == lex_vars.buf_idx)
				panic("");
			/* write the data byte $2 at address
			 * lex_vars.addr here */
			/* write_mem(...); */
			/* fill the write buffer */
			lex_vars.mem_write_buf[lex_vars.buf_idx] = $2;
			lex_vars.buf_idx ++;
			lex_vars.mem_write_len;
		}
		;
ioctl_data
		: NUM ','
		{
			lex_vars.ioctl_buf_idx = 0;
			lex_vars.ioctl_req_len = $1;
			if ($1)
				if (!(lex_vars.ioctl_buf = malloc($1 * sizeof * lex_vars.ioctl_buf)))
					panic("");
		}
		| ioctl_data NUM ','
		{
			if (lex_vars.ioctl_buf_idx >= lex_vars.ioctl_req_len)
				panic("");
			lex_vars.ioctl_buf[lex_vars.ioctl_buf_idx++] = $2;
		}
		;

%%


int xprintf(int sock_fd, const char * format, ...)
{
va_list ap;
int res;
char buf[256];

	va_start(ap, format);
	res = vsnprintf(buf, sizeof buf, format, ap);
	if (send(sock_fd, buf, res, 0) != res)
		panic("");
	/* vfprintf(stdout, format, ap);*/
	va_end(ap);
	return res;
}

int main(void)
{
int fd;	
struct sockaddr_in addr;
int len;
unsigned long i;
struct target_ctl_context * ctx;

	printf("target core controller stub - put your id here\n");
	printf("initializing gear communication channels...\n");

#ifdef __LINUX__	
#else
	{
		WSADATA w;
		if (WSAStartup(0x0101, &w))
		{
			printf("failed to initialize winsock\n");
			exit(1);
		}
	}
#endif

	if (!(ctx = malloc(sizeof * ctx)))
		panic("out of core");
	if (target_get_core_access(ctx) != GEAR_ERR_NO_ERROR)
		panic("");

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		panic("socket");
	}

	/* create comm socket */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(TARGET_CTL_DEFAULT_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(fd, (struct sockaddr *) &addr, sizeof addr) == -1)
	{
		panic("bind");
	}

	printf("listening on port %i...\n", TARGET_CTL_DEFAULT_PORT);
	if (listen(fd, 0) == -1)
	{
		panic("listen");
	}

	if (!(lex_vars.comm_buf = malloc(COMM_BUF_SIZE)))
		panic("");

	/* incomming connection dequeuing loop */
	while (1)
	{
		printf("waiting for connection... ");
		fflush(stdout);
		len = sizeof addr;
#ifdef __LINUIX__
		if ((gear_comm_fd = accept(fd, (struct sockaddr *) &addr, (socklen_t *) &len)) == -1)
#else
		if ((gear_comm_fd = accept(fd, (struct sockaddr *) &addr, &len)) == -1)
#endif
		{
			panic("accept");
		}
		i = ntohl(addr.sin_addr.s_addr);
		printf("ok, connection request accepted from %s\n", inet_ntoa(addr.sin_addr));

		/* request dispatch loop */
		while (1)
		{
			lex_vars.lex_idx = lex_vars.comm_buf_idx = 0;
			lex_vars.mem_write_len = 0;
			lex_vars.regmask = lex_vars.reg_nr = 0;
			if (yyparse(ctx))
			{
				if (lex_vars.is_scanner_aborted)
				/* connection aborted */
				{
					printf("connection shut down\n");
					if (target_is_core_running(ctx))
						/*! \todo	properly detach from the target */
						panic("");
					if (ctx->cc->core_close(ctx)
							!= GEAR_ERR_NO_ERROR)
						panic("");
					if (shutdown(gear_comm_fd, SD_BOTH))
						panic("");
					printf("*** handle memory leaks here ***\n");
					break;
				}
				else
					panic("");
			}
		}
	}
}

