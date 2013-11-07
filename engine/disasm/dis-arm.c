#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../include/engine-err.h"
#include "../include/target/arm/target-defs.h"
#include "../dwarf-common.h"
#include "../gear-engine-context.h"
#include "../core-access.h"

#define printf miprintf


#define panic(__x)	do { printf("%s, %i: %s\n", __FILE__, __LINE__, __x); while(1); } while (0)

/*! returns the bit stride of a word, between the __start-th and the __end-th bits, inclusive
 *
 * here, __end must be greater than or equal to __start */
#define GET_BITS(__start, __end, __data) \
	(((__data) >> (__start)) & ((1 << ((__end) - (__start) + 1)) - 1))
/*! extracts a single bit from a data word */
#define GET_BIT(__x, __data) (((__data) >> (__x)) & 1)



static void dump_cond_code(ARM_CORE_WORD insn)
{
	printf("%s",
			GET_BITS(28, 31, insn)[(const char * const[16])
	{
		"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
		"hi", "ls", "ge", "lt", "gt", "le", ("al", ""), "<invalid>"
	}]
	);
}

static void dump_alu_opcode(ARM_CORE_WORD insn)
{
	printf("%s",
			GET_BITS(21, 24, insn)[(const char * const[16])
	{
		"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
		"tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"
	}]
	);
}

static bool is_alu_opcode_mov_insn(ARM_CORE_WORD insn)
{
			return GET_BITS(21, 24, insn)[(const bool [16])
	{
		false, false, false, false, false, false, false, false,
		false, false, false, false, false, true, false, true,
	}];
}


static bool is_alu_opcode_test_insn(ARM_CORE_WORD insn)
{
			return GET_BITS(21, 24, insn)[(const bool [16])
	{
		false, false, false, false, false, false, false, false,
		true, true, true, true, false, false, false, false,
	}];
}

static ARM_CORE_WORD addr_mode_1(struct gear_engine_context * ctx, ARM_CORE_WORD insn, ARM_CORE_WORD pc)
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

	dump_alu_opcode(insn);
 	dump_cond_code(insn);
	if (GET_BIT(20, insn))
		printf("s");
	printf("\t");
	printf("r%i, ", GET_BITS(12, 15, insn));
	if (!is_alu_opcode_test_insn(insn) && !is_alu_opcode_mov_insn(insn))
		printf("r%i, ", GET_BITS(16, 19, insn));
	/* last - process the shifter operand */
	switch (GET_BITS(25, 27, insn))
	{
		case 1:
			{
				ARM_CORE_WORD t;
				int i;
				t = GET_BITS(0, 7, insn);
				i = GET_BITS(8, 11, insn);
				/* this is times two */
				i <<= 1;
				/* rotate right */
				t = (t >> i) | (t << (32 /* bits in an arm word */ - i));
				printf("#0x%08x", t);
			}
			break;
		case 0:
			if (GET_BIT(4, insn))
			{
	 
				if (GET_BIT(7, insn) != 0)
					panic("");
				else
				{
					printf("r%i, %s r%i", GET_BITS(0, 3, insn), GET_BITS(5, 6, insn)
							[(char * [4])
							{
								"lsl",
								"lsr",
								"asr",
								"ror",
							}],
							GET_BITS(8, 11, insn));
				}
			}
			else
			{
				/* bit(4) == 0 */
				if (GET_BITS(7, 11, insn) == 0)
					/* a special case - rotate right with extend */
					printf("r%i, rrx", GET_BITS(0, 3, insn));
				else
				{
					if (GET_BITS(5, 6, insn) == 0 || GET_BITS(5, 6, insn) == 3)
						printf("r%i, %s #%i", GET_BITS(0, 3, insn), GET_BITS(5, 6, insn)
								[(char * [4])
								{
									"lsl",
									"lsr",
									"asr",
									"ror",
								}],
								GET_BITS(8, 11, insn));
					else
						printf("r%i, %s #%i", GET_BITS(0, 3, insn), GET_BITS(5, 6, insn)
								[(char * [4])
								{
									"lsl",
									"lsr",
									"asr",
									"ror",
								}],
								GET_BITS(8, 11, insn) ? GET_BITS(8, 11, insn) : 32);
				}
			}

			break;
		default:
			panic("");
	}
}


static ARM_CORE_WORD addr_mode_2(struct gear_engine_context * ctx, ARM_CORE_WORD insn, ARM_CORE_WORD pc)
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

	if (GET_BIT(20, insn))
		printf("ldr");
	else
		printf("str");
 	dump_cond_code(insn);
	if (GET_BIT(22, insn))
		printf("b");
	if (GET_BIT(24, insn) == 0 && GET_BIT(21, insn) == 1)
		printf("t");
	printf("\tr%i, ", GET_BITS(12, 15, insn));

	/* finally, process the addressing mode */
	printf("[r%i", GET_BITS(16, 19, insn));
	switch(GET_BITS(25, 27, insn))
	{
		case 2:
			/* immediate offset/index */
			if (GET_BIT(24, insn) == 0)
				/* post indexed access */
				printf("], #%c%i", (GET_BIT(23, insn) == 0) ? '-':'+', GET_BITS(0, 11, insn));
			else
				printf(", #%c%i]%c", (GET_BIT(23, insn) == 0) ? '-':'+', GET_BITS(0, 11, insn), (GET_BIT(21, insn) == 0) ? ' ':'!');
			break;
		case 3:
			if (GET_BITS(4, 11, insn) == 0)
			{
				/* register offset/index */
				if (GET_BIT(24, insn) == 0)
					/* post indexed access */
					printf("], %cr%i", (GET_BIT(23, insn) == 0) ? '-':'+', GET_BITS(0, 3, insn));
				else
					printf(", #%cr%i]%c", (GET_BIT(23, insn) == 0) ? '-':'+', GET_BITS(0, 3, insn), (GET_BIT(21, insn) == 0) ? ' ':'!');
			}
			break;
		default:
			panic("");
	}

}


static ARM_CORE_WORD addr_mode_3(struct gear_engine_context * ctx, ARM_CORE_WORD insn, ARM_CORE_WORD pc)
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


	printf("[r%i", GET_BITS(16, 19, insn));
	/* finally, process the addressing mode */
	if (GET_BIT(22, insn) == 0)
	{
		/* register offset/index */
		if (GET_BITS(8, 11, insn) != 0)
			printf("unpredictable");
		else if (GET_BIT(24, insn) == 0)
		{
			if (GET_BIT(21, insn) == 1)
				printf("unpredictable");
			else
				printf("], %cr%i", (GET_BIT(23, insn) == 0) ? '-':'+', GET_BITS(0, 3, insn));
		}
		else
			if (GET_BIT(21, insn) == 0)
				printf(", %cr%i]%c", (GET_BIT(23, insn) == 0) ? '-':'+', GET_BITS(0, 3, insn), (GET_BIT(21, insn) == 0) ? ' ':'!');
	}
	else
	{
		/* immediate offset/index */
		if (GET_BIT(24, insn) == 0)
		{
			if (GET_BIT(21, insn) == 1)
				printf("unpredictable");
			else
				printf("], #%c%i", (GET_BIT(23, insn) == 0) ? '-':'+', GET_BITS(0, 3, insn) | (GET_BITS(8, 11, insn) << 4));
		}
		else
			if (GET_BIT(21, insn) == 0)
				printf(", #%c%i]%c", (GET_BIT(23, insn) == 0) ? '-':'+', GET_BITS(0, 3, insn) | (GET_BITS(8, 11, insn) << 4), (GET_BIT(21, insn) == 0) ? ' ':'!');
	}

}

static ARM_CORE_WORD addr_mode_4(struct gear_engine_context * ctx, ARM_CORE_WORD insn, ARM_CORE_WORD pc)
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

int i, j;

	printf("%s", GET_BIT(20, insn) ? "ldm" : "stm");
	dump_cond_code(insn);
	printf("%s", ((GET_BIT(24, insn) << 1) | (GET_BIT(23, insn) << 0))[
			(const char * [4]){
			"da",
			"ia",
			"db",
			"ib",
			}
			]
	      );
	printf("r%i", GET_BITS(16, 19, insn));
	if(GET_BIT(21, insn))
		printf("!");
	printf(", {");
	j = GET_BITS(0, 15, insn);
	if (!j)
		printf("unpredictable");
	else
		for (i = 0; i < NR_ARM_CORE_REGS; i++)
			if (j & (1 << i))
				printf("r%i, ", i);
	printf("}");
	if (GET_BIT(22, insn))
		printf("^");

}

static ARM_CORE_WORD addr_mode_5(struct gear_engine_context * ctx, ARM_CORE_WORD insn, ARM_CORE_WORD pc)
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

	panic("");
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

}


dump_misc_insns(struct gear_engine_context * ctx, ARM_CORE_WORD insn, ARM_CORE_WORD pc)
{
	/* here, bits(25, 27) of insn equal 000b, bits(23, 24) == 10b,
	 * bit(20) == 0, and
	 * (bit(4) == 0) or (bit(4) == 1 and bit(7) == 0 */
	if (GET_BIT(7, insn) == 1)
	{
		panic("");
		//dump_signed_multiplies_type_2();
	}
	/* here, bit(7) == 0 */
	else switch (GET_BITS(4, 6, insn))
	{
		case 0:
			if (GET_BIT(21, insn) == 0)
			{
				if (GET_BITS(0, 3, insn) != 0 || GET_BITS(8, 11, insn) != 0
						|| GET_BITS(16, 19, insn) != 0xf)
					printf("<warning> - possibly unpredictable");
				printf("mrs");
				dump_cond_code(insn);
				printf("r%i,", GET_BITS(12, 15, insn));
				printf("%s", GET_BIT(22, insn) ? "spsr" : "cpsr");
			}
			else /* bit(21) == 1 */
			{
				if (GET_BITS(8, 11, insn) != 0 || GET_BITS(12, 15, insn) != 0xf)
					printf("<warning> - possibly unpredictable");
				printf("msr");
				dump_cond_code(insn);
				printf("%s", GET_BIT(22, insn) ? "spsr" : "cpsr");
				printf("_");
				/* dump flag fields */
				if (GET_BIT(16, insn))
					printf("c");
				if (GET_BIT(17, insn))
					printf("x");
				if (GET_BIT(18, insn))
					printf("s");
				if (GET_BIT(19, insn))
					printf("f");
				printf("\t, r%i", GET_BITS(0, 3, insn));
			}
			break;
		case 1:
			if (GET_BITS(21, 22, insn) == 1)
			{
				if (GET_BITS(8, 19, insn) != 0xfff)
					printf("<warning> - possibly unpredictable");
				printf("bx");
				dump_cond_code(insn);
				printf("\tr%i", GET_BITS(0, 3, insn));
			}
			else if (GET_BITS(21, 22, insn) == 3)
			{
				if (GET_BITS(8, 11, insn) != 0xf || GET_BITS(16, 19, insn) != 0xf)
					printf("<warning> - possibly unpredictable");
				printf("clz");
				dump_cond_code(insn);
				printf("\tr%i, r%i", GET_BITS(12, 15, insn), GET_BITS(0, 3, insn));
			}
			else
				printf("<undefined>");
			break;
		case 2:
			if (GET_BITS(21, 22, insn) == 1)
			{
				if (GET_BITS(8, 19, insn) != 0xfff)
					printf("<warning> - possibly unpredictable");
				printf("bxj");
				dump_cond_code(insn);
				printf("\tr%i", GET_BITS(0, 3, insn));
			}
			else
				printf("<undefined>");
			break;
		case 3:
			if (GET_BITS(21, 22, insn) == 1)
			{
				if (GET_BITS(8, 19, insn) != 0xfff)
					printf("<warning> - possibly unpredictable");
				printf("blx");
				dump_cond_code(insn);
				printf("\tr%i", GET_BITS(0, 3, insn));
			}
			else
				printf("<undefined>");
			break;
		case 4:
			printf("<undefined>");
			break;
		case 5:
			if (GET_BITS(8, 11, insn) != 0)
				printf("<warning> - possibly unpredictable");
			printf("%s", GET_BITS(21, 22, insn)[
					(const char * [4]){
					"qadd",
					"qsub",
					"qdadd",
					"qdsub",
					}
					]
			      );
			dump_cond_code(insn);
			printf("\tr%i, r%i, r%i", GET_BITS(12, 15, insn), GET_BITS(0, 3, insn),
					GET_BITS(16, 19, insn));
			break;
		default:
		case 6:
			printf("<undefined>");
			break;
		case 7:
			if (GET_BITS(21, 22, insn) == 1)
				/* software breakpoint */
				printf("bkpt\t#%i", GET_BITS(0, 3, insn)
						| (GET_BITS(8, 19, insn) << 4));
			else
				printf("<undefined>");
			break;
	}
}

static void dump_uncond_insn_extension_space_insn(void)
{
	panic("");
}

static void dump_b_bl(struct gear_engine_context * ctx, ARM_CORE_WORD insn, ARM_CORE_WORD pc)
{
int t;
	printf("b");
	if (GET_BIT(24, insn))
		printf("l");
	dump_cond_code(insn);
	t = GET_BITS(0, 23, insn);
	if (GET_BIT(23, insn))
	{
		/* complement & sign extend */
		t = (1 << 24) - t;
		t = -t;
	}
	t <<= 2;
	printf("\t 0x%08x", (ARM_CORE_WORD) t + pc + 2 * sizeof(ARM_CORE_WORD));
}

int disassemble_arm_insn(struct gear_engine_context * ctx, ARM_CORE_WORD insn, ARM_CORE_WORD pc)
{
	/* see if the condition code is all ones (a special case) */
	if (GET_BITS(28, 31, insn) == 0xf)
	{
		printf("unpredictable");
		/*
		if (arch == v4)
		{
			printf("unpredictable";
		}
		else
		{
			/ * arch >= v5 * /
			dump_uncond_insn_extension_space_insn();
		}
		*/
	}
	/* here, the condition code does not equal 1111b */
	else switch (GET_BITS(25, 27, insn))
	{
		case 0:
			if (GET_BIT(4, insn) == 1 && GET_BIT(7, insn) == 1)
			{
				/*! \todo	fix this */
				printf("cant disassemble opcode 0x%08x", insn);
				return sizeof(ARM_CORE_WORD);
				//dump_multiplies_extra_load_stores();
				panic("");
			}
			else if (GET_BITS(23, 24, insn) == 2 && GET_BIT(20, insn) == 0)
			{
				if (GET_BIT(4, insn) == 0)
					dump_misc_insns(ctx, insn, pc);
				/* here bit(4) == 1 and bit(7) == 0*/
				else /* if (GET_BIT(7, insn) == 0) */
					dump_misc_insns(ctx, insn, pc);
			}
			else
				addr_mode_1(ctx, insn, pc);
			break;
		case 1:
			if (GET_BITS(23, 24, insn) == 2 && GET_BIT(20, insn) == 0)
			{
				if (GET_BIT(21, insn) == 0)
					printf("<undefined>");
				else /* if bit(21) == 1 */
					//dump_mov_imm_to_status_reg();
					panic("");
			}
			else
				addr_mode_1(ctx, insn, pc);
			break;
		case 2:
			addr_mode_2(ctx, insn, pc);
			break;
		case 3:
			if (GET_BITS(20, 24, insn) == 0x1f && GET_BITS(4, 7, insn) == 0xf)
				printf("<architecturally undefined>");
			else if (GET_BIT(4, insn) == 0)
				addr_mode_2(ctx, insn, pc);
			else /* if (bit(4) == 1) */
				//dump_media_insn();
				panic("");
			break;
		case 4:
			addr_mode_4(ctx, insn, pc);
			break;
		case 5:
			dump_b_bl(ctx, insn, pc);
			break;
		case 6:
			//dump_coproc_load_store_and_double_reg_xfers();
			panic("");
			break;
		case 7:
			if (GET_BIT(24, insn) == 0)
			{
				if (GET_BIT(4, insn) == 0)
				{
					/*! \todo	fix this */
					printf("cant disassemble opcode 0x%08x", insn);
					return sizeof(ARM_CORE_WORD);
					//dump_coproc_data_proc();
					panic("");
				}
				else /* if (bit(4) == 1 */
					//dump_coproc_reg_xfers();
					panic("");
			}
			else /* if bit(24) == 1 */
				//dump_swi();
				panic("");
			break;
		default:
			/* impossible */
			panic("");
	}
	return sizeof(ARM_CORE_WORD);
}

int main_testdrive(void)
{
ARM_CORE_WORD insn;
int i, j;
int fd;

/*
	while (scanf("%x", &insn) != EOF)
		disassemble_arm_insn(0, insn);
		*/
	/* test for base opcode 000, bit4) == 0 */
	if ((fd = creat("out.bin", 0)) == -1)
	{
		perror("failed to create output file");
		exit(1);
	}
	if (0) for (i = 0; i < (1 << 13); i++)
	{
/*
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 0|0|opcode |s|  rn   |  rd   |shift_imm|shf|0|  rm   |
 * */
		insn = 0xe0000000;
		j = i;
		insn |= (j & ((1 << 7) - 1)) << 5;
		j >>= 7;
		insn |= (j & ((1 << 5) - 1)) << 20;
		disassemble_arm_insn(0, insn, 0);
		printf("\n");
		write(fd, &insn, sizeof insn);
	}
	if (0) for (i = 0; i < (1 << 7); i++)
	{
/*
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * | cond  |0 0|0|opcode |s|  rn   |  rd   |  rs   |0|shf|1|  rm   |
 * */
		insn = 0xe0000000 | (1 << 4);
		j = i;
		insn |= (j & ((1 << 2) - 1)) << 5;
		j >>= 7;
		insn |= (j & ((1 << 5) - 1)) << 20;
		disassemble_arm_insn(0, insn, 0);
		printf("\n");
		write(fd, &insn, sizeof insn);
	}
	close(fd);
	return 0;
}


int arm_disassemble_insn(struct gear_engine_context * ctx, ARM_CORE_WORD addr,
		int (*print_fn)(const char * format, va_list ap))
{
unsigned int nbytes;
ARM_CORE_WORD insn;

	nbytes = sizeof insn;
	if (ctx->cc->core_mem_read(ctx, &insn, addr, &nbytes)
			!= GEAR_ERR_NO_ERROR)
		panic("");

	printf("[0x%08x]\t", insn);
	return disassemble_arm_insn(ctx, insn, addr);
}

void init_arm_disassemble(void) {}
