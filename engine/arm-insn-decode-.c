/*!
 *	\file	arm-insn-decode.c
 *	\brief	arm instruction decoder (arm mode)
 *	\author	shopov
 *
 *	this file contains a simple instruction decoder which
 *	attempts to detect if an instruction changes the instruction
 *	pointer, and if so, attempts to compute the new value for
 *	the instruction pointer; this information is needed by
 *	the executor for things such as single stepping; above, the
 *	word 'attempts' has been deliberately used, because the
 *	instruction decoder needs to access target memory on some
 *	occasions in order to compute the new value for the program
 *	counter, and this can have potentially disastrous effects
 *	in the case of volatile memory; nonexistant memory is
 *	currently not handled too
 *
 *	\note	this module accesses target memory
 *
 *	\todo	this module accesses target memory; code some
 *		protection/checking mechanism for accessing volatile
 *		and/or unavailable regions of target memory
 *		when support for for target memory mapping
 *		is coded in the gear; fix the comments above
 *		when this is done
 *
 *
 *	Revision summary:
 *
 *	$Log: $
 */


/* an arm instruction encoding template
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |
 */

/*
 *
 * include section follows
 *
 */

#include <stdarg.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gprintf.h"
#include "gear-constants.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "core-access.h"
#include "util.h"

/*
 *
 * local definitions follow
 *
 */

/*! returns the bit stride of a word, between the __start-th and the __end-th bits, inclusive
 *
 * here, __end must be greater than or equal to __start */
#define GET_BITS(__start, __end, __data) \
	(((__data) >> (__start)) & ((1 << ((__end) - (__start) + 1)) - 1))
/*! extracts a single bit from a data word */
#define GET_BIT(__x, __data) (((__data) >> (__x)) & 1)


/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn	static ARM_CORE_WORD core_reg_read(struct gear_engine_context * ctx, int reg_nr)
 *	\brief	reads a general purpose arm core register performing program counter fixup (if needed)
 *
 *	this function is used within this module to read a general purpose
 *	arm core register, performing a fixup to the register value in
 *	the case that the program counter (r15) is read - this means
 *	that whenever the pc is read, two instruction lengths (2 * 4 = 8 bytes)
 *	are added to the value read to make up for the arm pipeline
 *	prefetching
 *
 *	\param	ctx	context to work in
 *	\param	reg_nr	core register number, must be less than 16
 *	\return	the value of the arm core register number reg_nr, fixed
 *		up in the case of program counter read */
static ARM_CORE_WORD core_reg_read(struct gear_engine_context * ctx, int reg_nr)
{
ARM_CORE_WORD r;

	if (reg_nr < 0 || reg_nr > 15)
		panic("");
	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx, 0, 1 << reg_nr, &r) !=
			GEAR_ERR_NO_ERROR)
		panic("");
	if (reg_nr == 15)
		r += 2 * 4;
	return r;

}

/*!
 *	\fn	static int cond_code_check(ARM_CORE_WORD psr, ARM_CORE_WORD insn)
 *	\brief	checks if the condition for execution of an arm instruction is fulfilled
 *
 *	\param	psr	program status register containing
 *			the flag values to use for the checks
 *	\param	insn	instruction opcode which to test
 *			for eligibility for execution
 *	\return	zero, if the condition for execution of the
 *		instruction is not fulfilled, 1 otherwise */
static int cond_code_check(ARM_CORE_WORD psr, ARM_CORE_WORD insn)
{
/* this is taken from arm ddi 0100i (available at www.arm.com)
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * |n|z|c|v|q|res|j|  res  |ge[3:0]| reserved  |E|A|I|F|T| m[4:0]  |
 *
 * A1.1.3 Status registers
 * All processor state other than the general-purpose register contents is held in status registers. The current
 * operating processor status is in the Current Program Status Register (CPSR). The CPSR holds:
 * • four condition code flags (Negative, Zero, Carry and oVerflow).
 * • one sticky (Q) flag (ARMv5 and above only). This encodes whether saturation has occurred in
 * saturated arithmetic instructions, or signed overflow in some specific multiply accumulate
 * instructions.
 * • four GE (Greater than or Equal) flags (ARMv6 and above only). These encode the following
 * conditions separately for each operation in parallel instructions:
 * — whether the results of signed operations were non-negative
 * — whether unsigned operations produced a carry or a borrow.
 * • two interrupt disable bits, one for each type of interrupt (two in ARMv5 and below).
 * • one (A) bit imprecise abort mask (from ARMv6)
 * • five bits that encode the current processor mode.
 * • two bits that encode whether ARM instructions, Thumb instructions, or Jazelle opcodes are being
 * executed.
 * • one bit that controls the endianness of load and store operations (ARMv6 and above only).
 * Each exception mode also has a Saved Program Status Register (SPSR) which holds the CPSR of the task
 * immediately before the exception occurred. The CPSR and the SPSRs are accessed with special
 * instructions.
 * For more details on status registers, refer to Program status registers on page A2-11.
 * Table A1-1 Status register summary
 * Field Description Architecture
 * N Z C V Condition code flags All
 * J Jazelle state flag 5TEJ and above
 * GE[3:0] SIMD condition flags 6
 * E Endian Load/Store 6
 * A Imprecise Abort Mask 6
 * I IRQ Interrupt Mask All
 * F FIQ Interrupt Mask All
 * T Thumb state flag 4T and above
 * Mode[4:0] Processor mode All
 *
 *
 * A3.2.1 Condition code 0b1111
 * If the condition field is 0b1111, the behavior depends on the architecture version:
 * • In ARMv4, any instruction with a condition field of 0b1111 is UNPREDICTABLE.
 * • In ARMv5 and above, a condition field of 0b1111 is used to encode various additional instructions
 * which can only be executed unconditionally (see Unconditional instruction extension space on
 * page A3-41). All instruction encoding diagrams which show bits[31:28] as cond only match
 * instructions in which these bits are not equal to 0b1111.
 * Table A3-1 Condition codes
 * Opcode
 * [31:28]
 * Mnemonic
 * extension
 * Meaning Condition flag state
 * 0000 EQ Equal Z set
 * 0001 NE Not equal Z clear
 * 0010 CS/HS Carry set/unsigned higher or same C set
 * 0011 CC/LO Carry clear/unsigned lower C clear
 * 0100 MI Minus/negative N set
 * 0101 PL Plus/positive or zero N clear
 * 0110 VS Overflow V set
 * 0111 VC No overflow V clear
 * 1000 HI Unsigned higher C set and Z clear
 * 1001 LS Unsigned lower or same C clear or Z set
 * 1010 GE Signed greater than or equal N set and V set, or
 * N clear and V clear (N == V)
 * 1011 LT Signed less than N set and V clear, or
 * N clear and V set (N != V)
 * 1100 GT Signed greater than Z clear, and either N set and V set, or
 * N clear and V clear (Z == 0,N == V)
 * 1101 LE Signed less than or equal Z set, or N set and V clear, or
 * N clear and V set (Z == 1 or N != V)
 * 1110 AL Always (unconditional) -
 * 1111 - See Condition code 0b1111 -
 */
int res;
int n, z, c, v;

	n = GET_BIT(31, psr);
	z = GET_BIT(30, psr);
	c = GET_BIT(29, psr);
	v = GET_BIT(28, psr);

	res = 0;
	switch (GET_BITS(28, 31, insn))
	{
		case 0:
			if (z)
				res = 1;
			break;
		case 1:
			if (!z)
				res = 1;
			break;
		case 2:
			if (c)
				res = 1;
			break;
		case 3:
			if (!c)
				res = 1;
			break;
		case 4:
			if (n)
				res = 1;
			break;
		case 5:
			if (!n)
				res = 1;
			break;
		case 6:
			if (v)
				res = 1;
			break;
		case 7:
			if (!v)
				res = 1;
			break;
		case 8:
			if (c && !z)
				res = 1;
			break;
		case 9:
			if (!c || z)
				res = 1;
			break;
		case 10:
			if ((n && v) || (!n && !v))
				/* n == v */
				res = 1;
			break;
		case 11:
			if ((n && !v) || (!n && v))
				/* n != v */
				res = 1;
			break;
		case 12:
			if (!z && (n == v))
				res = 1;
			break;
		case 13:
			if (z || (n != v))
				res = 1;
			break;
		case 14:
			res = 1;
			break;
		default:
			panic("");
	}
	return res;
}

/*!
 *	\fn	static ARM_CORE_WORD do_arm_alu_op(ARM_CORE_WORD op0, ARM_CORE_WORD op1, ARM_CORE_WORD opcode, ARM_CORE_WORD psr)
 *	\brief	performs an arm alu operation
 *
 *	\todo	flags are not synthesized; if these are to be generated,
 *		also take care about the barrel shifter carry out flag
 *
 *	\param	op0	first operand
 *	\param	op1	second operand, may be unused for some opcodes
 *	\param	opcode	alu operation code
 *	\param	psr	program status register value - used to
 *			read the carry flag if needed by the instruction
 *			opcode
 *	\return	the result of the computation; as some instructions
 *		are only supposed to update the flags, the first
 *		operand is returned in these cases instead
 */
static ARM_CORE_WORD do_arm_alu_op(ARM_CORE_WORD op0, ARM_CORE_WORD op1, ARM_CORE_WORD opcode, ARM_CORE_WORD psr)
{
ARM_CORE_WORD op = op0;

	switch (opcode)
	{
		case 0:
			op &= op1;
			break;
		case 1:
			op ^= op1;
			break;
		case 2:
			op -= op1;
			break;
		case 3:
			op = op1 - op;
			break;
		case 4:
			op += op1;
			break;
		case 5:
			op += op1 + GET_BIT(29, psr);
			break;
		case 6:
			op -= op1 + (GET_BIT(29, psr) ^ 1);
			break;
		case 7:
			op = op1 - op - (GET_BIT(29, psr) ^ 1);
			break;
		case 8:
			/* should never get here */
			panic("");
			break;
		case 9:
			/* should never get here */
			panic("");
			break;
		case 10:
			/* should never get here */
			panic("");
			break;
		case 11:
			/* should never get here */
			panic("");
			break;
		case 12:
			op |= op1;
			break;
		case 13:
			op = op1;
			break;
		case 14:
			op &=~ op1;
			break;
		case 15:
			op = ~op1;
			break;
		default:
			panic("");
	}
	return op;

}

/*!
 *	\fn	static ARM_CORE_WORD arm_barrel_shifter(ARM_CORE_WORD shifter_op, ARM_CORE_WORD shift_amount, int shift_cmd)
 *	\brief	a barrel shifter for an arm core
 *
 *	this routine performs barrel shifting tasks such as rotation and
 *	shifting
 *
 *	\param	shifter_op	shifter operand
 *	\param	shift_amount	the number of bit positions to shift or
 *				rotate the shifter operand; the value 0
 *				is special here; for the logical shift
 *				right and the arithmetic shift right, this
 *				value encodes the value 32; for rotate
 *				right, this value requests that a rotate
 *				right with extend be performed; also,
 *				see the shift_cmd comments below
 *	\param	shift_cmd	a two bit value encoding the operation
 *				that the barrel shifter should perform;
 *				the encoding is:
 *					- 00 - logical shift left;
 *						a value of zero for
 *						shift_amount is no special
 *					- 01 - logical shift right;
 *						a value of zero for
 *						shift_amount is special,
 *						and encodes a shift amount
 *						of 32 bits
 *					- 10 - arithmetic shift right;
 *						a value of zero for
 *						shift_amount is special,
 *						and encodes a shift amount
 *						of 32 bits
 *					- 11 - rotate right;
 *						a value of zero for
 *						shift_amount is special,
 *						and specifies that
 *						a 'rotate right with extend'
 *						(rrx) be performed instead
 *						of rotate right; the
 *						definition of rrx (in
 *						pseudocode) is:
 *							carry_out = GET_BIT(0, shifter_op);
 *							result = (shifter_op >> 1) | (carry_flag << 31);
 *	\return	the result of the barrel shifting operation
 */
static ARM_CORE_WORD arm_barrel_shifter(ARM_CORE_WORD shifter_op, ARM_CORE_WORD shift_amount, int shift_cmd)
{
/*! not used right now, may be used in the future though */
int carry_out;

	/*! \todo	fix this if necessary */
	//carry_out = get_carry_flag();
	/* if the checks here seem stupid, then do bear in mind that on an x86,
	 * for example:
	 * i << j == i << (j % 32)
	 * so, for example, if j == 32, then i << 32 == i, which may not be
	 * exactly what you expect (because, as far as i(sgs) understand the
	 * c standard, this should not be the case); why this is so on an x86
	 * (and *not* on other architectures (e.g. the arm)) is a matter
	 * of hardware and (to a lesser extent) compiler implementation */
	switch (shift_cmd)
	{
		case 0:
			/* logical shift left */
			if (shift_amount == 0)
				;
			else if (shift_amount <= 32)
			{
				carry_out = GET_BIT(32 - shift_amount, shifter_op);
				shifter_op <<= shift_amount;
			}
			else /* if shift_amount > 32) */
			{
				carry_out = 0;
				shifter_op = 0;
			}
			break;
		case 1:
			/* logical shift right */
			if (shift_amount == 0)
				/* special case */
				shift_amount = 32;
			else if (shift_amount <= 32)
			{
				carry_out = GET_BIT(shift_amount - 1, shifter_op);
				shifter_op >>= shift_amount;
			}
			else /* if (shift_amount > 32) */
			{
				carry_out = 0;
				shifter_op = 0;
			}
			break;
		case 2:
			/* arithmetic shift right */
			/*! \warning	here, types' signs and bitwidths are ***deadly vital*** */
			if (shift_amount == 0)
				/* special case */
				shift_amount = 32;
			else if (shift_amount < 32)
			{
				carry_out = GET_BIT(shift_amount - 1, shifter_op);
				shifter_op = ((signed int) shifter_op) >> shift_amount;
			}
			else /* if (shift_amount >= 32) */
			{
				if ((carry_out = GET_BIT(31, shifter_op)))
					shifter_op = 0xffffffff;
				else
					shifter_op = 0;
			}
			break;
		case 3:
			/* rotate right */
			/*! \warning	here, types' signs and bitwidths are ***deadly vital*** */
			if (shift_amount == 0)
			{
				/* a special case - rotate right with
				 * extend (rrx) */
				carry_out = GET_BIT(0, shifter_op);
				shifter_op >>= 1;
				/*! \todo	fix this if necessary */
				//shifter_op |= get_carry_flag() << 31;
				panic("carry flag not properly read - fix this if needed");
			}
			else if ((shift_amount & 31) == 0)
			{
				carry_out = GET_BIT(31, shifter_op);
			}
			else /* if ((shift_amount & 31) > 0) */
			{
				carry_out = GET_BIT((shift_amount & 31) - 1, shifter_op);
				shifter_op = (shifter_op >> shift_amount)
					| (shifter_op << shift_amount);
			}
			break;
		default:
			/* should never happen */
			panic("bad arm insn decode");
	}

	return shifter_op;

}


/*!
 *	\fn	static ARM_CORE_WORD addr_mode_1(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn, int * is_pc_modified_flag)
 *	\brief	performs address mode 1 arm instruction analysis
 *
 *	for details, see the arm ddi 0100i document (available at
 *	www.arm.com )
 *
 *	addressing mode 1 - data processing operands
 *
 *	\note	the condition field of the instruction is not checked
 *		here; assumed is that the caller has already examined
 *		the condition field and has determined that the instruction
 *		is eligible for execution
 *
 *	\param	ctx	context to work in
 *	\param	pc	program counter (r15) value
 *	\param	insn	instruction opcode to analyze
 *	\param	is_pc_modified_flag	a pinter to a flag, which is set to nonzero
 *			if the instruction supplied modifies the
 *			program counter, and reset to zero if it does not
 *	\return	new computed value for the pc
 */
static ARM_CORE_WORD addr_mode_1(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn, int * is_pc_modified_flag)
{
/* this is taken from arm ddi 0100i (available at www.arm.com)
 *
 * There are 11 formats used to calculate the <shifter_operand> in an ARM data-processing instruction. The
 * general instruction syntax is:
 * <opcode>{<cond>}{S} <Rd>, <Rn>, <shifter_operand>
 * where <shifter_operand> is one of the following:
 * 1. #<immediate>
 * See Data-processing operands - Immediate on page A5-6.
 * 2. <Rm>
 * See Data-processing operands - Register on page A5-8.
 * 3. <Rm>, LSL #<shift_imm>
 * See Data-processing operands - Logical shift left by immediate on page A5-9.
 * 4. <Rm>, LSL <Rs>
 * See Data-processing operands - Logical shift left by register on page A5-10.
 * 5. <Rm>, LSR #<shift_imm>
 * See Data-processing operands - Logical shift right by immediate on page A5-11.
 * 6. <Rm>, LSR <Rs>
 * See Data-processing operands - Logical shift right by register on page A5-12.
 * 7. <Rm>, ASR #<shift_imm>
 * See Data-processing operands - Arithmetic shift right by immediate on page A5-13.
 * 8. <Rm>, ASR <Rs>
 * See Data-processing operands - Arithmetic shift right by register on page A5-14.
 * 9. <Rm>, ROR #<shift_imm>
 * See Data-processing operands - Rotate right by immediate on page A5-15.
 * 10. <Rm>, ROR <Rs>
 * See Data-processing operands - Rotate right by register on page A5-16.
 * 11. <Rm>, RRX
 * See Data-processing operands - Rotate right with extend on page A5-17.
 *
 *
 * encodings;
 *
 *	- 32 bit immediate
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0|0|1|opcode |s|  rn   |  rd   |rot_imm|  immed_8      |
 *
 *
 *	- immediate shifts
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 0|0|opcode |s|  rn   |  rd   |shift_imm|shf|0|  rm   |
 *
 *
 *	- register shifts
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 0|0|opcode |s|  rn   |  rd   |  rs   |0|shf|1|  rm   |
 *
 *
 * opcode Specifies the operation of the instruction.
 * S bit Indicates that the instruction updates the condition codes.
 * Rd Specifies the destination register.
 * Rn Specifies the first source operand register.
 * Bits[11:0] The fields within bits[11:0] are collectively called a shifter operand. This is described in The
 * shifter operand on page A5-4.
 * Bit[25] Is referred to as the I bit, and is used to distinguish between an immediate shifter operand
 * and a register-based shifter operand.
 *
 * shf means:
 * - when 00 - logical shift left
 * - when 01 - logical shift right
 * - when 10 - arithmetic shift right; for immediate(not register specified)
 *	shift amounts, 0 encodes the value 32
 * - when 11 - rotate right - but an exception is: when
 *	this is a rotate by immediate (the second encoding above),
 *	and the immediate (rot_imm) equals zero, then this is
 *	a rotate right with extend - a 1 position, 33 bit rotate
 *	right with the carry flag as the 33-rd bit (the most
 *	significant one)
 *
 * If all three of the following bits have the values shown, the instruction is not a data-processing instruction,
 * but lies in the arithmetic or Load/Store instruction extension space:
 * bit[25] == 0
 * bit[4] == 1
 * bit[7] == 1
 *
 * See Extending the instruction set on page A3-32 for more information.
 * Addressing mode 3, MCRR{2}, MRRC{2}, STC{2} are examples of instructions that reside in this space.
 *
 * A5.1.2 The shifter operand
 * As well as producing the shifter operand, the shifter produces a carry-out which some instructions write into
 * the Carry Flag. The default register operand (register Rm specified with no shift) uses the form register shift
 * left by immediate, with the immediate set to zero.
 * The shifter operand takes one of the following three basic formats.
 * Immediate operand value
 * An immediate operand value is formed by rotating an 8-bit constant (in a 32-bit word) by an even number
 * of bits (0,2,4,8...26,28,30). Therefore, each instruction contains an 8-bit constant and a 4-bit rotate to be
 * applied to that constant.
 * Some valid constants are:
 * 0xFF,0x104,0xFF0,0xFF00,0xFF000,0xFF000000,0xF000000F
 * Some invalid constants are:
 * 0x101,0x102,0xFF1,0xFF04,0xFF003,0xFFFFFFFF,0xF000001F
 * For example:
 * MOV R0, #0 ; Move zero to R0
 * ADD R3, R3, #1 ; Add one to the value of register 3
 * CMP R7, #1000 ; Compare value of R7 with 1000
 * BIC R9, R8, #0xFF00 ; Clear bits 8-15 of R8 and store in R9
 * Register operand value
 * A register operand value is simply the value of a register. The value of the register is used directly as the
 * operand to the data-processing instruction. For example:
 * MOV R2, R0 ; Move the value of R0 to R2
 * ADD R4, R3, R2 ; Add R2 to R3, store result in R4
 * CMP R7, R8 ; Compare the value of R7 and R8
 * Shifted register operand value
 * A shifted register operand value is the value of a register, shifted (or rotated) before it is used as the
 * data-processing operand. There are five types of shift:
 * ASR Arithmetic shift right
 * LSL Logical shift left
 * LSR Logical shift right
 * ROR Rotate right
 * RRX Rotate right with extend.
 * The number of bits to shift by is specified either as an immediate or as the value of a register. For example:
 * MOV R2, R0, LSL #2 ; Shift R0 left by 2, write to R2, (R2=R0x4)
 * ADD R9, R5, R5, LSL #3 ; R9 = R5 + R5 x 8 or R9 = R5 x 9
 * RSB R9, R5, R5, LSL #3 ; R9 = R5 x 8 - R5 or R9 = R5 x 7
 * SUB R10, R9, R8, LSR #4 ; R10 = R9 - R8 / 16
 * MOV R12, R4, ROR R3 ; R12 = R4 rotated right by value of R3
 */
/*! \warning	it is vital here that the type of this is a 32 bit unsigned one */
ARM_CORE_WORD shifter_op;
ARM_CORE_WORD new_pc;
int shift_amount;
/*! not really used, may be used in the future though */
int carry_out;
/* the shf mode above */
int shift_cmd;
/* the psr reading here is bogus, but it should have been cached anyway, so
 * the overhead should be minimal */
ARM_CORE_WORD psr;

	/* by default, the instruction does not modify the pc */
	new_pc = pc + 4;
	/* sanity checks */
	if (
			(GET_BIT(25, insn) == 0 &&
			GET_BIT(4, insn) == 1 &&
			GET_BIT(7, insn) == 1)
		||
			GET_BITS(26, 27, insn) != 0)
		panic("invalid address mode 1 encoding - instruction does not "
				"correspond to an address mode 1 encoding");

	*is_pc_modified_flag = 0;
	/* proceed only if the destination register is the pc */
	if (GET_BITS(12, 15, insn) != 15)
		return new_pc;
	/* also, see if this instruction requests an alu operation
	 * that does not modify operands, but flags only */
	if (GET_BITS(21, 24, insn) >= 8
			&& GET_BITS(21, 24, insn) <= 11)
		return new_pc;

	/* here, it is known that the program counter will be modified */
	*is_pc_modified_flag = 1;

	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx, 0, BIT17, &psr) !=
			GEAR_ERR_NO_ERROR)
		panic("");
	carry_out = GET_BIT(29, psr);
	/* first, compute the shifter operand */
	/* see which encoding is used */
	if (GET_BIT(25, insn) == 1)
	{
		/* a special case */
		/* the first encoding */
		shifter_op = GET_BITS(0, 7, insn);
		/* the next one is times two */
		shift_amount = GET_BITS(8, 11, insn) << 1;
		if (shift_amount == 0)
			;
		else
		{
			carry_out = GET_BIT(31, shifter_op);
			shifter_op = (shifter_op >> shift_amount)
				| (shifter_op << (32 - shift_amount));
		}
		goto got_shifter_operand;
	}
	else
	{
		shift_cmd = GET_BITS(5, 6, insn);
		shifter_op = core_reg_read(ctx, GET_BITS(0, 3, insn));
		/* either the second, or the third encoding */
		if (GET_BIT(4, insn) == 0)
		{
			/* second encoding */
			shift_amount = GET_BITS(7, 11, insn);
		}
		else
		{
			/* third encoding */
			shift_amount = core_reg_read(ctx, GET_BITS(8, 11, insn));
			/* not really necessary, but follow the arm documents */
			shift_amount &= 0xff;
			/* handle the special cases when shift_imm is zero
			 * for an arithmetic or a logical shift right -
			 * this cant be handled by the arm core barrel
			 * shifter */
			if ((shift_cmd == 1 || shift_cmd == 2)
					&& shift_amount == 0)
			{
				goto got_shifter_operand;
			}
			else if (shift_cmd == 3
					&& (shift_amount & 0x1f) == 0)
			{
				if (shift_amount != 0)
					carry_out = GET_BIT(31, shifter_op);
				goto got_shifter_operand;
			}
		}
	}
	/* compute shifter operand using the barrel shifter */
	shifter_op = arm_barrel_shifter(shifter_op, shift_amount, shift_cmd);

got_shifter_operand:
	/* good; we are now ready to perform the core-alu operation */
	new_pc = do_arm_alu_op(core_reg_read(ctx, GET_BITS(16, 19, insn)), shifter_op,
				GET_BITS(21, 24, insn), psr);
	/* validate new pc value */
	if (new_pc & 3)
		panic("bad pc value");
	return new_pc;
}


/*!
 *	\fn	static ARM_CORE_WORD addr_mode_2(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn, int * is_pc_modified_flag)
 *	\brief	performs address mode 2 arm instruction analysis
 *
 *	for details, see the arm ddi 0100i document (available at
 *	www.arm.com )
 *
 *	addressing mode 2 - load and store word or unsigned byte
 *
 *	\note	the condition field of the instruction is not checked
 *		here; assumed is that the caller has already examined
 *		the condition field and has determined that the instruction
 *		is eligible for execution
 *
 *	\param	ctx	context to work in
 *	\param	pc	program counter (r15) value
 *	\param	insn	instruction opcode to analyze
 *	\param	is_pc_modified_flag	a pinter to a flag, which is set to nonzero
 *			if the instruction supplied modifies the
 *			program counter, and reset to zero if it does not
 *	\return	new computed value for the pc
 */
static ARM_CORE_WORD addr_mode_2(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn, int * is_pc_modified_flag)
{
/* this is taken from arm ddi 0100i (available at www.arm.com)
 *
 *
 * There are nine formats used to calculate the address for a Load and Store Word or Unsigned Byte
 * instruction. The general instruction syntax is:
 * LDR|STR{<cond>}{B}{T} <Rd>, <addressing_mode>
 * where <addressing_mode> is one of the nine options listed below.
 * All nine of the following options are available for LDR, LDRB, STR and STRB. For LDRBT, LDRT, STRBT and STRBT,
 * only the post-indexed options (the last three in the list) are available. For the PLD instruction described in
 * PLD on page A4-90, only the offset options (the first three in the list) are available.
 * 1. [<Rn>, #+/-<offset_12>]
 * See Load and Store Word or Unsigned Byte - Immediate offset on page A5-20.
 * 2. [<Rn>, +/-<Rm>]
 * See Load and Store Word or Unsigned Byte - Register offset on page A5-21.
 * 3. [<Rn>, +/-<Rm>, <shift> #<shift_imm>]
 * See Load and Store Word or Unsigned Byte - Scaled register offset on page A5-22.
 * 4. [<Rn>, #+/-<offset_12>]!
 * See Load and Store Word or Unsigned Byte - Immediate pre-indexed on page A5-24.
 * 5. [<Rn>, +/-<Rm>]!
 * See Load and Store Word or Unsigned Byte - Register pre-indexed on page A5-25.
 * 6. [<Rn>, +/-<Rm>, <shift> #<shift_imm>]!
 * See Load and Store Word or Unsigned Byte - Scaled register pre-indexed on page A5-26.
 * 7. [<Rn>], #+/-<offset_12>
 * See Load and Store Word or Unsigned Byte - Immediate post-indexed on page A5-28.
 * 8. [<Rn>], +/-<Rm>
 * See Load and Store Word or Unsigned Byte - Register post-indexed on page A5-30.
 * 9. [<Rn>], +/-<Rm>, <shift> #<shift_imm>
 * See Load and Store Word or Unsigned Byte - Scaled register post-indexed on page A5-31.
 *
 *
 * encodings:
 *
 *	- immediate offset/index
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 1|0|p|u|b|w|l|  rn   |  rd   |      offset_12        |
 *
 *
 *	- register offset/index
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 1|1|p|u|b|w|l|  rn   |  rd   |0 0 0 0 0 0 0 0|  rm   |
 *
 *	- scaled register offset/index
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 1|1|p|u|b|w|l|  rn   |  rd   |shift_imm|shf|0|  rm   |
 *
 *	note that the second encoding is a special case of the third
 *	encoding (rm lsl 0, which is simply rm), so handled is the third
 *	encoding only
 *
 *
 * The P bit Has two meanings:
 * P == 0 Indicates the use of post-indexed addressing. The base register value is used for
 * the memory address, and the offset is then applied to the base register value and
 * written back to the base register.
 * P == 1 Indicates the use of offset addressing or pre-indexed addressing (the W bit
 * determines which). The memory address is generated by applying the offset to
 * the base register value.
 * The U bit Indicates whether the offset is added to the base (U == 1) or is subtracted from the base
 * (U == 0).
 * The B bit Distinguishes between an unsigned byte (B == 1) and a word (B == 0) access.
 * The W bit Has two meanings:
 * P == 0 If W == 0, the instruction is LDR, LDRB, STR or STRB and a normal memory access
 * is performed. If W == 1, the instruction is LDRBT, LDRT, STRBT or STRT and an
 * unprivileged (User mode) memory access is performed.
 * P == 1 If W == 0, the base register is not updated (offset addressing). If W == 1, the
 * calculated memory address is written back to the base register (pre-indexed
 * addressing).
 * The L bit Distinguishes between a Load (L == 1) and a Store (L == 0).
 */

ARM_CORE_WORD	addr;
ARM_CORE_WORD	offset;
ARM_CORE_WORD	new_base_reg_val;
ARM_CORE_WORD new_pc;

	/* by default, the instruction does not modify the pc */
	new_pc = pc + 4;

	*is_pc_modified_flag = 0;
	/* sanity checks */
	if (GET_BITS(26, 27, insn) != 1)
		panic("bad insn decoding");
	/* proceed only if the instruction intends to load the
	 * program counter */
	/* check for unpredictable behavior */
	if (GET_BITS(16, 19, insn) == 15)
	{
		if (GET_BIT(24, insn) == 0)
			panic("bad insn");
		else if (GET_BIT(21, insn) == 1)
			panic("bad insn");
	}
	if (GET_BITS(12, 15, insn) != 15 || GET_BIT(20, insn) == 0)
		return new_pc;
	else
	{
		/* check for unpredictable behavior */
		if (GET_BIT(22, insn) == 1
				|| (GET_BIT(24, insn) == 0 && GET_BIT(21, insn) == 1))
			panic("unpredictable behavior");
	}
	/* here, it is known that the program counter will be modified */
	*is_pc_modified_flag = 1;

	/* first, form the address */
	/* see which encoding is used */
	if (GET_BIT(25, insn) == 0)
	{
		/* the first encoding - immediate offset */
		offset = GET_BITS(0, 11, insn);
	}
	else
	{
		/* either the second, or the third encoding */
		/* the second encoding is actually a special case
		 * of the third encoding */
		offset = arm_barrel_shifter(
				GET_BITS(7, 11, insn),
				GET_BITS(5, 6, insn),
				core_reg_read(ctx, GET_BITS(0, 3, insn)));
	}
	addr = core_reg_read(ctx, GET_BITS(16, 19, insn));
	/* inspect the u bit */
	if (GET_BIT(23, insn) == 0)
		offset *= -1;
	/* inspect the p bit */
	if (GET_BIT(24, insn) == 0)
	{
		new_base_reg_val = addr + offset;
	}
	else
	{
		/* inspect the w bit */
		if (GET_BIT(21, insn) == 0)
			new_base_reg_val = addr;
		else
			new_base_reg_val = addr + offset;
		addr += offset;
	}
	/* now decode the actual type and width of the memory reference
	 * (load/store, byte/word, user mode access) */
	panic("");
}


/*!
 *	\fn	static ARM_CORE_WORD addr_mode_3(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn)
 *	\brief	performs address mode 3 arm instruction analysis
 *
 *	for details, see the arm ddi 0100i document (available at
 *	www.arm.com )
 *
 *	addressing mode 3 - miscellaneous loads and stores
 *
 *	this is similar to addressing mode 2
 *
 *	\note	the condition field of the instruction is not checked
 *		here; assumed is that the caller has already examined
 *		the condition field and has determined that the instruction
 *		is eligible for execution
 *
 *	\param	ctx	context to work in
 *	\param	pc	program counter (r15) value
 *	\param	insn	instruction opcode to analyze
 *	\return	new computed value for the pc
 */
static ARM_CORE_WORD addr_mode_3(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn)
{
/* this is taken from arm ddi 0100i (available at www.arm.com)
 *
 * There are six formats used to calculate the address for load and store (signed or unsigned) halfword, load
 * signed byte, or load and store doubleword instructions. The general instruction syntax is:
 * LDR|STR{<cond>}H|SH|SB|D <Rd>, <addressing_mode>
 * where <addressing_mode> is one of the following six options:
 * 1. [<Rn>, #+/-<offset_8>]
 * See Miscellaneous Loads and Stores - Immediate offset on page A5-35.
 * 2. [<Rn>, +/-<Rm>]
 * See Miscellaneous Loads and Stores - Register offset on page A5-36.
 * 3. [<Rn>, #+/-<offset_8>]!
 * See Miscellaneous Loads and Stores - Immediate pre-indexed on page A5-37.
 * 4. [<Rn>, +/-<Rm>]!
 * See Miscellaneous Loads and Stores - Register pre-indexed on page A5-38.
 * 5. [<Rn>], #+/-<offset_8>
 * See Miscellaneous Loads and Stores - Immediate post-indexed on page A5-39.
 * 6. [<Rn>], +/-<Rm>
 * See Miscellaneous Loads and Stores - Register post-indexed on page A5-40.
 * A5.3.1 Encoding
 * The following diagrams show the encodings for this addressing mode:
 *
 *
 * encodings:
 *
 *	- immediate offset/index
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 0 0|p|u|1|w|l|  rn   |  rd   |immedh |1|s|h|1|immedl |
 *
 *
 *	- register offset/index
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 0 0|p|u|0|w|l|  rn   |  rd   |  sbz  |1|s|h|1|  rm   |
 *
 *
 *
 * The P bit Has two meanings:
 * P == 0 Indicates the use of post-indexed addressing. The base register value is used for
 * the memory address, and the offset is then applied to the base register value and
 * written back to the base register.
 * P == 1 Indicates the use of offset addressing or pre-indexed addressing (the W bit
 * determines which). The memory address is generated by applying the offset to
 * the base register value.
 * The U bit Indicates whether the offset is added to the base (U == 1) or subtracted from the base
 * (U == 0).
 * The W bit Has two meanings:
 * P == 0 The W bit must be 0 or the instruction is UNPREDICTABLE.
 * P == 1 W == 1 indicates that the memory address is written back to the base register
 * (pre-indexed addressing), and W == 0 that the base register is unchanged (offset
 * addressing).
 * The L, S and H bits
 * These bits combine to specify signed or unsigned loads or stores, and doubleword, halfword,
 * or byte accesses:
 * L=0, S=0, H=1 Store halfword.
 * L=0, S=1, H=0 Load doubleword.
 * L=0, S=1, H=1 Store doubleword.
 * L=1, S=0, H=1 Load unsigned halfword.
 * L=1, S=1, H=0 Load signed byte.
 * L=1, S=1, H=1 Load signed halfword.
 * Prior to v5TE, the bits were denoted as Load/!Store (L), Signed/!Unsigned (S) and
 * halfword/!Byte (H) bits.
 * Signed bytes and halfwords can be stored with the same STRB and STRH instructions as are
 * used for unsigned quantities, so no separate signed store instructions are provided.
 * Unsigned bytes
 * If S == 0 and H == 0, apparently indicating an unsigned byte, the instruction is not one that
 * uses this addressing mode. Instead, it is a multiply instruction, a SWP or SWPB instruction, an
 * LDREX or STREX instruction, or an unallocated instruction in the arithmetic or load/store
 * instruction extension space (see Extending the instruction set on page A3-32).
 * 	Unsigned bytes are accessed by the LDRB, LDRBT, STRB and STRBT instructions, which use
 * 		addressing mode 2 rather than addressing mode 3.
 * 		Signed stores If S ==1 and L == 0, apparently indicating a signed store instruction, the encoding along
 * 		with the H-bit is used to support the LDRD (H == 0) and STRD (H == 1) instructions.
 */
ARM_CORE_WORD	addr;
ARM_CORE_WORD	offset;
ARM_CORE_WORD	new_base_reg_val;
ARM_CORE_WORD new_pc;

	/* by default, the instruction does not modify the pc */
	new_pc = pc + 4;

	/* sanity checks */
	if (GET_BITS(25, 27, insn) != 1
			|| GET_BIT(20, insn) != 1
			|| GET_BIT(7, insn) != 1
			|| GET_BIT(4, insn) != 1
			|| (GET_BIT(24, insn) == 0 && GET_BIT(21, insn) != 0))
		panic("bad insn decoding");
	if (GET_BIT(5, insn) == 0 &&
			GET_BIT(6, insn) == 0)
		panic("bad insn decode");
	/* proceed only if the instruction intends to load the
	 * program counter */
	/* check for unpredictable behavior */
	if (GET_BITS(16, 19, insn) == 15)
	{
		if (GET_BIT(24, insn) == 0)
			panic("bad insn");
		else if (GET_BIT(21, insn) == 1)
			panic("bad insn");
	}
	/* a special case - detect a ldrd instruction */
	if (GET_BIT(20, insn) == 0 && GET_BITS(5, 6, insn) == 2)
	{
		if (GET_BITS(12, 15, insn) == 14)
			panic("behavior is unpredictable");
		if (GET_BITS(12, 15, insn) & 1)
			panic("undefined instruction");
	}
	if (GET_BITS(12, 15, insn) != 15)
		return new_pc;
	else
		panic("behavior is unpredictable");
	/*
	 *
	 *	no further decoding is needed right now
	 *	should the code below become needed, it must
	 *	be carefully inspected and competed
	 *
	 */
	panic("");
	/* first, form the address */
	/* see which encoding is used */
	if (GET_BIT(22, insn) == 1)
	{
		/* the first encoding - immediate offset */
		offset = (GET_BITS(8, 11, insn) << 4)
			| GET_BITS(0, 3, insn);
	}
	else
	{
		/* either the second, or the third encoding */
		/* the second encoding is actually a special case
		 * of the third encoding */
		offset = core_reg_read(ctx, GET_BITS(0, 3, insn));
	}
	addr = core_reg_read(ctx, GET_BITS(16, 19, insn));
	/* inspect the u bit */
	if (GET_BIT(23, insn) == 0)
		offset = -1;
	/* inspect the p bit */
	if (GET_BIT(24, insn) == 0)
	{
		/* make sure the w bit is 0, otherwise the
		 * result is unpredictable */
		if (GET_BIT(21, insn) != 0)
			panic("bad opcode");
		new_base_reg_val = addr + offset;
	}
	else
	{
		/* inspect the w bit */
		if (GET_BIT(21, insn) == 0)
			new_base_reg_val = addr;
		else
			new_base_reg_val = addr + offset;
		addr += offset;
	}
	/* now decode the actual type and width of the memory reference */
	panic("");
}

/*!
 *	\fn	static ARM_CORE_WORD addr_mode_4(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn)
 *	\brief	performs address mode 4 arm instruction analysis
 *
 *	for details, see the arm ddi 0100i document (available at
 *	www.arm.com )
 *
 *	addressing mode 4 - load and store multiple
 *
 *	\note	the condition field of the instruction is not checked
 *		here; assumed is that the caller has already examined
 *		the condition field and has determined that the instruction
 *		is eligible for execution
 *
 *	\param	ctx	context to work in
 *	\param	pc	program counter (r15) value
 *	\param	insn	instruction opcode to analyze
 *	\return	new computed value for the pc
 */
static ARM_CORE_WORD addr_mode_4(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn)
{
/* this is taken from arm ddi 0100i (available at www.arm.com)
 *
 * Load Multiple instructions load a subset (possibly all) of the general-purpose registers from memory. Store
 * Multiple instructions store a subset (possibly all) of the general-purpose registers to memory.
 * Load and Store Multiple addressing modes produce a sequential range of addresses. The lowest-numbered
 * register is stored at the lowest memory address and the highest-numbered register at the highest memory
 * address.
 * The general instruction syntax is:
 * LDM|STM{<cond>}<addressing_mode> <Rn>{!}, <registers>{^}
 * where <addressing_mode> is one of the following four addressing modes:
 * 1. IA (Increment After)
 * See Load and Store Multiple - Increment after on page A5-43.
 * 2. IB (Increment Before)
 * See Load and Store Multiple - Increment before on page A5-44.
 * 3. DA (Decrement After)
 * See Load and Store Multiple - Decrement after on page A5-45.
 * 4. DB (Decrement Before)
 * See Load and Store Multiple - Decrement before on page A5-46.
 * There are also alternative mnemonics for these addressing modes, useful when LDM and STM are being used
 * to access a stack, see Load and Store Multiple addressing modes (alternative names) on page A5-47.
 *
 *
 * encoding:
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |1 0 0|p|u|s|w|l|  rn   |     register list             |
 *
 *
 * The P bit Has two meanings:
 * P==0 indicates that the word addressed by Rn is included in the range of memory
 * locations accessed, lying at the top (U==0) or bottom (U==1) of that range.
 * P==1 indicates that the word addressed by Rn is excluded from the range of memory
 * locations accessed, and lies one word beyond the top of the range (U==0) or one
 * word below the bottom of the range (U==1).
 * The U bit Indicates that the transfer is made upwards (U==1) or downwards (U==0) from the base
 * register.
 * The S bit For LDMs that load the PC, the S bit indicates that the CPSR is loaded from the SPSR. For
 * LDMs that do not load the PC and all STMs, the S bit indicates that when the processor is in a
 * privileged mode, the User mode banked registers are transferred instead of the registers of
 * the current mode.
 * LDM with the S bit set is UNPREDICTABLE in User or System mode.
 * The W bit Indicates that the base register is updated after the transfer. The base register is incremented
 * (U==1) or decremented (U==0) by four times the number of registers in the register list.
 * The L bit Distinguishes between Load (L==1) and Store (L==0) instructions.
 * Register list The register_list field of the instruction has one bit for each general-purpose register: bit[0]
 * for register zero through to bit[15] for register 15 (the PC). If no bits are set, the result is
 * UNPREDICTABLE.
 * The instruction syntax specifies the registers to load or store in <registers>, which is a
 * comma-separated list of registers, surrounded by { and }.
 */
int i, nr_regs;
ARM_CORE_WORD addr;
ARM_CORE_WORD new_pc;
int nbytes;

	/* by default, the instruction does not modify the pc */
	new_pc = pc + 4;

	/* sanity checks */
	if (GET_BITS(25, 27, insn) != 4)
		panic("bad decoding");
	/* check for an empty register list - the result is
	 * unpredictable */
	if (GET_BITS(0, 15, insn) == 0)
		panic("register list is empty");
	/* specifying r15 for rn leads to unpredictable results, too */
	if (GET_BITS(16, 19, insn) == 15)
		panic("bad insn");
	if (GET_BIT(15, insn) == 0 || GET_BIT(20, insn) == 0)
		return new_pc;
	/*! \todo	maybe also check for ldm with the s bit set
	 *		in user and system modes */
	for (nr_regs = i = 0; i < 16; i++)
		if (GET_BIT(i, insn))
			nr_regs++;
	addr = core_reg_read(ctx, GET_BITS(16, 19, insn));
	/* process the p bit */
	if (GET_BIT(24, insn) == 1)
	{
		/* examine the u bit */
		if (GET_BIT(23, insn) == 1)
			addr += 4;
		else
			addr -= 4;
	}
	/* compute address to load the pc from */
	/* exclude the pc itself */
	nr_regs -= 1;
	nr_regs *= 4;
	/* examine the u bit */
	if (GET_BIT(23, insn) == 0)
		nr_regs *= -1;
	addr += nr_regs;
	/* validate address */
	if (addr & 3)
		panic("unaligned address");
	nbytes = sizeof new_pc;
	if (ctx->cc->core_mem_read(ctx, &new_pc, addr, &nbytes) != GEAR_ERR_NO_ERROR ||
			nbytes != sizeof new_pc);
	if (new_pc & 3)
		panic("bad pc value");
	return new_pc;
}


/*!
 *	\fn	static ARM_CORE_WORD addr_mode_5(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn)
 *	\brief	performs address mode 5 arm instruction analysis
 *
 *	for details, see the arm ddi 0100i document (available at
 *	www.arm.com )
 *
 *	addressing mode 5 - load and store coprocessor
 *
 *	\note	the condition field of the instruction is not checked
 *		here; assumed is that the caller has already examined
 *		the condition field and has determined that the instruction
 *		is eligible for execution
 *
 *	\param	ctx	context to work in
 *	\param	pc	program counter (r15) value
 *	\param	insn	instruction opcode to analyze
 *	\return	new computed value for the pc
 */
static ARM_CORE_WORD addr_mode_5(struct gear_engine_context * ctx, ARM_CORE_WORD pc, ARM_CORE_WORD insn)
{
/* this is taken from arm ddi 0100i (available at www.arm.com)
 *
 *
 * There are four addressing modes which are used to calculate the address of a Load or Store Coprocessor
 * instruction. The general instruction syntax is:
 * <opcode>{<cond>}{L} <coproc>,<CRd>,<addressing_mode>
 * where <addressing_mode> is one of the following four options:
 * 1. [<Rn>,#+/-<offset_8>*4]
 * See Load and Store Coprocessor - Immediate offset on page A5-51.
 * 2. [<Rn>,#+/-<offset_8>*4]!
 * See Load and Store Coprocessor - Immediate pre-indexed on page A5-52.
 * 3. [<Rn>],#+/-<offset_8>*4
 * See Load and Store Coprocessor - Immediate post-indexed on page A5-53.
 * 4. [<Rn>],<option>
 * See Load and Store Coprocessor - Unindexed on page A5-54.
 *
 *
 * encoding:
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |1 1 0|p|u|n|w|l|  rn   |  crd  |cp_num |  offset_8     |
 *
 *
 * The P bit Has two meanings:
 * P == 0 Indicates the use of post-indexed addressing or unindexed addressing (the W bit
 * determines which). The base register value is used for the memory address.
 * P == 1 Indicates the use of offset addressing or pre-indexed addressing (the W bit
 * determines which). The memory address is generated by applying the offset to
 * the base register value.
 * The U bit Has two meanings:
 * U == 1 Indicates that the offset is added to the base.
 * U == 0 Indicates that the offset is subtracted from the base
 * The N bit The meaning of this bit is coprocessor-dependent. Its recommended use is to distinguish
 * between different-sized values to be transferred.
 * The W bit Has two meanings:
 * W == 1 Indicates that the memory address is written back to the base register.
 * W == 0 Indicates that the base register value is unchanged.
 * Also:
 *  If P == 0, this distinguishes unindexed addressing (W == 0) from post-indexed
 *  addressing (W == 1). For unindexed addressing, U must equal 1 or the result is either
 *  UNDEFINED or UNPREDICTABLE (see Coprocessor instruction extension space on
 *  page A3-40).
 *   If P == 1, this distinguishes offset addressing (W == 0) from pre-indexed addressing
 *   (W == 1).
 *   The L bit Distinguishes between Load (L == 1) and Store (L == 0) instructions.
 */

	/* these instructions are harmless to the program counter,
	 * unless base register writeback is specified for r15, in
	 * which case the results are unpredictable; so just
	 * detect this case here */
	/* sanity checks */
	if (GET_BITS(25, 27, insn) != 6)
		panic("");
	/* see if writeback to the program counter is requested */
	if (GET_BIT(21, insn) == 1
			&& GET_BITS(16, 19, insn) == 15)
		panic("coprocessor transfer with r15 with writeback addressing detected");
	return pc + 4;

}

/*
 *
 * exported functions follow
 *
 */


/*!
 *	\fn	int arm_insn_decode(struct gear_engine_context * ctx, ARM_CORE_WORD * next_insn_addr, int * is_probably_call_insn)
 *	\brief	decodes an arm instruction and determines the location of the next instruction
 *
 *	this routine is used by the executor step into and step over
 *	routines; it decodes the instruction that is to be executed
 *	and computes the address of the instruction that succeeds
 *	the current instruction; this address can then be used to insert
 *	breakpoints for the purposes of single stepping
 *
 *	\note	this routine reads target memory and therefore may
 *		have side effects
 *	\todo	this routine must be very well tested, probably with
 *		a simulator running in parallel, and with automatic
 *		instruction opcodes generation - thus, with the simulator
 *		running in parallel, it can be tested if this routine
 *		properly computes the address of the next instruction
 *
 *	\param	ctx	context to work in
 *	\param	next_insn_addr	a pointer to where to store the address
 *			of the instruction which will be executed after
 *			the current instruction
 *	\param	is_probably_call_insn	a pointer to a flag which is set
 *			(to nonzero) in case the current instruction
 *			looks like a call to a subroutine, and is reset
 *			(to zero) otherwise; currently, there are several cases
 *			in which instructions are identified as function calls:
 *				- a branch with link (bl) instruction or a
 *				  branch with link and exchange instruction -
 *				  this is always treated as a subroutine call
 *				- one of a:
 *					- branch with exchange (bx) instruction
 *					- a move (move) instruction
 *					- a load register (ldr) instruction
 *				that modifies the program counter while at
 *				the same time the link register contains
 *				the address of the immediately succeeding
 *				the program counter modifying instruction;
 *				as far as i(sgs) was able to decipher the
 *				gcc sources (in files arm.md and arm.c in
 *				the gcc source code tree), these are the
 *				methods employed by the gcc compiler to
 *				perform a subroutine call, but then
 *				again - i might be missing something;
 *				apparently, this doesnt quite work for
 *				sibling/tailcalls, but these should really
 *				only reside in optimized code, which is
 *				a whole another matter
 *			this parameter can be null if this information
 *			is not needed
 *
 *	\note	according to the file arm.md in the source code tree
 *		of the gcc, it is possible that code for sibling calls
 *		be implemented via branch instructions
 *
 *	\todo	maybe consult someone competent on the matter of arm
 *		code generation to make sure the call detection here
 *		is reliable enough; fix comments above when this is done
 *	\todo	handle tailcalls somehow; fix comments above when this
 *		is done
 *	\todo	maybe handle iterworking here, or maybe handle it in
 *		the executor
 *
 *	\return	the length (in bytes) of the instruction residing at
 *		the current program counter address
 *	\todo	define and return error codes here
 *
 */
int arm_insn_decode(struct gear_engine_context * ctx, ARM_CORE_WORD * next_insn_addr, int * is_probably_call_insn)
{
ARM_CORE_WORD insn;
ARM_CORE_WORD pc;
ARM_CORE_WORD new_pc;
ARM_CORE_WORD psr;
int nbytes;
/* this flag is not always set if the program counter is directly modified;
 * instead, it is set only when the program counter is directly modified by
 * an instruction that can be used for the generation of a subroutine
 * call instruction sequence */
int is_pc_call_modified;
int is_bl_insn;

	/* initialize */
	is_pc_call_modified = 0;
	is_bl_insn = 0;
	if (is_probably_call_insn)
		*is_probably_call_insn = 0;

	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx, 0, BIT17, &psr) !=
			GEAR_ERR_NO_ERROR)
		panic("");
	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx, 0, BIT15, &pc) !=
			GEAR_ERR_NO_ERROR)
		panic("");

	/* by default, the instruction does not modify the pc */
	new_pc = pc + 4;

	nbytes = sizeof insn;
	if (ctx->cc->core_mem_read(ctx, &insn, pc, &nbytes) !=
			GEAR_ERR_NO_ERROR || nbytes != sizeof insn)
		panic("");

	/* first, process the special case where the condition code
	 * of the instruction is 'all-ones' - an unconditional
	 * instruction */
	if (GET_BITS(29, 31, insn) == 0xf)
	{
		/*! \todo	handle this case */
		panic("");
	}
	if (!cond_code_check(psr, insn))
	{
		/* condition not true */
		*next_insn_addr = new_pc;
		return sizeof(ARM_CORE_WORD);
	}
	/* otherwise, the instruction will be executed, and
	 * we must therefore decode it to see if any changes
	 * to the program counter will occur in order to properly
	 * determine the location of the successive instruction */
	switch (GET_BITS(25, 27, insn))
	{
		case 0:
			if (GET_BIT(4, insn) == 0)
			{
				if (GET_BIT(20, insn) == 0 &&
						GET_BITS(23, 24, insn) == 2)
				{
					/* miscellaneous instructions */
					if (GET_BITS(20, 21, insn) == 0
							&& GET_BITS(4, 7, insn) == 0)
					{
						/* move status register
						 * to register */
						if (GET_BITS(12, 15, insn) == 15)
							panic("unpredictable behavior");
					}
					else if (GET_BITS(20, 22, insn) == 2
							&& GET_BITS(5, 7, insn) == 1)
					{
						/* branch/exchange instruction
						 * set to java */
						panic("");
					}
					else if (GET_BIT(7, insn) == 1 && GET_BIT(20, insn) == 0)
					{
						/* signed multiplies, type 2 */
						panic("");
					}
					else if (GET_BITS(4, 7, insn) == 0 &&
							GET_BIT(21, insn) == 1)
					{
						/* move register to status register (msr) */
						/* instruction cannot modify the pc */
					}
					else
					{
						panic("undefined insn");
					}
				}
				else
				{
					/* data processing, immediate shift */
					new_pc = addr_mode_1(ctx, pc, insn, &is_pc_call_modified);
				}
			}
			else
			{
				if (GET_BIT(7, insn) == 0)
				{
					if (GET_BIT(20, insn) == 0 &&
							GET_BITS(23, 24, insn) == 2)
					{
						/* miscellaneous instructions */
						if (GET_BITS(20, 22, insn) == 2
								&& GET_BITS(4, 7, insn) == 1)
						{
							ARM_CORE_WORD branch_target;
							/* branch/exchange instruction set thumb */
							/* determine branch target */
							is_pc_call_modified = 1;
							branch_target = core_reg_read(ctx, GET_BITS(0, 3, insn));
							if (branch_target & 1)
								/* attempt to switch to thumb */
								panic("");
							if ((branch_target & 3) == 2)
								panic("branch with exchange target misaligned, behavior is unpredictable\n");
							new_pc = branch_target;
						}
						else if (GET_BITS(20, 22, insn) == 6
								&& GET_BITS(4, 7, insn) == 1)
						{
							/* count leading zeros */
							if (GET_BITS(12, 15, insn) == 15)
								panic("");
						}
						else if (GET_BITS(20, 22, insn) == 2
								&& GET_BITS(4, 7, insn) == 3)
						{
							/* branch and link/exchange
							 * instruction set thumb */
							panic("");
						}
						else if (GET_BITS(4, 7, insn) == 5)
						{
							/* saturating add/subtract */
							panic("");
						}
						else if (GET_BITS(20, 22, insn) == 2
								&& GET_BITS(4, 7, insn) == 7)
						{
							/* software breakpoint */
							panic("");
						}
						else
						{
							/* nothing more to do */
						}
					}
					else
					{
						/* data processing, register shifts */
						new_pc = addr_mode_1(ctx, pc, insn, &is_pc_call_modified);
					}
				}
				else
				{
					/* multiplies and
					 * extra loads/stores */
					if (GET_BITS(0, 3, insn) == 15
							|| GET_BITS(8, 11, insn) == 15
							|| GET_BITS(12, 15, insn) == 15
							|| GET_BITS(16, 19, insn) == 15)
						panic("unpredictable behavior");
				}
			}
			break;
		case 1:
			if (GET_BITS(23, 24, insn) == 2 &&
					GET_BITS(20, 21, insn) == 0)
			{
				/* undefined */
				panic("");
			}
			else if (GET_BITS(23, 24, insn) == 2 &&
					GET_BITS(20, 21, insn) == 2)
			{
				/* move immediate to status register */
			}
			else
			{
				/* data processing immediate */
				if (GET_BITS(23, 24, insn) == 2
						&& GET_BIT(20, insn) == 0)
				{
					/* move immediate to status register */
				}
				else
					new_pc = addr_mode_1(ctx, pc, insn, &is_pc_call_modified);
			}
			break;
		case 2:
			/* load/store immediate offset */
			new_pc = addr_mode_2(ctx, pc, insn, &is_pc_call_modified);
			break;
		case 3:
			if (GET_BIT(4, insn) == 0)
			{
				/* load/store register offset */
				new_pc = addr_mode_2(ctx, pc, insn, &is_pc_call_modified);
			}
			else
			{
				if (GET_BITS(20, 24, insn) == 0x1f &&
						GET_BITS(4, 7, insn) == 0xf)
				{
					/* architecturally undefined */
					panic("");
				}
				else
				{
					/* media instructions */
					panic("");
				}
			}
			break;
		case 4:
			/* load/store multiple */
			new_pc = addr_mode_4(ctx, pc, insn);
			break;
		case 5:
			/* branch and branch with link */
			/* see if this is a branch with link instruction */
			if (GET_BIT(24, insn) == 1)
				is_bl_insn = 1;
			new_pc = GET_BITS(0, 23, insn);
			new_pc <<= 2;
			/* sign extend */
			if (GET_BIT(23, insn))
				new_pc |= 0xfc000000;
			new_pc += pc + /* adjust the pc value */ 8;
			break;
		case 6:
			/* coprocessor load/store and double register
			 * transfers */
			new_pc = addr_mode_5(ctx, pc, insn);
			break;
		case 7:
			if (GET_BIT(24, insn) == 0)
			{
				if (GET_BIT(4, insn) == 0)
				{
					/* coprocessor data processing */
					gprintf("cant disassemble opcode 0x%08x at address 0x%08x\n", insn, pc);
					panic("");
				}
				else
				{
					/* coprocessor register transfers */
					panic("");
				}
			}
			else
			{
				/* swi */
				panic("");
			}
			break;
	}

	*next_insn_addr = new_pc;
	/* see if this instruction looks like a part of a subroutine
	 * call instruction sequence */
	if (is_bl_insn)
		/* this is always regarded as an subroutine call instruction */
		if (is_probably_call_insn)
			*is_probably_call_insn = 1;
	if (is_pc_call_modified)
	{
		ARM_CORE_WORD lr;
		/* examine the link register */
		lr = core_reg_read(ctx, 14);
		if (lr == pc + 4)
			/* with high probability, we have a subroutine
			 * call instruction sequence */
			if (is_probably_call_insn)
				*is_probably_call_insn = 1;
	}
	return sizeof(ARM_CORE_WORD);
}

