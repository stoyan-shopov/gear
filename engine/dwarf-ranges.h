/*!
 *	\file	dwarf-ranges.h
 *	\brief	dwarf address range support related data structure definitions
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * exported data functrion prototypes follow
 */

struct dwarf_ranges_struct * dwarf_ranges_get_ranges(struct gear_engine_context * ctx,
		Dwarf_Die die);
/*! \todo	change the ARM_CORE_WORD-s to Dwarf_Addr-s */
bool dwarf_ranges_is_in_range(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges, ARM_CORE_WORD addr, ARM_CORE_WORD cu_base_addr);
int dwarf_ranges_get_range_count(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges);
/*! \todo	this interface is flawed; arrange for the function to returned an indication
 *		that special range entries (e.g. address selection entries) have been encountered */ 
/*! \todo	change the ARM_CORE_WORD-s to Dwarf_Addr-s */
void dwarf_ranges_get_range_data_at_idx(struct gear_engine_context * ctx, struct dwarf_ranges_struct * ranges, int range_idx, ARM_CORE_WORD cu_base_address, ARM_CORE_WORD * low_addr, ARM_CORE_WORD * hi_addr);
