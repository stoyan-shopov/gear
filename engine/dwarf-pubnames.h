/*!
 *	\file	dwarf-pubnames.h
 *	\brief	provides support for handling the dwarf global symbols section, .debug_pubnames
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *  
 * exported function prototypes follow
 *
 */

void init_dwarf_pubnames(struct gear_engine_context * ctx);
int dwarf_pubnames_scan(char * name, Dwarf_Off * die_offset, Dwarf_Off * cu_offset);

