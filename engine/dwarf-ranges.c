/*!
 *	\file	dwarf-ranges.h
 *	\brief	dwarf address range support routines
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	this module provides support for handling dwarf address
 *	range related operations; following is a verbatim excerpt
 *	from the dwarf 3 standard document:

<<< start of verbatim dwarf3 standard document excerpt >>>

2.17 Code Addresses and Ranges
Any debugging information entry describing an entity that has a machine code address or range of machine code addresses, which includes compilation units, module initialization, subroutines, ordinary blocks, try/catch blocks, labels and the like, may have
• A DW_AT_low_pc attribute for a single address,
• A DW_AT_low_pc and DW_AT_high_pc pair of attributes for a single contiguous range of addresses, or
• A DW_AT_ranges attribute for a non-contiguous range of addresses.
If an entity has no associated machine code, none of these attributes are specified.
2.17.1 Single Address
When there is a single address associated with an entity, such as a label or alternate entry point of a subprogram, the entry has a DW_AT_low_pc attribute whose value is the relocated address for the entity.
GENERAL DESCRIPTION
December 20, 2005 Page 33
While the DW_AT_entry_pc attribute might also seem appropriate for this purpose, historically the DW_AT_low_pc attribute was used before the DW_AT_entry_pc was introduced (in DWARF Version 3). There is insufficient reason to change this.
2.17.2 Contiguous Address Range
When the set of addresses of a debugging information entry can be described as a single continguous range, the entry may have a DW_AT_low_pc and DW_AT_high_pc pair of attributes. The value of the DW_AT_low_pc attribute is the relocated address of the first instruction associated with the entity, and the value of the DW_AT_high_pc is the relocated address of the first location past the last instruction associated with the entity.
The high PC value may be beyond the last valid instruction in the executable.
The presence of low and high PC attributes for an entity implies that the code generated for the entity is contiguous and exists totally within the boundaries specified by those two attributes. If that is not the case, no low and high PC attributes should be produced.
2.17.3 Non-Contiguous Address Ranges
When the set of addresses of a debugging information entry cannot be described as a single contiguous range, the entry has a DW_AT_ranges attribute whose value is of class rangelistptr and indicates the beginning of a range list.
Range lists are contained in a separate object file section called .debug_ranges. A range list is indicated by a DW_AT_ranges attribute whose value is represented as an offset from the beginning of the .debug_ranges section to the beginning of the range list.
Each entry in a range list is either a range list entry, a base address selection entry, or an end of list entry.
A range list entry consists of:
1. A beginning address offset. This address offset has the size of an address and is relative to the applicable base address of the compilation unit referencing this range list. It marks the beginning of an address range.
2. An ending address offset. This address offset again has the size of an address and is relative to the applicable base address of the compilation unit referencing this range list. It marks the first address past the end of the address range.The ending address must be greater than or equal to the beginning address.
DWARF Debugging Information Format, Version 3
Page 34 December 20, 2005
A range list entry (but not a base address selection or end of list entry) whose beginning and ending addresses are equal has no effect because the size of the range covered by such an entry is zero.
The applicable base address of a range list entry is determined by the closest preceding base address selection entry (see below) in the same range list. If there is no such selection entry, then the applicable base address defaults to the base address of the compilation unit (see Section 3.1).
In the case of a compilation unit where all of the machine code is contained in a single contiguous section, no base address selection entry is needed.
Address range entries in a range list may not overlap. There is no requirement that the entries be ordered in any particular way.
A base address selection entry consists of:
1. The value of the largest representable address offset (for example, 0xffffffff when the size of an address is 32 bits).
2. An address, which defines the appropriate base address for use in interpreting the beginning and ending address offsets of subsequent entries of the location list.
A base address selection entry affects only the list in which it is contained.
The end of any given range list is marked by an end of list entry, which consists of a 0 for the beginning address offset and a 0 for the ending address offset. A range list containing only an end of list entry describes an empty scope (which contains no instructions).
A base address selection entry and an end of list entry for a range list are identical to a base address selection entry and end of list entry, respectively, for a location list (see Section 2.6.6) in interpretation and representation.

<<< start of verbatim dwarf3 standard document excerpt >>>
 *
 *	in short:

Any debugging information entry describing an entity that has a machine code address or range of machine code addresses, which includes compilation units, module initialization, subroutines, ordinary blocks, try/catch blocks, labels and the like, may have
• A DW_AT_low_pc attribute for a single address,
• A DW_AT_low_pc and DW_AT_high_pc pair of attributes for a single contiguous range of addresses, or
• A DW_AT_ranges attribute for a non-contiguous range of addresses.
If an entity has no associated machine code, none of these attributes are specified.

 *	so there are mainly three cases to consider:
 *	- an entity is comprised of only a single code address (e.g. labels)
 *		\todo	these are currently not handled properly at all
 *	- an entity is comprised of a single, contiguous range of code addresses
 *		(e.g. functions, lexical blocks, compilation units...) -
 *		in this case the single address range spanned is given
 *		by a pair of *relocated* addresses; this is the most common case
 *		in practice
 *	- an entity is comprised of multiple address ranges (e.g. functions,
 *		(lexical blocks, compilation units...) - in this case the multiple
 *		address ranges spanned are specified as a list of contiguous address
 *		range pairs - the pairs being given as (highlighted by me(sgs)):

1. A beginning address *offset*. This address *offset* has the size of an address and is *relative* to the applicable *base address* of the compilation unit referencing this range list. It marks the beginning of an address range.
2. An ending address *offset*. This address *offset* again has the size of an address and is *relative* to the applicable *base address* of the compilation unit referencing this range list. It marks the first address past the end of the address range.The ending address must be greater than or equal to the beginning address.
 *
 *	the key words in the paragraphs above are *relocated* and *relative*;
 *	in order to simplify processing - a single, contiguous
 *	address range can be described by (and is handled in this module as)
 *	a range list of a single entry - so if a dwarf debug information entity
 *	only has associated with it a single, contiguous code address range
 *	(described by the DW_AT_low_pc and DW_AT_high_pc - which are relocated
 *	address values, and do not need any further relocations applied prior to
 *	their use) - such an entity has its address range artificially
 *	transformed to a range list entry (which, however, must have their addresses
 *	given as *offsets* from a compilation unit base address, and therefore
 *	need relocation prior to their use), so:
 *	the current code uses a special flag (namely, the 'is_single_relocated_addr_range'
 *	flag) in the 'dwarf_ranges_struct' below associated
 *	with dwarf information entities that possess address range information;
 *	this seems like a poor decision, though, and one solution to this problem,
 *	for example, could be the addition of an extra parameter to
 *	dwarf_ranges_get_ranges(), which will then automatically apply the
 *	necessary offsetting to the range list entries, so that these lists
 *	always have their address ranges given as final, relocated values; however,
 *	as of time of writing this note, this is not too straightforward to do...
 *	also, this way the stored information more closely corresponds to what
 *	actually is recorded in the dwarf debug information, which may
 *	be useful while still heavily debugging the gear...
 *
 *	\todo	think about and maybe code the solution proposed above
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
#include "gear-engine-context.h"
#include "target-defs.h"
#include "dwarf-ranges.h"

#include "util.h"


/*
 *
 * local data types follow
 *
 */

/*! dwarf address ranges data structure
 *
 * this is used to describe the address range(s) of
 * dwarf debug information entries that possess such
 * data associated with them:
 * - compilation units,
 * - partial units (currently not supported)
 * - modules (currently not supported)
 * - 'with' statements (currently not supported)
 * - subprograms (including inlined subroutines - concrete
 *	inline instances)
 * - c++ try/catch blocks
 * - lexical blocks
 *
 */
struct dwarf_ranges_struct
{
	union
	{
		struct
		{
			/*! the ranges table itself
			 * 
			 * \note	applicable (along with the field below) only
			 *		if the is_single_relocated_addr_range below is 0
			 *
			 * the table is retrieved via the libdwarf dwarf_get_ranges_a()
			 * function; the type Dwarf_Ranges type is defined in libdwarf.h:
			 *
			 *	enum Dwarf_Ranges_Entry_Type { DW_RANGES_ENTRY, 
			 *	    DW_RANGES_ADDRESS_SELECTION,
			 *	    DW_RANGES_END };
			 *
			 *	typedef struct {
			 *	    Dwarf_Addr dwr_addr1;
			 *	    Dwarf_Addr dwr_addr2; 
			 *	    enum Dwarf_Ranges_Entry_Type  dwr_type;
			 *	} Dwarf_Ranges;
			 *
			 * the entries in this table are given as address pairs that need
			 * to be offset by the compilation unit base address associated
			 * with the entity possessing this range list prior to their use;
			 * also read the comments at the beginning of this file */
			Dwarf_Ranges	* ranges;
			/*! the number of entries in the table above
			 * 
			 * \note	applicable (along with the field above) only
			 *		if the is_single_relocated_addr_range below is 0 */
			Dwarf_Signed	nr_ranges;
		};
		/*! the range table, in the most common case that it contains only one entry
		 * 
		 * applicable only if the is_single_relocated_addr_range below is 1
		 *
		 * this single address pair entry contains final, relocated
		 * addresses (in contrast to the 'ranges' table above), that
		 * do not need any further relocations prior to their use;
		 * also read the comments at the beginning of this file
		 */
		Dwarf_Ranges	single_range;
	};
	/*! a special-case flag denoting if the range list entries need relocations applied prior to their use
	 *
	 * - when 1, this flag denotes that this range list consists of a single,
	 *	contiguous address range entry which needs no further relocations
	 *	applied prior to its use; as there is only one address range entry
	 *	in this case, it is directly given in the 'single_range' union
	 *	member above, and the 'ranges' union member is not applicable
	 * - when 0, this flag denotes that the range list entry/entries need
	 *	to be offset by the compilation unit base address associated
	 *	with the entity possessing this range list prior to their use;
	 *	in this case, the range list is given by the 'ranges' union
	 *	member above and the 'single_range' union member above
	 *	is not applicable
	 *
	 * also read the comments at the beginning of this file */
	bool	is_single_relocated_addr_range;
};


/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	struct dwarf_ranges_struct * dwarf_ranges_get_ranges(struct gear_engine_context * ctx, Dwarf_Die die)
 *	\brief	retrieves the address ranges associated with the dwarf die of interest
 *
 *	the debug information entry passed is examined if it has a
 *	DW_AT_ranges attribute, and if so - the ranges associated
 *	with this die are retrieved via the libdwarf function
 *	dwarf_get_ranges_a() and this table is returned;
 *	if the die does not have a DW_AT_ranges	attribute,
 *	but still has a pair of DW_AT_low_pc/DW_AT_high_pc
 *	attributes, a table of just one entry (consisting
 *	of the single address range [DW_AT_low_pc; DW_AT_high_pc))
 *	is built, and this table is returned;
 *	if the die has neither a DW_AT_ranges attribute, nor a
 *	DW_AT_low_pc/DW_AT_high_pc attributes pair, 0 is returned
 *
 *	\note	it is an error if the die has both a DW_AT_ranges
 *		attribute, and a DW_AT_low_pc/DW_AT_high_pc attributes pair,
 *		and 0 is returned in this case as well
 *
 *	\param	ctx	context to work in
 *	\param	die	the debug information entry of interest
 *			for which to retrieve address range information
 *	\return	the address ranges table associated with the die passed
 *		if address information is available for this die, 0 otherwise
 */
struct dwarf_ranges_struct * dwarf_ranges_get_ranges(struct gear_engine_context * ctx, Dwarf_Die die)
{
Dwarf_Error err;
Dwarf_Bool range_flag, low_pc_flag, hi_pc_flag;
Dwarf_Attribute attr;
Dwarf_Addr low_pc, hi_pc;
Dwarf_Unsigned offset;
Dwarf_Half form;
struct dwarf_ranges_struct * p;

	p = 0;
	/* retrieve flags */
	if (dwarf_hasattr(die, DW_AT_ranges, &range_flag, &err) != DW_DLV_OK)
		panic("");
	if (range_flag)
	{
		if (!(p = calloc(1, sizeof * p)))
			panic("out of core");
		if (dwarf_attr(die, DW_AT_ranges, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the rangelistptr */
		if (dwarf_whatform(attr, & form, & err) != DW_DLV_OK)
			panic("");
		switch (form)
		{
			case DW_FORM_data4:
				if (dwarf_formudata(attr, &offset, &err) != DW_DLV_OK)
					panic("");
				break;
			case DW_FORM_sec_offset:
				if (dwarf_global_formref(attr, &offset, &err) != DW_DLV_OK)
					panic("");
				break;
			default:
				panic("");
		}
		if (dwarf_get_ranges_a(ctx->dbg, offset, die, &p->ranges, &p->nr_ranges, 0, &err)
				!= DW_DLV_OK)
			panic("");
		/* don't count the end of list entry (if present),
		 * it is not really useful and should be ignored */  
		if (p->ranges[p->nr_ranges - 1].dwr_type == DW_RANGES_END)
		{
			if (!--(p->nr_ranges))
				panic("");
		}
	}
	if (dwarf_hasattr(die, DW_AT_low_pc, &low_pc_flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (low_pc_flag)
	{
		if (dwarf_lowpc(die, &low_pc, &err)!= DW_DLV_OK )
			panic("dwarf_lowpc()");
	}
	if (dwarf_hasattr(die, DW_AT_high_pc, &hi_pc_flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (hi_pc_flag)
	{
		if (dwarf_highpc(die, &hi_pc, &err)!= DW_DLV_OK )
			panic("dwarf_highpc()");
	}
	if (!((range_flag && /* allowed for compilation units,
				* in this case, the low pc
				* value gives the compilation
				* unit base address value for use
				* in location lists and range
				* lists */ low_pc_flag && !hi_pc_flag)
			|| /* a common case - the die processed
			      describes an object that occupies a
			      contiguous memory region */
				(!range_flag && low_pc_flag && hi_pc_flag)
			|| /* another common case - the die processed
			      describes an object that occupies a
			      discontiguous memory region */
				(range_flag && !low_pc_flag && !hi_pc_flag)
			|| /* also a common case - the die processed does
			      describes an object that occupies no
			      memory */
				(!range_flag && !low_pc_flag && !hi_pc_flag)))
	{
{ Dwarf_Off x; if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic(""); printf("cu relative offset: %i\n", (int) x); }
		panic("");
	}

	if (low_pc_flag && hi_pc_flag)
	{
		/* a special case - a single address range */
		if (!(p = calloc(1, sizeof * p)))
			panic("out of core");
		p->is_single_relocated_addr_range = true;
		p->single_range.dwr_addr1 = low_pc;
		p->single_range.dwr_addr2 = hi_pc;
		p->single_range.dwr_type = DW_RANGES_ENTRY;
	}
	return p;
}


/*!
 *	\fn	bool dwarf_ranges_is_in_range(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges, ARM_CORE_WORD addr, ARM_CORE_WORD cu_base_addr)
 *	\brief	determines if an address is within the bounds of a given address range
 *
 *	\todo	change the ARM_CORE_WORD-s to Dwarf_Addr-s
 *
 *
 *	\param	ctx	context to work in
 *	\param	ranges	the ranges to scan, must have been returned
 *			by a dwarf_ranges_get_ranges()
 *	\param	addr	the address of interest
 *	\param	cu_base_addr	compilation unit base address of the
 *				die to which the ranges table passed belongs
 *	\return	true, if the address passed is within the ranges passed,
 *		false otherwise */
bool dwarf_ranges_is_in_range(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges, ARM_CORE_WORD addr, ARM_CORE_WORD cu_base_addr)
{
int i;

	if (!ranges)
		return false;

	if (ranges->is_single_relocated_addr_range)
	{
		/* a single address range */
		if (ranges->single_range.dwr_addr1 <= addr
				&& addr < ranges->single_range.dwr_addr2)
			return true;
	}
	else
	{
		/* multiple address ranges */
		for (i = 0; i < ranges->nr_ranges; i++)
		{
			switch (ranges->ranges[i].dwr_type)
			{
				case DW_RANGES_ENTRY:
					if (ranges->ranges[i].dwr_addr1 + cu_base_addr <= addr
							&& addr < ranges->ranges[i].dwr_addr2 + cu_base_addr)
						return true;
					break;
				case DW_RANGES_ADDRESS_SELECTION:
					panic("");
					cu_base_addr = ranges->ranges[i].dwr_addr1;
					break;
				case DW_RANGES_END:
					/* should be impossible, as end of list entries
					 * are being ignored when building the ranges
					 * tables in the dwarf_ranges_get_ranges()
					 * function above */	 
					panic("");
					break;
				default:
					panic("");
			}
		}
	}
	return false;
}



/*!
 *	\fn	int dwarf_ranges_get_range_count(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges)
 *	\brief	returns the number of contiguous address ranges contained in the address range structure passed
 *	\note	data about entries in the 'ranges' list should be obtained by calling dwarf_ranges_get_range_data_at_idx()
 *	\todo	the number returned currently includes also special dwarf address range list
 *		entries (e.g., end-of-list, base address selection) - maybe handle these in
 *		some more appropriate manner
 *
 *	\param	ctx	context to work in
 *	\param	ranges	the address ranges structure of interest
 *	\return	the number of contiguous address ranges contained
 *		in the address range structure passed */
int dwarf_ranges_get_range_count(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges)
{
	if (!ranges)
		return 0;
	return ranges->is_single_relocated_addr_range ? 1 : ranges->nr_ranges;
}


/*!
 *	\fn	void dwarf_ranges_get_range_data_at_idx(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges, int range_idx, ARM_CORE_WORD cu_base_address, ARM_CORE_WORD * low_addr, ARM_CORE_WORD * hi_addr)
 *	\brief	retrieves the address range data associated with the 'ranges' list passed, at index 'range_idx'
 *	\note	the number of address ranges in the list should be retrieved by caling dwarf_ranges_get_range_count()
 *	\todo	any special dwarf address range list entries
 *		(e.g., end-of-list, base address selection) must be handled in
 *		some appropriate manner
 *	\todo	change the ARM_CORE_WORD-s to Dwarf_Addr-s
 *
 *	\param	ctx	context to work in
 *	\param	ranges	the range list of interest
 *	\param	range_idx	the index of the range list entry of interest
 *	\param	cu_base_addr	compilation unit base address of the
 *				die to which the ranges table passed belongs
 *	\param	low_addr	a pointer to where to store the lowest (first)
 *				address spanned by the range list entry
 *				of interest; this address is included in the
 *				address range spanned by this range list entry
 *	\param	hi_addr		a pointer to where to store the first address past
 *				the memory range spanned by the range list entry
 *				of interest; this address is not belonging to the
 *				address range spanned by this range list entry
 *	\return	none */
void dwarf_ranges_get_range_data_at_idx(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges, int range_idx, ARM_CORE_WORD cu_base_address, ARM_CORE_WORD * low_addr, ARM_CORE_WORD * hi_addr)
{
	if (ranges->is_single_relocated_addr_range)
	{
		if (range_idx != 0)
			panic("");
		/* a single address range */
		* low_addr = ranges->single_range.dwr_addr1;
		* hi_addr = ranges->single_range.dwr_addr2;
	}
	else
	{
		if (range_idx >= ranges->nr_ranges)
			panic("");
		/* multiple address ranges */
		if (ranges->ranges[range_idx].dwr_type != DW_RANGES_ENTRY)
			panic("");
		* low_addr = ranges->ranges[range_idx].dwr_addr1 + cu_base_address;
		* hi_addr = ranges->ranges[range_idx].dwr_addr2 + cu_base_address;
	}
}

