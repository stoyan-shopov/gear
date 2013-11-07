
%option	noyywrap

D			[0-9]
H			[a-fA-F0-9]
IS			(u|U|l|L)*

%{
#include "target-ctl-parse.tab.h"

#define	YY_INPUT(__buf, __result, __max_size)\
	do {\
		int i, j;\
		/* sanity checks */\
		if (lex_vars.comm_buf_idx < lex_vars.lex_idx)\
			panic("communication buffer indices corrupt");\
\
		j = 0;\
		/* first, see if the input buffer is empty */\
		if (lex_vars.comm_buf_idx == lex_vars.lex_idx)\
		{\
			int res;\
			/* input buffer empty - refill it */\
			lex_vars.comm_buf_idx = lex_vars.lex_idx = 0;\
			res = recv(gear_comm_fd, lex_vars.comm_buf, COMM_BUF_SIZE, 0);\
			/* see if an error occurred */\
			if (res < 0 || !res)\
			{\
				if (!res)\
				{\
				/* end of file reached - \
				 * this usually means the\
			         * gear engine connection\
				 * has been terminated */\
					lex_vars.is_scanner_aborted = 1;\
					goto abort_scanning;\
				}\
				perror("");\
				panic("read error");\
			}\
			lex_vars.comm_buf_idx = res;\
		}\
		i = __max_size;\
		while (lex_vars.comm_buf_idx != lex_vars.lex_idx && i--)\
		{\
			int c;\
			c = lex_vars.lex_idx[lex_vars.comm_buf];\
			if (c == '\n')\
			{\
				if (lex_vars.lex_idx != lex_vars.comm_buf_idx - 1)\
					panic("extra input after end-of-record marker");\
				/*lex_idx[comm_buf] = 0;*/\
				break;\
			}\
			lex_vars.lex_idx++;\
			__buf[j++] = c;\
		}\
abort_scanning:\
		/* uncomment this for debugging to view incoming data\
		 * from a gear engine */\
		/*write(1, __buf, j);*/\
		__result = j;\
\
	} while (0)


%}

%%

"MEM_READ"		{ return MEM_READ_KW; }
"MEM_WRITE"		{ return MEM_WRITE_KW; }
"REGS_READ"		{ return REGS_READ_KW; }
"REGS_WRITE"		{ return REGS_WRITE_KW; }
"CORE_RUN"		{ return CORE_RUN_KW; }
"BKPT_SET"		{ return BKPT_SET_KW; }
"BKPT_CLEAR"		{ return BKPT_CLEAR_KW; }
"GET_STATE"		{ return GET_STATE_KW; }
"IOCTL"			{ return IOCTL_KW; }

0[xX]{H}+{IS}?	{
			/*! \todo	process conversion errors here */
			yylval.num = strtoull(yytext, 0, 0);
			return NUM;
		}
0{D}+{IS}?	{
			/*! \todo	process conversion errors here */
			yylval.num = strtoull(yytext, 0, 0);
			return NUM;
		}
{D}+{IS}?	{ 
			/*! \todo	process conversion errors here */
			yylval.num = strtoull(yytext, 0, 0);
			return NUM;
		}

[ \t]		{ /* ignore whitespace */ }

.		{ return * yytext; }

%%

