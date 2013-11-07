/*!
 * \file	target-dump-cstring.c
 * \brief	code for dumping c-like formatted (escaped) strings from target memory
 * \author	shopov
 *
 *	basically, this module provides a simple version for the
 *	'gear_engine_context.dump_cstring_from_target_mem()' routine;
 *	for details, read comments about this routine in file
 *	'gear-engine-context.h'
 *
 * Revision summary:
 *
 * $Log: $
 */

/*
 *
 * include section follows
 *
 */
#include <ctype.h>
#include <stdarg.h>

#include "dwarf-common.h"
#include "gear-limits.h"
#include "gear-engine-context.h"
#include "core-access.h"
#include "engine-err.h"
#include "miprintf.h"

/*
 *
 * local functions follow
 *
 */


/*!
 *	\fn	void dump_cstring_from_target_mem(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
 *	\brief	dumps c-like formatted (escaped) strings from target memory
 *
 *	basically, this routine provides a simple version for the
 *	'gear_engine_context.dump_cstring_from_target_mem()' routine;
 *	for details, read comments about this routine in file
 *	'gear-engine-context.h'
 *
 *	\param	ctx	context to work in
 *	\param	addr	starting address from which to start dumping a string
 *	\return	none */
void dump_cstring_from_target_mem(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
{
unsigned char tbuf[64];
unsigned char c;
/* total number of non-printable characters encountered so far;
 * if this gets above a certain limit, this function assumes
 * that what it is trying to dump is probably not some meaningful
 * string, after all, and gives up */
int nr_nonprint;
unsigned int nbytes;
int i, t;

	nr_nonprint = 0;
	t = 0;

	while (t < MAX_DUMPED_CSTRLEN_FROM_TARGET_MEM)
	{
		/* fill dump buffer from target memory */
		nbytes = sizeof tbuf;
		if (ctx->cc->core_mem_read(ctx, tbuf, addr, &nbytes)
				!= GEAR_ERR_NO_ERROR
				|| nbytes != sizeof tbuf)
		{
			miprintf("! bad memory !", addr);
			break;
		}
		addr += sizeof tbuf;
		for (i = 0; i < sizeof tbuf; i ++)
		{
			c = tbuf[i];
			if (c == 0)
				/* this is the end of the string */
				return;
			if (!isprint(c))
			{
				if (nr_nonprint ++ > 16)
					/* too many non-printable
					 * characters encountered -
					 * assume this is not a human
					 * readable string and give up */
					return;
				/*! \todo	the escaping below is a bit
				 *		strange, that is because of
				 *		the machine interface
				 *		format peculiarities... */
				miprintf("\\x%02x", c);
			}
			else
			{
				/* an ordinary character - see if escaping
				 * is necessary */
				switch (c)
				{
					case '"': miprintf("\\\""); break;
					case '\\': miprintf("\\\\"); break;
					case '\a': miprintf("\\a"); break;
					case '\b': miprintf("\\b"); break;
					case '\f': miprintf("\\f"); break;
					case '\n': miprintf("\\n"); break;
					case '\r': miprintf("\\r"); break;
					case '\t': miprintf("\\t"); break;
					case '\v': miprintf("\\v"); break;
					default: miprintf("%c", c); break;
				}
			}

		}
	}
}

/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	void init_target_dump_cstring(struct gear_engine_context * ctx)
 *	\brief	hooks the 'gear_engine_context.dump_cstring_from_target_mem' function pointer to the routine supplied above in this module
 *
 *	\param	ctx	context to work in
 *	\return	none */
void init_target_dump_cstring(struct gear_engine_context * ctx)
{
	ctx->dump_cstring_from_target_mem = dump_cstring_from_target_mem;
}

