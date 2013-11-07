/*!
 *	\file	engine-err.h
 *	\brief	gear engine error definitions
 *	\author	shopov
 *	
 *	the error codes that the gear engine can generate are
 *	enumerated here; they are primarily of use to frontend code
 *	and code that parses gear engine output in general;
 *	the error codes here are intended to be useful when interpreting
 *	errors in gear engine output parsing code
 *
 *	Revision summary:
 *
 *	$Log: $
 */
#ifndef __GEAR_ENGINE_ERR_H__
#define __GEAR_ENGINE_ERR_H__

/*
 *
 * exported declarations follow
 *
 */

/*! gear engine error enumeration */
enum GEAR_ENGINE_ERR_ENUM
{
	/*! no error */
	GEAR_ERR_NO_ERROR = 0,
	/*! generic error - indicates just that an error occurred, but no details are supplied */
	GEAR_ERR_GENERIC_ERROR,

	/*
	 *
         * dbx command parser relater errors	 
	 *
	 */
	/*! the requested dbx command is not yet coded */
	GEAR_ERR_DBX_CMD_NOT_CODED,
	/*! command not recognized */
	GEAR_ERR_DBX_CMD_NOT_RECOGNIZED,
	/*! dbx command string received is empty - not really an error */
	GEAR_ERR_DBX_CMD_STRING_EMPTY,

	/*
	 *
         * data object/expression related error codes
	 *
	 */
	/*! a syntax error occurred when parsing an expression */
	GEAR_ERR_EXPR_SYNTAX_ERROR,
	/*! a generic semantic error occurred when evaluating an expression */
	GEAR_ERR_EXPR_SEMANTIC_ERROR,

	/*
	 *
         * target core controller returned error codes
	 *
	 */
	/*! a connection to a target core controller could not be established */
	GEAR_ERR_TARGET_CORE_CONNECTION_FAILED,
	/*! the target core has died */
	GEAR_ERR_TARGET_CORE_DEAD,
	/*! invalid parameters passed to a target core controller */
	GEAR_ERR_TARGET_CTL_BAD_PARAMS,
	/*! error accessing target memory */
	GEAR_ERR_TARGET_CTL_MEM_READ_ERROR,
	/*! a generic error accessing the target */
	GEAR_ERR_TARGET_ACCESS_ERROR,
	/*! breakpoint already set on the requested address */
	GEAR_ERR_BKPT_ALREADY_SET_AT_ADDR,
	/*! cannot set hardware breakpoint */
	GEAR_ERR_CANT_SET_HW_BKPT,
	/*! target resource(s) unavailable/inaccessible while the target is running */
	GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING,
	/*
	 *
	 * target call stack frame unwind related error codes
	 *
	 */
	/*! a target call stack frame could not be unwound
	 *
	 * this is returned when there is no unwind information
	 * available for a target call stack frame, or some error
	 * accessing the target occurred when attempting to unwind
	 *
	 * \todo	maybe define more specific error indications
	 * 		for stack unwinding */
	GEAR_ERR_CANT_UNWIND_STACK_FRAME,
	/*! a target call stack frame could not be rewound
	 *
	 * this is returned in case a stack crawl towards
	 * the most recent frame is requested, but the
	 * number of frames to rewind is too big, resulting
	 * in an attempt to move down the target call stack,
	 * past the most recent frame (which is not possible) */
	GEAR_ERR_CANT_REWIND_STACK_FRAME,
	/*! rewinding or unwinding of the stack was requested, but backtrace data is unavailable
	 *
	 * such an error can occur e.g. when the target is unavailable or running */
	GEAR_ERR_BACKTRACE_DATA_UNAVAILABLE,
	/*! number of error code constants */
	GEAR_ERR_NR_ERR_CODES,

};

#endif /* __GEAR_ENGINE_ERR_H__ */

