int arm_disassemble_insn(struct gear_engine_context * ctx, ARM_CORE_WORD addr,
		int (*print_fn)(const char * format, va_list ap));
void init_arm_disassemble(void);
