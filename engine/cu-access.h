/*!
 *	\file	cu-access.h
 *	\brief	compilation unit access header
 *	\author	shopov
 *	
 *	Revision summary:
 *
 *	here, described is in brief how the gear reads debug
 *	information for some file (be it an executable or
 *	a shared library); first, some benchmark results
 *	are given; the benchmark shows the time and memory
 *	that the gear consumes in order
 *	to load the QtGuid4.dll file (details for memory
 *	section sizes of this file obtained via objdump
 *	are given below);
 *	4 tests have been performed - in all tests the
 *	debug information is being read by scanning
 *	consecutive compilation units and scanning and
 *	(depending on their type) reading compilation unit
 *	die children; in all tests DW_TAG_variable dies
 *	which are immediate children of a compilation unit
 *	(DW_TAG_compile_unit) are always being fully read
 *	(i.e. read is its type, a specification die if
 *	appropriate, location information, etc.);
 *	briefly the tests are:
1 - types not read, subprograms only partially read - read
	is the name, the address range(s) and others, but
	not return type, formal parameter(s), lexical
	block, local variables
2 - types not read, subprograms fully read
3 - types read, subprograms only partially read - as in test 1
4 - types read, subprograms fully read

for test results, read below


QtGuid4.dll: 347 313 025 bytes

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         00807980  68f41000  68f41000  00000600  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE, DATA
  1 .data         0012066c  69749000  69749000  00808000  2**5
                  CONTENTS, ALLOC, LOAD, DATA
  2 .rdata        00002460  6986a000  6986a000  00928800  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .eh_frame     00b94f74  6986d000  6986d000  0092ae00  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  4 .bss          000040a0  6a402000  6a402000  00000000  2**5
                  ALLOC
  5 .edata        000ab48b  6a407000  6a407000  014bfe00  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  6 .idata        0000ef00  6a4b3000  6a4b3000  0156b400  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  7 .rsrc         00000324  6a4c2000  6a4c2000  0157a400  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  8 .reloc        000dfbf0  6a4c3000  6a4c3000  0157a800  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  9 .debug_aranges 003d5478  6a5a3000  6a5a3000  0165a400  2**3
                  CONTENTS, READONLY, DEBUGGING
 10 .debug_pubnames 00e59461  6a979000  6a979000  01a2fa00  2**0
                  CONTENTS, READONLY, DEBUGGING
 11 .debug_pubtypes 001ec0ad  6b7d3000  6b7d3000  02889000  2**0
                  CONTENTS, READONLY, DEBUGGING
 12 .debug_info   0d1e7c12  6b9c0000  6b9c0000  02a75200  2**0
                  CONTENTS, READONLY, DEBUGGING
 13 .debug_abbrev 0017fb50  78ba8000  78ba8000  0fc5d000  2**0
                  CONTENTS, READONLY, DEBUGGING
 14 .debug_line   00c7af6c  78d28000  78d28000  0fddcc00  2**0
                  CONTENTS, READONLY, DEBUGGING
 15 .debug_frame  0109e49c  799a3000  799a3000  10a57c00  2**2
                  CONTENTS, READONLY, DEBUGGING
 16 .debug_str    00309dc0  7aa42000  7aa42000  11af6200  2**0
                  CONTENTS, READONLY, DEBUGGING
 17 .debug_loc    01be9370  7ad4c000  7ad4c000  11e00000  2**0
                  CONTENTS, READONLY, DEBUGGING
 18 .debug_ranges 004089f0  7c936000  7c936000  139e9400  2**0
                  CONTENTS, READONLY, DEBUGGING


-------- test 1 --------
basic subs, no types:
0.960 G; 1:30

dtype_data usage counts:
nr_base_type_nodes == 4270
nr_typedef_nodes == 1711
nr_tqual_nodes == 3983
nr_arr_type_nodes == 1762
nr_ptr_nodes == 2092
nr_struct_nodes == 1066
nr_union_nodes == 424
nr_member_nodes == 4947
nr_enumerator_nodes == 3280
nr_enumeration_nodes == 342
nr_dobj_nodes == 14815
nr_subprogram_nodes == 665637
nr_lexblocks == 0
nr_subprogram_prototype_nodes == 593279
nr_symtab_nodes == 586275

-------- test 2 --------
subs, no types:
1.844 G; 06:00

dtype_data usage counts:
nr_base_type_nodes == 13622
nr_typedef_nodes == 69612
nr_tqual_nodes == 385175
nr_arr_type_nodes == 17715
nr_ptr_nodes == 227072
nr_struct_nodes == 64750
nr_union_nodes == 14795
nr_member_nodes == 249748
nr_enumerator_nodes == 593954
nr_enumeration_nodes == 12846
nr_dobj_nodes == 5664402
nr_subprogram_nodes == 670385
nr_lexblocks == 52465
nr_subprogram_prototype_nodes == 2125693
nr_symtab_nodes == 598779

-------- test 3 --------
basic subs, types:
1.396 G; 03:30

dtype_data usage counts:
nr_base_type_nodes == 12748
nr_typedef_nodes == 54343
nr_tqual_nodes == 195362
nr_arr_type_nodes == 18648
nr_ptr_nodes == 258833
nr_struct_nodes == 75327
nr_union_nodes == 12276
nr_member_nodes == 259134
nr_enumerator_nodes == 221029
nr_enumeration_nodes == 12750
nr_dobj_nodes == 14815
nr_subprogram_nodes == 672231
nr_lexblocks == 0
nr_subprogram_prototype_nodes == 2156276
nr_symtab_nodes == 598683

-------- test 4 --------
subs, types:
1.883 G; 06:18

dtype_data usage counts:
nr_base_type_nodes == 13636
nr_typedef_nodes == 91975
nr_tqual_nodes == 414608
nr_arr_type_nodes == 18648
nr_ptr_nodes == 258833
nr_struct_nodes == 75614
nr_union_nodes == 15259
nr_member_nodes == 265162
nr_enumerator_nodes == 602063
nr_enumeration_nodes == 12931
nr_dobj_nodes == 5723604
nr_subprogram_nodes == 672231
nr_lexblocks == 52465
nr_subprogram_prototype_nodes == 2156276
nr_symtab_nodes == 598864
---------------------------------

 * as it is normal to expect, the results from test (2)
 * and test (4) differ insignificantly - this can be explained
 * this way: even though type information is not
 * read in cu-access.h, the majority of the type
 * information is still getting read when fully reading
 * subprogram dies, the minor increase in required
 * memory and time in test (4), with regard to test (2),
 * is due to reading type information for types that
 * are actually not found to be referenced by some die
 * when reading subprogram dies;
 * \todo	finish this
 *
 *
 *	$Log: $
 */

/*
 *
 * exported definitions follow
 *
 */

/*
 *
 * exported types follow
 *
 */

/*! compilation unit information holding data structure
 *
 * \note	this data structure also used to contain
 *		a list of the subprograms for the compilation
 *		unit described, but that required that the
 *		complete information for subprogram dies
 *		be read when processing compilation units
 *		(in cu-access.c); as of time of writing
 *		this (30092010) type information (about
 *		structures, classes, enumerations, etc.)
 *		is not explicitly read in cu-access.c */
struct cu_data
{
	/*! the head of this structure */
	struct dwarf_head_struct	head;
	/*! version stamp for this compilation unit */
	Dwarf_Half	version_stamp;
	/*! a list of subroutines in the compilation unit */
	struct subprogram_data *	subs;
	/*! the table of address range(s) that this compilation unit covers */
	struct dwarf_ranges_struct * addr_ranges;
	/*! default compilation unit base address for use in dwarf location lists and range lists (read below)
	 *
	 * the dwarf3 standard document, dated December 20, 2005,
	 * section 3.1.1 Normal and Partial Compilation Unit Entries,
	 * on page 38 says:
	 *
	 * Compilation unit entries may have the following attributes:
	 * 1. Either a DW_AT_low_pc and DW_AT_high_pc pair of attributes or a DW_AT_ranges attribute whose values encode the contiguous or non-contiguous address ranges, respectively, of the machine instructions generated for the compilation unit (see Section 2.17).
	 * A DW_AT_low_pc attribute may also be specified in combination with DW_AT_ranges to specify the default base address for use in location lists (see Section 2.6.6) and range lists (see Section 2.17.3).
	 */
	Dwarf_Addr	default_cu_base_address;
	/*! name of the primary source file from which this compilation unit was derived */
	char		* name;
	/*! language of the source file for this compilation unit */
	Dwarf_Unsigned	language;
	/*! a list of variables in the compilation unit */
	struct dobj_data *		vars;

	/* source and line number data fields follow
	 *
	 * \todo	preprocess and store only the data
	 *		that is indeed of interest to us, i.e.
	 *		source file names, line numbers, core
	 *		addresses and end-of-sequence flags;
	 *		accessing this information via libdwarf is pure
	 *		overkill and is orders of magnitude
	 *		slower; it is very important that
	 *		the line number information is sent
	 *		in core-address-ascending order as
	 *		a frontend generally expects this */

	/*! source line number information for the compilation unit
	 *
	 * see the dwarf_srclines() documentation in libdwarf for details */
	Dwarf_Line *			linebuf;
	/*! the number of elements in the ::linebuf array above
	 *
	 * see the dwarf_srclines() documentation in libdwarf for details */
	Dwarf_Signed			linecount;
	/*! the current working directory of the compilation command that produced this comilation unit
	 *
	 * this is the 'DW_AT_comp_dir' attribute value (a null terminated string) - if any - associated
	 * with this compilation unit */
	char	* comp_dir;
	/*! the set of file names contributing to the line number information for the compilation unit
	 *
	 * see the dwarf_srcfiles() documentation in libdwarf for details */
	char **				srcfiles;
	/*! the number of elements in the ::srcfiles array above
	 *
	 * see the dwarf_srcfiles() documentation in libdwarf for details */
	Dwarf_Signed			srccount;
};

/*
 *
 * exported function prototypes follow
 *
 */
struct cu_data * cu_process(struct gear_engine_context * ctx, Dwarf_Unsigned cu_die_offset);
struct subprogram_data * cu_get_subprogram_list(struct gear_engine_context, struct cu_data * cu);

