/*!
 *	\file	target-comm.c
 *	\brief	engine - target core controller communication routines
 *	\author	shopov
 *
 *	this module provides the link between the gear engine
 *	and a target core controller; a target core controller
 *	is defined as an agent that is capable of exercising control
 *	(be it directly or indirectly)
 *	over a target processor core for the purposes of debug,
 *	and which is also capable of receiving and
 *	processing target core control requests from the gear
 *	engine; this can be for example a standalone process that
 *	listens on a pipe or a socket for incoming target control
 *	requests from the gear engine and dispatches these requests
 *	to a target processor core by using some dedicated piece
 *	of hardware (e.g. a wiggler on a parallel port); this is
 *	just one example, many other combinations are possible
 *
 *	this module contains the communication bridge between
 *	the gear engine and a target core controller
 *
 *	read the comments at the beginning of file exec.c;
 *	amongst other things, there it is described why and when
 *	the target state change notification callbacks are
 *	invoked
 *	as stated there:
 *			as an implementaion detail, it is notable that
 *			all the points where the transitions between
 *			the target core connection states are possible to occur,
 *			physically reside within module target-comm.c,
 *			and at all such points a target state change
 *			notification callback function (which, most
 *			commonly, is function exec_update_target_state()
 *			from this module) is invoked; also see comments
 *			about the core_register_target_state_change_callback
 *			data field in structure core_control in file
 *			core-access.h
 *	for further details, read the comments in the beginnig
 *	of file exec.c and the documentation about enum TARGET_CORE_STATE_ENUM
 *	in file target-state.h
 *	\note	in order to minimize perplexity of someone hacking
 *		the gear, all modules, and all callback
 *		functions contained in these modules, that are
 *		present in the gear engine, *MUST* be listed
 *		in file core-control.h
 *
 *	\note	as can be seen from the architectural design,
 *		for the gear engine it is far more important
 *		whether a connection to a target core
 *		is established or not, than whether a connection
 *		to a target core controller is established or not;
 *		whether there is a live connection
 *		to a target core controller is of no interest
 *		to the majority of the code
 *		in the gear engine (indeed - this is flagged
 *		by a single internal flag in this module -
 *		see the is_core_controller_connected flag
 *		in the core_connection_data structure below);
 *		so, it has been my(sgs) intention to keep
 *		the details about the proper maintenance
 *		of the connection to a target core controller
 *		wholly within this module and as transparent
 *		as possible to other modules of the gear engine
 *
 *
 *	\note	it is important to invoke the core_open()
 *		routine to connect to a target core prior
 *		to invoking any other target core related
 *		routines
 *	\warning	it is possible that an error occurs, but,
 *			due to internal lexical scanner buffering,
 *			the scanner does not return immediately
 *			zero, signifying that it cannot continue
 *			scanning; thus, it is advisable to check
 *	 		the lex_err_code field after *each*
 *			invocation of the scanner; also read
 *			the comments about the buf field
 *			in struct core_connection_data below
 *
 *	\note	read the comments about the incoming data
 *		buffer variables in struct core_connection_data 
 *		if interested in how incoming data from a
 *		target core controller is buffered and processed
 *
 *	\todo	write here some notes regarding buffer usage
 *
 *	\todo	provide a reference to the documentation files
 *		when they are written
 *
 *	\todo	provide a mechanism for invoking a
 *		corresponding yylex_destroy() to the yy_init()
 *		call in init_target_comm()
 *	
 *	\todo	proofread and edit *ALL* of the comments in this file
 *		as they did change rapidly over a short period of
 *		time
 *	\todo	document the generated request data records and
 *		expected reply data records syntax
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*! \todo	clean this up */
extern int frontend_fds[2];
/*
 *
 * include section follows
 *
 */

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

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "core-access.h"
#include "util.h"
#include "fdprintf.h"
#include "gprintf.h"

/*
 *
 * local types follow
 *
 */ 

/* these two below are for the needs of the flex generated
 * lexer - really not anything exciting... */
/*! this one is to satisfy the lexer declarations */
#define YY_TYPEDEF_YY_SCANNER_T
/*! this one is to satisfy the lexer declarations */
typedef void * yyscan_t;

/*! an enumeration denoting the type of the lexical token recognized
 *
 * individual members denote which additional member (if any)
 * of the yylval union in struct core_connection_data below
 * is applicable for the lexical token type described */
enum LEX_TOKEN_TYPE_ENUM
{
	/*! no more lexical tokens available
	 *
	 * it is important that this be zero, as this value is used
	 * by the flex generated code */
	LEX_TOKEN_NO_MORE_TOKENS = 0,
	/*! error code string; applicable is the 'status_code' field */
	LEX_TOKEN_STATUS_CODE = 257,
	/*! a number; applicable is the 'n' field */
	LEX_TOKEN_NUMBER,
	/*! target core state information; applicable is the 'state' field */
	LEX_TOKEN_TARGET_STATE_INFO,
	/*! target core controller error message hint keyword */
	LEX_TOKEN_ERRMSG_HINT_KW,
	/*! a c-type escaped string, applicable is the 'cstr' field */
	LEX_TOKEN_CSTR,
};

/*! a data structure to hold target core and target core controller connection parameters */
struct core_connection_data
{
	/*! a flag to denote whether connection to a target core controller is currently established
	 *
	 * when cleared to zero - a connection to a target core controller is
	 * not currently established, when set to non-zero - the connection
	 * is active
	 *
	 * this flag is used in this modules in various routines to catch
	 * simple errors (such as trying to talk to a non-connected target core) */
	int is_core_controller_connected;

	/* file descriptors used for communicating with a target core controller -
	 * read nd write direction - direction is *always* viewed from the gear
	 * engine side
	 *
	 * a few notes - there are two file descriptors - one for reading and
	 * one for writing - in order to accomodate different scenarios of
	 * communication channel usage; for example - the communication channel
	 * may be a pipe set (in which case two different file descriptors
	 * have to be used), the stdin/stdout pair (in this case two file
	 * descriptors are also needed), a socket (in this case one file
	 * descriptor suffices - it is simply duplicated in the file descriptors
	 * for either direction), etc. */

	/*! file descriptor for the gear engine to read data from a target core controller */
	int read_core_fd;
	/*! file descriptor for the gear engine to write data to a target core controller */
	int write_core_fd;
#ifdef __LINUX__
	/*! a file pointer associated with the write_core_fd file descriptor
	 *
	 * \todo	maybe clean this up, it is needed for the
	 *		fprintf()-s to work... */
	FILE	* write_core_file;
#endif

	/*! target core controller internet address, in case communications is via sockets */
	struct sockaddr_in	core_inet_addr;

	/*! the target core state change callback function stack/list
	 *
	 * for more details, read the comments about 
	 * core_register_target_state_change_callback in data structure
	 * core_control in file core-access.h */
	struct
	{
/*! the maximum number of elements in the stack
 *
 * \todo	maybe move this somewhere else */
#define CALLBACK_STACK_SIZE	10		
		/*! the stack itself */
		bool (*callback_stack[CALLBACK_STACK_SIZE])
			(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state);
		/*! the stack pointer value
		 *
		 * an 'empty ascending' convention for the stack
		 * is being used here - that means that the stack
		 * pointer equals the index of the first unoccupied
		 * element of the stack, and the stack grows upwards;
		 * therefore, when the stack pointer equals zero, the
		 * stack is empty, and when it equals CALLBACK_STACK_SIZE,
		 * the stack is full */
		int stack_ptr;
		
	}
	target_state_change_callback_stack;
	/*! extra lexer state data structure to be passed to the lexer */
	struct
	{
		/*! target core controller input lexer state variables */
		yyscan_t	yyscanner;

		/* these variables are handling buffering input from a target core controller;
		 * in order to improve responsiveness and to minimize glitches
		 * and various artifacts caused by slow connection speeds,
		 * input from a target core controller is buffered and processed in chunks;
		 * such a buffering scheme actually only makes any difference 
		 * (and comes in effect) only in large data transfers in the
		 * direction from a target core controller to the gear engine,
		 * which is currently the case only for reading target memory;
		 * the catch here is that if a large read request from target 
		 * core memory is issued in one piece, an arbitrary large amount
		 * of time may pass for all of the data to be fetched to the gear
		 * engine, which may adversely affect responsiveness; this is quite
		 * unacceptable for a debugging engine, which the gear engine is;
		 * in order to cope with such situations, large data transfer
		 * requests are broken into smaller parts and smaller data amount
		 * requests are issued to a target core controller; thus whenever
		 * one such smaller data request transfer completes, it is possible
		 * to send some kind of a notification event to be consumed by
		 * a gear engine output consumer (e.g., a frontend), which may
		 * further provide some kind of feedback to an end-user to
		 * keep him at bay during long data transfers; while the buffering
		 * scheme described does not apply in the direction from the gear engine
		 * to a target core controller, it should be evident that it is
		 * highly desirable that large data transfers to target core
		 * memory (writing data to target memory) are similarly
		 * broken down into smaller data transfer requests (a common
		 * example is writing to target memory during executable
		 * image load, which can, in whole, also take arbitrary
		 * amounts of time)
		 *
		 * in order for the buffering scheme to be properly explained,
		 * it should be remembered that the current format of the
		 * data transferred from a target core controller to
		 * the gear engine is in a lavish human readable, ascii text
		 * form instead of some much more compact and efficient binary
		 * format; this is done to facilitate debugging in the early
		 * development stages, and may be changed later; in this form,
		 * however, data received by the gear engine is not immediately
		 * usable - it should be converted to a binary form first, and
		 * this can be achieved by a simple lexical scanner that
		 * extracts tokens from the incoming data stream; also, currently
		 * the end of an input record from a target core controler to
		 * the gear engine is signified by a newline ('\n') character
		 * (which must not be followed by any other characters)
		 *
		 * currently, the following scheme of data buffering and
		 * lexical scanning is employed:
		 *	- a consumer of incoming data from a target core controller
		 *	is *forbidden* to invoke the buffer filling function
		 *	(with the deliberately not very nice name)
		 *	must_be_only_called_from_yylex_wait_incoming_data()
		 *	directly - only the lexical scanner is allowed
		 *	to invoke this function directly; the only
		 *	allowed method for a consumer to extract lexical
		 *	tokens from the incoming data stream is to invoke
		 *	a supplied lexical scanner
		 *	- the lexical scanner is the only routine that is
		 *	allowed to invoke the buffer filling function
		 *	must_be_only_called_from_yylex_wait_incoming_data();
		 *	the lexical scanner then returns recognized lexical
		 *	tokens, which are of an integral type, with the value
		 *	zero reserved for the lexical scanner to return in
		 *	case the lexical scanning cannot continue; this
		 *	is the case on two occassions:
		 *		- the input is exhausted and so no more
		 *		lexical tokens are available
		 *		- there is an error returned by the
		 *		buffer filling function
		 *	in any case, the details why the scanner cannot
		 *	continue are available to routines invoking the
		 *	scanner via the lex_err_code field
		 *	below, and it is highly recommended that this
		 *	status code is *always* inspected and handled
		 *	properly
		 *
		 *	\warning	it is possible that an error occurs, but,
		 *			due to internal lexical scanner buffering,
		 *			the scanner does not return immediately
		 *			zero, signifying that it cannot continue
		 *			scanning; thus, it is advisable to check
		 *	 		the lex_err_code field after *each*
		 *			invocation of the scanner
		 */

		/*! a buffer holding incoming data from a target core controller*/
		unsigned char	* buf;
		/*! the buffer size, in bytes */
		int		buf_size;
		/*! number of bytes currently in the buffer 
		 *
		 * this is also equal to the index of the first byte that is ok
		 * to be overwritten by incoming data - hence the name of this field
		 *
		 * must be initialized to zero prior to invoking
		 * the lexical scanner
		 */
		int		buf_idx;
		/*! lexical scanner buffer index - the index of the first byte not read by the lexer
		 *
		 * this one is used by the lexical scanner (in YY_INPUT()) to
		 * decide when to read more data by invoking
		 * must_be_only_called_from_yylex_wait_incoming_data()
		 *
		 * must be initialized to zero prior to invoking the
		 * lexical scanner
		 */
		int		lex_idx;
		/*! error code of the lexer when it returns 0, that is, when lexical scanning cannot continue
		 *
		 * this is either GEAR_ERR_NO_ERROR, in case lexical
		 * scanning terminated because the input was exhausted
		 * (a newline character was read and no errors were
		 * encountered), or the corresponding error code of
		 * must_be_only_called_from_yylex_wait_incoming_data(),
		 * if this routine encountered an error 
		 *
		 * must be initialized to GEAR_ERR_NO_ERROR prior to invoking the
		 * lexical scanner
		 */
		enum GEAR_ENGINE_ERR_ENUM	lex_err_code;
		/*! a union holding additional data for a lexical token
		 *
		 * what member of this is applicable depends on the
		 * value of the lexical token type enumeration value
		 * (enum LEX_TOKEN_TYPE_ENUM above) recognized
		 * by the lexer */
		union
		{
			/*! status code, applicable when the token is of type LEX_TOKEN_STATUS_CODE
			 *
			 * this is the error status code as reported
			 * by a target core controller to a request from
			 * the gear engine, this is *not* the error
			 * code returned by the lexer when it can
			 * not continue scanning */
			enum GEAR_ENGINE_ERR_ENUM	status_code;
			/*! a number, applicable when the token is of type LEX_TOKEN_NUMBER */
			int	n;
			/*! target core status */
			enum TARGET_CORE_STATE_ENUM state;
			/*! a c-type escaped string; must be free()-d when no longer needed */
			const char * cstr;
		}
		yylval;

	}
	lex_state;
};

/*
 *
 * local function prototypes follow
 *
 */ 
static enum GEAR_ENGINE_ERR_ENUM must_be_only_called_from_yylex_wait_incoming_data(struct gear_engine_context * ctx);


/* directly include the lexer source - after the struct core_connection_data
 * as it is needed by the lexer */
#include "lex.target_.c"


/*
 *
 * local constants follow
 *
 */

/*! size of the buffer to receive incoming data from a target core controller, in bytes
 *
 * this cant be too small; also, see the comments about the buf field
 * in struct core_connection_data above
 *
 * \todo	this is deliberately small now in the active development
 *		phase so that bugs show themselves easier; once
 *		debugging is finished, make this bigger */
static const int CORE_CONTROLLER_BUFFER_SIZE = 8;
/*! timeout value used when waiting for input from a target core controler
 *
 * this is used in must_be_only_called_from_yylex_wait_incoming_data() */ 
static const int CORE_CONTROLLER_READ_TIMEOUT_MS = 10000/*0*/;

/*! the number of bytes to transfer (read or write) in one chunk when accessing target memory
 *
 * \todo	a sensible way must be provided for computing
 *		this in order to minimize communication overhead
 *		while preserving sane responsiveness parameters;
 *		this shouldnt be too hard
 *
 * \todo	this is deliberately small now in the active development
 *		phase so that bugs show themselves easier; once
 *		debugging is finished, make this bigger */
static const int MEM_XFER_CHUNK_SIZE = 64;

/*
 *
 * local functions follow
 *
 */


/*!
 *	\fn	static void invoke_target_state_change_callback(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
 *	\brief	invokes the target state change callbacks
 *
 *	this function will first invoke the callback at the top of the
 *	stack, if it returns true (nonzero), then the callback
 *	further down the stack is invoked, and so on until either a
 *	callback returns false (zero), or the stack is exhausted
 *	
 *	\param	ctx	context to work in
 *	\param	state	current target state to pass to the callback
 *	\return	none
 */
static void invoke_target_state_change_callback(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
{
struct core_connection_data * p;
int i;
bool res;

	p = ctx->core_comm;
	/* see if the callback stack is empty */
	i = p->target_state_change_callback_stack.stack_ptr;
	if (i == 0)
		panic("");
	do
	{
		res = p->target_state_change_callback_stack.callback_stack[i - 1](ctx, state);
	}
	while (res != false && --i != 0);

}

/*! \todo	document this */
static void init_lexer_state(struct gear_engine_context * ctx)
{
	ctx->core_comm->lex_state.lex_idx = 0;
	ctx->core_comm->lex_state.buf_idx = 0;
	ctx->core_comm->lex_state.lex_err_code = GEAR_ERR_NO_ERROR;
	target_set_extra(ctx, ctx->core_comm->lex_state.yyscanner);
}

static void sync_target_lexer(struct gear_engine_context * ctx)
{
	/* consume characters from the target lexer input until a newline
	 * is found - this will synchronize the target lexer to the next
	 * input line from the target core controller */
	while (target_lex(ctx->core_comm->lex_state.yyscanner) != LEX_TOKEN_NO_MORE_TOKENS)
		;
}


/*!
 *	\fn	static bool is_connected(struct gear_engine_context * ctx)
 *	\brief	determines if a connection to a target core controller is currently established
 *
 *	\param	ctx	context to work in
 *	\return	true if a connection to a target core controller is currently established, false otherwise
 */
static bool is_connected(struct gear_engine_context * ctx)
{
struct core_connection_data * p;

	p = ctx->core_comm;
	return p->is_core_controller_connected ? true : false;
}


/*!
 *	\fn	static enum GEAR_ENGINE_ERR_ENUM core_open(struct gear_engine_context * ctx, const char ** errmsg_hint)
 *	\brief	establishes a connection to a target core
 *
 *	\note	it is important to invoke the core_open()
 *		routine to connect to a target core prior
 *		to invoking any other target core related
 *		routines
 *
 *	\param	ctx	context to work in
 *	\param	errmsg_hint	in case of an error, it is possible that
 *				an informative error message is available;
 *				e.g. in the case a connection to a target
 *				core controller was successfully established,
 *				but there is an error communicating with
 *				the target core (e.g. the target core is
 *				not powered, there is no functional debug probe
 *				connected to the target core, etc.), the
 *				target core controller is allowed to return
 *				an error message hint; this is purely an
 *				informative message that is not in any way
 *				processed by the gear engine, and it is
 *				intended to be delivered verbatim to a gear
 *				engine frontend in order to be presented
 *				to a debugger user so that (s)he can
 *				perform corrective actions; in case
 *				an error message hint is available
 *				(e.g. is provided a target core controller),
 *				a ponter to it
 *				is stored in this variable, and it is the
 *				caller's responsibility to free() this
 *				error message string when no longer
 *				needed; in case this error message is
 *				not needed, this parameter can be null
 *	\return	for definitions of error codes, see the GEAR_ENGINE_ERR_ENUM
 *		documentation; this routine returns one of:
 *		- GEAR_ERR_NO_ERROR - if everything is fine and a 
 *		connection to a target core was succesfully established
 *		- GEAR_ERR_TARGET_CORE_CONNECTION_FAILED - if a connection
 *		to a target core controller could not be established
 *		- GEAR_ERR_GENERIC_ERROR - if a connection to a target
 *		core controller was established, but there is an error
 *		communicating with the target core (e.g. the target core is
 *		not powered, there is no functional debug probe
 *		connected to the target core, etc.); in this case, if
 *		the 'errmsg_hint' parameter is not null and the
 *		target core controller provided an error message hint,
 *		the 'errmsg_hint' variable will be set to the error
 *		message hint provided by the target core controller
 *		and it is the caller's responsibility to free() this
 *		error message hint when no longer needed
 */
static enum GEAR_ENGINE_ERR_ENUM core_open(struct gear_engine_context * ctx, const char ** errmsg_hint)
{
int fd;
struct core_connection_data * p;
enum GEAR_ENGINE_ERR_ENUM err;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	p = ctx->core_comm;
	if (!p)
		panic("");
	if (errmsg_hint)
		* errmsg_hint = 0;
	if (!p->is_core_controller_connected)
	{
		/* first, create the socket */
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd == -1)
			panic("");
		if (connect(fd, (const struct sockaddr *) &ctx->core_comm->core_inet_addr,
					sizeof ctx->core_comm->core_inet_addr) == -1)
		{
			invoke_target_state_change_callback(ctx, TARGET_CORE_STATE_DEAD);
			if (errmsg_hint)
				* errmsg_hint = strdup("unable to connect to a target core controller");
			return GEAR_ERR_TARGET_CORE_CONNECTION_FAILED;
		}
		p->write_core_fd = p->read_core_fd = fd;
#ifdef __LINUX__
		p->write_core_file = fdopen(fd, "w+");
		if (!p->write_core_file)
			panic("");
#else
#endif
		p->is_core_controller_connected = 1;
		gprintf("ok, established a connection to a target controller\n");
	}
#ifdef __LINUX__
	fprintf(p->write_core_file, "CORE_OPEN\n");
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "CORE_OPEN\n");
#endif

	init_lexer_state(ctx);
	/* parse data record header */
	/* read error status */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");

	/* expect a comma */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != ',')
	{
		gprintf("unexpected token: %c (%i)\n", token, token);
		panic("");
	}

	err = p->lex_state.yylval.status_code;

	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");

	if (err == GEAR_ERR_GENERIC_ERROR) switch (0)
	{
		default:
			/* check for an optional 'ERRMSG_HINT' error message hint */
			if (token != LEX_TOKEN_ERRMSG_HINT_KW)
				break;

			token = target_lex(p->lex_state.yyscanner);
			if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
				panic("");
			if (token != '=')
			{
				gprintf("unexpected token: %c (%i)\n", token, token);
				panic("");
			}

			token = target_lex(p->lex_state.yyscanner);
			if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
				panic("");
			if (token != LEX_TOKEN_CSTR)
			{
				gprintf("unexpected token: %c (%i)\n", token, token);
				panic("");
			}
			if (errmsg_hint)
			{
				* errmsg_hint = p->lex_state.yylval.cstr;
			}
			else
				free(p->lex_state.yylval.cstr);


			token = target_lex(p->lex_state.yyscanner);
			if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
				panic("");
			if (token != ',')
			{
				gprintf("unexpected token: %c (%i)\n", token, token);
				panic("");
			}

			token = target_lex(p->lex_state.yyscanner);
			if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
				panic("");
	}
	if (token != LEX_TOKEN_TARGET_STATE_INFO)
	{
		gprintf("unexpected token: %i\n", token);
		fflush(stdout);
		fflush(stderr);
		panic("");
	}
	/* make sure the input from the target controller
	 * is exhausted */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_NO_MORE_TOKENS)
		panic("");
	/* propagate any eventual target state change
	 * to interested parties */
	invoke_target_state_change_callback(ctx, p->lex_state.yylval.state);
	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return err;
}

/*! \todo	document this */
static enum GEAR_ENGINE_ERR_ENUM must_be_only_called_from_yylex_wait_incoming_data(struct gear_engine_context * ctx)
{
struct core_connection_data * p;
/* variables for use in the select() call below *
 * only the target core controller file descriptor is monitored */
fd_set read_fds, err_fds;
int nfds;
int cc_fd;
struct timeval timeout;
int res;

	/* sanity checks */
	if (ctx->core_comm->lex_state.buf_idx >= ctx->core_comm->lex_state.buf_size - 1)
		panic("");
	p = ctx->core_comm;
	cc_fd = p->read_core_fd;
	FD_ZERO(&read_fds);
	FD_ZERO(&err_fds);
	/*! \todo	clean up the hacked frontend_fds[0] usage here */
	FD_SET(cc_fd, &err_fds);
	FD_SET(cc_fd, &read_fds);
	nfds = cc_fd;

	nfds++;
	timeout.tv_sec = CORE_CONTROLLER_READ_TIMEOUT_MS / 1000;
	timeout.tv_usec = 0;
	res = select(nfds, &read_fds, 0, &err_fds, &timeout);
	if (res == -1)
		panic("");
	if (!FD_ISSET(cc_fd, &read_fds))
		panic("");
	if (FD_ISSET(cc_fd, &read_fds))
	{
		/* ok, there is incoming data from a target core controller */
#ifdef __LINUX__
		res = read(cc_fd, ctx->core_comm->lex_state.buf, 
				ctx->core_comm->lex_state.buf_size - 1 - ctx->core_comm->lex_state.buf_idx);
#else
		res = recv(cc_fd, ctx->core_comm->lex_state.buf, 
				ctx->core_comm->lex_state.buf_size - 1 - ctx->core_comm->lex_state.buf_idx, 0);
#endif
		if (res == -1)
			panic("");
		if (!res)
			/* a really strange thing... */
			panic("");
		p->lex_state.buf_idx += res;
	}

	return GEAR_ERR_NO_ERROR;
}



/*! \todo	document this */
/*! \todo	document the expected incoming data records format, as well
 *		as the generated data request records format */
static enum GEAR_ENGINE_ERR_ENUM core_mem_read(struct gear_engine_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes)
{
struct core_connection_data * p;
int bytes_remaining;
int bytes_this_run;
int i;
ARM_CORE_WORD addr;
enum LEX_TOKEN_TYPE_ENUM token;
enum GEAR_ENGINE_ERR_ENUM result;

	/* sanity checks */
	if (!ctx || !dest || !nbytes || !*nbytes || !ctx->core_comm)
		panic("");
	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	bytes_remaining = *nbytes;
	*nbytes = 0;
	addr = source;

	/* be optimistic and assume everything will succeed */
	result = GEAR_ERR_NO_ERROR;
	while (bytes_remaining)
	{
		/* data reading loop */
		/* issue a memory read request */
		bytes_this_run = (bytes_remaining > MEM_XFER_CHUNK_SIZE) ?
			MEM_XFER_CHUNK_SIZE : bytes_remaining;
#ifdef __LINUX__
		fprintf(p->write_core_file, "MEM_READ(0x%08x, 0x%08x)\n", addr, bytes_this_run);
		fflush(p->write_core_file);
#else
		sockprintf(p->write_core_fd, "MEM_READ(0x%08x, 0x%08x)\n", addr, bytes_this_run);
#endif
		init_lexer_state(ctx);
		/* parse data record header */
		/* read error status */
		token = target_lex(p->lex_state.yyscanner);
		if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
			panic("");
		if (token != LEX_TOKEN_STATUS_CODE)
			panic("");
		if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		{
			/* make sure the input from the target controller
			 * is exhausted */
			sync_target_lexer(ctx);
			result = p->lex_state.yylval.status_code;
			break;
		}
		/* after the error code, a comma should follow,
		 * followed by the payload - the data bytes read
		 * from target memory */	 
		token = target_lex(p->lex_state.yyscanner);
		if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
			panic("");
		if (token != ',')
		{
			gprintf("unexpected token: %c (%i)\n", token, token);
			panic("");
		}
		/* read data bytes */
		i = bytes_this_run;
		while (i--)
		{
			token = target_lex(p->lex_state.yyscanner);
			if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
				panic("");
			if (token != LEX_TOKEN_NUMBER)
				panic("");
			*(unsigned char *)dest =
				(unsigned char) p->lex_state.yylval.n;
			dest = (void *)((int) dest + 1);
		}
		/* make sure the input from the target controller
		 * is exhausted */
		token = target_lex(p->lex_state.yyscanner);
		if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
			panic("");
		if (token != LEX_TOKEN_NO_MORE_TOKENS)
			panic("");
		*nbytes += bytes_this_run;
		addr += bytes_this_run;
		bytes_remaining -= bytes_this_run;
	}

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return result;
}

/*! \todo	document this */
/*! \todo	document the expected incoming data records format, as well
 *		as the generated data request records format */
static enum GEAR_ENGINE_ERR_ENUM core_mem_write(struct gear_engine_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes)
{
struct core_connection_data * p;
int bytes_remaining;
int bytes_this_run;
int i;
ARM_CORE_WORD addr;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	if (!ctx || !source || !nbytes || !*nbytes || !ctx->core_comm)
		panic("");
	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	bytes_remaining = *nbytes;
	*nbytes = 0;
	addr = dest;

	while (bytes_remaining)
	{
		/* data writing loop */
		/* issue a memory read request */
		bytes_this_run = (bytes_remaining > MEM_XFER_CHUNK_SIZE) ?
			MEM_XFER_CHUNK_SIZE : bytes_remaining;
#ifdef __LINUX__
		fprintf(p->write_core_file, "MEM_WRITE(%i, %i, ", addr, bytes_this_run);

		/* write data bytes */
		i = bytes_this_run;
		while (i--)
		{
			fprintf(p->write_core_file, "0x%02x ", (int)*(unsigned char *)source);
			source = (void *)((int) source + 1);
		}
		fprintf(p->write_core_file, ")\n");
		fflush(p->write_core_file);
#else
		sockprintf(p->write_core_fd, "MEM_WRITE(%i, %i, ", addr, bytes_this_run);

		/* write data bytes */
		i = bytes_this_run;
		while (i--)
		{
			sockprintf(p->write_core_fd, "0x%02x ", (int)*(unsigned char *)source);
			source = (void *)((int) source + 1);
		}
		sockprintf(p->write_core_fd, ")\n");
#endif

		init_lexer_state(ctx);
		/* parse data record header */
		/* read error status */
		token = target_lex(p->lex_state.yyscanner);
		if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
			panic("");
		if (token != LEX_TOKEN_STATUS_CODE)
			panic("");
		if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
			panic("");
		/* make sure the input from the target controller
		 * is exhausted */
		token = target_lex(p->lex_state.yyscanner);
		if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
			panic("");
		if (token != LEX_TOKEN_NO_MORE_TOKENS)
			panic("");
		*nbytes += bytes_this_run;
		addr += bytes_this_run;
		bytes_remaining -= bytes_this_run;
	}

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return GEAR_ERR_NO_ERROR;
}


/*!
 * \todo	document this
 * \todo	define mode enumerators; the mode is ignored now
 */
static enum GEAR_ENGINE_ERR_ENUM core_reg_read(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
struct core_connection_data * p;
int i, j;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	if (!ctx || !ctx->core_comm || !buffer || !mask)
		panic("");

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	/* issue a register read request */
#ifdef __LINUX__
	fprintf(p->write_core_file, "REGS_READ(%i)\n", (int) mask);
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "REGS_READ(%i)\n", (int) mask);
#endif

	init_lexer_state(ctx);
	/* parse data record header */
	/* read error status */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
	{
		sync_target_lexer(ctx);
		return p->lex_state.yylval.status_code;
	}
	/* after the error code, a comma should follow,
	 * followed by the payload - the data bytes read
	 * from target memory */	 
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != ',')
		panic("");
	/* read register contents */
	/*! \todo	cleanup the register encoding scheme; provide
	 *		a sensible constant name definition for the
	 *		hard coded constant (19) below */ 
	for (i = j = 0; i < 19; i++)
		if (mask & (1 << i))
		{
			token = target_lex(p->lex_state.yyscanner);
			if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
				panic("");
			if (token != LEX_TOKEN_NUMBER)
				panic("");
			buffer[j++] = p->lex_state.yylval.n;
		}

	/* make sure the input from the target controller
	 * is exhausted */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_NO_MORE_TOKENS)
		panic("");

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return GEAR_ERR_NO_ERROR;
}


static enum GEAR_ENGINE_ERR_ENUM core_reg_write(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
struct core_connection_data * p;
int i;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	if (!ctx || !ctx->core_comm || !buffer || !mask)
		panic("");

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	/* issue a register read request */
#ifdef __LINUX__
	fprintf(p->write_core_file, "REGS_WRITE(%i,", (int) mask);
	for (i = 0; mask; mask >>= 1)
		if (mask & 1)
			fprintf(p->write_core_file, "0x%x ", buffer[i++]);
	fprintf(p->write_core_file, ")\n");
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "REGS_WRITE(%i,", (int) mask);
	for (i = 0; mask; mask >>= 1)
		if (mask & 1)
			sockprintf(p->write_core_fd, "0x%x ", buffer[i++]);
	sockprintf(p->write_core_fd, ")\n");
#endif

	init_lexer_state(ctx);
	/* only status code is expected */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		panic("");

	/* make sure the input from the target controller
	 * is exhausted */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_NO_MORE_TOKENS)
		panic("");

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return GEAR_ERR_NO_ERROR;

}

static enum GEAR_ENGINE_ERR_ENUM core_get_status(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM * status)
{
enum LEX_TOKEN_TYPE_ENUM token;
struct core_connection_data * p;

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	init_lexer_state(ctx);

	/* parse data record header */
	/* read error status */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		panic("");
	/* after the error code, a comma should follow,
	 * followed by the payload - the target state */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != ',')
		panic("");

	/* read the target state token itself */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_TARGET_STATE_INFO)
		panic("");
	/* make sure the input from the target controller
	 * is exhausted */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_NO_MORE_TOKENS)
		panic("");
	/* propagate any eventual target state change
	 * to interested parties */
	invoke_target_state_change_callback(ctx, p->lex_state.yylval.state);
	*status = p->lex_state.yylval.state;
	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM core_set_break(struct gear_engine_context * ctx, ARM_CORE_WORD address)
{
struct core_connection_data * p;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	if (!ctx || !ctx->core_comm)
		panic("");

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	/* issue a breakpoint set request */
#ifdef __LINUX__
	fprintf(p->write_core_file, "BKPT_SET(0x%08x)\n", (int) address);
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "BKPT_SET(0x%08x)\n", (int) address);
#endif

	init_lexer_state(ctx);
	/* only status code is expected */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		panic("");

	/* make sure the input from the target controller
	 * is exhausted */
	sync_target_lexer(ctx);

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM core_clear_break(struct gear_engine_context * ctx, ARM_CORE_WORD address)
{
struct core_connection_data * p;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	if (!ctx || !ctx->core_comm)
		panic("");

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	/* issue a breakpoint clear request */
#ifdef __LINUX__
	fprintf(p->write_core_file, "BKPT_CLEAR(0x%08x)\n", (int) address);
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "BKPT_CLEAR(0x%08x)\n", (int) address);
#endif

	init_lexer_state(ctx);
	/* only status code is expected */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		panic("");

	/* make sure the input from the target controller
	 * is exhausted */
	sync_target_lexer(ctx);

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM core_run(struct gear_engine_context * ctx/*ARM_CORE_WORD * halt_addr*/)
{
struct core_connection_data * p;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	if (!ctx || !ctx->core_comm)
		panic("");

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	/* issue a target core run request */
#ifdef __LINUX__
	fprintf(p->write_core_file, "CORE_RUN\n");
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "CORE_RUN\n");
#endif

	init_lexer_state(ctx);
	/* only status code is expected */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		panic("");

	/* make sure the input from the target controller
	 * is exhausted */
	sync_target_lexer(ctx);

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	/* update the target core status */
	invoke_target_state_change_callback(ctx, TARGET_CORE_STATE_RUNNING);
	return GEAR_ERR_NO_ERROR;
}


static enum GEAR_ENGINE_ERR_ENUM core_halt(struct gear_engine_context * ctx/*ARM_CORE_WORD * halt_addr*/)
{
struct core_connection_data * p;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	if (!ctx || !ctx->core_comm)
		panic("");

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	/* issue a target core halt request */
#ifdef __LINUX__
	fprintf(p->write_core_file, "CORE_HALT\n");
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "CORE_HALT\n");
#endif

	init_lexer_state(ctx);
	/* only status code is expected */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		panic("");

	/* make sure the input from the target controller
	 * is exhausted */
	sync_target_lexer(ctx);

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	/* update the target core status */
	invoke_target_state_change_callback(ctx, TARGET_CORE_STATE_HALTED);
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM core_insn_step(struct gear_engine_context * ctx)
{
struct core_connection_data * p;
enum LEX_TOKEN_TYPE_ENUM token;

	/* sanity checks */
	if (!ctx || !ctx->core_comm)
		panic("");

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	/* issue a target core run request */
#ifdef __LINUX__
	fprintf(p->write_core_file, "CORE_INSN_STEP\n");
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "CORE_INSN_STEP\n");
#endif

	init_lexer_state(ctx);
	/* only status code is expected */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		panic("");

	/* make sure the input from the target controller
	 * is exhausted */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_NO_MORE_TOKENS)
		panic("");

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	/* update the target core status */
	invoke_target_state_change_callback(ctx, TARGET_CORE_STATE_RUNNING);
	return GEAR_ERR_NO_ERROR;
}



/*! \todo	document this, define error codes */
static enum GEAR_ENGINE_ERR_ENUM core_register_target_state_change_callback(struct gear_engine_context * ctx, bool (*target_state_change_notification_callback)(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state))
{
struct core_connection_data * p;

	if (!ctx)
		panic("");
	p = ctx->core_comm;
	/* see if the stack is full */
	if (p->target_state_change_callback_stack.stack_ptr == CALLBACK_STACK_SIZE)
		panic("");
	p->target_state_change_callback_stack.callback_stack[p->target_state_change_callback_stack.stack_ptr++] = target_state_change_notification_callback;
	return GEAR_ERR_NO_ERROR;
}


/*! \todo	document this, define error codes */
static enum GEAR_ENGINE_ERR_ENUM core_unregister_target_state_change_callback(struct gear_engine_context * ctx, bool (*target_state_change_notification_callback)(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state))
{
struct core_connection_data * p;

	if (!ctx)
		panic("");
	p = ctx->core_comm;
	/* see if the stack is empty */
	if (p->target_state_change_callback_stack.stack_ptr == 0)
		panic("");
	/* make sure the top of stack matches the routine removed */
	if (p->target_state_change_callback_stack.callback_stack[p->target_state_change_callback_stack.stack_ptr - 1] != target_state_change_notification_callback)
		panic("");
	p->target_state_change_callback_stack.callback_stack[--(p->target_state_change_callback_stack.stack_ptr)] = 0;
	return GEAR_ERR_NO_ERROR;
}

/*! input-output target control, semantics of this are target dependent
 *
 * generally, this is a catch-all for everything that doesnt cleanly
 * fit the target control interface; one can think of this as a placeholder
 * for hiding hacks, a bag of tricks, an interface for performing anything
 * that is not yet polished enough to be put in the gear engine proper,
 * or simply a (temporary) hack that is to be put somewhere instead of
 * poluting the gear engine proper
 *
 * details about supported input/output control request for specific
 * targets, target architectures and variants, and target access drivers
 * must be put in the target specific "target-defs.h" header file residing
 * in directory "./include/targets/xxx/", where "xxx" is the architecture
 * name (e.g., "arm")
 *
 * please, abstain from using this routine
 *
 * \todo	document this */
static enum GEAR_ENGINE_ERR_ENUM io_ctl(struct gear_engine_context * ctx, int request_len, ARM_CORE_WORD * request, ARM_CORE_WORD response_len, ARM_CORE_WORD * response)
{
struct core_connection_data * p;
enum LEX_TOKEN_TYPE_ENUM token;
ARM_CORE_WORD i;

	/* sanity checks */
	if (!ctx || !ctx->core_comm)
		panic("");

	p = ctx->core_comm;
	if (!p->is_core_controller_connected)
		return GEAR_ERR_TARGET_CORE_DEAD;

	/* issue a breakpoint clear request */
#ifdef __LINUX__
	fprintf(p->write_core_file, "IOCTL(0x%08x,", request_len);
	while (request_len--)
		fprintf(p->write_core_file, "0x%08x,", *request++);
	fprintf(p->write_core_file, ")\n");
	fflush(p->write_core_file);
#else
	sockprintf(p->write_core_fd, "IOCTL(0x%08x,", request_len);
	while (request_len--)
		sockprintf(p->write_core_fd, "0x%08x,", *request++);
	sockprintf(p->write_core_fd, ")\n");
#endif

	init_lexer_state(ctx);
	/* parse data record header */
	/* read error status */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_STATUS_CODE)
		panic("");
	if (p->lex_state.yylval.status_code != GEAR_ERR_NO_ERROR)
		panic("");
	/* after the error code, a comma should follow,
	 * followed by the payload - the number of target core
	 * words in the response, followed by the data words themselves */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != ',')
		panic("");
	/* read response length */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_NUMBER)
		panic("");
	if ((i = p->lex_state.yylval.n) != response_len)
		panic("");
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != ',')
		panic("");

	/* pull the data in */
	while (i--)
	{
		token = target_lex(p->lex_state.yyscanner);
		if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
			panic("");
		if (token != LEX_TOKEN_NUMBER)
			panic("");
		*response++ = p->lex_state.yylval.n;
	}

	/* make sure the input from the target controller
	 * is exhausted */
	token = target_lex(p->lex_state.yyscanner);
	if (p->lex_state.lex_err_code != GEAR_ERR_NO_ERROR)
		panic("");
	if (token != LEX_TOKEN_NO_MORE_TOKENS)
		panic("");

	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);
	return GEAR_ERR_NO_ERROR;
}

/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	void init_target_comm(struct gear_engine_context * ctx)
 *	\brief	initialize target core controller connection parameters
 *
 *	\param	ctx	gear engine context to work in
 *	\return	none
 */
void init_target_comm(struct gear_engine_context * ctx)
{
struct core_connection_data * p;

	if (!(p = calloc(1, sizeof * p)))
		panic("");
	/*! \todo	make the internet address here configurable
	 *		(ip address and port number) */
	p->core_inet_addr.sin_family = AF_INET;
	p->core_inet_addr.sin_port = ctx->settings.target_ctl_port_nr;
	p->core_inet_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/* allocate buffer space */
	if (!(p->lex_state.buf = malloc(CORE_CONTROLLER_BUFFER_SIZE)))
		panic("");
	p->lex_state.buf_size = CORE_CONTROLLER_BUFFER_SIZE;
	/* create the lexer state variables */
	/*! \todo	provide a mechanism for invoking a
	 *		corresponding yylex_destroy() */
       	if (target_lex_init(&p->lex_state.yyscanner))
		panic("");
	/* zero this out to help catch errors */
	target_set_extra(0, p->lex_state.yyscanner);

	ctx->core_comm = p;
}
/*! \todo	document this, define error codes */
void target_comm_issue_core_status_request(struct gear_engine_context * ctx)
{
#ifdef __LINUX__
	fprintf(ctx->core_comm->write_core_file, "GET_STATE\n");
	fflush(ctx->core_comm->write_core_file);
#else
	sockprintf(ctx->core_comm->write_core_fd, "GET_STATE\n");
#endif
}

/*! \todo	clean these up */
static struct core_control target_cc =
{
	is_connected,
	core_open,
	0,
	core_mem_read,
	core_mem_write,
	core_reg_read,
	core_reg_write,
	0,
	0,
	core_set_break,
	core_clear_break,
	core_run,
	core_halt,
	core_insn_step,
	io_ctl,
	core_get_status,
	core_register_target_state_change_callback,
	core_unregister_target_state_change_callback,
};

int target_comm_get_core_access(struct core_control * cc)
{
	*cc = target_cc;
	return 0;
}

