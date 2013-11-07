/*!
 *	\file	dbx-parser.c
 *	\brief	dbx mode command input parser
 *	\author	shopov
 *
 *	this module contains a dbx compatible command parser
 *	for use in the arm gear; the primary function in this
 *	module is the dbx_parse() function, which is passed a string
 *	containing a command to be parsed and executed; it should
 *	be invoked whenever there is command input pending (e.g., from
 *	some frontend)
 *	this is designed as a large but simple hand-written parser,
 *	which uses a very simple flex generated lexer for determining
 *	which dbx command is requested
 *
 *	\todo	this is largely incomplete
 *	\todo	in the output to an engine output consumer - properly
 *		support output packet token numbering
 *	\todo	implement the dbx internal default variables (e.g. - '+', etc.)
 *	\todo	the current YY_INPUT macro is buggy, it does not
 *		properly advance in the scan buffer between
 *		subsequent invocations (becomes an issue if the
 *		whole buffer is not consumed with a single invocation
 *		of YY_INPUT)
 *	\todo	code limits here - limit arguments to some commands
 *		(e.g. - disallow huge disassembly or memory dumps) as
 *		a primitive means of protection; should large arguments
 *		really become needed, devise some scheme for optimizing
 *		large data transfers between the engine and an engine
 *		output consumer - right now large data requests will
 *		lead to a very big net slowdown as the engine data
 *		format has not been designed to support large data
 *		transfers; should large data transfers really become
 *		necessary, some scheme that uses a more efficient
 *		binary encoding should be devised and coded here
 *	\todo	maybe make the lexer reentrant
 *	\todo	make the lexer directly read from a string
 *	\todo	the formatting of the code here is horrible; too much
 *		code is being duplicated at places
 *
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * include section follows
 *
 */
#include <stdarg.h>
#include <ctype.h>

#include "gear-limits.h"
#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "core-access.h"
#include "target-description.h"
#include "engine-err.h"
#include "dbx-support.h"
#include "gprintf.h"
#include "miprintf.h"
#include "util.h"
#include "gear-constants.h"
#include "cparse/c-expression.h"
#include "cu-access.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "aranges-access.h"
#include "breakpoint.h"
#include "frame-reg-cache.h"
#include "srcfile.h"
#include "exec.h"

#include "gprintf.h"

/*
 *
 * local data follows
 *
 */
/*! the string to pass to the lexer (lexer scan buffer)
 *
 * must be properly initialized prior to invoking the lexer
 *
 * \todo	this should not be static - move it elsewhere
 */
static char * dbx_lexer_str;
/*! current recognized token length, in number of characters
 *
 * whenever the lexer succesfully scans a token, this is set
 * to the legth (in number of characters) of the token recognized,
 * this information is needed in the parser to determine how much
 * of the lexer scan buffer (dbx_lexer_str) to skip forward
 *
 * \todo	this should not be static - move it elsewhere
 */
static int token_len;

/*! the lexer counting function, invoked whenever a token has been successfully scanned */
static void lex_count(void);

/*! the prototype for the lexical scanner function */
#define YY_DECL int dbx_lex(void)
YY_DECL;



/*! \todo	a quickhack, the lexer must be reworked altogether... */
static int lexidx;
/*! lexer input character reading macro
 *
 * \todo	maybe switch to the in-memory string flex scanning
 *		interface (yy_scan_string()) and remove this altogether
 *
 * \todo	the current YY_INPUT macro is buggy, it does not
 *		properly advance in the scan buffer between
 *		subsequent invocations (becomes an issue if the
 *		whole buffer is not consumed with a single invocation
 *		of YY_INPUT) */
#define YY_INPUT(buf, result, max_size) \
{\
int i;\
	i = 0;\
	while (i < (max_size) && dbx_lexer_str[lexidx])\
	{\
		(buf)[i] = dbx_lexer_str[i];\
		i++;\
		lexidx++;\
	}\
	result = i ? i : YY_NULL;\
}
/* while this may seem dirty, it simplifies things much and prevents
 * global namespace pollution */

/*! a union with the purpose of yylval, used to pass information from the scanner to the parser
 *
 * \todo	this should not be static - move it elsewhere
 *		(somewhere in the gear_engine_context)
 */
static union
{
	/*! register number, in case a register name was recognized
	 *
	 * \note	this comes in effect when the dbx lexer
	 *		returns HACK_REG_KW
	 * \todo	change the reference to HACK_REG_KW above when
	 *		the token name is changed
	 *
	 *	\todo	only general purpose registers are handled
	 *		right now; this is deliberately so, because
	 *		by far the numbering of status and other registers
	 *		has not been settled definitely; when the numbering
	 *		for these registers are defined, add support
	 *		for them too */
	int	reg_nr;
	/*! a pointer to a c-like string
	 * 
	 * by 'c like string' here meant is a string of characters,
	 * enclosed by double quotation marks ("-s), which can contain
	 * double quotation marks escaped with backslashes in it (\"),
	 * e.g.:
	 * "this is a \"string\""; the enclosing double quotation marks
	 * are removed by the scanner and a pointer to the resulting
	 * string is placed in this field; the string *must* be 
	 * deallocated (free()-d) by the parser when no longer of interest
	 *
	 *	\note	the string *must* be deallocated (free()-d) when
	 *		no longer of interest */
	char	* cstr;
	/*! a numeric constant token value */
	int	num;
}
dbx_lval;

#include "lex.dbx.c"

/*! counts the current column, updated by lex_count(), can be used for error reporting */
static int column = 0;

/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn	static void lex_count(void)
 *	\brief	lexer counting function
 *
 *	this is invoked from the lexer whenever a token is successfully
 *	scanned; it updates the column and token_len variables; see
 *	the comments for these variables for details
 *
 *	\return	none
 */
static void lex_count(void)
{
	int i;

	for (i = 0; dbxtext[i] != '\0'; i++, token_len++)
		if (dbxtext[i] == '\n')
			column = 0;
		else if (dbxtext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;
	/* suppress lexer output */
	/* ECHO; */
}

/*!
 *	\fn	static char * extract_expression(void)
 *	\brief	extracts a double quoted expression string from another string
 *
 *	given an input string (dbx_lexer_str), this function scans it for a
 *	double quotes delimited string (which is expected to be a
 *	valid expression in some of the supported expression grammars),
 *	extracts it to a new string, and advances the input string
 *	(dbx_lexer_str) past the expression string extracted; any
 *	whitespace before and after the double quotes delimited string
 *	is skipped over in the process of scanning, so a caller may
 *	safely assume that the input string (modified prior to
 *	returning) points to characters that are not whitespace; if
 *	the string passed does not start with a double quotation
 *	mark, it is duplicated and returned verbatim
 *
 *	\note	this is here to facilitate expression string extraction
 *		used at many places in the dbx command parser
 *	\note	this function invokes the flex generated lexer and
 *		makes sure to keep the lexer variables as consistent
 *		as possible (see the calls to YY_FLUSH_BUFFER and
 *		the handling of token_len)
 *	\note 	the input string (dbx_lexer_str) is modified to
 *		point past the string just scanned;
 *		this string *must* be deallocated (free()-d) by
 *		the caller when no longer of use
 *
 *	\return	if the input string (dbx_lexer_str) starts with a
 *		double quotation marks delimited substring, returns
 *		a pointer to the double quotes delimited string
 *		extracted, with double quotes removed; otherwise
 *		returns a verbatim duplicate of the input string */
static char * extract_expression(void)
{
int token;

	YY_FLUSH_BUFFER;
	token_len = 0;
	lexidx = 0;
	token = dbx_lex();
	YY_FLUSH_BUFFER;
	if (token != HACK_CSTR)
	{
		char * s;
		token_len = 0;
		if (!(s = strdup(dbx_lexer_str)))
			panic("");
		dbx_lexer_str += strlen(s);
		return s;
	}
	dbx_lexer_str += token_len;
	token_len = 0;
	dbx_lexer_str += strspn(dbx_lexer_str, " \t");

	return dbx_lval.cstr;
}

/*!
 *	\fn	static void dump_errcode(enum GEAR_ENGINE_ERR_ENUM errcode, char * msg)
 *	\brief	prints a command output status code in gear machine interface foemat
 *
 *	the status code for a command must always be output when
 *	a command input from the user has been processed and a response
 *	is returned to the sender of the command (e.g., a frontend);
 *	see the command response format documentation for details
 *
 *	\todo	provide a proper reference to the command output format
 *		documentation when one is written
 *	\todo	move this to a general purpose machine interface
 *		utility module
 *
 *	\param	errcode	status code to print
 *	\param	msg	status message to print
 *	\return	none
 */
static void dump_errcode(enum GEAR_ENGINE_ERR_ENUM errcode, char * msg)
{
	miprintf("ERRCODE=[%i,\"%s\"],", errcode, msg);
}


/*
 *
 * exported functions follow
 *
 */


/*!
 *	\fn	int dbx_parse(struct gear_engine_context * ctx, char * lex_buf)
 *	\brief	parses command input in dbx format
 *
 *	this routine is intended to be invoked whenever a whole record
 *	(one that ends with a newline) has been received for parsing;
 *	the routine parses the command received and invokes appropriate
 *	code to handle the command; the command string passed to this
 *	routine should not contain any newline characters
 *
 *	\note	the command descriptions presented here
 *		are taken directly from the sun studio 12
 *		dbx manual, as, e.g., available here:
 *		http://docs.sun.com/app/docs/doc/819-5257
 *
 *	\note	this routine is fairly trivial, it is just
 *		one simple (albeit quite big) switch
 *		statement, but all of the cases handled
 *		here are short and to the point; do not
 *		be daunted by its size, it is big on the
 *		outside, but very simple and small on
 *		the inside
 *
 *	\param	ctx	context to work in
 *	\param	lex_buf	the command string to parse; must not
 *		contain any newline characters
 *	\return	zero on success, nonzero on failure
 *	\todo	maybe define some error codes to return, if such
 *		become useful
 *	\todo	06112011 - the error status returning looks totally broken
 *
 */
int dbx_parse(struct gear_engine_context * ctx, char * lex_buf)
{
int token;
/* the return value(result) is kept here */
int res;
/* this is used for reporting the currently selected/active stack
 * frame number to a frontend */
int frame_nr;

	res = 1;

	/* properly initialize lexer variables */
	dbx_lexer_str = lex_buf;
	token_len = 0;
	column = 0;

	gprintf("%s(): lex_buf: %s\n", __func__, lex_buf);

	/* first of all, read and send back any numeric constant
	 * tokens (i.e. numbers) that may precede the command
	 * string received; these numbers do not currently
	 * possess any strictly defined meaning and purpose,
	 * but they may be used by a gear engine machine
	 * interface consumer for purposes such as:
	 * 	- machine interface record sequencing
	 * 	- provision of placeholders for state data;
	 * 	an example use of this could be
	 * 	synchronization when a frontend has issued
	 * 	some command to the gear engine and should
	 * 	wait for a response to that particular command,
	 * 	and ignore other response records while waiting
	 * 	- machine interface record tagging; an example
	 * 	use of this could be determining which gear
	 * 	engine response records correspond to which
	 * 	frontend requests - e.g. discriminating between
	 * 	gear engine machine output generated in
	 *	response to a user-typed command, a
	 *	frontend-originating command - and in the
	 *	case there are multiple frontends connected
	 *	to the gear engine server - which particular
	 *	frontend instance sequenced the command
	 *	- et cetera
	 */
	lexidx = 0;
	while ((token = dbx_lex()) == DBX_LEX_NUM_CONSTANT_TOKEN)
	{
		dbx_lexer_str += token_len;
		token_len = 0;
		miprintf("%i ", dbx_lval.num);
		lexidx = 0;
	}
	/* important - flush the lexer buffer
	 *
	 * the lexer used here is designed to only
	 * scan the initial portion of a command string,
	 * and so its buffer must be empty each time a
	 * new command string is scanned - this is why
	 * the lexer buffer is flushed here
	 *
	 * the lexer internal buffer-filling code,
	 * in YY_INPUT, is also coded with the idea that only
	 * the initial portion of a command string will be
	 * needed in the lexer, and the lexer shall not
	 * be later invoked without its internal buffers
	 * first being flushed by a call to YY_FLUSH_BUFFER
	 *
	 * this means that this scenario is very error-prone:
	 * first, invoke the lexer to determine
	 * what command is being requested, do some processing
	 * (which modifies the lexer scanning buffer used in
	 * YY_INPUT - dbx_lexer_str) and later invoke the lexer
	 * again (without flushing it by invoking YY_FLUSH_BUFFER),
	 * to get some tokens; while this scenario is
	 * possible, none of the code here has been designed
	 * with it in mind (indeed, the parser here is very
	 * simple), so if it becomes necessary to process commands
	 * in the way outlined above, the code must be rewritten;
	 * i(sgs) dont really think this should ever be needed
	 */
	YY_FLUSH_BUFFER;
	if (!token_len)
	{
		dump_errcode(GEAR_ERR_DBX_CMD_STRING_EMPTY, "dbx command string empty");
		miprintf("\n");
		return 0;
	}
	dbx_lexer_str += token_len;
	token_len = 0;
	dbx_lexer_str += strspn(dbx_lexer_str, "\t ");
	switch (token)
	{
		case ATTACH_KW:
			{
				enum GEAR_ENGINE_ERR_ENUM err;
				const char * errmsg_hint;

				if (*dbx_lexer_str)
					panic("");

				err = ctx->cc->core_open(ctx, & errmsg_hint);
				dump_errcode(GEAR_ERR_NO_ERROR, "");
				miprintf("TARGET_ATTACH_STATUS, [TARGET_ATTACH_STATUS = ");
				switch (err)
				{
					default:
						panic("");
						break;
					case GEAR_ERR_NO_ERROR:
						miprintf("GEAR_ERR_NO_ERROR, ");
						break;
					case GEAR_ERR_GENERIC_ERROR:
						miprintf("GEAR_ERR_GENERIC_ERROR, ");
						break;
					case GEAR_ERR_TARGET_CORE_CONNECTION_FAILED:
						miprintf("GEAR_ERR_TARGET_CORE_CONNECTION_FAILED, ");
						break;
				}
				if (errmsg_hint)
				{
					miprintf("ERRMSG_HINT = \"%s\", ", errmsg_hint);
				}
				miprintf("TARGET_STATE = ");
				switch (exec_get_target_state(ctx))
				{
					default:
						panic("");
					case TARGET_CORE_STATE_DEAD:
						miprintf("DEAD,");
						break;
					case TARGET_CORE_STATE_RUNNING:
						miprintf("RUNNING,");
						break;
					case TARGET_CORE_STATE_HALTED:
						miprintf("HALTED,");
						break;
				}
				miprintf("]\n");
			}
			break;
		case INVALID_DBX_KW:
		case ASSIGN_KW:
		case BSEARCH_KW:
		case CALL_KW:
		case CANCEL_KW:
		case CATCH_KW:
		case CHECK_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case CLEAR_KW:
		{
			ARM_CORE_WORD addr;
			char * srcname;
			int srcline_nr;
			bool is_addr_at_src_boundary;
			int i;
			char * c_exp;
			signed long long exp_val;

			token_len = 0;
			lexidx = 0;
			if (dbx_lex() != AT_KW)
				panic("");
			dbx_lexer_str += token_len;
			dbx_lexer_str += strspn(dbx_lexer_str, " \t");
			srcname = dbx_lexer_str;
			i = 0;
			if (*dbx_lexer_str == '"')
			{
				i = /* count the initial double quote character */ 1;
				i += strcspn(srcname + 1, "\"");
				if (!*dbx_lexer_str)
					panic("");
				dbx_lexer_str += i + 1;
				if (!*dbx_lexer_str)
					panic("");
				dbx_lexer_str++;
			}
			dbx_lexer_str += strspn(dbx_lexer_str, " \t:");
			if (!*dbx_lexer_str)
				panic("");

			c_exp = extract_expression();
			if (!c_expression_eval_int(ctx, c_exp, &exp_val))
				panic("");
			free(c_exp);

			srcname[i] = 0;

			if (*srcname)
			{
				/* halt at source code location */
				if (srcfile_get_core_addr_for_line_nr(ctx, srcname + 1, exp_val, &addr)
						!= GEAR_ERR_NO_ERROR)
				{
					dump_errcode(GEAR_ERR_GENERIC_ERROR, "could not locate target core address for "
"the source file/line number supplied");
					miprintf("BREAKPOINT_DATA,");
					miprintf("\n");
					break;
				}
			}
			else
				/* halt at target core address */
				addr = exp_val;

			if (!bkpt_locate_at_addr(ctx, addr))
			{
				dump_errcode(GEAR_ERR_GENERIC_ERROR, "no breakpoint at the location supplied");
				miprintf("BREAKPOINT_DATA,");
				miprintf("\n");
				break;
			}
			if (bkpt_clear_at_addr(ctx, addr) != GEAR_ERR_NO_ERROR)
			{
				dump_errcode(GEAR_ERR_GENERIC_ERROR, "error clearing breakpoint");
				miprintf("BREAKPOINT_DATA,");
				miprintf("\n");
				break;
			}
			dump_errcode(GEAR_ERR_NO_ERROR, "");
			srcfile_get_srcinfo_for_addr(ctx, addr, 0, 0, &srcname, &srcline_nr, &is_addr_at_src_boundary); 
			miprintf("BREAKPOINT_DATA, BREAKPOINT_REMOVED = ([ADDR = 0x%08x,", addr);
			miprintf("SRCFILE = \"%s\",", srcname ? srcname : "");
			miprintf("SRCLINE = %i,", srcline_nr);
			miprintf("IS_AT_START_OF_STATEMENT = %i,", is_addr_at_src_boundary ? 1 : 0);
			miprintf("],)\n");
			break;
		}
		case COLLECTOR_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case CONT_KW:
			/*
			   cont Command

			   The cont command causes the process to continue execution. It has identical syntax and identical functionality in native mode and Java mode.
			   Syntax

			   cont

			   Continue execution. In an MT process all threads are resumed. Use Control-C to stop executing the program.
			   cont ... -sig signal

			   Continue execution with signal signal.
			   cont ... id

			   The id specifies which thread or LWP to continue.
			   cont at line [id]

			   Continue execution at line line. id is required if the application is multi-threaded.
			   cont ... -follow parent|child|both

			   If the dbx follow_fork_mode environment variable is set to ask, and you have chosen stop, use this option to choose which process to follow. both is only applicable in the Sun Studio IDE.

			 */
			/*! \todo	validate target and engine state here */
			/*
			if (exec_get_engine_state(ctx) != EXEC_STATE_IDLE)
				panic("");
			*/

			if (*dbx_lexer_str)
				panic("");
			/* rewind to the innermost (most recent)
			 * frame and execute from there */
			/*
			if (frame_move_to_relative(ctx, 0, 0) != GEAR_ERR_NO_ERROR)
				panic("");
			*/
			frame_move_to_relative(ctx, 0, 0);
			exec_target_run(ctx);
			break;
		case HALT_KW:
			if (*dbx_lexer_str)
				panic("");
			exec_target_halt(ctx);
			break;
		case DALIAS_KW:
		case DBX_KW:
		case DBXENV_KW:
		case DEBUG_KW:
		case DELETE_KW:
		case DETACH_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case DIS_KW:
			/*
			dis Command

			The dis command disassembles machine instructions. It is valid only in native mode.
			Syntax

			dis [ -a ] address [/count]

			Disassemble count instructions (default is 10), starting at address address.
			dis address1, address2

			Disassemble instructions from address1 through address2.
			dis

			Disassemble 10 instructions, starting at the value of + (see examine Command).

			where:

			address is the address at which to start disassembling. The default value of address is the address after the last address previously assembled. This value is shared by the examine command (see examine Command).

			address1 is the address at which to start disassembling.

			address2 is the address at which to stop disassembling.

			count is the number of instructions to disassemble. The default value of count is 10.
			Options

			-a

			When used with a function address, disassembles the entire function. When used without parameters, disassembles the remains of the current visiting function, if any.

			*/

			/*! \todo	support the full dbx syntax */
		{
			signed long long sll;
			unsigned int cnt;
			char * c_exp;
			ARM_CORE_WORD pc;
			bool is_counting_insns;
			ARM_CORE_WORD first_addr_past_disassembly;

			c_exp = extract_expression();
			if (!c_expression_eval_int(ctx, c_exp, &sll))
				panic("");
			free(c_exp);
			first_addr_past_disassembly = sll + 10;

			/* process count field (if any) */
			cnt = 10;
			dbx_lexer_str += strspn(dbx_lexer_str, " \t");
			is_counting_insns = false;
			if  (*dbx_lexer_str)
			{
				if (*dbx_lexer_str == '/')
				{
					dbx_lexer_str++;
					/* expected is a count field denoting
					 * how many instruction to disassemble */
					dbx_lexer_str += strspn(dbx_lexer_str, " \t");
					if (*dbx_lexer_str++ != '"')
						panic("");
					dbx_lexer_str += strspn(dbx_lexer_str, " \t");
					/* extract the count field, this is in
					 * decimal */
					cnt = 0;
					while (isdigit(*dbx_lexer_str))
					{
						cnt *= 10;
						cnt += *dbx_lexer_str++ - '0';
					}
					/*! \todo	provide a sane maximum
					 *		of instructions to
					 *		disassemble here */
					if (cnt <= 0 || cnt > (1 << 16))
					/* provide a sensible default */
						/*! \todo	this is arbitrary */
						cnt = 10;
					dbx_lexer_str += strspn(dbx_lexer_str, " \t");
					if (*dbx_lexer_str++ != '"')
						panic("");
					is_counting_insns = true;
				}
				else if (*dbx_lexer_str == ',')
				{
					dbx_lexer_str++;
					signed long long sll1 = sll;
					/* expected is a second address at which
					 * to stop disassembling */
					c_exp = extract_expression();
					gprintf("\n\n%s\n\n", c_exp);
					if (!c_expression_eval_int(ctx, c_exp, &sll1))
						panic("");
					free(c_exp);
					/*! \todo	provide a sane maximum
					 *		of instructions to
					 *		disassemble here */
					if (sll1 - sll <= 0 || sll1 - sll > (1 << 16))
					{
						/*! \todo	this is arbitrary */
						cnt = 10;
						is_counting_insns = true;
					}
					else
					{
						first_addr_past_disassembly = sll1;
						gprintf("sll == %i\n", first_addr_past_disassembly);
					}
				}
				else
					panic("");
			}

			if (ctx->cc->core_reg_read(ctx,
						0,
						1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
						&pc) != GEAR_ERR_NO_ERROR)
				panic("");
			srcfile_disassemble_addr_range(ctx, is_counting_insns, (ARM_CORE_WORD) sll, is_counting_insns ? cnt : first_addr_past_disassembly, pc);
			break;
		}
		case DISPLAY_KW:
			panic("");
			break;
		case DOWN_KW:
			/*
			   down Command

			   The down command moves down the call stack (away from main). It has identical syntax and identical functionality in native mode and Java mode.
			   Syntax

			   down

			   Move down the call stack one level.
			   down number

			   Move down the call stack number levels.
			   down -h [number]

			   Move down the call stack, but don.t skip hidden frames.

where:

number is a number of call stack levels.
			 */
			{
				int frame_cnt;

				/* by default, if no argument is present,
				 * go down one frame */
				frame_cnt = 1;

				token_len = 0;
				lexidx = 0;
				if (dbx_lex() == DBX_LEX_NUM_CONSTANT_TOKEN)
					frame_cnt = dbx_lval.num;
				frame_move_to_relative(ctx, frame_cnt, &frame_nr);
				miprintf("STACK_FRAME_CHANGED, SELECTED_FRAME = %i\n", frame_nr);
				break;
			}
		case DUMP_KW:
		case EDIT_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case EXAMINE_KW:
			/*
			examine Command

			The examine command shows memory contents. It is valid only in native mode.
			Syntax

			examine [address] [ / [count] [format]]

				Display the contents of memory starting at address for count items in format format.
			examine address1, address2 [ / [format]]

				Display the contents of memory from address1 through address2 inclusive, in format format.
			examine address= [format]

				Display the address (instead of the contents of the address) in the given format.

			The address may be +, which indicates the address just after the last one previously displayed (the same as omitting it).

			x is a predefined alias for examine.

			where:

			address is the address at which to start displaying memory contents. The default value of address is the address after the address whose contents were last displayed. This value is shared by the dis command (see dis Command).

			address1 is the address at which to start displaying memory contents.

			address2 is the address at which to stop displaying memory contents.

			count is the number of addresses from which to display memory contents. The default value of count is 1.

			format is the format in which to display the contents of memory addresses. The default format is X (hexadecimal) for the first examine command, and the format specified in the previous examine command for subsequent examine commands. The following values are valid for format:

			o,O

				octal (2 or 4 bytes)
			x,X

				hexadecimal (2 or 4 bytes)
			b

				octal (1 byte)
			c

				character
			w

				wide character
			s

				string
			W

				wide character string
			f

				hexadecimal and floating point (4 bytes, 6 digit precision)
			F

				hexadecimal and floating point (8 bytes, 14 digit precision )
			g

				same as F
			E

				hexadecimal and floating point (16 bytes, 14 digit precision)
			ld,lD

				decimal (4 bytes, same as D)
			lo,lO

				octal 94 bytes, same as O
			lx,lX

				hexadecimal (4 bytes, same as X)
			Ld,LD

				decimal (8 bytes)
			Lo,LO

				octal (8 bytes)
			Lx,LX

				hexadecimal (8 bytes)
			*/
			/*! \note	the above documentation excerpt
			 *		is inconsistent; there are missing
			 *	 	format specifiers, which are
			 *		mentioned elsewhere in the
			 *		sun dbx manual (namely:
			 *		Sun Studio 12 Collection >> Sun Studio 12: Debugging a Program With dbx >> 18.  Debugging at the Machine-Instruction Level  >> Examining the Contents of Memory:);
			 *		the following format specifiers are
			 *		copied verbatim from this chapter:
			 *		i Display as an assembly instruction. 
			 *		d Display as 16 bits (2 bytes) in decimal. 
			 *		D Display as 32 bits (4 bytes) in decimal. 
			 *		< end of excerpt >
			 *
			 *		also - the documentation above
			 *		for the "f, F, E" format is a bit
			 *		vague (which one - hexadecimal or
			 *		floating point?), and from the
			 *		chapter cited in this comment
			 *		block, verbatim:
			 *		f Display as a single-precision floating point number. 
			 *		F, g Display as a double-precision floating point number. 
			 *		E Display as an extended-precision floating point number. 
			 *		< end of excerpt >
			 *
			 *		as one can see - no mention of
			 *		"hexadecimal" here, which i(sgs)
			 *		deem plausible in this case
			 *
			 *		but this very chapter should be
			 *		taken with a grain of salt as
			 *		well, because it states (verbatim):
			 *		W Display as a wide character. 
			 *		< end of excerpt >
			 *
			 *		which contradicts the documentation
			 *		above (saying that this format
			 *		specifiers should mean a "wide
			 *		character string" - which seems
			 *		plausible to me(sgs) in this case)
			 *
			 *		also in the text above, the line
			 *		reading "octal 94 bytes, same as O"
			 *		is apparently mistyped and should
			 *		read "octal (4 bytes, same as O)" */
			{
				char * c_exp;
				signed long long addr;
				signed long long cnt;
				int i, j;
				enum { FORMAT_INVALID = 0,
					FORMAT_HEX,
					FORMAT_OCTAL,
					FORMAT_DECIMAL,
					FORMAT_FLOATING_POINT,
					FORMAT_CHAR,
					FORMAT_WIDE_CHAR,
					FORMAT_STRING,
					FORMAT_WIDE_STRING,
				} format;
				/*! \todo	properly adjust
				 *		the buffer here
				 *		for the largest
				 *		memory transfer
				 *		supported */ 
				char dump_buf[16];
				int byte_width;


				c_exp = extract_expression();
				if (!c_expression_eval_int(ctx, c_exp, &addr))
					panic("");
				free(c_exp);
				if (*dbx_lexer_str++ != '/')
					panic("");
				dbx_lexer_str += strspn(dbx_lexer_str, " \t");
				/* extract the count field, this is in
				 * decimal */
				cnt = 0;
				while (isdigit(*dbx_lexer_str))
				{
					cnt *= 10;
					cnt += *dbx_lexer_str++ - '0';
				}
				if (!cnt)
				/*! \todo	provide a sensible default */
					panic("");
				dbx_lexer_str += strspn(dbx_lexer_str, " \t");
				/* process the format specifier string */
				format = FORMAT_INVALID;
				byte_width = 0;
				if (*dbx_lexer_str == 'l')
					dbx_lexer_str++;
				switch (*dbx_lexer_str)
				{
					case 'd':
						format = FORMAT_DECIMAL;
						byte_width = 2;
						break;
					case 'D':
						format = FORMAT_DECIMAL;
						byte_width = 4;
						break;
					case 'o':
						format = FORMAT_OCTAL;
						byte_width = 2;
						break;
					case 'O':
						format = FORMAT_OCTAL;
						byte_width = 4;
						break;
					case 'x':
						format = FORMAT_HEX;
						byte_width = 2;
						break;
					case 'X':
						format = FORMAT_HEX;
						byte_width = 4;
						break;
					case 'b':
						format = FORMAT_OCTAL;
						byte_width = 1;
						break;
					case 'c':
						format = FORMAT_CHAR;
						break;
					case 'w':
						format = FORMAT_WIDE_CHAR;
						break;
					case 's':
						format = FORMAT_STRING;
						break;
					case 'W':
						format = FORMAT_WIDE_STRING;
						break;
					case 'f':
						panic("");
						break;
					case 'F':
					case 'g':
						panic("");
						break;
					case 'E':
						panic("");
						break;
					case 'l':
					case 'L':
						if (*dbx_lexer_str++ == 'l')
							byte_width = 4;
						else
							byte_width = 8;
						switch (tolower(*dbx_lexer_str))
						{
							case 'd':
								format = FORMAT_DECIMAL;
								break;
							case 'o':
								format = FORMAT_OCTAL;
								break;
							case 'x':
								format = FORMAT_HEX;
								break;
							default:
								panic("");
						}
						break;
					default:
						panic("");
				}
				dbx_lexer_str++;
				dbx_lexer_str += strspn(dbx_lexer_str, " \t");
				if (*dbx_lexer_str)
					panic("");
				/* ok, command string parsed, start
				 * dumping data */
				/*! \todo	handle strings, wide
				 *		character strings,
				 *		characters and wide
				 *		characters here */
				if (!byte_width)
					panic("");
				/*! \todo	handle endianness here,
				 *		right now, only
				 *		low endian machines are
				 *		handled */
				/*! \todo	do some caching here by
				 *		reading larger blocks
				 *		at a time, the current
				 *		code is dead-slow */
				/* optimize for a common case for reading
				 * memory... */
				if (format == FORMAT_HEX && byte_width == 4)
				{
					uint32_t * buf;
					int bytes_read;
					if (!(buf = calloc(cnt, sizeof * buf)))
						panic("");
					i = bytes_read = cnt * sizeof * buf;
					if (ctx->cc->core_mem_read(ctx, buf, addr, &bytes_read) != GEAR_ERR_NO_ERROR)
					{
						dump_errcode(GEAR_ERR_TARGET_CTL_MEM_READ_ERROR, "error accessing target memory");
						miprintf("\n");
						break;
					}
					if (bytes_read != i)
						panic("");

					dump_errcode(GEAR_ERR_NO_ERROR, "");
					miprintf("MEMORY_CONTENTS_DUMP,");
					miprintf("[NR_DATA_ITEMS=%i,", cnt);
					if (byte_width)
						miprintf("BYTE_WIDTH=%i,", byte_width);
					miprintf("START_ADDR=0x%08x,", addr);
					miprintf("DATA_LIST=(");

					/*! \todo	remove this */
					miprintf_switch_debug(false);
					i = 0;
					while (i + 16 <= cnt)
					{
						miprintf(
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							"0x%08x,"
							, buf[i + 0]
							, buf[i + 1]
							, buf[i + 2]
							, buf[i + 3]
							, buf[i + 4]
							, buf[i + 5]
							, buf[i + 6]
							, buf[i + 7]
							, buf[i + 8]
							, buf[i + 9]
							, buf[i + 10]
							, buf[i + 11]
							, buf[i + 12]
							, buf[i + 13]
							, buf[i + 14]
							, buf[i + 15]
							);
						i += 16;
					}
					for (; i < cnt; i ++)
						miprintf("0x%08x,", buf[i]);
					/*! \todo	remove this */
					miprintf_switch_debug(true);
					free(buf);
				}
				/* handle other cases */
				else for (i = 0; i < cnt; i++)
				{
					memset(dump_buf, 0, sizeof dump_buf);
					int bytes_read;
					bytes_read = byte_width;
					if (ctx->cc->core_mem_read(ctx, dump_buf, addr, &bytes_read) != GEAR_ERR_NO_ERROR)
						panic("");
					if (byte_width != bytes_read)
						panic("");

					dump_errcode(GEAR_ERR_NO_ERROR, "");
					miprintf("MEMORY_CONTENTS_DUMP,");
					miprintf("[NR_DATA_ITEMS=%i,", cnt);
					if (byte_width)
						miprintf("BYTE_WIDTH=%i,", byte_width);
					miprintf("START_ADDR=0x%08x,", addr);
					miprintf("DATA_LIST=(");

					//miprintf("ADDR=0x%x,DATA_STRING=\"", addr);
					addr += byte_width;
					/*! \todo	handle all formats */
					switch(format)
					{
						unsigned long long tmp;
						unsigned long t1, t2;
						case FORMAT_DECIMAL:
							panic("");
							break;
						case FORMAT_HEX:
							miprintf("0x");
							/*! \todo	support all byte widths */
							if (byte_width > sizeof tmp)
								panic("");
							tmp = *(unsigned long long *)dump_buf;

							/* reverse half-bytes */
							/* process lower half */
							t1 = tmp & 0xffffffff;
							/* flip half-bytes */
							t1 = ((t1 & 0xf0f0f0f0) >> 4)
								| ((t1 & 0x0f0f0f0f) << 4);
							/* flip bytes */
							t1 = ((t1 & 0xff00ff00) >> 8)
								| ((t1 & 0x00ff00ff) << 8);
							/* flip half-words */
							t1 = ((t1 & 0xffff0000) >> 16)
								| ((t1 & 0x0000ffff) << 16);

							/* process upper half */
							/* flip half-bytes */
							t2 = (tmp >> 32) & 0xffffffff;
							t2 = ((t2 & 0xf0f0f0f0) >> 4)
								| ((t2 & 0x0f0f0f0f) << 4);
							/* flip bytes */
							t2 = ((t2 & 0xff00ff00) >> 8)
								| ((t2 & 0x00ff00ff) << 8);
							/* flip half-words */
							t2 = ((t2 & 0xffff0000) >> 16)
								| ((t2 & 0x0000ffff) << 16);
							/* combine halves */
							tmp = ((unsigned long long)(t1 & 0xffffffff) << 32) |
								(unsigned long long)(t2 & 0xffffffff);
							tmp >>= ((sizeof tmp) - byte_width) * 8;
							/*! \todo	maybe handle padding
							 *		here, right now all
							 *		hex numbers are padded
							 *		according to their
							 *		requested byte width */
							for (j = 0; j < byte_width * 2; j++, tmp >>= 4)
								miprintf("%c", (tmp & 0xf)["0123456789abcdef"]);

							break;
						case FORMAT_OCTAL:
							panic("");
							break;
							
					}
					miprintf(",");
				}
				miprintf("),]\n");
				break;
			}

		case EXCEPTION_KW:
		case EXISTS_KW:
		case FILE_KW:
			panic("");
/*			
files Command

In native mode, the files command lists file names that match a regular expression. In Java mode, the files command lists all of the Java source files known to dbx. If your Java source files are not in the same directory as the .class or .jar files, dbx might not find them unless you have set the $JAVASRCPATH environment variable (see Specifying the Location of Your Java Source Files).
Native Mode Syntax

files

    List the names of all files that contributed debugging information to the current program (those that were compiled with -g).
    files regular_expression

        List the names of all files compiled with-g that match the given regular expression.

where:

regular_expression is a regular expression.

For example:

(dbx) files ^r
myprog:
retregs.cc
reg_sorts.cc
reg_errmsgs.cc
rhosts.cc

Java Mode Syntax

files

    List the names of all of the Java source files known to dbx.
*/

		case FILES_KW:
			if (*dbx_lexer_str)
				panic("");
			dump_errcode(GEAR_ERR_NO_ERROR, "");
			srcfile_dump_sources_info_mi(ctx);
			miprintf("\n");
			break;
		case FIX_KW:
		case FIXED_KW:
		case FORTRAN_MODULES_KW:
		case FRAME_KW:
		case FUNC_KW:
		case FUNCS_KW:
		case GDB_KW:
		case HANDLER_KW:
		case HIDE_KW:
		case IGNORE_KW:
		case IMPORT_KW:
		case INTERCEPT_KW:
		case JAVA_KW:
		case JCLASSES_KW:
		case JOFF_KW:
		case JON_KW:
		case JPKGS_KW:
		case KILL_KW:
		case LANGUAGE_KW:
		case LINE_KW:
		case LIST_KW:
		case LISTI_KW:
		case LOADOBJECT_KW:
		case LWP_KW:
		case LWPS_KW:
		case MMAPFILE_KW:
		case MODULE_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case MODULES_KW:
			/*
			modules Command

			The modules command lists module names. It is valid only in native mode.
			Syntax

			modules [-v]

				List all modules.
			modules [-v] -debug

				List all modules containing debugging information.
			modules [-v] -read

				List names of modules containing debugging information that have been read in already.

				where:

				-v specifies verbose mode, which prints language, file names, etc.
			*/
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case NATIVE_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case NEXT_KW:
/*
next Command

The next command steps one source line (stepping over calls).

The dbx step_events environment variable (see Setting dbx Environment Variables) controls whether breakpoints are enabled during a step.
Native Mode Syntax

next

    Step one line (step over calls). With multithreaded programs when a function call is stepped over, all LWPs (lightweight processes) are implicitly resumed for the duration of that function call in order to avoid deadlock. Non-active threads cannot be stepped.
    next n

        Step n lines (step over calls).
	next ... -sig signal

	    Deliver the given signal while stepping.
	    next ... thread_id

	        Step the given thread.
		next ... lwp_id

		    Step the given LWP. Will not implicitly resume all LWPs when stepping over a function.

where:

n is the number of lines to step.

signal is the name of a signal.

thread_id is a thread ID.

lwp_id is an LWP ID.

When an explicit thread_id or lwp_id is given, the deadlock avoidance measure of the generic next command is defeated.

See also nexti Command for machine-level stepping over calls.
Note 

For information on lightweight processes (LWPs), see the Solaris Multithreaded Programming Guide. 
*/
			if (*dbx_lexer_str)
				panic("");
			/* rewind to the innermost (most recent)
			 * frame and execute from there */
			if (frame_move_to_relative(ctx, 0, 0) != GEAR_ERR_NO_ERROR)
				panic("");
			exec_step_over_src(ctx);
			break;

		case NEXTI_KW:
/*
nexti Command

The nexti command steps one machine instruction (stepping over calls). It is valid only in native mode.
Syntax

nexti

    Step one machine instruction (step over calls).
    nexti n

        Step n machine instructions (step over calls).
	nexti -sig signal

	    Deliver the given signal while stepping.
	    nexti ... lwp_id

	        Step the given LWP.
		nexti ... thread_id

		    Step the LWP on which the given thread is active. Will not implicitly resume all LWPs when stepping over a function.

where:

n is the number of instructions to step.

signal is the name of a signal.

thread_id is a thread ID.

lwp_id is an LWP ID.
*/
			if (*dbx_lexer_str)
				panic("");
			/* rewind to the innermost (most recent)
			 * frame and execute from there */
			if (frame_move_to_relative(ctx, 0, 0) != GEAR_ERR_NO_ERROR)
				panic("");
			exec_step_over_insn(ctx);
			break;

		case PATHMAP_KW:
		case POP_KW:
			panic("");
		case PRINT_KW:
			/*
			print Command

In native mode, the print command prints the value of an expression. In Java mode, the print command prints the value of an expression, local variable, or parameter.
Native Mode Syntax

print expression, ...

Print the value of the expression(s) expression, ... .
print -r expression

Print the value of the expression expression including its inherited members (C++ only).
print +r expression

Don’t print inherited members when the dbx output_inherited_members environment variable is on (C++ only).
print -d [-r] expression

Show dynamic type of expression expression instead of static type (C++ only).
print +d [-r] expression

Don’t use dynamic type of expression expression when the dbx output_dynamic_type environment variable is on (C++ only).
print -p expression

Call the prettyprint function.
print +p expression

Do not call the prettyprint Function when the dbx output_pretty_print environment variable is on.
print -L expression

If the printing object expression is larger than 4K, enforce the printing.
print +l expression

If the expression is a string (char *), print the address only, do not print the literal.
print -l expression

('Literal’) Do not print the left side. If the expression is a string (char *), do not print the address, just print the raw characters of the string, without quotes.
print -fformat expression

Use format as the format for integers, strings, or floating-point expressions.
print -Fformat expression

Use the given format but do not print the left hand side (the variable name or expression).
print -o expression

Print the value of expression, which must be an enumeration as an ordinal value. You may also use a format string here (-fformat). This option is ignored for non-enumeration expressions.
print -- expression

”--’ signals the end of flag arguments. This is useful if expression may start with a plus or minus (seeProgram Scope for scope resolution rules.

where:

expression is the expression whose value you want to print.

format is the output format you want used to print the expression. If the format does not apply to the given type, the format string is silently ignored and dbx uses its built-in printing mechanism.

The allowed formats are a subset of those used by the printf(3S) command. The following restrictions apply:

*

No n conversion.
*

No * for field width or precision.
*

No %<digits>$ argument selection.
*

Only one conversion specification per format string.

The allowed forms are defined by the following simple grammar:

FORMAT ::= CHARS % FLAGS WIDTH PREC MOD SPEC CHARS

CHARS ::= <any character sequence not containing a %>

| %%

| <empty>

| CHARS CHARS

FLAGS ::= + | - | <space> | # | 0 | <empty>

WIDTH ::= <decimal_number> | <empty>

PREC ::= . | . <decimal_number> | <empty>

MOD ::= h | l | L | ll | <empty>

SPEC ::= d | i | o | u | x | X | f | e | E | g | G |

c | wc | s | ws | p

If the given format string does not contain a %, dbx automatically prepends one. If the format string contains spaces, semicolons, or tabs, the entire format string must be surrounded by double quotes.
*/
		{
			char * c_exp;

			if (*dbx_lexer_str == '-')
			{
				panic("");
			}

			c_exp = extract_expression();
			if (*dbx_lexer_str)
				panic("");
			c_expression_eval_and_print(ctx, c_exp,
					(struct expr_eval_flags) { .print_type = 1, .print_val = 1, });
			free(c_exp);

			break;
		}
		case PROC_KW:
		case PROG_KW:
		case QUIT_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case REGS_KW:
			/*
			regs Command

			The regs command prints the current value of registers. It is valid only in native mode.
			Syntax

			regs [-f] [-F]

				where:

				-f includes floating-point registers (single precision) (SPARC platform only)

				-F includes floating-point registers (double precision) (SPARC platform only)
			*/	
			if (*dbx_lexer_str == '-')
			{
				switch (*++dbx_lexer_str)
				{
					case 'f':
						panic("");
						break;
					case 'F':
						panic("");
						break;
					default:
						panic("");
				}
			}
			else if (*dbx_lexer_str)
				panic("");
			else
			{
				ARM_CORE_WORD * regs;
				int i;
				int nr_target_core_regs;
				enum GEAR_ENGINE_ERR_ENUM err;

				if (!(nr_target_core_regs = ctx->tdesc->get_nr_target_core_regs(ctx)))
					panic("");
				if (!(regs = malloc(sizeof(ARM_CORE_WORD[nr_target_core_regs]))))
					panic("");
				err = ctx->cc->core_reg_read(ctx,
						0,
						(1 << nr_target_core_regs) - 1,
						regs);
				switch (err)
				{
					default:
						dump_errcode(GEAR_ERR_TARGET_ACCESS_ERROR, "error accessing target");
						miprintf("\n");
						break;
					case GEAR_ERR_TARGET_CORE_DEAD:
						dump_errcode(GEAR_ERR_TARGET_ACCESS_ERROR, "no target connected");
						miprintf("\n");
						break;
					case GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING:
						dump_errcode(GEAR_ERR_TARGET_ACCESS_ERROR, "cannot access registers while target is running; halt the target first");
						miprintf("\n");
						break;
					case GEAR_ERR_NO_ERROR:
						break;
				}
				if (err != GEAR_ERR_NO_ERROR)
					break;

				dump_errcode(GEAR_ERR_NO_ERROR, "");
				miprintf("REGLIST,(");

				for (i = 0; i < nr_target_core_regs; i++)
				{
				const char * alt_reg_name;
					alt_reg_name = ctx->tdesc->translate_target_core_reg_nr_to_human_readable(ctx, i);
					if (alt_reg_name)
						miprintf("\"%s(r%i)\" = 0x%08x, ", alt_reg_name, i, regs[i]);
					else
						miprintf("\"r%i\" = 0x%08x, ", i, regs[i]);
				}
				miprintf(")\n");
				free(regs);
				res = 0;
			}

			break;
		case REPLAY_KW:
		case RERUN_KW:
		case RESTORE_KW:
		case RPRINT_KW:
		case RTC_KW:
		case RUN_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case RUNARGS_KW:
		case SAVE_KW:
		case SCOPES_KW:
		case SEARCH_KW:
		case SHOWBLOCK_KW:
		case SHOWLEAKS_KW:
		case SHOWMEMUSE_KW:
		case SOURCE_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
			/*
			   status Command

			   The status command lists event handlers (breakpoints, etc.). It has identical syntax and identical functionality in native mode and Java mode.
			   Syntax

			   status

			   Print trace, when, and stop breakpoints in effect.
			   status handler_id

			   Print status for handler handler_id.
			   status -h

			   Print trace, when, and stop breakpoints in effect including the hidden ones.
			   status -s

			   The same, but the output can be read by dbx.

where:

handler_id is the identifier of an event handler.
			 */
		case STATUS_KW:
		{
			char * srcname;
			int srcline_nr;
			bool is_addr_at_src_boundary;
			struct bkpt_struct * bkpt;

			dump_errcode(GEAR_ERR_NO_ERROR, "");
			miprintf("BREAKPOINT_DATA, BREAKPOINT_LIST");

			if (bkpt = ctx->bkpts)
			{
				miprintf(" = (");
				while (bkpt)
				{
					srcfile_get_srcinfo_for_addr(ctx, bkpt->addr, 0, 0, &srcname, &srcline_nr, &is_addr_at_src_boundary); 
							
					miprintf("[ADDR = 0x%08x,", bkpt->addr);
					miprintf("SRCFILE = \"%s\",", srcname ? srcname : "");
					miprintf("SRCLINE = %i,", srcline_nr);
					miprintf("IS_AT_START_OF_STATEMENT = %i,", is_addr_at_src_boundary ? 1 : 0);
					miprintf("],");

					bkpt = bkpt->next;
				}
				miprintf(")");
			}
				miprintf("\n");
			break;
		}
		case STEP_KW:
/*
step Command

The step command steps one source line or statement (stepping into calls that were compiled with the -g option).

The dbx step_events environment variable controls whether breakpoints are enabled during a step.

The dbx step_granularity environment variable controls granularity of source linestepping.

The dbx step_abflow environment variable controls whether dbx stops when it detects that "abnormal" control flow change is about to occur. Such control flow change can be caused by a call to siglongjmp() or longjmp() or an exception throw.
Native Mode Syntax

step

    Single step one line (step into calls). With multithreaded programs when a function call is stepped over, all threads are implicitly resumed for the duration of that function call in order to avoid deadlock. Non-active threads cannot be stepped.
    step n

        Single step n lines (step into calls).
	step up

	    Step up and out of the current function.
	    step ... -sig signal

	        Deliver the given signal while stepping. If a signal handler exists for the signal, step into it if the signal handler was compiled with the -g option.
		step ...thread_id

		    Step the given thread. Does not apply to step up.
		    step ...lwp_id

		        Step the given LWP. Does not implicitly resume all LWPs when stepping over a function.
			step to [ function ]

			    Attempts to step into function called from the current source code line. If function is not given, steps into the last function, helping to avoid long sequences of step commands and step up commands. Examples of the last function are:

			        f()->s()-t()->last();

				    last(a() + b(c()->d()));

where:

n is the number of lines to step.

signal is the name of a signal.

thread_id is a thread ID.

lwp_id is an LWP ID.

function is a function name.

Only when an explicit lwp_id is given, the deadlock avoidance measure of the generic step command is defeated.

When executing the step tocommand, while an attempt is made to step into the last assemble call instruction or step into a function (if specified) in the current source code line, the call may not be taken due to a conditional branch. In a case where the call is not taken or there is no function call in the current source code line, the step to command steps over the current source code line. Take special consideration on user-defined operators when using the step to command.

See also stepi Command for machine-level stepping.
*/
			if (*dbx_lexer_str)
				panic("");
			/* rewind to the innermost (most recent)
			 * frame and execute from there */
			if (frame_move_to_relative(ctx, 0, 0) != GEAR_ERR_NO_ERROR)
				panic("");
			exec_single_step_src(ctx);
			break;
		case STEPI_KW:
/*
stepi Command

	The stepi command steps one machine instruction (stepping into calls). It is valid only in native mode.
	Syntax

	stepi

	    Single step one machine instruction (step into calls).
	    stepi n

		Single step n machine instructions (step into calls).
		stepi -sig signal

		    Step and deliver the given signal.
		    stepi ...lwp_id

			Step the given LWP.
			stepi ...thread_id

			    Step the LWP on which the given thread is active.

			    where:

			    n is the number of instructions to step.

			    signal is the name of a signal.

			    lwp_id is an LWP ID.

			    thread_id is a thread ID.
*/
			if (*dbx_lexer_str)
				panic("");
			/* rewind to the innermost (most recent)
			 * frame and execute from there */
			if (frame_move_to_relative(ctx, 0, 0) != GEAR_ERR_NO_ERROR)
				panic("");
			exec_single_step_insn(ctx);
			break;
		case STOP_KW:
		{
			ARM_CORE_WORD addr;
			char * srcname;
			int srcline_nr;
			bool is_addr_at_src_boundary;
			int i;
			char * c_exp;
			signed long long exp_val;

			token_len = 0;
			lexidx = 0;
			if (dbx_lex() != AT_KW)
				panic("");
			dbx_lexer_str += token_len;
			dbx_lexer_str += strspn(dbx_lexer_str, " \t");
			srcname = dbx_lexer_str;
			i = 0;
			if (*dbx_lexer_str == '"')
			{
				i = /* count the initial double quote character */ 1;
				i += strcspn(srcname + 1, "\"");
				if (!*dbx_lexer_str)
					panic("");
				dbx_lexer_str += i + 1;
				if (!*dbx_lexer_str)
					panic("");
				dbx_lexer_str++;
			}
			dbx_lexer_str += strspn(dbx_lexer_str, " \t:");
			if (!*dbx_lexer_str)
				panic("");

			c_exp = extract_expression();
			if (!c_expression_eval_int(ctx, c_exp, &exp_val))
				panic("");
			free(c_exp);

			srcname[i] = 0;

			if (*srcname)
			{
				/* halt at source code location */
				gprintf("searching for address for %s:%i\n", srcname + 1, (unsigned int) exp_val);
				if (srcfile_get_core_addr_for_line_nr(ctx, srcname + 1, exp_val, &addr)
						!= GEAR_ERR_NO_ERROR)
				{
					dump_errcode(GEAR_ERR_GENERIC_ERROR, "could not locate target core address for "
"the source file/line number supplied");
					miprintf("BREAKPOINT_DATA,");
					miprintf("\n");
					break;
				}
				gprintf("ok, address is 0x%08x\n", (unsigned int) addr);
			}
			else
				/* halt at target core address */
				addr = exp_val;

			if (bkpt_setup_at_addr(ctx, addr) == 0)
			{
				dump_errcode(GEAR_ERR_GENERIC_ERROR, "breakpoint already set");
				miprintf("BREAKPOINT_DATA,");
				miprintf("\n");
				break;
			}
			dump_errcode(GEAR_ERR_NO_ERROR, "");
			srcfile_get_srcinfo_for_addr(ctx, addr, 0, 0, &srcname, &srcline_nr, &is_addr_at_src_boundary); 
			miprintf("BREAKPOINT_DATA, BREAKPOINT_INSERTED = ([ADDR = 0x%08x,", addr);
			miprintf("SRCFILE = \"%s\",", srcname ? srcname : "");
			miprintf("SRCLINE = %i,", srcline_nr);
			miprintf("IS_AT_START_OF_STATEMENT = %i,", is_addr_at_src_boundary ? 1 : 0);
			miprintf("],)\n");
			break;
		}
		case STOPI_KW:
		/*
		   stopi Command

		   The stopi command sets a machine-level breakpoint. It is valid only in native mode.
		   Syntax

		   The stopi command has the following general syntax:

		   stopi event_specification [modifier]

		   When the specified event occurs, the process is stopped.

		   The following specific syntaxes are valid:

		   stopi at address

		   Stop execution at location address.
		   stopi in function

		   Stop execution when function is called.

where:

address is any expression resulting in or usable as an address.

function is the name of a function.

For a list and the syntax of all events see Setting Event Specifications.
		 */
		{
			signed long long exp_val;
			char * c_exp;
			ARM_CORE_WORD val;
			char * srcfile;
			int srcline_nr;
			bool is_addr_at_src_boundary;

			token_len = 0;
			lexidx = 0;
			if (dbx_lex() != AT_KW)
				panic("");
			dbx_lexer_str += token_len;
			c_exp = extract_expression();
			if (!c_expression_eval_int(ctx, c_exp, &exp_val))
				panic("");
			free(c_exp);
			val = exp_val;
			/* breakpoint address alignment checking */
			/*! \todo	modify this to handle thumb mode
			 *		as well */
			//if (val & 3)
				//panic("");
			/*! \todo	validate target and engine state here */
			/*
			if (exec_get_engine_state(ctx) != EXEC_STATE_IDLE)
				panic("");
			*/
			if (bkpt_setup_at_addr(ctx, val) == 0)
				panic("");
			dump_errcode(GEAR_ERR_NO_ERROR, "");
			srcfile_get_srcinfo_for_addr(ctx, val, 0, 0, &srcfile, &srcline_nr, &is_addr_at_src_boundary); 
			miprintf("BREAKPOINT_DATA, BREAKPOINT_INSERTED = ([ADDR = 0x%08x,", val);
			miprintf("SRCFILE = \"%s\",", srcfile ? srcfile : "");
			miprintf("SRCLINE = %i,", srcline_nr);
			miprintf("IS_AT_START_OF_STATEMENT = %i,", is_addr_at_src_boundary ? 1 : 0);
			miprintf("],)\n");
			break;
		}
		case SUPPRESS_KW:
		case SYNC_KW:
		case SYNCS_KW:
		case THREAD_KW:
		case THREADS_KW:
		case TRACE_KW:
		case TRACEI_KW:
		case UNCHECK_KW:
		case UNDISPLAY_KW:
		case UNHIDE_KW:
		case UNINTERCEPT_KW:
		case UNSUPPRESS_KW:
		case UNWATCH_KW:
			panic("");
			break;
		case UP_KW:
/*
up Command

	The up command moves up the call stack (toward main). It has identical syntax and identical functionality in native mode and in Java mode.
	Syntax

	up

	    Move up the call stack one level.
	    up number

	        Move up the call stack number levels.
		up -h [number]

		    Move up the call stack, but don.t skip hidden frames.

		    where:

		    number is a number of call stack levels.
*/
			{
				int frame_cnt;

				/* by default, if no argument is present,
				 * go up one frame */
				frame_cnt = -1;

				token_len = 0;
				lexidx = 0;
				if (dbx_lex() == DBX_LEX_NUM_CONSTANT_TOKEN)
					frame_cnt = - dbx_lval.num;
				frame_move_to_relative(ctx, frame_cnt, &frame_nr);
				miprintf("STACK_FRAME_CHANGED, SELECTED_FRAME = %i\n", frame_nr);
				break;
			}
		case USE_KW:
		case WATCH_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		case WHATIS_KW:
/*
whatis Command

In native mode, the whatis command prints the type of expression or declaration of type. In Java mode, the whatis command prints the declaration of an identifier. If the identifier is a class, it prints method information for the class, including all inherited methods.
Native Mode Syntax

what is [-n] [-r] name

Print the declaration of the non-type name.
whatis -t [-r] type

Print the declaration of the type type.
whatis -e [-r] [-d] expression

Print the type of the expression expression.

where:

name is the name of a non-type.

type is the name of a type.

expression is a valid expression.

-d shows dynamic type instead of static type (C++ only).

-e displays the type of an expression.

-n displays the declaration of a non-type. It is not necessary to specify -n; this is the default if you type the whatis command with no options.

-r prints information about base classes (C++ only).

-t displays the declaration of a type.

The whatis command, when run on a C++ class or structure, provides you with a list of all the defined member functions (undefined member functions are not listed), the static data members, the class friends, and the data members that are defined explicitly within that class.

Specifying the -r (recursive) option adds information from the inherited classes.

The-d flag, when used with the -e flag, uses the dynamic type of the expression.

For C++, template-related identifiers are displayed as follows:

*

All template definitions are listed with whatis -t.
*

Function template instantiations are listed with whatis.
*

Class template instantiations are listed with whatis -t.
*/
		{
			char * c_exp;

			if (*dbx_lexer_str == '-')
			{
				/* the non c++ options seem redundant... */
				if (*++dbx_lexer_str != 'e')
					panic("");
				dbx_lexer_str++;
			}

			c_exp = extract_expression();
			if (*dbx_lexer_str)
				panic("");
			c_expression_eval_and_print(ctx, c_exp,
					(struct expr_eval_flags) { .print_type = 1, .print_val = 0, });
			free(c_exp);

			break;
		}
		case WHEN_KW:
		case WHENI_KW:
		case WHERE_KW:
/*
where Command

The where command prints the call stack.
Native Mode Syntax

where

    Print a procedure traceback.
    where number

        Print the number top frames in the traceback.
	where -f number

	    Start traceback from frame number.
	    where -fp address_expression

	        Print traceback as if fp register hadaddress_expression value.
		where -h

		    Include hidden frames.
		    where -l

		        Include library name with function name.
			where -q

			    Quick traceback (only function names).
			    where -v

			        Verbose traceback (include function args and line info).

where:

number is a number of call stack frames.

Any of the above forms may be combined with a thread or LWP ID to obtain the traceback for the specified entity.

The -fp option is useful when the fp (frame pointer) register is corrupted, in which event dbx cannot reconstruct call stack properly. This option provides a shortcut for testing a value for being the correct fp register value. Once you have identified the correct value has been identified, you can set it with an assign command or lwp command.
Java Mode Syntax

where [thread_id]

    Print a method traceback.
    where [thread_id] number

        Print the number top frames in the traceback.
	where -f [thread_id] number

	    Start traceback from frame number.
	    where -q [thread_id]

	        Quick trace back (only method names).
		where -v [thread_id]

		    Verbose traceback (include method arguments and line information).

		    where:

		    number is a number of call stack frames.

		    thread_id is a dbx-style thread ID or the Java thread name specified for the thread.
*/
		{
		ARM_CORE_WORD pc;
		struct cu_data * cu;
		struct subprogram_data * subp;
		char * srcfile;
		int srcline_nr;
		int selected_frame_nr;

			if (*dbx_lexer_str)
				panic("");
			if ((res = frame_move_to_relative(ctx, 0, &selected_frame_nr)) != GEAR_ERR_NO_ERROR)
			{
				switch (res)
				{
					case GEAR_ERR_CANT_UNWIND_STACK_FRAME:
						dump_errcode(res, "cannot unwind more");
						miprintf("\n");
						break;
					case GEAR_ERR_CANT_REWIND_STACK_FRAME:
						dump_errcode(res, "cannot rewind more");
						miprintf("\n");
						break;
					case GEAR_ERR_BACKTRACE_DATA_UNAVAILABLE:
						dump_errcode(res, "backtrace data unavailable");
						miprintf("\n");
						break;
				}
				break;
			}
			dump_errcode(GEAR_ERR_NO_ERROR, "");
			miprintf("BACKTRACE,(");
			/* start from the innermost frame - rewind to
			 * the innermost (most recent) frame */
			while (1) 
			{
				miprintf("[");
				/*! \todo	handle mode correctly */
				if (ctx->cc->core_reg_read(ctx,
							0,
							1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
							&pc) != GEAR_ERR_NO_ERROR)
					panic("");
				miprintf("PC_ADDR = 0x%08x,", (int) pc);

				srcfile_get_srcinfo_for_addr(ctx, pc, &cu, &subp,
					&srcfile, &srcline_nr, 0);

				if (cu)
				{
					miprintf("COMP_UNIT = \"%s\", ", cu->name);
				}
				if (subp)
				{
					miprintf("SUBPROGRAM = \"%s\", ", subp->name);
				}
				if (srcfile)
				{
					miprintf("SRCFILE = \"%s\", ", srcfile);
				}
				if (srcline_nr)
					miprintf("SRCLINE_NR = %i, ", srcline_nr);
				miprintf("], ");
				if (selected_frame_nr == MAX_NR_FRAMES_TO_UNWIND)
					break;
				res = frame_move_to_relative(ctx, -1, &selected_frame_nr);
				if (res == GEAR_ERR_CANT_UNWIND_STACK_FRAME)
					break;
				else if (res != GEAR_ERR_NO_ERROR)
					panic("");
			}
			/* rewind to the innermost (most recent) frame */
			if (frame_move_to_relative(ctx, 0, 0) != GEAR_ERR_NO_ERROR)
				panic("");
			/* dump machine interface message footer */
			miprintf(")\n");

		}
		break;
		case WHEREAMI_KW:
		case WHEREIS_KW:
		case WHICH_KW:
		case WHOCATCHES_KW:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_CODED, "dbx command not yet coded");
			miprintf("\n");
			break;
		/* the following are hacks not in the original dbx
		 * command set */
		case HACK_REG_KW:
		/* try to recognize a pattern of the form:
		 * reg = "expression"
		 * this is here to support assignment to registers, which
		 * is currently not supported in the 'normal' dbx syntax
		 *
		 *	\todo	remove this once assignment to registers
		 *		is properly coded */

		{
			int dest_reg;
			signed long long exp_val;
			char * c_exp;
			ARM_CORE_WORD val;

			dest_reg = dbx_lval.reg_nr;
			token_len = 0;
			lexidx = 0;
			if (dbx_lex() != '=')
				panic("");
			dbx_lexer_str += token_len;
			c_exp = extract_expression();
			if (!c_expression_eval_int(ctx, c_exp, &exp_val))
				panic("");
			free(c_exp);
			val = exp_val;

			if (ctx->cc->core_reg_write(ctx, 0,
						1 << dest_reg, &val) != GEAR_ERR_NO_ERROR)
				panic("");
			/* leave the scanner buffer in a known state */
			YY_FLUSH_BUFFER;

			break;
		}
		default:
			dump_errcode(GEAR_ERR_DBX_CMD_NOT_RECOGNIZED, "dbx command not recognized");
			miprintf("\n");
			break;
	}

	/* clean up the lexer buffer for any successive
	 * lexer calls */
	YY_FLUSH_BUFFER;
	return res;
}

