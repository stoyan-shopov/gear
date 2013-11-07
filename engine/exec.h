/*!
 *	\file	exec.h
 *	\brief	executor header file
 *	\author	shopov
 *	
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * exported function declarations follow
 *
 */ 

void exec_single_step_insn(struct gear_engine_context * ctx);
void exec_step_over_insn(struct gear_engine_context * ctx);
void exec_single_step_src(struct gear_engine_context * ctx);
void exec_step_over_src(struct gear_engine_context * ctx);
void exec_target_run(struct gear_engine_context * ctx);
void exec_target_halt(struct gear_engine_context * ctx);
enum TARGET_CORE_STATE_ENUM exec_get_target_state(struct gear_engine_context * ctx);
void init_exec(struct gear_engine_context * ctx);

