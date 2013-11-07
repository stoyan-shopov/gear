/*!
 *	\file	dwarf-pubnames.c
 *	\brief	provides support for handling the dwarf global symbols section, .debug_pubnames
 *	\author	shopov
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

#include <stdio.h>

#include "dwarf-common.h"
#include "gear-engine-context.h"

#include "dwarf-pubnames.h"
#include "util.h"

/*
 *
 * local types follow
 *
 */

/*! a data structure used for describing a global symbol (one residing in .debug_pubtypes) 
 *
 *	\todo	some of the members here may be redundand; revise this
 *		and fix this comment when this has been in active use
 *		for some time */
struct dwarf_glob_sym_struct
{
	/*! name of the symbol */
	char		* name;
	/*! offset in .debug_info of the die for the symbol */
	Dwarf_Off	die_offset;
	/*! offset in .debug_info of the compilation unit containing the symbol */
	Dwarf_Off	cu_offset;
};

/*
 *
 * local data follows
 *
 */

/*! a pointer to an array of Dwarf_Global data structures describing the debuggees global symbols */
static Dwarf_Global * dwarf_globsyms;
/*! number of elements in the dwarf_globsyms array above */
static Dwarf_Signed globsyms_cnt;

/*! a pointer to an array of dwarf_glob_sym_struct structures
 *
 * this is used to cache the data of interest for the global symbols;
 * the entries in this array correspond to the entries in the
 * dwarf_globsyms array; the number of elements in this array
 * is, likewise, given by the value of ::globsyms_cnt */ 
static struct dwarf_glob_sym_struct * globsyms;

/*
 *
 * local functions follow
 *
 */ 

/*
 *  
 * exported functions follow
 *
 */


/*!
 *	\fn	int dwarf_pubnames_scan(char * name, Dwarf_Off * die_offset, Dwarf_Off * cu_offset)
 *	\brief	scans the global symbol list for a given symbol
 *
 *	\todo	it may be more useful to actually return here a
 *		pointer to the debug information tree node for
 *		the global symbol
 *	\todo	define error codes here
 *
 *	\param	name	the name of the symbol to be searched for
 *	\param	die_offset	a pointer to a Dwarf_Off where to
 *			store the die offset of the symbol (if found);
 *			can be null
 *
 *	\param	cu_offset	a pointer to a Dwarf_Off where to
 *			store the compilation unint offset for the symbol
 *			(if found); can be null
 *	\return	zero, if the symbol is found and the offset fields
 *		were set properly; non-zero, if some error occurred
 */
int dwarf_pubnames_scan(char * name, Dwarf_Off * die_offset, Dwarf_Off * cu_offset)
{
int i;	
	if (!name)
		panic("");
	for (i = 0; i < globsyms_cnt; i++)
		if (!strcmp(globsyms[i].name, name))
		{
			if (die_offset)
				*die_offset = globsyms[i].die_offset;
			if (cu_offset)
				*cu_offset = globsyms[i].cu_offset;
			return 0;
		}
	return 1;
}

/*!
 *	\fn	void init_dwarf_pubnames(struct gear_engine_context * ctx)
 *	\brief	loads the global symbols for a program (reads section .debug_pubnames)
 *
 *	\param	ctx	context used to access the debuggee debug information
 *	\return	none
 */
void init_dwarf_pubnames(struct gear_engine_context * ctx)
{
Dwarf_Error err;
int res, i;

	gprintf("loading global symbols...\n");

	globsyms = 0;
	res = dwarf_get_globals(ctx->dbg, &dwarf_globsyms, &globsyms_cnt, &err);
	if (res == DW_DLV_ERROR)
		panic("dwarf_get_globals()"); 
	else if (res == DW_DLV_NO_ENTRY)
		/* .debug_pubnames section non-present */
		return;
	if (!(globsyms = (struct dwarf_glob_sym_struct *)
				malloc(globsyms_cnt * sizeof(*globsyms))))
		panic("out of core");
	for (i = 0; i < globsyms_cnt; i++)
	{
		res = dwarf_global_name_offsets(
				dwarf_globsyms[i],
				&globsyms[i].name,
				&globsyms[i].die_offset,
				&globsyms[i].cu_offset,
				&err);
		if (res == DW_DLV_ERROR)
			panic("dwarf_global_name_offsets()");
	}
}

