/*!
 * \file	srcfile.h
 * \brief	srcfile.c header
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

void srcfile_build_src_cu_tab(struct gear_engine_context * ctx);
/*
void srcfile_dump_source_info_mi(struct gear_engine_context * ctx);
void srcfile_dump_srcinfo_mi(struct gear_engine_context * ctx, const char * srcname);
*/
void srcfile_get_srcinfo_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr,
		struct cu_data ** cu, struct subprogram_data ** subp,
		char ** srcname, int * srcline_nr, bool * is_addr_at_src_boundary);
void srcfile_dump_sources_info_mi(struct gear_engine_context * ctx);
enum GEAR_ENGINE_ERR_ENUM srcfile_get_core_addr_for_line_nr(struct gear_engine_context * ctx,
		char * srcname, int line_nr, ARM_CORE_WORD * addr);

void srcfile_disassemble_addr_range(struct gear_engine_context * ctx,
		bool is_disassembling_insn_count,
		ARM_CORE_WORD start_addr,
		ARM_CORE_WORD first_addr_past_range_or_insn_count,
		ARM_CORE_WORD value_to_dump_for_program_counter);

