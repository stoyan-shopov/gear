/*!
 * \file	dwarf-loc.h
 * \brief	dwarf location expression manipulation header file
 * \author	shopov
 *
 *
 * Revision summary:
 *
 * $Log: $
 */

#ifndef __DWARF_LOC_H__
#define __DWARF_LOC_H__

/*
 *
 * exported types follow
 *
 */

/*! a generic dwarf location information data structure
 *
 * this is designed to hold a location list, even though
 * the location described might be a single
 * location expression; it is designed to be used with
 * dwarf_loclist_n() - see the libdwarf documentation
 * for details */
struct dwarf_location
{
	/*! the location list buffer */
	Dwarf_Locdesc		** llbuf;
	/*! the number of elements in the location list buffer */
	Dwarf_Signed		listlen;
};

/*
 *
 * exported function prototypes follow
 *
 */
enum GEAR_ENGINE_ERR_ENUM dwarf_loc_eval_loc_from_list(struct gear_engine_context * ctx,
		ARM_CORE_WORD * result, bool * is_result_a_register,
		enum DWARF_EXPR_INFO_ENUM * eval_info,
		struct dwarf_location * loclist, ARM_CORE_WORD addr,
	       	ARM_CORE_WORD cu_base_addr, Dwarf_Unsigned fbreg);

bool dwarf_loc_are_locations_the_same(struct gear_engine_context * ctx, struct dwarf_location * loc1, struct dwarf_location * loc2);


#endif /* __DWARF_LOC_H__ */

