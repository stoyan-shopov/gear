#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "core-access.h"

#include "cu-access.h"
#include "srcfile.h"
#include "util.h"


/* process each compilation unit in .debug_info */
static void exit_error(char * errmsg)
{
	gprintf("fatal error: %s\n", errmsg);
	exit(1);
}



void print_error(Dwarf_Debug dbg, char * errmsg, int sres, Dwarf_Error err)
{
	gprintf("print_error(): %s\n", errmsg);
}

void dwarf_hacked_init(struct gear_engine_context * ctx)
{
int elf_fd;
Dwarf_Debug dbg;
int res = DW_DLV_OK;
Dwarf_Error err;
Dwarf_Unsigned	next_cu_header_offset;
Dwarf_Off cur_cu_offset;
Elf * libelf_d;

	elf_fd = open(ctx->dbg_info_elf_disk_file_name, O_RDONLY | O_BINARY, 0);
	/* validate libelf version */
	if (elf_version(EV_CURRENT) == EV_NONE)
	{
		exit_error("libelf out of date\n");
	}
	if (elf_fd < 0)
	{
		exit_error("could not open input elf file\n");
		exit(1);
	}
	ctx->dbg_elf_fd = elf_fd;
	if (!(libelf_d = elf_begin(elf_fd, ELF_C_READ, 0)))
	{
		exit_error("elf_begin() failed\n");
	}
	ctx->libelf_elf_desc = libelf_d;

	//if ((res = dwarf_init(elf_fd, DW_DLC_READ, NULL, NULL, &dbg, &err)) != DW_DLV_OK)
	if ((res = dwarf_elf_init(libelf_d, DW_DLC_READ, NULL, NULL, &dbg, &err)) != DW_DLV_OK)
	{
		exit_error("dwarf_elf_init() failure");
	}
	ctx->dbg = dbg;

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
	if (0) init_aranges(ctx);
	if (res != DW_DLV_NO_ENTRY)
		panic("");
	/* start from offset 0 - the start of the debug information
	 * (.dbg_info) section */	 
	cur_cu_offset = 0;
	/* process all compilation units */
	while ((res = dwarf_next_cu_header(ctx->dbg,
					/* dont need these */
					0,
					0,
					0,
					0,
					&next_cu_header_offset,
					&err)) == DW_DLV_OK)
	{
		if (dwarf_get_cu_die_offset_given_cu_header_offset
			(ctx->dbg, cur_cu_offset, &cur_cu_offset, &err)
				!= DW_DLV_OK)
			panic("");
		cu_process(ctx, cur_cu_offset);
		cur_cu_offset = next_cu_header_offset;
	}

	gprintf("dwarf engine successfully initialized\n");

}

ssize_t write_to_frontends(const void *buf, size_t count)
{
	return write(1, buf, count);
}
#if 0

/*!
 *	\fn	static void dump_srcfile_machine_code_details(struct gear_engine_context * ctx, struct srclist_srcfile_node * p)
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
static struct srclist_type_struct dump_srcfile_machine_code_details(struct gear_engine_context * ctx, struct srclist_srcfile_node * p)
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
	sidx = 0;	
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
						if (!(sdata->srcaddr_pairs = realloc((slen *= 2) * sizeof * sdata->srcaddr_pairs)))
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
			gprintf("hack_addr: 0x%08x; addr: 0x%08x\n", (int) hack_addr, (int) addr);
			if (hack_addr > addr)
			{
				defer_panic = true;//panic("");
			}
			if (i == cu->linecount - 1 && !flag)
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
					if (!(sdata->srcaddr_pairs = realloc((slen *= 2) * sizeof * sdata->srcaddr_pairs)))
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
					if (!(sdata->srcaddr_pairs = realloc((slen *= 2) * sizeof * sdata->srcaddr_pairs)))
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
	subdata->bkpt_bmap = bkpt_bmap;
	subdata->bmap_size = greatest_line_nr / (8 /* bits in a byte */ * sizeof * bkpt_bmap);

	return sdata;
}



/*!
 *	\fn	void srcfile_get_srcinfo(struct gear_engine_context * ctx)
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
void srcfile_get_srcinfo(struct gear_engine_context * ctx)
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
	slist = & srcionfo->srclist;
	for (src = ctx->src_data->srcs; src; src = src->next)
		if ((snode = dump_srcfile_machine_code_details(ctx, src)))
			* slist = snode, snode->next = 0, slist = & snode->next;
}


#endif
