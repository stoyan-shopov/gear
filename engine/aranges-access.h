/*!
 * \file	aranges-access.h
 * \brief	.debug_aranges address ranges elf section access header file
 * \author	shopov
 *
 *
 * Revision summary:
 *
 * $Log: $
 */

/*
 *
 * exported function prototypes follow
 *
 */
struct cu_data * aranges_get_cu_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr);
struct subprogram_data * aranges_get_subp_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr);
struct lexblock_data * aranges_get_lexblock_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr);
void init_aranges(struct gear_engine_context * ctx);

