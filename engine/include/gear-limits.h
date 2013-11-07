/*!
 *	\file	gear-limits.h
 *	\brief	various gear engine limit constant definitions
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * exported constants follow
 *
 */

/*! an enumeration to hold various limit constants */
enum
{
	/*! maximum number of frames to unwind from the target call stack frame
	 *
	 * any request to unwind the target call stack past
	 * this many frames shall fail */
	MAX_NR_FRAMES_TO_UNWIND		= 0x800,

	/*! maximum number of characters to be dumped by the gear_engine_context.dump_cstring_from_target_mem() routine
	 *
	 * you may also want to see the comments about the
	 * 'dump_cstring_from_target_mem()' field in
	 * 'struct gear_engine_context' in file
	 * 'gear-engine-context.h' for the purpose of
	 * this constant */
	MAX_DUMPED_CSTRLEN_FROM_TARGET_MEM = 512,
};

