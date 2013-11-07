/* i386 instruction format:
 *
 * prefix - upto four prefixes, 1 byte each (optional)
 * opcode - 1 byte
 * ModR/M - 1 byte (if required) - bits[6-7] - Mod, bits[3-5] - Reg/Opcode, bits[0-2] - R/M
 * SIB - 1 byte (if required) - bits[6-7] - Scale, bits[3-5] - Index, bits[0-2] - Base
 * Displacement - address displacement of 1, 2, or 4 bytes or none
 * Immediate data of 1, 2, 4 bytes or none
 */

#include <stdarg.h>
#include <stdlib.h>
#include <dis-asm.h>
#include <stdbool.h>
#include <stdarg.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gprintf.h"
#include "gear-constants.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "engine-err.h"
#include "core-access.h"
#include "util.h"

static int read_memory_func(bfd_vma memaddr, bfd_byte *myaddr, unsigned int length,
		struct disassemble_info *info)
{
struct gear_engine_context * ctx = (struct gear_engine_context *)info->application_data;

	if (ctx->cc->core_mem_read(ctx, myaddr, memaddr, &length)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	return 0;
}

static int my_fprintf (void * stream, const char* format, ...)
{
	return 0;
}

static struct disassemble_info dis_info =
{
  .fprintf_func = my_fprintf,
  .stream = 0,
  .application_data = 0,

  /* Target description.  We could replace this with a pointer to the bfd,
     but that would require one.  There currently isn't any such requirement
     so to avoid introducing one we record these explicitly.  */
  /* The bfd_flavour.  This can be bfd_target_unknown_flavour.  */
  .flavour = bfd_target_unknown_flavour,
  /* The bfd_arch value.  */
  .arch = bfd_arch_i386,
  /* The bfd_mach value.  */
  .mach = bfd_mach_i386_i386,
  /* Endianness (for bi-endian cpus).  Mono-endian cpus can ignore this.  */
  .endian = BFD_ENDIAN_LITTLE,
  /* An arch/mach-specific bitmask of selected instruction subsets, mainly
     for processors with run-time-switchable instruction sets.  The default,
     zero, means that there is no constraint.  CGEN-based opcodes ports
     may use ISA_foo masks.  */
  .insn_sets = 0,

  /* Some targets need information about the current section to accurately
     display insns.  If this is NULL, the target disassembler function
     will have to make its best guess.  */
  .section = 0,

  /* An array of pointers to symbols either at the location being disassembled
     or at the start of the function being disassembled.  The array is sorted
     so that the first symbol is intended to be the one used.  The others are
     present for any misc. purposes.  This is not set reliably, but if it is
     not NULL, it is correct.  */
  .symbols = 0,
  /* Number of symbols in array.  */
  .num_symbols = 0,

  /* Symbol table provided for targets that want to look at it.  This is
     used on Arm to find mapping symbols and determine Arm/Thumb code.  */
  ////!!!!.symtab = 0,
  ////!!!!.symtab_pos = 0,
  ////!!!!.symtab_size = 0,

  /* For use by the disassembler.
     The top 16 bits are reserved for public use (and are documented here).
     The bottom 16 bits are for the internal use of the disassembler.  */
  .flags = 0,
#define INSN_HAS_RELOC	0x80000000
  .private_data = 0,

  /* Function used to get bytes to disassemble.  MEMADDR is the
     address of the stuff to be disassembled, MYADDR is the address to
     put the bytes in, and LENGTH is the number of bytes to read.
     INFO is a pointer to this struct.
     Returns an errno value or 0 for success.  */
  /*
  int (*read_memory_func)
    (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length,
     struct disassemble_info *info);
     */
  .read_memory_func = read_memory_func,

  /* Function which should be called if we get an error that we can't
     recover from.  STATUS is the errno value from read_memory_func and
     MEMADDR is the address that we were trying to read.  INFO is a
     pointer to this struct.  */
  /*
  void (*memory_error_func)
    (int status, bfd_vma memaddr, struct disassemble_info *info);
    */
  .memory_error_func = 3,

  /* Function called to print ADDR.  */
  /*
  void (*print_address_func)
    (bfd_vma addr, struct disassemble_info *info);
    */
  .print_address_func = generic_print_address,

  /* Function called to determine if there is a symbol at the given ADDR.
     If there is, the function returns 1, otherwise it returns 0.
     This is used by ports which support an overlay manager where
     the overlay number is held in the top part of an address.  In
     some circumstances we want to include the overlay number in the
     address, (normally because there is a symbol associated with
     that address), but sometimes we want to mask out the overlay bits.  */
	  /*
  int (* symbol_at_address_func)
    (bfd_vma addr, struct disassemble_info * info);
    */
	  .symbol_at_address_func = 5,

  /* Function called to check if a SYMBOL is can be displayed to the user.
     This is used by some ports that want to hide special symbols when
     displaying debugging outout.  */
	  /*
  bfd_boolean (* symbol_is_valid)
    (asymbol *, struct disassemble_info * info);
    */
	  .symbol_is_valid = 6,

  /* These are for buffer_read_memory.  */
  .buffer = 0,
  .buffer_vma = 0,
  .buffer_length = 0,

  /* This variable may be set by the instruction decoder.  It suggests
      the number of bytes objdump should display on a single line.  If
      the instruction decoder sets this, it should always set it to
      the same value in order to get reasonable looking output.  */
  .bytes_per_line = 0,

  /* The next two variables control the way objdump displays the raw data.  */
  /* For example, if bytes_per_line is 8 and bytes_per_chunk is 4, the */
  /* output will look like this:
     00:   00000000 00000000
     with the chunks displayed according to "display_endian". */
  .bytes_per_chunk = 0,
  .display_endian = 0,

  /* Number of octets per incremented target address
     Normally one, but some DSPs have byte sizes of 16 or 32 bits.  */
  .octets_per_byte = 0,

  /* The number of zeroes we want to see at the end of a section before we
     start skipping them.  */
  ////!!!!.skip_zeroes = 0,

  /* The number of zeroes to skip at the end of a section.  If the number
     of zeroes at the end is between SKIP_ZEROES_AT_END and SKIP_ZEROES,
     they will be disassembled.  If there are fewer than
     SKIP_ZEROES_AT_END, they will be skipped.  This is a heuristic
     attempt to avoid disassembling zeroes inserted by section
     alignment.  */
  ////!!!!.skip_zeroes_at_end = 0,

  /* Whether the disassembler always needs the relocations.  */
  ////!!!!.disassembler_needs_relocs = FALSE,

  /* Results from instruction decoders.  Not all decoders yet support
     this information.  This info is set each time an instruction is
     decoded, and is only valid for the last such instruction.

     To determine whether this decoder supports this information, set
     insn_info_valid to 0, decode an instruction, then check it.  */

  .insn_info_valid = 0,		/* Branch info has been set. */
#if 0
  char branch_delay_insns;	/* How many sequential insn's will run before
				   a branch takes effect.  (0 = normal) */
  char data_size;		/* Size of data reference in insn, in bytes */
  enum dis_insn_type insn_type;	/* Type of instruction */
  bfd_vma target;		/* Target address of branch or dref, if known;
				   zero if unknown.  */
  bfd_vma target2;		/* Second target address for dref2 */
#endif

  /* Command line options specific to the target disassembler.  */
  .disassembler_options = 0,

};

static bool is_cond_true(int condition_code, int eflags)
{
/*
 * table taken from:
 *
 * Intel® 64 and IA-32 Architectures
 * Software Developer.s Manual
 * Volume 1:
 * Basic Architecture
 * APPENDIX B
 * EFLAGS CONDITION CODES
Mnemonic (cc)	| Condition Tested For	| Instruction Subcode	| Status Flags Setting
--------------------------------------------------------------------------------------
o		| overflow		| 0000			| of == 1
no		| no overflow		| 0001			| of == 0
b/na		| below			| 0010			| cf == 1
nb/ae		| not below		| 0011			| cf == 0
e/z		| equal			| 0100			| zf == 1
ne/nz		| not equal		| 0101			| zf == 0
be/na		| below or equal	| 0110			| (cf or zf) == 1
nbe/a		|neither below nor equal| 0111			| (cf or zf) == 0
s		| sign			| 1000			| sf == 1
ns		| no sign		| 1001			| sf == 0
p/pe		| parity even		| 1010			| pf == 1
np/po		| parity odd		| 1011			| pf == 0
l/nge		| less			| 1100			| (sf xor of) == 1
nl/ge		| not less		| 1101			| (sf xor of) == 0
le/ng		| less or equal		| 1110			| ((sf xor of) or zf) == 1
nle/g		| neither less nor equal| 1111			| ((sf xor of) or zf) == 0
*/
bool res;
enum I386_EFLAGS_COND_BITS
{
	EFLAGS_OF	= BIT11,
	EFLAGS_CF	= BIT0,
	EFLAGS_ZF	= BIT6,
	EFLAGS_SF	= BIT7,
	EFLAGS_PF	= BIT2,
};
bool of, cf, zf, sf, pf;

	of = (eflags & EFLAGS_OF) ? true : false;
	cf = (eflags & EFLAGS_CF) ? true : false;
	zf = (eflags & EFLAGS_ZF) ? true : false;
	sf = (eflags & EFLAGS_SF) ? true : false;
	pf = (eflags & EFLAGS_PF) ? true : false;

	res = false;
	switch (condition_code & 0xf)
	{
		case 0:
			res = of == true;
			break;
		case 1:
			res = of == false;
			break;
		case 2:
			res = cf == true;
			break;
		case 3:
			res = cf == false;
			break;
		case 4:
			res = zf == true;
			break;
		case 5:
			res = zf == false;
			break;
		case 6:
			res = (cf || zf) == true;
			break;
		case 7:
			res = (cf || zf) == false;
			break;
		case 8:
			res = sf == true;
			break;
		case 9:
			res = sf == false;
			break;
		case 10:
			res = pf == true;
			break;
		case 11:
			res = pf == false;
			break;
		case 12:
			res = (/*sf ^ of*/
				(sf && !of)
				|| (!sf && of)
					
					) == true;
			break;
		case 13:
			res = (/*sf ^ of*/
				(sf || of)
				&& (!sf || !of)
					
					) == false;
			break;
		case 14:
			res = (/*sf ^ of*/
				(sf == true && of == false)
				|| (sf == false && of == true)
				|| zf
					
					) == true;
			break;
		case 15:
			res = (/*sf ^ of*/
				(sf == true && of == false)
				|| (sf == false && of == true)
				|| zf
					
					) == false;
			break;
	}
	return res;
}

static ARM_CORE_WORD compute_32_addr_mode_result(struct gear_engine_context * ctx,
		unsigned char * mod_rm_insn_buf, int buf_len)
{
/*! \warning this table must match sys/user.h */
static const int reg_to_target_map[8] =
{
/*! \todo	this is inexact/incomplete, maybe incorrect
---------------------------------------------------------------------------------------------
i386 register	gcc_reg_nr	dwarf_reg_nr	ptrace_reg_nr	win32 winnt.h CONTEXT reg_nr
								registers are given as offsets
								from the GS register in the
								CONTEXT structure (in winnt.h)
---------------------------------------------------------------------------------------------
eax		0		0		6		9
ebx		3		3		0		6
ecx		2		1		1		8
edx		1		2		2		7
esi		4		6		3		5
edi		5		7		4		4
esp		7		4		15		14
ebp		6		5		5		10
eip		-		-		12		11
eflags		17		9		14		13
xcs		-		-		13		12
xds		-		-		7		3
xes		-		-		8		2
xss		-		-		16		15
xfs		-		-		9		1
xgs		-		-		10		0
---------------------------------------------------------------------------------------------
*/
#ifdef __LINUX__
	/* eax */6,
	/* ecx */1,
	/* edx */2,
	/* ebx */0,
	/* esp */15,
	/* ebp */5,
	/* esi */3,
	/* edi */4,
#else /* win32 */
	/* eax */9,
	/* ecx */8,
	/* edx */7,
	/* ebx */6,
	/* esp */14,
	/* ebp */10,
	/* esi */5,
	/* edi */4,
#endif	
};
ARM_CORE_WORD displacement_val;
ARM_CORE_WORD val;
int reg_nr;
/* the Mod and R/M fields of the insn addressing mode */
int mod, rm;
/* scale for sib addressing */
int scale;
/* index for sib addressing */
int idx;
/* base for sib addressing */
int base;
unsigned char sib;

int nbytes;

	if (buf_len < 1)
		panic("");

	mod = (*mod_rm_insn_buf >> 6) & 3;
	rm = (*mod_rm_insn_buf >> 0) & 7;

	reg_nr = reg_to_target_map[rm];

	switch (mod)
	{
		default:
			/* no displacement needed */
			displacement_val = 0;
			break;
		case 1:
			/* 8 bit, sign extended displacement */
			if (buf_len != 2)
			{
				gprintf("buf_len == %i, expected 2\n", buf_len);
				gprintf("mod rm address mode byte: %02x\n", mod_rm_insn_buf[0]);
				panic("");
			}
			displacement_val = *((signed char *) (mod_rm_insn_buf + 1));
			gprintf("displacement is %i\n", displacement_val);
			break;
		case 2:
			/* 32 bit, sign extended displacement */
			displacement_val = *(((signed int *) (mod_rm_insn_buf + 1)));
			break;
	}

	/* handle special cases */
	if (mod != 3)
		switch (rm)
	{
		case 5:
			if (mod == 0)
			{
				if (buf_len != 5)
					panic("");
				/* disp32 - a 32 bit displacement */
				displacement_val = *(((signed int *) (mod_rm_insn_buf + 1)));
				gprintf("displacement is 0x%08x\n", displacement_val);
				nbytes = sizeof(ARM_CORE_WORD);
				if (ctx->cc->core_mem_read(ctx, &val, displacement_val, &nbytes) !=
						GEAR_ERR_NO_ERROR || nbytes != sizeof(ARM_CORE_WORD))
					panic("");
				gprintf("final_val: 0x%08x\n", val);
				break;
			}
		case 4:
			if (buf_len < 2)
				panic("");
			/* a sib byte follows... */
			sib = mod_rm_insn_buf[1];
			scale = sib >> 6;
			idx = (sib >> 3) & 7;
			base = sib & 7;	
			if (mod != 0 || scale != 2 || idx != 1 || base != 5)
				panic("");
			if (base == 5 && idx != 4)
			{
				if (ctx->cc->core_reg_read(ctx, 0,
							1 << reg_to_target_map[idx],
							&val)
						!= GEAR_ERR_NO_ERROR)
					panic("");
				val <<= scale;
				if (mod != 0)
					panic("");
				if (buf_len != 6)
					panic("");
				displacement_val = *(((signed int *) (mod_rm_insn_buf + 2)));
				displacement_val += val;
				nbytes = sizeof(ARM_CORE_WORD);
				gprintf("indirection addr is: 0x%08x\n", displacement_val);
				if (ctx->cc->core_mem_read(ctx, &val, displacement_val, &nbytes) !=
						GEAR_ERR_NO_ERROR || nbytes != sizeof(ARM_CORE_WORD))
					panic("");
				gprintf("val there is: 0x%08x\n", val);
			}
			else
				panic("");
			break;
		/* 32 bit, sign extended displacement * /
		displacement_val = *(((signed short int *) mod_rm_insn_buf)[1]);
		////return *---displacement_val;
		// */
	}
	else
	{
		if (ctx->cc->core_reg_read(ctx, 0,
					1 << reg_nr,
					&val)
				!= GEAR_ERR_NO_ERROR)
			panic("");
		gprintf("reg_val: 0x%08x\n", val);
		if (mod != 3)
		{
			val += displacement_val;
			nbytes = sizeof(ARM_CORE_WORD);
			if (ctx->cc->core_mem_read(ctx, &val, val, &nbytes) !=
					GEAR_ERR_NO_ERROR || nbytes != sizeof(ARM_CORE_WORD))
				panic("");
			gprintf("final_val: 0x%08x\n", val);
		}

		gprintf("reg_nr: %i\n", reg_nr);
		gprintf("disp: %i\n", (int)displacement_val);
		gprintf("computed val: 0x%08x\n", val);
	}

	return val;
}

/*! the maximum byte size of an instruction that can change the program counter */
#define MAX_CONTROL_XFER_INSN_SIZE_IN_BYTES	7

int arm_insn_decode(struct gear_engine_context * ctx, ARM_CORE_WORD * next_insn_addr, int * is_probably_call_insn)
{
int insn_len;
ARM_CORE_WORD eflags, pc, new_pc, core_word;
/*! the maximum byte size of an instruction that can change the program counter */
unsigned char opcode[MAX_CONTROL_XFER_INSN_SIZE_IN_BYTES];
bool is_call_insn;
int nbytes;

	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&pc)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	/* obtain instruction length */
	dis_info.application_data = ctx;
	insn_len = print_insn_i386(pc, &dis_info);
	if (!insn_len)
		panic("");

	/* by default - the instruction does not change the program counter */
	new_pc = *next_insn_addr = pc + insn_len;
	if (is_probably_call_insn)
		*is_probably_call_insn = 0;
	if (insn_len > MAX_CONTROL_XFER_INSN_SIZE_IN_BYTES)
		return insn_len;

	/* read eflags */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pstat_reg_nr(ctx),
				&eflags) !=
			GEAR_ERR_NO_ERROR)
		panic("");
	printf("read eflags: 0x%08x", (int)eflags);
	/*! \todo	handle prefixes here */

	/* fetch instruction opcode */
	nbytes = insn_len;
	if (ctx->cc->core_mem_read(ctx, opcode, pc, &nbytes) !=
			GEAR_ERR_NO_ERROR || nbytes != insn_len)
		panic("");
	{
		int i;
		gprintf("program counter: 0x%08x, decoding insn bytes: ", (unsigned int) pc);
		for (i = 0; i < insn_len; i++)
			miprintf("%02x ", opcode[i]);
		gprintf("\n");
	}

	is_call_insn = false;
	/* look up the instruction opcode */
	switch(opcode[0])
	{
		case 0xe8:
			/* call near relative, displacement
			 * relative to next instruction */
			if (insn_len != 5)
				panic("");
			is_call_insn = true;
			new_pc = opcode[1] | (opcode[2] << 8)
				| (opcode[3] << 16) | (opcode[4] << 24);
			new_pc += pc + insn_len;
			break;
		case 0x9a:
			/* call far absolute */
			panic("");
			break;
		case 0xff:
			/* miscellaneous instructions */
			switch ((opcode[1] >> 3) & 7)
			{
				case 3:
					/* call far absolute indirect */
					panic("");
					break;
				case 2:
					/* call near absolute indirect */
					gprintf("ok, 1400 at 040709\n");
					if (insn_len < 2 || insn_len > 6)
						panic("");
					is_call_insn = true;
					new_pc = compute_32_addr_mode_result(ctx, opcode + 1, insn_len - 1);
					break;
				case 5:
					/* jmp far */
					panic("");
					break;
				case 4:
					/* jmp near, absolute indirect -
					 * address == sign extended
					 * r/m32 */
					if (insn_len < 2 || insn_len > 7)
						panic("");
					new_pc = compute_32_addr_mode_result(ctx, opcode + 1, insn_len - 1);
					break;
				default:
					/* assume the instruction does not modify the program counter */
					break;
			}
			break;
		case 0x0f:
			switch (opcode[1])
			{

				case 0x87: case 0x83: case 0x82: case 0x86: case 0x84:
				case 0x8F: case 0x8D: case 0x8C: case 0x8E: case 0x85:
				case 0x81: case 0x8B: case 0x89: case 0x80: case 0x8A:
				case 0x88:
					/* conditional jumps - jcc rel32 sign extended, relative
					 * to next instruction */
					if (insn_len != 6)
						panic("");
					if (is_cond_true(opcode[1] & 0xf, eflags))
					{
						/* jump taken */
						new_pc = opcode[2] | (opcode[3] << 8)
							| (opcode[4] << 16) | (opcode[5] << 24);
						new_pc += pc + insn_len;
					}
					else
						/* jump not taken */
						;
							
					break;
				default:
					break;
			}
			break;
		case 0xc3:
			/* ret near */
			/* fetch the stack pointer value */
			if (ctx->cc->core_reg_read(ctx, 0, 1 << ctx->tdesc->get_target_sp_reg_nr(ctx),
						&core_word)
					!= GEAR_ERR_NO_ERROR)
				panic("");
			/* fetch the return address - the core word
			 * at the top of the (full descending) stack */
			nbytes = sizeof(ARM_CORE_WORD);
			if (ctx->cc->core_mem_read(ctx, &new_pc, core_word, &nbytes) !=
					GEAR_ERR_NO_ERROR || nbytes != sizeof(ARM_CORE_WORD))
				panic("");
			break;
		case 0xcb:
			/* ret far */
			panic("");
			break;
		case 0xc2:
			/* ret near, imm16 */
			panic("");
			break;
		case 0xca:
			/* ret far, imm16 */
			panic("");
			break;
		case 0xcc:
			/* int3 */
			panic("");
			break;
		case 0xcd:
			/* int */
			panic("");
			break;
		case 0xce:
			/* into */
			panic("");
			break;
		case 0xcf:
			/* iret */
			panic("");
			break;
		case 0xeb:
			/* jmp rel8 - short jump,
			 * 8 bit sign extended displacement
			 * relative to next instruction */
			if (insn_len != 2)
				panic("");
			new_pc = (signed)(signed char) opcode[1] + pc + insn_len;
			break;
		case 0xe9:
			/* jmp rel32 - near jump, 32 bit displacement
			 * relative to next instruction */
			if (insn_len != 5)
				panic("");
			new_pc = opcode[1] | (opcode[2] << 8)
				| (opcode[3] << 16) | (opcode[4] << 24);
			new_pc += pc + insn_len;
			break;
		case 0xea:
			/* jmp far */
			panic("");
			break;

		case 0x77: case 0x73: case 0x72: case 0x76: case 0x74:
		case 0x7F: case 0x7D: case 0x7C: case 0x7E: case 0x75: case 0x71:
		case 0x7B: case 0x79: case 0x70: case 0x7A: case 0x78:
			/* conditional jumps - jcc rel8 sign extended, relative
			 * to next instruction */
			if (insn_len != 2)
				panic("");
			if (is_cond_true(opcode[0] & 0xf, eflags))
			{
				/* jump taken */
				new_pc = (signed)(signed char) opcode[1] + pc + insn_len;
				printf("eflags: 0x%08x, jump taken to address 0x%08x\n", (int)eflags, new_pc);
			}
			else
				/* jump not taken */
				;
			break;
		case 0xE3:
			/* JCXZ rel8 Jump short if CX register is 0. */
			if (insn_len != 2)
				panic("");
			panic("");
			break;
		case 0xe0:
			/* loopne rel8 */
			panic("");
			break;
		case 0xe1:
			/* loope rel8 */
			panic("");
			break;
		case 0xe2:
			/* loop rel8 */
			panic("");
			break;
		default:
			/* assume the instruction does not modify the program counter */
			break;
	}
	*next_insn_addr = new_pc;
	if (is_probably_call_insn)
		*is_probably_call_insn = is_call_insn;
	return insn_len;
}


#define MAX_RELOCATED_INSN_BYTE_SIZE		32
void x86_relocate_insn_to_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr_to_reloc_to, int * insn_byte_len, 
		/*static*/ unsigned char reloc_buf[MAX_RELOCATED_INSN_BYTE_SIZE])
{
int insn_len, patched_insn_len;
ARM_CORE_WORD pc, pc0;
int nbytes;

	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&pc)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	/* obtain instruction length */
	dis_info.application_data = ctx;
	insn_len = print_insn_i386(pc, &dis_info);
	if (!insn_len)
		panic("");

	if (insn_len + /* the size of the appended jmp instruction */ 5
			>= MAX_RELOCATED_INSN_BYTE_SIZE)
		panic("");

	/* fetch instruction opcode */
	nbytes = insn_len;
	if (ctx->cc->core_mem_read(ctx, reloc_buf, pc, &nbytes) !=
			GEAR_ERR_NO_ERROR || nbytes != insn_len)
		panic("");
	/* append a jmp instruction */
	reloc_buf[insn_len] = /* jmp rel32 opcode */ 0xe9;
	/* compute displacement for the jump */
	pc0 = pc - (addr_to_reloc_to + /* the length of the jmp rel32 insn */ 5);
	reloc_buf[insn_len + 1] = pc0;
	reloc_buf[insn_len + 2] = pc0 >> 8;
	reloc_buf[insn_len + 3] = pc0 >> 16;
	reloc_buf[insn_len + 4] = pc0 >> 24;

	patched_insn_len = insn_len + /* the length of the generated jmp insn */ 5;

	/* look up the instruction opcode */
	switch(reloc_buf[0])
	{
		case 0xe8:
			/* call near relative, displacement
			 * relative to next instruction */
			/* a special case - translate the call instruction
			 * to a push instruction and jump to the
			 * subroutine directly */
			if (insn_len != 5)
				panic("");
			pc0 = reloc_buf[1] | (reloc_buf[2] << 8)
				| (reloc_buf[3] << 16) | (reloc_buf[4] << 24);
			/* convert to absolute */
			pc0 += pc + insn_len;
			/* assemble a push imm32 instruction */
			reloc_buf[0] = /* push imm32 opcode */ 0x68;
			pc += insn_len;
			reloc_buf[1] = pc;
			reloc_buf[2] = pc >> 8;
			reloc_buf[3] = pc >> 16;
			reloc_buf[4] = pc >> 24;
			/* assemble a jump instruction - jump
			 * directly to the subroutine entry point */
			reloc_buf[5] = /* jmp rel32 opcode */ 0xe9;
			pc0 -= addr_to_reloc_to + 5 + 5;
			reloc_buf[6] = pc0;
			reloc_buf[7] = pc0 >> 8;
			reloc_buf[8] = pc0 >> 16;
			reloc_buf[9] = pc0 >> 24;
			patched_insn_len = 10;
			break;
		case 0x9a:
			/* call far absolute */
			panic("");
			break;
		case 0xff:
			/* miscellaneous instructions */
			switch ((reloc_buf[1] >> 3) & 7)
			{
				case 3:
					/* call far absolute indirect */
					panic("");
					break;
				case 2:
					/* call near absolute indirect */
					/* a special case - translate the call instruction
					 * to a push instruction and jump to the
					 * subroutine directly */
					if (insn_len < 2 || insn_len > 6)
						panic("");
					pc0 = reloc_buf[2];
					/* assemble a push r/m32 instruction */
					reloc_buf[0] = 0xff;
					reloc_buf[1] = (6 << 3) | (pc0 & 7);
					/* assemble a jump r/m32 instruction - jump
					 * directly to the subroutine entry point */
					reloc_buf[2] = 0xff;
					reloc_buf[3] = (4 << 3) | (pc0 & 7);
					patched_insn_len = 4;
					break;
				case 5:
					/* jmp far */
					panic("");
					break;
				case 4:
					/* jmp near, absolute indirect -
					 * address == sign extended
					 * r/m32 */
					/* do nothing - instruction is relocatable */
					if (insn_len < 2 || insn_len > 7)
						panic("");
					break;
				default:
					/* assume the instruction does not modify the program counter */
					break;
			}
			break;
		case 0x0f:
			switch (reloc_buf[1])
			{

				case 0x87: case 0x83: case 0x82: case 0x86: case 0x84:
				case 0x8F: case 0x8D: case 0x8C: case 0x8E: case 0x85:
				case 0x81: case 0x8B: case 0x89: case 0x80: case 0x8A:
				case 0x88:
					/* conditional jumps - jcc rel32 sign extended, relative
					 * to next instruction */
					if (insn_len != 6)
						panic("");
					/* relocate jump target */
					pc0 = reloc_buf[2] | (reloc_buf[3] << 8)
						| (reloc_buf[4] << 16) | (reloc_buf[5] << 24);
					/* convert to absolute */
					pc0 += pc /* + insn_len */;
					pc0 -= addr_to_reloc_to /* + insn_len */;
					reloc_buf[1] = pc0;
					reloc_buf[2] = pc0 >> 8;
					reloc_buf[3] = pc0 >> 16;
					reloc_buf[4] = pc0 >> 24;
					break;
				default:
					panic("");
					break;
			}
			break;
		case 0xc3:
			/* ret near */
			panic("");
			break;
		case 0xcb:
			/* call far */
			panic("");
			break;
		case 0xc2:
			/* ret near, imm16 */
			panic("");
			break;
		case 0xca:
			/* ret far, imm16 */
			panic("");
			break;
		case 0xcc:
			/* int3 */
			panic("");
			break;
		case 0xcd:
			/* int */
			panic("");
			break;
		case 0xce:
			/* into */
			panic("");
			break;
		case 0xcf:
			/* iret */
			panic("");
			break;
		case 0xeb:
			/* jmp rel8 - short jump,
			 * 8 bit sign extended displacement
			 * relative to next instruction */
			if (insn_len != 2)
				panic("");
			/* reassemble instruction - convert to the longer
			 * format - jmp rel32 */
			/* relocate jump target */
			pc0 = (signed)(signed char)reloc_buf[1];
			/* convert to absolute */
			pc0 += pc + insn_len;
			pc0 -= addr_to_reloc_to + /* new instruction length */ 5;
			/* assemble the new jcc rel32 opcode */
			reloc_buf[0] = /* jmp rel32 opcode */ 0xe9;
			reloc_buf[1] = pc0;
			reloc_buf[2] = pc0 >> 8;
			reloc_buf[3] = pc0 >> 16;
			reloc_buf[4] = pc0 >> 24;
			patched_insn_len = 5;
			break;
		case 0xe9:
			/* jmp rel32 - near jump, 32 bit displacement
			 * relative to next instruction */
			if (insn_len != 5)
				panic("");
			/* relocate jump target */
			pc0 = reloc_buf[1] | (reloc_buf[2] << 8)
				| (reloc_buf[3] << 16) | (reloc_buf[4] << 24);
			/* convert to absolute */
			pc0 += pc /*+ insn_len*/;
			pc0 -= addr_to_reloc_to /* + insn_len */;
			reloc_buf[1] = pc0;
			reloc_buf[2] = pc0 >> 8;
			reloc_buf[3] = pc0 >> 16;
			reloc_buf[4] = pc0 >> 24;
			patched_insn_len = /* the new instruction length */ 5;
			break;
		case 0xea:
			/* jmp far */
			panic("");
			break;

		case 0x77: case 0x73: case 0x72: case 0x76: case 0x74:
		case 0x7F: case 0x7D: case 0x7C: case 0x7E: case 0x75: case 0x71:
		case 0x7B: case 0x79: case 0x70: case 0x7A: case 0x78:
			/* conditional jumps - jcc rel8 sign extended,
			 * relative to next instruction */
			if (insn_len != 2)
				panic("");
			/* reassemble instruction - convert to the longer
			 * format - jcc rel32 */
			/* relocate jump target */
			pc0 = (signed)(signed char)reloc_buf[1];
			/* convert to absolute */
			pc0 += pc + insn_len;
			pc0 -= addr_to_reloc_to + /* new instruction length */ 6;
			/* assemble the new jcc rel32 opcode */
			reloc_buf[1] = 0x80 | (reloc_buf[0] & 15);
			reloc_buf[0] = 0x0f;
			reloc_buf[2] = pc0;
			reloc_buf[3] = pc0 >> 8;
			reloc_buf[4] = pc0 >> 16;
			reloc_buf[5] = pc0 >> 24;
			/* reassemble the jump instruction */
			pc0 = pc + insn_len - (addr_to_reloc_to + 11);
			reloc_buf[5 + 1] = /* jmp rel32 opcode */ 0xe9;
			reloc_buf[5 + 2] = pc0;
			reloc_buf[5 + 3] = pc0 >> 8;
			reloc_buf[5 + 4] = pc0 >> 16;
			reloc_buf[5 + 5] = pc0 >> 24;
			patched_insn_len = 11;
			break;
		case 0xE3:
			/* JCXZ rel8 Jump short if CX register is 0. */
			if (insn_len != 2)
				panic("");
			panic("");
			break;
		case 0xe0:
			/* loopne rel8 */
			panic("");
			break;
		case 0xe1:
			/* loope rel8 */
			panic("");
			break;
		case 0xe2:
			/* loop rel8 */
			panic("");
			break;
		default:
			/* assume the instruction does not modify the program counter */
			printf("patched len: %i\n", patched_insn_len);
			printf("insn_len: %i\n", insn_len);
			printf("patched bytes:\n");
			for (nbytes = 0; nbytes < patched_insn_len; nbytes++)
				printf("%02x ", reloc_buf[nbytes]);
			printf("\n");
			break;
	}
	*insn_byte_len = patched_insn_len;
}

