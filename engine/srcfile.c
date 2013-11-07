/*!
 * \file	srcfile.c
 * \brief	utility functions for manipulation of source-code related information
 * \author	shopov
 *
 *	the main data structure used herein is the src_data data structure;
 *	it basically consists of two lists - one for the compilation units
 *	comprising an executable file being debugged, and one for the
 *	source files used to compile this executable; each node in the
 *	compilation unit list contains itself a list of pointers to
 *	nodes in the source file list, used to compile the compilation
 *	unit; analogously - each node in the source file list contains
 *	itself a list of pointers to nodes in the compilation unit list
 *	for which the source file has participated in building the object
 *	file corresponding to the compilation unit
 *
 *	these lists are useful for things like listing the source files
 *	of an executable, retrieving line number information for compilation
 *	units, determining which source files a compilation unit contains,
 *	determining target core addresses for breakpoint placement,
 *	disassembling source code files, etc.;
 *	the lists are built by a quick scan over all compilation units
 *	comprising an executable
 *
 *	\note	the most common case for a compilation unit is that its
 *		object file is produced from only one source code file, and
 *		vice versa - a source code file is used only once to build a single
 *		compilation unit only; in this most common case a source
 * 		file name and a compilation unit can be used interchangeably,
 *		as there is a one-to-one correspondence between them; but
 *		it is possible to have other cases whose handling needs
 *		more care; these special cases are:
 *			- a compilation unit is produced by more than
 *			one source code file; an example of this could be
 *			a piece of machine generated source code (such
 *			as a flex scanner or a bison parser), or c code
 *			that "#include"s other source code files
 *			- a single source code file is used once or 
 *			several times to build several compilation units
 *			- a blend of these
 *
 *	\todo	currently, the gear handles none of the forementioned
 *		special cases at all; when support is added, do thoroughly
 *		document the gear handling behavior of these cases,
 *		as there are some points of speculation and different
 *		possible interpretations
 *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "core-access.h"
#include "cu-access.h"
#include "subprogram-access.h"
#include "srcfile.h"
#include "util.h"
#include "arm-dis.h"
#include "gprintf.h"
#include "miprintf.h"
#include "aranges-access.h"
#include "dwarf-ranges.h"
#include "engine-err.h"


/*
 *
 * local definitions follow
 *
 */

/*! constants used to determine how many lines of context to output upon disassembly requests 
 *
 * \todo	maybe obsolete this and move it to the frontend */
enum LINE_CONTEXT_QUANTITY
{
	/*!
	 *
	 * total lines of disassembly to print when no line number information match has
	 * been found for a given address; this is arbitrary */
	ASSEMBLY_ONLY_OUTPUT = 15,
};

/*
 *
 * local data types follow
 *
 */

/*! the structure holding the list heads for the compilation units and source file names
 *
 * for details on the lists, see the comments in the start of this file */
struct src_data
{
	/*! compilation unit list head */
	struct srclist_cu_node		* cus;
	/*! source file names list head */
	struct srclist_srcfile_node	* srcs;
};

/*! compilation unit list node */
struct srclist_cu_node
{
	/*! next pointer link in a linked list */
	struct srclist_cu_node	* next;
	/*! the offset of the compilation unit die in the debug information section .debug_info
	 *
	 * used to obtain data about the compilation unit */
	Dwarf_Off	cu_die_offset;
	/*! number of source files used to construct this compilation unit
	 *
	 * alternatively, the number of nodes in the source file list
	 * for this compilation unit (the src_list_head below) */ 
	int		src_cnt;
	/*! the list of source files for this compilation unit */
	struct cunode	* src_list_head;
	/*! compilation unit index number - an index value of zero is invalid, index numbers start from 1
	 *
	 * this is used for dumping compilation unit indices when
	 * dumping source code line number information
	 * via the machine interface */
	int	idx;

};

/*! source file list node */
struct srclist_srcfile_node
{
	/*! next pointer link in a linked list */
	struct srclist_srcfile_node	* next;
	/*! the name of the source code file described by this node */
	char		* name;
	/*! number of compilation units that this file contributes to
	 *
	 * also, the number of nodes in the compilation unit list
	 * for this source code file (the cu_list_head below) */ 
	int		cu_cnt;
	/*! the list of compilation units for this source code file */
	struct srcnode	* cu_list_head;

};

/*! a node in the list of source code files for a compilation unit */
struct cunode
{
	/*! next pointer link in a linked list */
	struct cunode	* next;
	/*! a pointer to the source code file associated with this node */
	struct srclist_srcfile_node * src;
};

/*! a node in the list of compilation units for a source code file */
struct srcnode
{
	/*! next pointer link in a linked list */
	struct srcnode	* next;
	/*! a pointer to the compilation unit associated with this node */
	struct srclist_cu_node * cu;
};


/*! \deprecated	this is deprecated... brings memories, though...
 * 
 * source file access helper data structure
 *
 * this structure provides quick access to a source file
 * by line number; this may eventually become deprecated and
 * be moved to the frontend
 *
 * \todo	maybe deprecate and remove this */
struct srcfile_data
{
	/*! the name of the source file */
	char	* name;
	/*! buffer holding the source file text
	 *
	 * this holds a verbatim copy of the contents of the source file
	 * with the exception that newlines are replaced with null byte terminators;
	 * this is because the line numbers buffer ::lines below points to locations within this
	 * buffer, which are null terminated strings corresponding to the line numbers */
	char	* textbuf;
	/*! size of textbuf, in bytes */
	int	textbuf_size;
	/*! the line numbers buffer
	 *
	 * there is an entry here for each line in the source file, and it is a pointer
	 * to some location within the ::textbuf above, which is a null terminated string
	 * corresponding to the source code text for the appropriate line number; lines here
	 * start from zero
	 *
	 * \todo	maybe start lines from index 1 */
	char	**lines;
	/*! the number of elements in the ::lines buffer; also, the number of source lines in the described source file */
	int	nr_lines;
};

/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn	static struct srclist_srcfile_node * locate_srcname(struct gear_engine_context * ctx, const char * srcname)
 *	\brief	retrieves the compilation unit data for a given source file name
 *
 *	\param	ctx	gear context to work into
 *	\param	srcname	the source file name to retrieve compilation
 *			unit data for
 *	\return	a pointer to the compilation unit data for the file,
 *		if found, null if the source file name is not found
 */ 
static struct srclist_srcfile_node * locate_srcname(struct gear_engine_context * ctx, const char * srcname)
{
struct srclist_srcfile_node * p;

	p = ctx->src_data->srcs;
	while (p)
	{
		if (!strcmp(p->name, srcname))
			break;
		p = p->next;
	}
	return p;
}

/*!
 *	\fn	static void print_escaped(const char * str)
 *	\brief	machine-interface-prints a string, escaping any double quotation marks it contains
 *
 *	\note	this is primarily useful in this module for literal printing of source
 *		code lines
 *
 *	\todo	this is buggy - must properly escape double
 *		quotation marks here - how exactly, though, is
 *		not yet clearly agreed on
 *	\todo	currently, there is a limitation to the lengths of the strings
 *		that can be printed (see the hardcoded 's' character
 *		array length below) - if this ever becomes an issue -
 *		resolve it
 *
 *	\param	str	the string to print
 *	\return	none
 */
static void print_escaped(const char * str)
{
int i;
char s[512];

	i = 0;
	while ((s[i] = *str++) && i < sizeof(s) - 1)
		if (s[i++] == '"')
			s[i++] = '"';
	s[i] = 0;
	miprintf("%s", s);
}


/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	void srcfile_build_src_cu_tab(struct gear_engine_context * ctx)
 *	\brief	builds the compilation unit - source file correspondence lists for an executable file
 *
 *	this function is used to build the compilation unit - source file
 *	correspondence lists, which are used for operating on the source files
 *	contributing to an executable file; for details on the lists,
 *	see the comments in the start of this file
 *
 *	\todo	should the representation of the lists change,
 *		fix the comments here
 *
 *	\todo	memory management/allocation here is *very*
 *		inefficient, maybe we could do better with an
 *		entirely new source table/lists organization
 *
 *	\param	ctx	gear context to operate into
 *	\return	none
 */

void srcfile_build_src_cu_tab(struct gear_engine_context * ctx)
{
int res;
Dwarf_Unsigned	cur_cu_offset;
Dwarf_Unsigned	next_cu_header_offset;
Dwarf_Half	version_stamp;
Dwarf_Die	cu_die;
Dwarf_Error	err;
char ** srcfiles;
Dwarf_Signed src_cnt;
struct srclist_srcfile_node * src;
struct srclist_cu_node * cu;
int i;

	/* initialize the table */
	if (!(ctx->src_data = calloc(1, sizeof * ctx->src_data)))
		panic("out of core");

	/* move to the first compilation unit in the debugging
	 * information section (.debug_info) of the executable
	 * and process all compilation units */	 
	/* this rewinding here is absolutely braindamaged; it
	 * is probably my fault/misreading of the libdwarf
	 * documentation, but it seems there is not a simple
	 * way to move to the first compilation unit in an executable
	 * and then process all compilation units in turn via calls to
	 * dwarf_next_cu_header; such rewinding is evidently necessary
	 * as comments in the libdwarf sources (namely in file
	 * dwarf_die_deliv.c), before function dwarf_next_cu_header
	 * state:
/ *
    Returns offset of next compilation-unit thru next_cu_offset
	pointer.
    It basically sequentially moves from one
    cu to the next.  The current cu is recorded
    internally by libdwarf.
* /
	* with the key word here being "internally";
	* do this (braindamaged) rewinding in a "documented"
	* manner - invoke dwarf_next_cu_header() until the
	* beast fails with a DW_DLV_NO_ENTRY return code, which means
	* we have arrived at the last compilation unit - and
	* thus the next invocation of dwarf_next_cu_header()
	* shall retrieve the offset of the first compilation
	* unit; this is what we need and we then fall into
	* the real processing loop */	 

	while ((res = dwarf_next_cu_header(ctx->dbg,
					/* dont need these */
					0,
					0,
					0,
					0,
					&next_cu_header_offset,
					&err)) == DW_DLV_OK)
		/* kill time; as weve bloody got any of it to waste... */	 
		;
	if (res != DW_DLV_NO_ENTRY)
		panic("");
	/* start from offset 0 - the start of the debug information
	 * (.dbg_info) section */	 
	cur_cu_offset = 0;
	/* at last, process all compilation units */
	while ((res = dwarf_next_cu_header(ctx->dbg,
					/* dont need this */
					0,
					&version_stamp,
					/* dont need these */
					0,
					0,
					&next_cu_header_offset,
					&err)) == DW_DLV_OK)
	{
		/*! \todo	enable this; resolve how exactly to
		 *		determine the dwarf version employed
		 *		by the one true compiler (gcc) */	 
	       /*
		if (version_stamp != 3)
			panic("");
		*/	 
		if (dwarf_get_cu_die_offset_given_cu_header_offset
			(ctx->dbg, cur_cu_offset, &cur_cu_offset, &err)
				!= DW_DLV_OK)
			panic("");
		/* read cu die */
		if (dwarf_offdie(ctx->dbg, cur_cu_offset,
					&cu_die, &err) != DW_DLV_OK)
			panic("");
		/* retrieve source files for this cu */
		if (dwarf_srcfiles(cu_die, &srcfiles, &src_cnt, &err) != DW_DLV_OK)
			panic("");
		dwarf_dealloc(ctx->dbg, cu_die, DW_DLA_DIE);
		/* put the new compilation unit in the list */
		if (!(cu = calloc(1, sizeof * cu)))
			panic("");
		cu->next = ctx->src_data->cus;
		cu->cu_die_offset = cur_cu_offset;
		ctx->src_data->cus = cu;
		cu->src_cnt = src_cnt;
		/* make references to the source file(s) for this
		 * compilation unit (eventually inserting them
	         * in the list of source files for the executable) */	 
		for (i = 0; i < src_cnt; i++)
		{
			struct srcnode * snode;
			struct cunode * cnode;

			src = locate_srcname(ctx, srcfiles[i]);
			if (!src)
			{
				/* source file not yet in the list - put it there */
				if (!(src = calloc(1, sizeof * src)))
					panic("");
				src->next = ctx->src_data->srcs;
				ctx->src_data->srcs = src;
				if (!(src->name = strdup(srcfiles[i])))
					panic("");
			}
			src->cu_cnt++;
			/* get core for list nodes */
			if (!(snode = malloc(sizeof * snode)))
				panic("");
			if (!(cnode = malloc(sizeof * cnode)))
				panic("");
			/* update compilation unit list */
			cnode->next = cu->src_list_head;
			cu->src_list_head = cnode;
			cnode->src = src;
			/* update source file list */
			snode->next = src->cu_list_head;
			src->cu_list_head = snode;
			snode->cu = cu;
			/* deallocate libdwarf supplied source file name -
			 * we have already stashed this ourselves */
			dwarf_dealloc(ctx->dbg, srcfiles[i], DW_DLA_STRING);
		}
		/* we are done with this compilation unit - discard
		 * the libdwarf supplied array of source file names */  
		dwarf_dealloc(ctx->dbg, srcfiles, DW_DLA_LIST);
		/* move to the next compilation unit */	 
		cur_cu_offset = next_cu_header_offset;
	}
	/* see if everything went smooth */
	if (res != DW_DLV_NO_ENTRY)
		panic("");
	/* set up the compilation unit index number fields */
	for (i = /* indices start from 1, a value of zero is invalid */ 1,
			cu = ctx->src_data->cus; cu; cu->idx = i++, cu = cu->next)
		;
}


/*!
 *
 *	\fn	void srcfile_get_srcinfo_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr,
 *			struct cu_data ** cu, struct subprogram_data ** subp,
 *			char ** srcname, int * srcline_nr, bool * is_addr_at_src_boundary)
 *	\brief	retrieves source code related information about a supplied core address
 *
 *	\param	ctx	context to work into
 *	\param	addr	core address of interest for which to retrieve
 *			source code related information
 *	\param	cu	a pointer to where to store the compilation
 *			unit data containing the supplied address, if at all
 *			found; if a compilation unit is not found, this one
 *			is set to null; this can be null if compilation
 *			unit information is not of interest
 *	\param	subp	a pointer to where to store the subprogram
 *			data containing the supplied address, if at all
 *			found; if a subprogram is not found, this one
 *			is set to null; this can be null if subprogram
 *			data is not of interest
 *	\param	srcname	a pointer to where to store the source code file name
 *			containing the source code that was used for
 *			generating the machine code for the supplied address,
 *			if at all found; this *must* not be deallocated
 *			by the caller; if a source code file is not found
 *			this is set to null; this can be null in the case
 *			that source code file information is not of
 *			interest
 *	\param	srcline_nr	a pointer to where to store the source code
 *			line number used to generate the machine code
 *			for the address supplied; if a source code line is
 *			not found for the supplied address, zero is stored
 *			here (as usual, the convention adopted here is
 *			that source line numbers start from 1);
 *			this can be null if source code line number
 *			information is not of interest
 *	\param	is_addr_at_src_boundary	set to true if the supplied address
 *		exactly equals an address that is the starting address
 *		corresponding to a dwarf line number program (this usually
 *		means it is reasonably safe to assume the supplied address
 *		is containing an instruction corresponding to some
 *		source code construct and is a suitable site for setting
 *		breakpoints and examining data; things are a bit vague here,
 *		though - e.g. gcc seems to never set the dwarf specified
 *		basic_block, prologue_end, epilogue_begin source
 *		line program registers), set to false otherwise (meaning
 *		that quite probably the supplied address does not fall
 *		on the start of machine code generated for some source
 *		code construct, and is rather somewhere "in the middle"
 *		of such a code sequence); this can be null if no
 *		such information is of interest to the caller;
 *		in other words, this is being set to true, if,
 *		with reasonably good probability, this address
 *		resides at a program sequence point (in the c sense)
 *
 *	\note	gcc seems to toggle the dwarf line number flag 'is_stmt',
 *		at least this is the case with optimised code; i(sgs)
 *		am not at all sure if this flag is any good, this
 *		flag is currently ignored and not used anywhere in
 *		the gear
 *
 *	\return	none
 *
 *	\todo	cache the lookups here
 */
void srcfile_get_srcinfo_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr,
		struct cu_data ** cu, struct subprogram_data ** subp,
		char ** srcname, int * srcline_nr, bool * is_addr_at_src_boundary)
{
/* compilation unit pointer */	
struct cu_data * cup;
int i;
Dwarf_Error err;
Dwarf_Bool flag;
Dwarf_Addr a1, a2;
Dwarf_Unsigned tmp;

	/* sanity checks */
	if (!ctx)
		panic("");
	/* initialize output data to default values */
	if (cu)
		*cu = 0;
	if (subp)
		*subp = 0;
	if (srcname)
		*srcname = 0;
	if (srcline_nr)
		*srcline_nr = 0;
	if (is_addr_at_src_boundary)
		*is_addr_at_src_boundary = false;

	cup = aranges_get_cu_for_addr(ctx, addr);
	if (cu)
	{
		*cu = cup;
	}
	if (subp)
	{
		*subp = aranges_get_subp_for_addr(ctx, addr);
	}
	if (!cup)
		/* nothing more we could do - there is probably no
		 * debug information for the address supplied */
		return;
	/* search the compilation unit statement list for the halt address to
	 * see at which source line we stopped */
	i = 0;
	/* sanity check */
	if (cup->linecount <= 1)
		panic("malformed dwarf source statement program");
	while (i < cup->linecount - 1)
	{
		/* if this is an end of sequence entry - skip to the
		 * next one */
		if (dwarf_lineendsequence(cup->linebuf[i], &flag, &err)
				!= DW_DLV_OK)
			panic("");
		if (flag)
		{
			/* skip to the nexet sequence of addresses */
			i++;
			continue;
		}
		/* obtain the addresses of the two consecutive source line table
		 * entries and use them to see if our halt address lies
	         * between these source line table entries */	 
		if (dwarf_lineaddr(cup->linebuf[i], &a1, &err)
				!= DW_DLV_OK)
			panic("");
		if (dwarf_lineaddr(cup->linebuf[i + 1], &a2, &err)
				!= DW_DLV_OK)
			panic("");
		if (a1 <= addr && addr < a2)
			/* match found */
			break;
		/* skip to the next pair of lines */
		i++;
	}
	/* see if a match was found */
	if (i == cup->linecount - 1)
		/* no match found */
		return;
	if (srcname)
	{
		if (dwarf_line_srcfileno(cup->linebuf[i], &tmp, &err)
				!= DW_DLV_OK)
			panic("");
		if (tmp)
		{
			if (tmp > cup->srccount)
				panic("");
			*srcname = cup->srcfiles[tmp - 1];
		}
	}
	if (srcline_nr)
	{
		if (dwarf_lineno(cup->linebuf[i], &tmp, &err)
				!= DW_DLV_OK)
			panic("");
		*srcline_nr = tmp;
	}
	/* see if the halt address falls on a source line information
	 * boundary */
	if (is_addr_at_src_boundary)
	{
		if (addr == a1)
			*is_addr_at_src_boundary = true;
	}
}


/*!
 *	\fn	enum GEAR_ENGINE_ERR_ENUM srcfile_get_core_addr_for_line_nr(struct gear_engine_context * ctx, char * srcname, int line_nr, ARM_CORE_WORD * addr)
 *	\brief	attempts to compute the target core address corresponding to a line number in a source code file
 *
 *	\param	ctx	context to work in
 *	\param	srcname	the name of the source code file to get information for
 *	\param	line_nr	the line number in the file for which to retrieve
 *			the target core address (if any) of the machine code
 *			(if any) generated for this line number in the given
 *			source code file
 *	\param	addr	a pointer to where to store the target core address
 *			of the first machine code instruction (if any)
 *			generated for code residing at the given source code
 *			line number in the given source code file
 *	\return	GEAR_ERR_NO_ERROR, if the target core address was successfully computed - in
 *		this case the addr output parameter is also set; GEAR_ERR_GENERIC_ERROR, if
 *		no target core address of machine code generated for the supplied source code file/line
 *		number could be computed (probably because, the source file was not found, the
 *		line number was out of bounds, no machine code has been generated at all for
 *		the supplied source file/line number combination, or the result is ambiguous -
 *		e.g. more than one core addresses correspond to a single source file/
 *		line number combination - this is a well known artifact of 'for' c loops, and
 *		may also occur with source files which are included in multiple compilation
 *		units) - in this case, the addr output parameter is not touched
 *	\todo	provide more specific error return codes - while maybe not all of these may be
 *		useful, at least the 'for' c loop case mentioned above must be handled with
 *		greater care */
enum GEAR_ENGINE_ERR_ENUM srcfile_get_core_addr_for_line_nr(struct gear_engine_context * ctx,
		char * srcname, int line_nr, ARM_CORE_WORD * addr)
{
struct srclist_srcfile_node * src;
int i;
int dwarf_srcfile_nr;
struct cu_data * cu;
Dwarf_Error err;
Dwarf_Unsigned t;
Dwarf_Addr ret_addr;

	if (!ctx->src_data)
		/* source file module probably not initialized */
		return GEAR_ERR_GENERIC_ERROR;
	src = ((struct src_data *) ctx->src_data)->srcs;
	while (src && strcmp(src->name, srcname))
		src = src->next;

	if (src->cu_cnt > 1)
	{
		gprintf("diagnostic: multiple code address ranges for a source code file\n");
	}
	if (!src || src->cu_cnt > 1)
		/* source file not found, or source file included
		 * in multiple compilation units (implying the result
		 * is probably ambiguous, but dont investigate any
		 * further here) */
		return GEAR_ERR_GENERIC_ERROR;
	/* search the line number information of the compilation
	 * unit corresponding to the supplied source code file */
	if (!(cu = cu_process(ctx, src->cu_list_head->cu->cu_die_offset)))
		/* compilation unit not found */
		return GEAR_ERR_GENERIC_ERROR;
	/* locate the source code file number in the dwarf debug line
	 * information for this compilation unit */  
	for (i = 0; i < cu->srccount; i ++)
	{
		if (!strcmp(srcname, cu->srcfiles[i]))
			break;
	}
	if (i == cu->srccount)
		panic("");
	dwarf_srcfile_nr = i /* dwarf source file numbers start from 1, 0 is reserved */ + 1;
	gprintf("cu name %s, srccount %i\n", cu->name, cu->srccount);
	gprintf("dwarf file nr is %i\n", i);

	for (i = 0; i < cu->linecount; i++)
	{
		if (dwarf_line_srcfileno(cu->linebuf[i], &t, &err) != DW_DLV_OK)
			panic("");

		if (dwarf_srcfile_nr != t)
			/* current line number program entry
			 * source code file number not for the
			 * file we are interested in - skip to
			 * the next entry */
			continue;
		if (dwarf_lineno(cu->linebuf[i], &t, &err)
				!= DW_DLV_OK)
			panic("dwarf_lineno()");
		if (t == line_nr)
			break;
	}
	if (i == cu->linecount)
		/* no code genereated for this source code line */
		return GEAR_ERR_GENERIC_ERROR;
	if (dwarf_lineaddr(cu->linebuf[i], &ret_addr, &err) == DW_DLV_ERROR)
		panic("dwarf_lineaddr()");

	*addr = ret_addr;
	/* everything looks fine */
	return GEAR_ERR_NO_ERROR;

}

/*!
 *	\fn	int srcfile_disassemble_addr_range(struct gear_engine_context * ctx, bool is_disassembling_insn_count, ARM_CORE_WORD start_addr, ARM_CORE_WORD first_addr_past_range_or_insn_count, ARM_CORE_WORD value_to_dump_for_program_counter)
 *	\brief	disassembles a target memory address range, or a certain number of instructions starting from a given address
 *
 *	this function generates a disassembly dump, starting to disassemble
 *	from the address given by the 'start_addr' parameter, and:
 *		- either disassembles a number of instructions
 *		(given by the 'first_addr_past_range_or_insn_count' parameter,
 *		when the 'is_disassembling_insn_count' parameter is true),
 *
 *	or
 *
 *		- disassembles upto but not including a given ending
 *		address (specified by the 'first_addr_past_range_or_insn_count',
 *		when the 'is_disassembling_insn_count' parameter is false
 *
 *	\note	in the case when disassembling a memory range (i.e. the
 *		half-open memory range [start_addr; first_addr_past_range_or_insn_count)
 *		(implying that the 'is_disassembling_insn_count' parameter
 *		is false), it may happen that - if the last instruction
 *		starts at an address lower than 'first_addr_past_range_or_insn_count',
 *		but its encoding spans memory at and/or above
 *		memory address 'first_addr_past_range_or_insn_count', then
 *		more bytes than specified by the ['start_addr'; first_addr_past_range_or_insn_count)
 *		address range may be fetched in order to completely decode this
 *		last instruction - this means that it is possible
 *		in this case that memory outside (actually, above)
 *		the ['start_addr'; first_addr_past_range_or_insn_count)
 *		memory range may be accessed for reading, which may
 *		be unacceptable
 *	\todo	maybe add some special handling regarding eventual issues
 *		described by the note above - if done, also fix the comments
 *		in the note above
 *
 *	\param	ctx	context to work in
 *	\param	is_disassembling_insn_count	if true, then the first_addr_past_range_or_insn_count
 *						parameter below is interpreted as an unsigned
 *						instruction number count to disassemble starting
 *						from the address given by the 'start_addr' parameter;
 *						if false, then the 'first_addr_past_range_or_insn_count'
 *						parameter below is interpreted as the first address
 *						on or above which to stop disassembling (again starting
 *						to disassemble from address 'start_addr'
 *	\param	first_addr_past_range_or_insn_count	when 'the is_disassembling_insn_count'
 *							parameter above is true, this specifies
 *							the number of instructions to disassemble,
 *							starting from address 'start_addr';
 *							when 'the is_disassembling_insn_count'
 *							parameter above is false, this specifies
 *							the first address on or above which to
 *							stop disassembling instructions, again
 *							starting from the address 'start_addr'
 *	\param	value_to_dump_for_program_counter	the value which to dump for the program
 *							counter when generating the machine interface
 *							disassembly record - this may be useful
 *							to some gear engine machine interface
 *							consumers
 *	\return	none
 */
void srcfile_disassemble_addr_range(struct gear_engine_context * ctx,
		bool is_disassembling_insn_count,
		ARM_CORE_WORD start_addr,
		ARM_CORE_WORD first_addr_past_range_or_insn_count,
		ARM_CORE_WORD value_to_dump_for_program_counter)
{
unsigned int insn_cnt;
ARM_CORE_WORD first_addr_past_disassembly;

	miprintf("ERRCODE=[%i,\"\"],", GEAR_ERR_NO_ERROR);
	miprintf("DISASSEMBLY_DUMP,");
	miprintf("[PC = 0x%08x,", (unsigned int) value_to_dump_for_program_counter);
	miprintf("DISASM_LIST = (");

	first_addr_past_disassembly = start_addr;
	insn_cnt = 0;

	if (is_disassembling_insn_count ||
			(first_addr_past_range_or_insn_count > start_addr))
		while (1)
		{
			struct cu_data * cu;
			int srcline_nr;
			bool is_addr_at_src_boundary;
			char * srcname;
			int insn_size_in_bytes;

			srcfile_get_srcinfo_for_addr(ctx,
					first_addr_past_disassembly,
					&cu,
					/*! \todo	if dumping the containing
					 *		subprogram information
					 *		could be useful,
					 *		do it here... */
					0 /* subprogram_data ** subp */,
					&srcname,
					&srcline_nr,
					&is_addr_at_src_boundary);

			if (cu && is_addr_at_src_boundary)
			{
				/* the currently disassembled instruction
				 * lies at the exact address for machine
				 * code generated for some source code
				 * line - so print the line number 
				 * before printing the disassembly;
				 * this can be useful for a gear engine
				 * machine interface output consumer
				 * when generating a mixed 
				 * source code/disassembly output */
				miprintf("SRCLINE_NR = %i, SRCFILE_NAME = \"%s\",",
						srcline_nr,
						/*! \todo	this is scheduled
						 *		to be replaced
						 *		by a source code
						 *		file number
						 *		(printing the
						 *		source code
						 *		names here is
						 *		a way too much)
						 *		when there is
						 *		time in the
						 *		(hopefully near)
						 *		future */
						srcname);
			}
			if ((is_disassembling_insn_count && insn_cnt >= first_addr_past_range_or_insn_count)
					|| (!is_disassembling_insn_count
						&& first_addr_past_disassembly >= first_addr_past_range_or_insn_count))
				/* enough disassembly printed */
				break;
			miprintf("ADDR = 0x%08x, DISASM = ", (unsigned int) first_addr_past_disassembly);
			miprintf("\"");
			insn_size_in_bytes = ctx->tdesc->print_disassembled_insn(ctx, first_addr_past_disassembly, miprintf);
			if (insn_size_in_bytes <= 0)
			{
				/* some error occurred while disassembling
				 * (most probably error accessing target memory) */
				/*! \todo	maybe just abort here... */
				insn_size_in_bytes = 1;
			}

			insn_cnt ++;
			first_addr_past_disassembly += insn_size_in_bytes;
			miprintf("\", ");
		}
	miprintf("), ");
	miprintf("DISASM_RANGE = [0x%08x, 0x%08x], ", (unsigned int) start_addr, (unsigned int) first_addr_past_disassembly);
	miprintf("INSTRUCTION_COUNT = %i", insn_cnt);

	miprintf("]\n");
}



























#if 1


#include "srcinfo.h"

/*!
 *	\fn	static struct srclist_type_struct * dump_srcfile_machine_code_details(struct gear_engine_context * ctx, struct srclist_srcfile_node * p)
 *	\brief	for a given source code file (specified by the 'p' parameter), dumps various machine code information related to this source code file
 *
 *
 *	for a given source code file (specified by the 'p' parameter),
 *	this routine dumps various generated machine code information
 *	related to this source code file, such as:
 *	(1)	- which source code line numbers have machine code instructions
 *		generated for them, i.e. - are breakpointable; this information
 *		is dumped as a bitmap, having a bit for each source code line
 *		number, which - when set - denotes the line is breakpointable
 *		(i.e. has machine code generated for it), and - when clear -
 *		denotes the source code line is not breakpointable; if
 *		the source code contains more lines than bits in the bitmap,
 *		it is the case that the bitmap describes the first 'n' number
 *		of source code lines on the source file ('n' being the number
 *		of bits in the bitmap), and the remaining source code lines
 *		in the file - at the end of the file - are not breakpointable,
 *		i.e. have their bits (were these bits present in the bitmap)
 *		implicitly set to zero
 *	(2)	- the (list of) address range(s) that machine code generated
 *		from source code in this source file spans, no matter in which
 *		compilation unit (i.e. included is information from all
 *		compilation units that possess machine code generated from 
 *		this source code file when building these compilation units);
 *		note that it may be
 *		the case that the generated machine code spans multiple,
 *		discontiguous address ranges - for example when the
 *		compiler performs optimizations such as function inlining,
 *		and/or when a single source code file participates in
 *		the building of multiple compilation units (e.g. by source
 *		code file inclusion when compiling); also read the
 *		comments below; the list of address ranges is dumped
 *		by this function as a set of target core address pairs,
 *		where, in each address pair, the first address is the
 *		start address of machine code generated for this range
 *		(memory at this address is included in the range),
 *		and the second address is the first address past the
 *		last address of machine code generated for this range
 *		(memory at this address is excluded from the range),
 *		i.e. the pair format is:
 *		(first_address_of_range first_address_past_the_end_of_range)
 *	(3)	a list of subprograms that have been declared in this
 *		source code file (and the compiler has not decided to
 *		optimize out), along with the source code line numbers
 *		of the declarations of these subprograms
 *	(4)	the list of source code line number - starting generated
 *		machine code address; this list contains, for each
 *		source code line number that has machine code generated
 *		for it, in any compilation unit, the starting address
 *		of the machine code generated for this line to be dumped;
 *		note that there may be multiple address ranges corresponding
 *		to a single source code line number (e.g., with compiler
 *		optimizations in effect - and even without optimizations,
 *		this is a common case for c 'for' loops), but it will
 *		never be the case that a single starting machine code
 *		address range corresponds to different source code
 *		line numbers; thus - in this list - source code
 *		line numbers are not unique, but starting generated
 *		machine code addresses are, so this list is dumped
 *		sorted by ascending value of the starting generated
 *		machine code address - this is important as gear
 *		engine machine interface consumers can rely on this,
 *		because it makes certain operations on this list
 *		more efficient; this list is the bulk of the information
 *		supplied by this function, and is additionally described
 *		below
 *
 *	the generation of the source code line number - starting
 *	generated machine code address
 *	in the simple case where a single source code file is
 *	contained in a single compilation unit, with optimizations
 *	disabled is actually very straightforward - this routine
 *	however attempts to handle the more general case, where
 *	it is possible that a single source code file may contribute
 *	generated machine code to multiple compilation units
 *	(e.g. by source code file inclusion), eventually
 *	with compiler optimizations such as function nesting and/or
 *	inlining in effect - all these artefacts can cause
 *	dwarf source code line number information to be
 *	generated for a single source code file in multiple
 *	compilation units, with the machine code ranges contributed
 *	by this source code file being disjoint and
 *	sparsely scattered in the targed address space
 *
 *	the bulk of information supplied by this function (the list
 *	of source code line number - starting generated machine code
 *	address) can be used by a gear engine frontend consumer
 *	to display which source code lines have machine code
 *	generated for them, and are therefore valid locations
 *	for setting breakpoints; this information can also be
 *	used for displaying disassembly dumps for source code
 *	in a, so to say, source-code-file-centric manner - i.e.
 *	to display all of the machine code contributed by
 *	a single source code file, no matter in which compilation
 *	unit; note that for optimized code this source-code-centric
 *	disassembly display can actually be very confusing for
 *	a human to interpret and understand - because of the
 *	optimizations and non-contiguous generated machine code 
 *	address ranges described in the paragraph above
 *
 *	for generating so-to-call 'plain' disassemblies (i.e.
 *	a disassembly of a single, contiguous address range),
 *	the srcfile_disassemble_range() function may be used;
 *	it may be also used for 'compilation-unit-centric'
 *	disasemblies (i.e. disassembling all of the machine
 *	code residing in a compilation unit) - in this case
 *	the srcfile_disassemble_range() function can simply
 *	be called for (all of) the target address space range(s)
 *	making up the compilation unit
 *
 *	all this said, it is probably a good idea to briefly
 *	outline how the dump_srcaddr_pairs() routine (the one
 *	below) generates the 'source code line number - starting
 *	generated machine code address' pairs:
 *	(1)	the (already built at this module's
 *		initialization time) list of compilation units
 *		that this source code file participates in
 *		is traversed and any starting address ranges
 *		of machine code generated for source code
 *		line numbers in this file are recorded in
 *		an array for later use (the 'runs' array)
 *	(2)	if the array built in step (1) is empty
 *		(a common case for header files), nothing
 *		more is done and this function returns;
 *		otherwise go to step (3)
 *	(3)	the array built in step (1) is qsort()-ed
 *		by ascending value of starting address
 *		of the generated machine code address ranges
 *	(4)	dumped is a list of dwarf subprogram die-s
 *		that have been found to have been declared in
 *		this source code file; during this step
 *		the 'subdata' data structure below
 *		is being built so that it is available for
 *		use in step (5)
 *	(5)	dumped is the bulk of the information record
 *		generated by this function - the source code
 *		line number - starting generated machine code
 *		address; in doing so, the records in the array
 *		built in step (1) are being adjusted to aid
 *		the successive steps, and the breakpointable
 *		lines bitmap is being built; these 'pairs'
 *		are actually quadruples, because, along with each
 *		pair, two other minor, additional numbers are also
 *		being output - the first number is the index
 *		in the compilation unit name array of the
 *		compilation unit in which the address dumped
 *		resides (this compilation unit name array is
 *		output by srcfile_dump_sources_info_mi() as
 *		part of the source code information), the
 *		second number is the index in the subprogram
 *		array (output here in step (4)) of the subprogram
 *		in which the address dumped lies; these two
 *		additional numbers provide additional, non-vital
 *		details and gear machine interface consumers
 *		may well ignore them; valid values for these
 *		two additional numbers start from 1, a value
 *		of zero for any of them denotes that such
 *		information is invalid/unavailable/unknown 
 *	(6)	dumped is the single address range containing
 *		all discontiguous address ranges containing
 *		the machine code generated for this source
 *		code file; note that this and the list dumped
 *		below in step (7) may actually be the same
 *	(7)	dumped is the list of individual, contiguous
 *		address ranges of machine code generated from
 *		this source code file
 *	(8)	dumped is the breakpointable source code lines
 *		bitmap
 *
 *	\note	for target core address - line number pairs
 *		printed for use by a frontend, there is a
 *		notable special case regarding the printing
 *		of end-of-sequence entries in the dwarf line
 *		number program - see the comments in the printing
 *		loop below for details
 *
 *	\todo	fix the ugly address/others miprintf casts in this function...
 *
 *	\param	ctx	context to work in
 *	\param	p	the data structure identifying the source
 *			code file for which to generate the information
 *			described above
 *	\return	a 'srclist_type_struct' data structure, containing
 *		information for the file specified
 */
static struct srclist_type_struct * dump_srcfile_machine_code_details(struct gear_engine_context * ctx, struct srclist_srcfile_node * p)
{
struct cu_data * cu;
struct srcnode * srcnode;
/* this array holds pointers to the line number program
 * entries for this source file, it must be sorted by
 * ascending value of the target core address of the
 * line number program entries */ 
struct srcaddr_struct
{
	struct srclist_cu_node		* xcu;
	int		dwlines_idx;
	Dwarf_Addr	start_addr;
	Dwarf_Addr	first_addr_past_end;
	int		file_nr;
}
* runs;
/* the size of the 'runs' array above, and the index
 * of the next usable (not yet occupied) element in it */
int runs_array_size, runs_array_idx;
/* this is used for building a bitmap of the breakpointable
 * source code lines for each source file
 *
 * \todo	no more than 65536 source code lines in
 * 		a single source file are handled right now */
unsigned int * bkpt_bmap;

int i, j, file_nr;
int greatest_line_nr;
Dwarf_Unsigned	t;
Dwarf_Unsigned line_nr;
Dwarf_Addr	addr;
Dwarf_Bool	flag;
Dwarf_Error	err;
Dwarf_Addr	hack_addr = 0;

struct
{
	struct subprogram_data	* last_subp;
	int last_name_idx;
	const char ** names;
	int idx;
	int len;
}
subdata;

/*! \todo	debug only */
bool defer_panic = false;

struct srclist_type_struct * sdata;
int slen, sidx;

/* hmmm, a nested function... */
int srcaddr_struct_compare(const struct srcaddr_struct * arg1, const struct srcaddr_struct * arg2)
{
	if (arg1->start_addr == arg2->start_addr)
		return 0;
	return (arg1->start_addr < arg2->start_addr) ? -1 : 1;
}

	gprintf("%s(): name is %s\n", __func__, p->name);

	/*
	 *
       	 *	step (1) - for details see the comments at the start of this function
	 *
	 */

	runs = 0;
	runs_array_idx = runs_array_size = 0;
	for (srcnode = p->cu_list_head; srcnode; srcnode = srcnode->next)
	{
		cu = cu_process(ctx, srcnode->cu->cu_die_offset);
		if (!cu)
			panic("");
		/* determine the dwarf file number in the line number program
		 * for this compilation unit */
		for (i = 0; i < cu->srccount; i ++)
		{
			if (!strcmp(p->name, cu->srcfiles[i]))
				break;
		}
		if (i == cu->srccount)
			panic("");
		/* the dwarf3 standard document, section 6.2.4, paragraph 9 says:
			The line number program assigns numbers to each of the file entries in order, beginning with 1, and uses those numbers instead of file names in the file register.

		 * the libdwarf documentation, paragraph 5.5.3.4 says:
When the number returned
through *returned_fileno is zero it means the file name is unknown (see the DWARF2/3 line table
specification). When the number returned through *returned_fileno is non-zero it is a file number:
subtract 1 from this file number to get an index into the array of strings returned by dwarf_srcfiles()
(verify the resulting index is in range for the array of strings before indexing into the array of strings). The
file number may exceed the size of the array of strings returned by dwarf_srcfiles() because
dwarf_srcfiles() does not return files names defined with the DW_DLE_define_file operator.
		*/
		file_nr = i + 1;
		if (0) for (i = 0; i < cu->linecount; i ++)
		{
			if (dwarf_lineaddr(cu->linebuf[i], &addr, &err)
					!= DW_DLV_OK)
				panic("");
			if (dwarf_lineno(cu->linebuf[i], &line_nr, &err)
					!= DW_DLV_OK)
				panic("");
			printf("found dwarf srcline entry: line_nr %i, addr 0x%08x\n", (int) line_nr, (int) addr);
		}
		for (i = 0; i < cu->linecount; i ++)
		{
			/* if the current entry in the line number program
			 * is for this source file, store a pointer in the
			 * 'runs' array for later sorting */
			if (dwarf_line_srcfileno(cu->linebuf[i], &t, &err) != DW_DLV_OK)
				panic("");

			if (file_nr != t)
				/* current line number program entry
				 * source code file number not for the
				 * file we are interested in - skip to
				 * the next entry */
				continue;
			if (dwarf_lineaddr(cu->linebuf[i], &addr, &err)
					!= DW_DLV_OK)
				panic("");
			/* this was attempted as a nested function, but gcc 4.4.3 choke on
			 * it... interesting in itself - investigate this...
			add_entry((struct srcaddr_struct) { .xcu = srcnode->cu,
					.dwlines_idx = i,
					.start_addr = addr,
					.first_addr_past_end = addr,
					.file_nr = file_nr });
			void add_entry(struct srcaddr_struct s) */
			{
				if (runs_array_idx == runs_array_size)
				{
					/* resize array */
					if (!(runs_array_size *= 2))
						/*! \todo	1 is actually an overwhelmingly common case here...
						 *		maybe optimize for this case... */ 
						runs_array_size = 16;
					if (!(runs = realloc(runs, runs_array_size * sizeof * runs)))
						panic("");
				}
				runs[runs_array_idx ++] = /* s */
					(struct srcaddr_struct)
					{ .xcu = srcnode->cu,
					.dwlines_idx = i,
					.start_addr = addr,
					.first_addr_past_end = addr,
					.file_nr = file_nr };
			}

			/* skip to next end-of-sequence line number program entry */
			for (i++; i < cu->linecount; i ++)
			{
				if (dwarf_lineendsequence(cu->linebuf[i], &flag, &err)
						!= DW_DLV_OK)
					panic("");
				if (flag)
					break;
			}
		}
	}

	/*
	 *
       	 *	step (2) - for details see the comments at the start of this function
	 *
	 */

	if (runs_array_idx == 0)
		/* array empty - no machine code generated for this file -
		 * a common case for header files; do not emit
		 * anything about such files... */
		return 0;

	/*
	 *
       	 *	step (3) - for details see the comments at the start of this function
	 *
	 */

	if (!(sdata = calloc(1, sizeof * sdata)))
		panic("");

	/* there is indeed machine code generated for this file -
	 * emit a source file record for it */
	sdata->srcname = strdup(p->name);

	qsort(runs, runs_array_idx, sizeof * runs, (int (*)(const void *, const void *)) srcaddr_struct_compare);
	if (0) for (i = 0; i < runs_array_idx; i++)
	{
		gprintf("start_addr: 0x%08x\n", (unsigned int) runs[i].start_addr);
	}

	/*
	 *
       	 *	step (4) - for details see the comments at the start of this function
	 *
	 */

	/* dump a list of the toplevel subprograms defined in this
	 * source code file, by iterating over all
	 * compilation units that it has contributed to
	 *
	 * \todo	also support inlined/nested subprograms */
	////miprintf("SUBPROGRAM_LIST=(");

	subdata.len = 16;
	if (!(subdata.names = calloc(subdata.len, sizeof * subdata.names)))
		panic("");
	/* index 0 is unused/invalid */
	subdata.idx = 1;
	subdata.names[0] = 0;
	subdata.last_subp = 0;
	subdata.last_name_idx = 0;
	////miprintf("[SUBPROGRAM_NAME=\"<<< unknown subprogram >>>\", DECL_LINE=0],");

	for (srcnode = p->cu_list_head; srcnode; srcnode = srcnode->next)
	{
		cu = cu_process(ctx, srcnode->cu->cu_die_offset);
		if (!cu)
			panic("");
		/* determine the dwarf file number in the line number program
		 * for this compilation unit */
		for (i = 0; i < cu->srccount; i ++)
		{
			if (!strcmp(p->name, cu->srcfiles[i]))
				break;
		}
		if (i == cu->srccount)
			panic("");
		/* adjust the file number to match the numbers
		 * as recorded in the subprogram data structures -
		 * as above (see the comments after the
		 * dwarf_line_srcfileno() call above), the line
		 * numbers are offset by 1 */
		file_nr = i + 1;

		if (cu->subs)
		{
		struct subprogram_data * subp;
			subp = cu->subs;

			do
			{
				if (subp->srcfile_nr == file_nr)
				{
					////miprintf("[SUBPROGRAM_NAME=\"%s\", DECL_LINE=%i],", subp->name, subp->srcline_nr);
					if (subdata.idx == subdata.len)
					{
						subdata.len *= 2;
						if (!(subdata.names = realloc(subdata.names, subdata.len * sizeof * subdata.names)))
							panic("");
					}
					subdata.names[subdata.idx++] = subp->name;
				}
				subp = subp->sib_ptr;
			}
			while (subp);
		}
	}
	////miprintf("),");

	if (!(sdata->subprogram_arr = calloc(subdata.idx, sizeof * sdata->subprogram_arr)))
		panic("");
	sdata->subprogram_arr_len = subdata.idx;	
	sdata->subprogram_arr->name = strdup("<<< unknown subprogram >>>");
	sdata->subprogram_arr->srcline_nr = -1;
	for (i = 1; i < subdata.idx; i++)
	{
		sdata->subprogram_arr[i].name = strdup(subdata.names[i]);
		sdata->subprogram_arr[i].srcline_nr = -1;
	}

	/*
	 *
       	 *	step (5) - for details see the comments at the start of this function
	 *
	 */

	/* next, dump the source code line number - generated machine code
	 * start address pairs; while doing this - also build the
	 * breakpointable source code line number bitmap for later dumping */
	/*! \todo	no more than 65536 source code lines in
	 * 		a single source file are handled right now */
	/*! \todo	make this equal the exact amount of needed elements */
	if (!(bkpt_bmap = calloc(0x10000 / ((sizeof * bkpt_bmap) * 8 /* bits in a byte */), sizeof * bkpt_bmap)))
		panic("");
	greatest_line_nr = 0;
	if (!(sdata->srcaddr_pairs = calloc(slen = 32, sizeof * sdata->srcaddr_pairs)))
		panic("");
	/* index zero is invalid/unused */
	sidx = 1;	
	////miprintf("SRCLINE_LIST = (");
	for (i = 0; i < runs_array_idx; i++)
	{
	bool seq_flag;

		seq_flag = false;

		cu = cu_process(ctx, runs[i].xcu->cu_die_offset);

		/* retrieve data for the current line */
		for (j = runs[i].dwlines_idx; j < cu->linecount; j ++)
		{
			/* locate the first line number in this run that belongs
			 * to the source code file processed */
			do
			{
				if (dwarf_line_srcfileno(cu->linebuf[j], &t, &err) != DW_DLV_OK)
					panic("");
				if (t == runs[i].file_nr)
					break;
				/* see if an end-of-sequence entry should
				 * be printed */
				if (seq_flag)
				{
					if (dwarf_lineaddr(cu->linebuf[j], &addr, &err)
							!= DW_DLV_OK)
						panic("");
					if (sidx == slen)
					{
						if (!(sdata->srcaddr_pairs = realloc(sdata->srcaddr_pairs, (slen *= 2) * sizeof * sdata->srcaddr_pairs)))
							panic("");
					}
					sdata->srcaddr_pairs[sidx ++] = (struct srcline_addr_pair_struct) { .addr = (ARM_CORE_WORD) addr,
											.srcline_nr = 0, .cuname_idx = 0, .subarr_idx = 0, .iextend = 0, };
					////miprintf("0x%x 0 0 0,", (ARM_CORE_WORD)addr);
					/* also store the first address past this address
					 * range for later use when dumping the list
					 * of spanned address ranges below */
					runs[i].first_addr_past_end = addr;
				}
				seq_flag = false;
				if (dwarf_lineendsequence(cu->linebuf[j], &flag, &err)
						!= DW_DLV_OK)
					panic("");
				if (flag)
				{
					/* force skip to the next run in the
					 * 'runs' array */
					j = cu->linecount;
					break;
				}
				j ++;
			}
			while (j < cu->linecount);
			if (j == cu->linecount)
				break;
			/*! \todo	some of these need fixing */
			if (dwarf_lineendsequence(cu->linebuf[j], &flag, &err)
					!= DW_DLV_OK)
				panic("");
			if (dwarf_lineno(cu->linebuf[j], &line_nr, &err)
					!= DW_DLV_OK)
				panic("");
			if (line_nr == 0)
				panic("");
			if (dwarf_lineaddr(cu->linebuf[j], &addr, &err)
					!= DW_DLV_OK)
				panic("");
			/*! \todo	these are simple sanity checks for
			 *		cases that are not handled right now;
			 *		remove them when all of the cases are
			 *		handled properly */
			gprintf("hack_addr: 0x%08x; addr: 0x%08x, line_nr: %i\n", (int) hack_addr, (int) addr, (int) line_nr);
			if (hack_addr > addr)
			{
				defer_panic = true;//panic("");
			}
			if (j == cu->linecount - 1 && !flag)
				panic("if this ever happens, handle it with great care!!!");
			/*
			   if (flag && i != nr_lines - 1)
			   panic("");
			 */
			/* end of hacks */

			/* update the breakpointable source code line bitmap */
			if (!line_nr || line_nr >= 65536)
			{
				if (0) panic("");
				line_nr = 65535;
			}
			if (greatest_line_nr < line_nr)
				greatest_line_nr = line_nr;
			/* set the appropriate bit in the array */
			bkpt_bmap[line_nr / (8 /* bits in a byte */ * sizeof * bkpt_bmap)]
				|= 1 << (line_nr % (8 /* bits in a byte */ * sizeof * bkpt_bmap));

			if (flag)
			{
				/*! \note	line numbers for end of sequence
				 *		dwarf line number program opcodes are
				 *		handled in a special way -
				 *		normally, the source code line number
				 *		in such entries equals the line number in the
				 *		predecessing line number dwarf program entry
				 *		(see the dwarf standard for details),
				 *		which is maybe not too useful (for a gear engine
				 *		machine interface consumer, at least);
				 *		however, the end-of-sequence entries
				 *		hold other quite important information -
				 *		namely, the first address
				 *		location past the current, increasing in value,
				 *		sequence of target core address values - this
				 *		information may prove quite useful
				 *		for gear engine machine interface consumers
				 *		which generate disassembly dumps, as this
				 *		value can serve as a clue about when to stop
				 *		disassembling
				 *
				 * this said - end-of-sequence dwarf program
				 * entries are output via the machine interface
				 * with having source code line number equal
				 * to zero - which is an invalid source code
				 * line number; this special value of zero 
				 * for the source code line number should be
				 * used by machine interface consumers to
				 * discriminate end-of-sequence entries from
				 * ordinary ones */
				if (sidx == slen)
				{
					if (!(sdata->srcaddr_pairs = realloc(sdata->srcaddr_pairs, (slen *= 2) * sizeof * sdata->srcaddr_pairs)))
						panic("");
				}
				sdata->srcaddr_pairs[sidx ++] = (struct srcline_addr_pair_struct) { .addr = (ARM_CORE_WORD) addr,
										.srcline_nr = 0, .cuname_idx = 0, .subarr_idx = 0, .iextend = 0, };
				////miprintf("0x%x 0 0 0,", (ARM_CORE_WORD)addr);
				/* also store the first address past this address
				 * range for later use when dumping the list
				 * of spanned address ranges below */
				runs[i].first_addr_past_end = addr;
				break;
			}
			else
			{
				/* retrieve the subprogram containing the address
				 * being output */
				if (1) if (!subdata.last_subp
						|| !dwarf_ranges_is_in_range(
							ctx,
							subdata.last_subp->addr_ranges,
							addr,
							cu->default_cu_base_address))
				{
					/* must update 'subdata' fields */
					if (0)
						subdata.last_subp = aranges_get_subp_for_addr(ctx, addr);
					else
					{
						struct subprogram_data * subp;

						subp = 0;

						/* ok, locate the subprogram within the compilation unit */
						subp = cu->subs;

						while (subp)
						{
							if (dwarf_ranges_is_in_range(ctx, subp->addr_ranges, addr,
										cu->default_cu_base_address))
								/* match found */
								break;
							subp = subp->sib_ptr;
						}
						subdata.last_subp = subp;
					}



					if (subdata.last_subp)
					{
						int i;
						for (i = 1; i < subdata.idx; i++)
							if (!strcmp(subdata.last_subp->name, subdata.names[i]))
								break;
						if (i == subdata.idx)
							/*! \todo	maybe an inlined subprogram...
							 *		handle these better... */ 
							subdata.last_name_idx = 0;
						else
							subdata.last_name_idx = i;
					}
					else
					{
						/* subprogram not found - fall back
						 * to the invalid name entry */
						subdata.last_name_idx = 0;
					}
				}
				if (sidx == slen)
				{
					if (!(sdata->srcaddr_pairs = realloc(sdata->srcaddr_pairs, (slen *= 2) * sizeof * sdata->srcaddr_pairs)))
						panic("");
				}
				sdata->srcaddr_pairs[sidx ++] = (struct srcline_addr_pair_struct) { .addr = (ARM_CORE_WORD) addr,
										.srcline_nr = line_nr, .cuname_idx = runs[i].xcu->idx, .subarr_idx = subdata.last_name_idx, .iextend = 0, };
				////miprintf("0x%x %i %i %i,", (ARM_CORE_WORD)addr, (int)line_nr, runs[i].xcu->idx, subdata.last_name_idx);
				seq_flag = true;
			}
			hack_addr = addr;
		}
	}
	////miprintf("),");
	sdata->srcaddr_len = sidx;

	/*
	 *
       	 *	step (6) - for details see the comments at the start of this function
	 *
	 */

	/* dump the address range that this source code file contributes
	 * machine code to - note that this address range will generally
	 * not be contiguous (that is, some other source files may also
	 * have machine code residing in this range), and there may also
	 * be "holes" in this address range (i.e. regions for which no
	 * machine code has been generated at all) */
	////miprintf("INCLUSIVE_SPANNED_ADDR_RANGE = [");
	/* at this point, 'addr' will already contain the first
	 * address past the last address range of machine code */  
	////miprintf("0x%08x 0x%08x", (unsigned int) runs[0].start_addr, (unsigned int) addr);
	////miprintf("],");
	sdata->low_pc = (ARM_CORE_WORD) runs[0].start_addr;
	sdata->hi_pc = (ARM_CORE_WORD) addr;

	/*
	 *
       	 *	step (7) - for details see the comments at the start of this function
	 *
	 */

	/* dump individual address ranges */
#if 0
	miprintf("SPANNED_ADDR_RANGES = (");
	for (i = 0; i < runs_array_idx; i++)
	{
		miprintf("0x%08x 0x%08x, ",
				(unsigned int) runs[i].start_addr,
				(unsigned int) runs[i].first_addr_past_end);
	}
	miprintf("),");
#endif
	/*
	 *
       	 *	step (8) - for details see the comments at the start of this function
	 *
	 */

	/* dump the breakpointable source code line bitmap */
#if 0
	i = greatest_line_nr / (8 /* bits in a byte */ * sizeof * bkpt_bmap);
	miprintf("BKPT_LINES_BMAP=(");
	j = 0;
	do
	{
		miprintf("0x%x,", bkpt_bmap[j++]);
	}
	while (i--);
	miprintf("),");

	miprintf("],");

	free(runs);
	free(subdata.names);
	////free(bkpt_bmap);

	if (0) if (defer_panic)
		panic("");
#endif
	/*! \todo	make this equal the exact amount of needed elements */
	sdata->bkpt_bmap = bkpt_bmap;
	sdata->bmap_size = greatest_line_nr / (8 /* bits in a byte */ * sizeof * bkpt_bmap);

	return sdata;
}



/*!
 *	\fn	struct srcinfo_type_struct * srcfile_get_srcinfo(struct gear_engine_context * ctx)
 *	\brief	prints various information about the source files of the current executable being debugged
 *
 *	\todo	this is a preliminary version, the printed data is incomplete
 *	\todo	fix the comments below when this is completed
 *	\todo	the cases where a single compilation unit contains
 *		multiple source code files or spans several
 *		address ranges, are not handled right now; fix
 *		the comments below about what is being printed
 *		when this issue is resolved
 *
 *	\note	some of the data dumped here is redundant, the
 *		redundancy is purely for the convenience of
 *		gear engine machine interface data consumers
 *
 *	this function prints various data about the
 *	source code files comprising the currently debugged
 *	executable; the current version prints a
 *	list of the compilation unit names, and a list of
 *	source files; for each source file -
 *	the address range(s) spanned by the compilation unit
 *	corresponding to the source file ( \todo handle the case
 *	when a single compilation unit contains multiple
 *	source files), a list of the subroutines found in the dwarf
 *	debug information of the comprising compilation unit,
 *	a bitmap of the source code line numbers
 *	that are suitable for putting a breakpoint on
 *	(that is, ones for which machine code has been generated),
 *	a list of core addresses - line number pairs for lines 
 *	for which such dwarf debug information has been found -
 *	this list is sorted by ascending values of the core
 *	addresses (note that while a certain target core address
 *	may be present in the list only once, this is not true
 *	for source line numbers - e.g. an artefact of c 'for'
 *	loops is that two target core addresses may correspond
 *	to a single source code line number)
 *
 *	\param	ctx	context to work in
 *	\return	none */
struct srcinfo_type_struct * srcfile_get_srcinfo(struct gear_engine_context * ctx)
{
struct srclist_srcfile_node	* src;
struct srclist_cu_node		* xcu;
struct cu_data	* cu;
int x;
struct cu_info_struct * cutab;
struct srclist_type_struct * snode, ** slist;
struct srcinfo_type_struct * srcinfo;

	if (!ctx->src_data)
		panic("");
	/* dump a list of compilation unit names */
	miprintf("COMPILATION_UNITS=(");
	/* when dumping source code information, compilation
	 * unit indices are being dumped - they start from 1,
	 * 0 is an invalid index, so dump a dummy name for
	 * the zeroth compilation unit */
	/* count the number of compilation units */
	for (x = 0, xcu = ctx->src_data->cus; xcu; xcu = xcu->next, x ++);

	if (!(cutab = calloc(x + /* one more entry for the zeroth/invalid entry */ 1, sizeof * cutab)))
		panic("");
	if (!(srcinfo = calloc(1, sizeof * srcinfo)))
		panic("");
	srcinfo->comp_units = cutab;
	srcinfo->nr_comp_units = x + 1;
	/* construct the zeroth/invalid entry */
	cutab->name = strdup("<<< unknown/invalid/dummy/n\\a compilation unit >>>");
	cutab->compdir = strdup("<<< no compilation unit directory >>>");

	for (x = 1, xcu = ctx->src_data->cus; xcu; xcu = xcu->next, x ++)
	{
		cu = cu_process(ctx, xcu->cu_die_offset);
		cutab[x].name = strdup(cu->name);
		cutab[x].compdir = strdup(cu->comp_dir);
	}
	slist = & srcinfo->srclist;
	for (src = ctx->src_data->srcs; src; src = src->next)
		if ((snode = dump_srcfile_machine_code_details(ctx, src)))
			* slist = snode, snode->next = 0, slist = & snode->next;
	return srcinfo;
}


#endif

