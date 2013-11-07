/*! maximum identifier length that can be supplied by the lexer */
#define MAX_LEXER_ID_LEN		1024

struct format_data
{
	char	print_format;
	/* this one in bytes */
	int	data_size;
	int	nr_elements;
};


extern char cmd_line[1024];
extern int cmd_idx;
extern struct format_data	def_exam_format;
extern ARM_CORE_WORD def_exam_addr;

void init_gear(void);

void dump_mem(struct gear_engine_context * ctx);
void dump_core_regs_mi(struct gear_engine_context * ctx);
void set_reg(struct gear_engine_context * ctx, int reg_nr, ARM_CORE_WORD val);
ARM_CORE_WORD get_reg(struct gear_engine_context * ctx, int reg_nr);
void insn_step(struct gear_engine_context * ctx);
void dump_exec_context(struct gear_engine_context * ctx);
void set_break(struct gear_engine_context * ctx, ARM_CORE_WORD addr);
void show_breakpoints(void);
void clear_break(struct gear_engine_context * ctx, int brekpoint_nr);
void code_run(struct gear_engine_context * ctx);
void dump_backtrace(struct gear_engine_context * ctx);

