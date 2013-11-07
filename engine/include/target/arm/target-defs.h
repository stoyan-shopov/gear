/*!
 *	\file	target-defs.h
 *	\brief	arm target specific definitions
 *	\author	shopov
 *	
 *	Revision summary:
 *
 *	$Log: $
 */

#ifndef __TARGET_DEFS_H__
#define __TARGET_DEFS_H__

/*! the base core word definition */
typedef unsigned int ARM_CORE_WORD;

/*! various arm-specific constants */
enum
{
	/*! the number of arm core registers */
	NR_ARM_CORE_REGS_	= 16,
	/*! the number of the arm core program counter register */
	ARM_CORE_PC_REG_NR_	= 15,
	/*! the number of the arm core link register (subroutine call return address) */
	ARM_CORE_LR_REG_NR_	= 14,
	/*! the number of the arm core stack pointer register */
	ARM_CORE_SP_REG_NR_	= 13,
};

#endif /* __TARGET_DEFS_H__ */

