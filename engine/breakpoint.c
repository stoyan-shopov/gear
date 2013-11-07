/*!
 *	\file	breakpoint.c
 *	\brief	breakpoint facilities for the arm gear
 *	\author	shopov
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

#include <stdlib.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "core-access.h"
#include "util.h"
#include "breakpoint.h"

/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	struct bkpt_struct * bkpt_setup_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
 *	\brief	installs a breakpoint in the breakpoint list at the address supplied
 *
 * 	\note	in order for this function to succeed, a breakpoint
 * 		at the address supplied should not already exist 
 *
 *	\param	ctx	context to work in
 *	\param	addr	target core address to set the breakpoint at
 *	\return	a pointer to the newly set up breakpoint node, 0 if
 *		there already is a breakpoint at the address supplied,
 *		or another error (such as memory exhaustion) occurs */
struct bkpt_struct * bkpt_setup_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
{
struct bkpt_struct * bp, * bnew, ** bprev;

	/* first, see if there already is a breakpoint at the address
	 * supplied */
	for (bprev = &ctx->bkpts, bp = ctx->bkpts; bp; bprev = &bp->next, bp = bp->next)
		if (bp->addr == addr)
			return 0;
		else if (bp->addr > addr)
			/* insertion point located */
			break;

	bnew = calloc(1, sizeof * bnew);
	bnew->addr = addr;
	bnew->next = *bprev;
	*bprev = bnew;

	return bnew;
}


/*!
 *	\fn	enum GEAR_ENGINE_ERR_ENUM bkpt_clear_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
 *	\brief	removes a breakpoint for the breakpoint supplied from the breakpoint list
 *
 * 	\note	in order for this function to succeed, a breakpoint
 * 		at the address supplied must already exist 
 *
 *	\param	ctx	context to work in
 *	\param	addr	target core address to remove the breakpoint at
 *	\return	GEAR_ERR_NO_ERROR, if the operation was successful,
 *		GEAR_ERR_GENERIC_ERROR if no breakpoint was found at the
 *		address supplied, and therefore the operation failed */
enum GEAR_ENGINE_ERR_ENUM bkpt_clear_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
{
struct bkpt_struct * bp, ** bprev;

	/* see if there is a breakpoint at the address supplied */
	for (bprev = &ctx->bkpts, bp = ctx->bkpts; bp; bprev = &bp->next, bp = bp->next)
		if (bp->addr == addr)
			break;
	if (!bp)
		panic("");
	*bprev = bp->next;
	free(bp);
	return GEAR_ERR_NO_ERROR;
}

/*!
 *	\fn	struct bkpt_struct * bkpt_locate_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
 *	\brief	locates a breakpoint at the address supplied
 *
 *	\param	ctx	context to work in
 *	\param	addr	target core address to locate a breakpoint node for
 *	\return	a pointer to the breakpoint node, if one is found, 0 otherwise */
struct bkpt_struct * bkpt_locate_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
{
struct bkpt_struct * bp;

	for (bp = ctx->bkpts; bp; bp = bp->next)
		if (bp->addr == addr)
			break;
	return bp;
}

/*!
 *	\fn	void init_bkpt(struct gear_engine_context * ctx)
 *	\brief	breakpoint facility module initialization
 *
 *	\param	ctx	context to work in
 *	\return	none */
void init_bkpt(struct gear_engine_context * ctx)
{
	ctx->bkpts = 0;
}

