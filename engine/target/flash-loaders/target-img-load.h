/*!
 *	\file	target-img-load.h
 *	\brief	target-img-load.c header file
 *	\author	shopov
 *
 */

/*
 *
 * exported definitions follow
 *
 */

/* flag constant definitions for the target_img_load_elf() load_flags parameter;
 * also see the comments about target_img_load_elf() in module
 * target-img-load.c, and the comments at the beginning of module
 * target-img-load.c */
/*! load image bytes (code or data) residing in target ram */
#define LOAD_RAM	BIT0
/*! load image bytes (code or data) residing in target flash */
#define LOAD_FLASH	BIT1

/*
 *
 * exported function prototypes follow
 *
 */

enum GEAR_ENGINE_ERR_ENUM target_img_load_elf(struct gear_engine_context * ctx, const char * elf_img_name, int load_flags);

