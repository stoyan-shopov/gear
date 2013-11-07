
#include <stdio.h>
#include <stdlib.h>
#include <dis-asm.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../include/engine-err.h"
#include "../include/target/arm/target-defs.h"
#include "../include/dwarf-common.h"
#include "../include/gear-engine-context.h"
#include "../core-access.h"


#define panic(__x)	do { printf("%s, %i: %s\n", __FILE__, __LINE__, __x); while(1); } while (0)

static int read_memory_func(bfd_vma memaddr, bfd_byte *myaddr, unsigned int length,
		struct disassemble_info *info)
{
struct gear_engine_context * ctx = (struct gear_engine_context *)info->application_data;

	gprintf("%s(): invoked", __func__);
	if (ctx->cc->core_mem_read(ctx, myaddr, memaddr, &length)
			!= GEAR_ERR_NO_ERROR)
		return 1;
	return 0;
}

  void memory_error_func
    (int status, bfd_vma memaddr, struct disassemble_info *info)
{
	gprintf("\nerror reading memory at address 0x%08x\n", (unsigned int) memaddr);
	miprintf("<<< error reading memory at address 0x%08x >>>", (unsigned int) memaddr);
}

static int my_fprintf (void * stream, const char* format, ...)
{
va_list ap;
int res;
char buf[1024];

	va_start(ap, format);
	res = vsnprintf(buf, sizeof buf, format, ap);
	if (res >= sizeof buf)
		panic("");
	if (write_to_frontends(buf, res) == -1)
		panic("");
	/*! \todo	remove this, debugging only */	 
	printf("%s", buf);
	va_end(ap);
	return res;
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
  .memory_error_func = memory_error_func,

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
	  .symbol_at_address_func = 0,

  /* Function called to check if a SYMBOL is can be displayed to the user.
     This is used by some ports that want to hide special symbols when
     displaying debugging outout.  */
	  /*
  bfd_boolean (* symbol_is_valid)
    (asymbol *, struct disassemble_info * info);
    */
	  .symbol_is_valid = 0,

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


int arm_disassemble_insn(struct gear_engine_context * ctx, ARM_CORE_WORD addr,
		int (*print_fn)(const char * format, va_list ap))
{
	gprintf("%s(): invoked", __func__);
	dis_info.application_data = ctx;
	return print_insn_i386(addr, &dis_info);
}

void init_arm_disassemble(void)
{
	gprintf("%s(): invoked", __func__);
	dis_info.stream = stdout;
}

