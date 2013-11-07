/*!
 * \file	frame-reg-cache.h
 * \brief	target stack frame operations and register cache support header
 * \author	shopov
 *
 *
 * Revision summary:
 *
 * $Log: $
 */

/*
 *
 * exported data types follow
 *
 */

/*
 *
 * exported function prototypes follow
 *
 */

enum GEAR_ENGINE_ERR_ENUM frame_move_to_relative(struct gear_engine_context * ctx, int amount, int * selected_frame_nr);
void init_frame_reg_cache(struct gear_engine_context * ctx);

