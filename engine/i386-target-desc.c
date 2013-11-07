/* this module uses the quite good udis86 library,
 * however:
 * 
 *	NEVER TRUST THE UDIS86 DOCUMENTATION
 *
 * read the sources instead... */



#include <stdarg.h>
#include <stdlib.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "engine-err.h"
#include "core-access.h"
#include "util.h"
#include "gprintf.h"
#include "dwarf-expr.h"
#include "cu-access.h"
#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "breakpoint.h"
#include "exec.h"

#include "gear-constants.h"


#include <udis86.h>

#define MEMBUF_BYTE_SIZE	256
struct tdesc_private_data
{
	/* this is the udis86 library data object */
	/*! \todo	(sgs) there is a hack regarding the usage
	 *		of this object in a reentrant fashion;
	 *		thing is, the udis86 has been designed
	 *		with concern to (quoting the official
	 *		documentation) "maintain reentrancy
	 *		and thread safety" - and this data
	 *		object acts as a handle being passed
	 *		to all of the udis86 library functions;
	 *		as the udis86 library supports calling
	 *		back a user supplied function to supply
	 *		the bytes to disassemble, with only a
	 *		'ud_t *' pointer as a parameter, and
	 *		this 'ud_t' data structure does not
	 *		normally support linking to user specific
	 *		data without changing the source code
	 *		(which i will not be doing, now
	 *		in the least) - true reentrancy cannot
	 *		really be achieved in a straightforward
	 *		manner; as this 'ud_t' data structure,
	 *		however, has a 'FILE *' pointer field
	 *		to be used when reading the bytes to
	 *		disassemble, from looking at the udis86
	 *		sources i concluded it is safe to stash
	 *		a 'gear_engine_context *' pointer in this
	 *		field in order to achieve true reentrancy
	 *		in the callback function passed to udis86
	 *		(here, this is the 'udis86_input_hook()
	 *		function below);
	 *		another thing is that this 'ud_t' data
	 *		structure actually contains pretty many
	 *		fields regarding disassembling from memory
	 *		buffers, so it may well be the case that
	 *		the 'membuf_*' fields below are actually
	 *		redundant, but as these 'ud_t' fields are
	 *		not explicitly documented in the official
	 *		udis86 documentation (and i don't have the
	 *		time to read the sources right now), i
	 *		decide to leave these as they are;
	 *		so, in short - before calling
	 *		ud_disassemble(), which in turn will
	 *		call the supplied callback (udis86_input_hook())
	 *		to fetch bytes to disassemble, the
	 *		'ud_t.inp_file' field must be set to
	 *		the proper 'gear_engine_context *' variable,
	 *		so that the callback can find the context
	 *		it should work in; of course, all this is
	 *		a hack...
	 *
	 *		at some time it may be worthwile reviewing
	 *		the points mentioned above... */
	ud_t		udis86_obj;
	unsigned char	* membuf;
	int		membuf_byte_size;
	int		membuf_idx;
	ARM_CORE_WORD	membuf_start_addr;
	bool		is_membuf_valid;
};


/*! \note	this here module heavily depends on
 *		source code file
 *		/usr/include/sys/user.h
 *		which might be volatile; and that is
 *		why its contents, on which this here
 *		module is originally based, are copied
 *		here verbatim; this copy is provided
 *		here only for reference, in case anything
 *		gets broken - this way, i(sgs) hope
 *		it will be easier to fix things
 *		up when they get messed up (which
 *		they will, trust me...) */

#if 0 /* sys/user.h inclusion */
/* start of the verbatim copy of file
 * /usr/include/sys/user.h
 * on which this here module is originally based on */
/* Copyright (C) 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _SYS_USER_H
#define _SYS_USER_H	1

/* The whole purpose of this file is for GDB and GDB only.  Don't read
   too much into it.  Don't use it for anything other than GDB unless
   you know what you are doing.  */

struct user_fpregs_struct
{
  long int cwd;
  long int swd;
  long int twd;
  long int fip;
  long int fcs;
  long int foo;
  long int fos;
  long int st_space [20];
};

struct user_fpxregs_struct
{
  unsigned short int cwd;
  unsigned short int swd;
  unsigned short int twd;
  unsigned short int fop;
  long int fip;
  long int fcs;
  long int foo;
  long int fos;
  long int mxcsr;
  long int reserved;
  long int st_space[32];   /* 8*16 bytes for each FP-reg = 128 bytes */
  long int xmm_space[32];  /* 8*16 bytes for each XMM-reg = 128 bytes */
  long int padding[56];
};

struct user_regs_struct
{
  long int ebx;
  long int ecx;
  long int edx;
  long int esi;
  long int edi;
  long int ebp;
  long int eax;
  long int xds;
  long int xes;
  long int xfs;
  long int xgs;
  long int orig_eax;
  long int eip;
  long int xcs;
  long int eflags;
  long int esp;
  long int xss;
};

struct user
{
  struct user_regs_struct	regs;
  int				u_fpvalid;
  struct user_fpregs_struct	i387;
  unsigned long int		u_tsize;
  unsigned long int		u_dsize;
  unsigned long int		u_ssize;
  unsigned long			start_code;
  unsigned long			start_stack;
  long int			signal;
  int				reserved;
  struct user_regs_struct*	u_ar0;
  struct user_fpregs_struct*	u_fpstate;
  unsigned long int		magic;
  char				u_comm [32];
  int				u_debugreg [8];
};

#define PAGE_SHIFT		12
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE-1))
#define NBPG			PAGE_SIZE
#define UPAGES			1
#define HOST_TEXT_START_ADDR	(u.start_code)
#define HOST_STACK_END_ADDR	(u.start_stack + u.u_ssize * NBPG)

#endif	/* _SYS_USER_H */
/* end of the verbatim copy of file
 * /usr/include/sys/user.h
 * on which this here module is originally based on */
#endif /* sys/user.h inclusion */


/* other code here heavily depends on the the gcc mapping
 * between target registers and dwarf registers; these below
 * are excerpts from various gcc sources residing in the
 * gcc/config/i386
 * directory in the gcc source code distribution; these
 * excerpts are provided here for reference only */

/******************************************************************/
/******************* start of gcc code snippets *******************/
/******************************************************************/


#if 0

/* Number of actual hardware registers.
   The hardware registers are assigned numbers for the compiler
   from 0 to just below FIRST_PSEUDO_REGISTER.
   All registers that the compiler knows about must be given numbers,
   even those that are not normally considered general registers.

   In the 80386 we give the 8 general purpose registers the numbers 0-7.
   We number the floating point registers 8-15.
   Note that registers 0-7 can be accessed as a  short or int,
   while only 0-3 may be used with byte `mov' instructions.

   Reg 16 does not correspond to any hardware register, but instead
   appears in the RTL as an argument pointer prior to reload, and is
   eliminated during reloading in favor of either the stack or frame
   pointer.  */

#define FIRST_PSEUDO_REGISTER 53

/* Number of hardware registers that go into the DWARF-2 unwind info.
   If not defined, equals FIRST_PSEUDO_REGISTER.  */

#define DWARF_FRAME_REGISTERS 17

/* 1 for registers that have pervasive standard uses
   and are not available for the register allocator.
   On the 80386, the stack pointer is such, as is the arg pointer.

   The value is zero if the register is not fixed on either 32 or
   64 bit targets, one if the register if fixed on both 32 and 64
   bit targets, two if it is only fixed on 32bit targets and three
   if its only fixed on 64bit targets.
   Proper values are computed in the CONDITIONAL_REGISTER_USAGE.
 */
#define FIXED_REGISTERS						\
/*ax,dx,cx,bx,si,di,bp,sp,st,st1,st2,st3,st4,st5,st6,st7*/	\
{  0, 0, 0, 0, 0, 0, 0, 1, 0,  0,  0,  0,  0,  0,  0,  0,	\
/*arg,flags,fpsr,fpcr,frame*/					\
    1,    1,   1,   1,    1,					\
/*xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7*/			\
     0,   0,   0,   0,   0,   0,   0,   0,			\
/*mmx0,mmx1,mmx2,mmx3,mmx4,mmx5,mmx6,mmx7*/			\
     0,   0,   0,   0,   0,   0,   0,   0,			\
/*  r8,  r9, r10, r11, r12, r13, r14, r15*/			\
     2,   2,   2,   2,   2,   2,   2,   2,			\
/*xmm8,xmm9,xmm10,xmm11,xmm12,xmm13,xmm14,xmm15*/		\
     2,   2,    2,    2,    2,    2,    2,    2 }


/* 1 for registers not available across function calls.
   These must include the FIXED_REGISTERS and also any
   registers that can be used without being saved.
   The latter must include the registers where values are returned
   and the register where structure-value addresses are passed.
   Aside from that, you can include as many other registers as you like.

   The value is zero if the register is not call used on either 32 or
   64 bit targets, one if the register if call used on both 32 and 64
   bit targets, two if it is only call used on 32bit targets and three
   if its only call used on 64bit targets.
   Proper values are computed in the CONDITIONAL_REGISTER_USAGE.
*/
#define CALL_USED_REGISTERS					\
/*ax,dx,cx,bx,si,di,bp,sp,st,st1,st2,st3,st4,st5,st6,st7*/	\
{  1, 1, 1, 0, 3, 3, 0, 1, 1,  1,  1,  1,  1,  1,  1,  1,	\
/*arg,flags,fpsr,fpcr,frame*/					\
    1,   1,    1,   1,    1,					\
/*xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7*/			\
     1,   1,   1,   1,   1,   1,   1,   1,			\
/*mmx0,mmx1,mmx2,mmx3,mmx4,mmx5,mmx6,mmx7*/			\
     1,   1,   1,   1,   1,   1,   1,   1,			\
/*  r8,  r9, r10, r11, r12, r13, r14, r15*/			\
     1,   1,   1,   1,   2,   2,   2,   2,			\
/*xmm8,xmm9,xmm10,xmm11,xmm12,xmm13,xmm14,xmm15*/		\
     1,   1,    1,    1,    1,    1,    1,    1 }




/* The "default" register map used in 32bit mode.  */

int const dbx_register_map[FIRST_PSEUDO_REGISTER] =
{
  0, 2, 1, 3, 6, 7, 4, 5,		/* general regs */
  12, 13, 14, 15, 16, 17, 18, 19,	/* fp regs */
  -1, -1, -1, -1, -1,			/* arg, flags, fpsr, fpcr, frame */
  21, 22, 23, 24, 25, 26, 27, 28,	/* SSE */
  29, 30, 31, 32, 33, 34, 35, 36,       /* MMX */
  -1, -1, -1, -1, -1, -1, -1, -1,	/* extended integer registers */
  -1, -1, -1, -1, -1, -1, -1, -1,	/* extended SSE registers */
};

static int const x86_64_int_parameter_registers[6] =
{
  5 /*RDI*/, 4 /*RSI*/, 1 /*RDX*/, 2 /*RCX*/,
  FIRST_REX_INT_REG /*R8 */, FIRST_REX_INT_REG + 1 /*R9 */
};

static int const x86_64_ms_abi_int_parameter_registers[4] =
{
  2 /*RCX*/, 1 /*RDX*/,
  FIRST_REX_INT_REG /*R8 */, FIRST_REX_INT_REG + 1 /*R9 */
};

static int const x86_64_int_return_registers[4] =
{
  0 /*RAX*/, 1 /*RDX*/, 5 /*RDI*/, 4 /*RSI*/
};

/* The "default" register map used in 64bit mode.  */
int const dbx64_register_map[FIRST_PSEUDO_REGISTER] =
{
  0, 1, 2, 3, 4, 5, 6, 7,		/* general regs */
  33, 34, 35, 36, 37, 38, 39, 40,	/* fp regs */
  -1, -1, -1, -1, -1,			/* arg, flags, fpsr, fpcr, frame */
  17, 18, 19, 20, 21, 22, 23, 24,	/* SSE */
  41, 42, 43, 44, 45, 46, 47, 48,       /* MMX */
  8,9,10,11,12,13,14,15,		/* extended integer registers */
  25, 26, 27, 28, 29, 30, 31, 32,	/* extended SSE registers */
};

/* Define the register numbers to be used in Dwarf debugging information.
   The SVR4 reference port C compiler uses the following register numbers
   in its Dwarf output code:
	0 for %eax (gcc regno = 0)
	1 for %ecx (gcc regno = 2)
	2 for %edx (gcc regno = 1)
	3 for %ebx (gcc regno = 3)
	4 for %esp (gcc regno = 7)
	5 for %ebp (gcc regno = 6)
	6 for %esi (gcc regno = 4)
	7 for %edi (gcc regno = 5)
   The following three DWARF register numbers are never generated by
   the SVR4 C compiler or by the GNU compilers, but SDB on x86/svr4
   believes these numbers have these meanings.
	8  for %eip    (no gcc equivalent)
	9  for %eflags (gcc regno = 17)
	10 for %trapno (no gcc equivalent)
   It is not at all clear how we should number the FP stack registers
   for the x86 architecture.  If the version of SDB on x86/svr4 were
   a bit less brain dead with respect to floating-point then we would
   have a precedent to follow with respect to DWARF register numbers
   for x86 FP registers, but the SDB on x86/svr4 is so completely
   broken with respect to FP registers that it is hardly worth thinking
   of it as something to strive for compatibility with.
   The version of x86/svr4 SDB I have at the moment does (partially)
   seem to believe that DWARF register number 11 is associated with
   the x86 register %st(0), but that's about all.  Higher DWARF
   register numbers don't seem to be associated with anything in
   particular, and even for DWARF regno 11, SDB only seems to under-
   stand that it should say that a variable lives in %st(0) (when
   asked via an `=' command) if we said it was in DWARF regno 11,
   but SDB still prints garbage when asked for the value of the
   variable in question (via a `/' command).
   (Also note that the labels SDB prints for various FP stack regs
   when doing an `x' command are all wrong.)
   Note that these problems generally don't affect the native SVR4
   C compiler because it doesn't allow the use of -O with -g and
   because when it is *not* optimizing, it allocates a memory
   location for each floating-point variable, and the memory
   location is what gets described in the DWARF AT_location
   attribute for the variable in question.
   Regardless of the severe mental illness of the x86/svr4 SDB, we
   do something sensible here and we use the following DWARF
   register numbers.  Note that these are all stack-top-relative
   numbers.
	11 for %st(0) (gcc regno = 8)
	12 for %st(1) (gcc regno = 9)
	13 for %st(2) (gcc regno = 10)
	14 for %st(3) (gcc regno = 11)
	15 for %st(4) (gcc regno = 12)
	16 for %st(5) (gcc regno = 13)
	17 for %st(6) (gcc regno = 14)
	18 for %st(7) (gcc regno = 15)
*/
int const svr4_dbx_register_map[FIRST_PSEUDO_REGISTER] =
{
  0, 2, 1, 3, 6, 7, 5, 4,		/* general regs */
  11, 12, 13, 14, 15, 16, 17, 18,	/* fp regs */
  -1, 9, -1, -1, -1,			/* arg, flags, fpsr, fpcr, frame */
  21, 22, 23, 24, 25, 26, 27, 28,	/* SSE registers */
  29, 30, 31, 32, 33, 34, 35, 36,	/* MMX registers */
  -1, -1, -1, -1, -1, -1, -1, -1,	/* extended integer registers */
  -1, -1, -1, -1, -1, -1, -1, -1,	/* extended SSE registers */
};

#endif

/* copied verbatim from the mingw winnt.h */
#if 0

#ifdef _X86_
#define SIZE_OF_80387_REGISTERS	80
#define CONTEXT_i386	0x10000
#define CONTEXT_i486	0x10000
#define CONTEXT_CONTROL	(CONTEXT_i386|0x00000001L)
#define CONTEXT_INTEGER	(CONTEXT_i386|0x00000002L)
#define CONTEXT_SEGMENTS	(CONTEXT_i386|0x00000004L)
#define CONTEXT_FLOATING_POINT	(CONTEXT_i386|0x00000008L)
#define CONTEXT_DEBUG_REGISTERS	(CONTEXT_i386|0x00000010L)
#define CONTEXT_EXTENDED_REGISTERS (CONTEXT_i386|0x00000020L)
#define CONTEXT_FULL	(CONTEXT_CONTROL|CONTEXT_INTEGER|CONTEXT_SEGMENTS)
#define MAXIMUM_SUPPORTED_EXTENSION  512
typedef struct _FLOATING_SAVE_AREA {
	DWORD	ControlWord;
	DWORD	StatusWord;
	DWORD	TagWord;
	DWORD	ErrorOffset;
	DWORD	ErrorSelector;
	DWORD	DataOffset;
	DWORD	DataSelector;
	BYTE	RegisterArea[80];
	DWORD	Cr0NpxState;
} FLOATING_SAVE_AREA;
typedef struct _CONTEXT {
	DWORD	ContextFlags;
	DWORD	Dr0;
	DWORD	Dr1;
	DWORD	Dr2;
	DWORD	Dr3;
	DWORD	Dr6;
	DWORD	Dr7;
	FLOATING_SAVE_AREA FloatSave;
	DWORD	SegGs;
	DWORD	SegFs;
	DWORD	SegEs;
	DWORD	SegDs;
	DWORD	Edi;
	DWORD	Esi;
	DWORD	Ebx;
	DWORD	Edx;
	DWORD	Ecx;
	DWORD	Eax;
	DWORD	Ebp;
	DWORD	Eip;
	DWORD	SegCs;
	DWORD	EFlags;
	DWORD	Esp;
	DWORD	SegSs;
	BYTE	ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
} CONTEXT;

#endif
#endif
/* end of verbatim copy from the mingw winnt.h */



/******************************************************************/
/******************* start of gcc code snippets *******************/
/******************************************************************/


static int get_nr_target_core_regs(struct gear_engine_context * ctx)
{
	return 17;
}


static int get_target_pc_reg_nr_ptrace(struct gear_engine_context * ctx)
{
	return 12;
}
static int get_target_sp_reg_nr_ptrace(struct gear_engine_context * ctx)
{
	return 15;
}

static int get_target_pstat_reg_nr_ptrace(struct gear_engine_context * ctx)
{
	return 14;
}


static int get_target_pc_reg_nr_win32_context(struct gear_engine_context * ctx)
{
	return 11;
}
static int get_target_sp_reg_nr_win32_context(struct gear_engine_context * ctx)
{
	return 14;
}

static int get_target_pstat_reg_nr_win32_context(struct gear_engine_context * ctx)
{
	return 13;
}


/* as the 'system v application binary interface intel386(tm)
 * architecture processor supplement fourth edition' document
 * (google for abi386-4.pdf), paragraph 'function calling sequence',
 * page 3-11, says (verbatim):
 *
 * Registers %ebp, %ebx, %edi, %esi, and %esp "belong" to the cal-
 * ling function. In other words, a called function must preserve these registers'
 * values for its caller. Remaining registers "belong" to the called function. If a cal-
 * ling function wants to preserve such a register value across a function call, it must
 * save the value in its local stack frame.
 *
 * for the register numbering consult source code file
 * /usr/include/sys/user.h
 */
static bool is_dwarf_reg_nr_callee_saved(struct gear_engine_context * ctx, int dwarf_reg_nr)
{
	switch (dwarf_reg_nr)
	{
		case 3: case 6: case 7: case 4: case 5:
			return true;
			break;
		default: return false;
	}
	panic("");
}

enum GEAR_ENGINE_ERR_ENUM translate_dwarf_reg_nr_to_target_reg_nr_ptrace(struct gear_engine_context * ctx, int * inout_reg_nr)
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
int res;

	switch (res = *inout_reg_nr)
	{
		case 0:
			res = 6;
			break;
		case 1:
			res = 1;
			break;
		case 2:
			res = 2;
			break;
		case 3:
			res = 0;
			break;
		case 4:
			res = 15;
			break;
		case 5:
			res = 5;
			break;
		case 6:
			res = 3;
			break;
		case 7:
			res = 4;
			break;
		case 8:
			panic("");
			break;
		case 9:
			res = 14;
			break;
		case 10:
			panic("");
			break;
		case 11:
			panic("");
			break;
		case 12:
			panic("");
			break;
		case 13:
			panic("");
			break;
		case 14:
			panic("");
			break;
		case 15:
			panic("");
			break;
		case 16:
			panic("");
			break;
		default:
			gprintf("%s(): unknown register number %i\n", __func__, * inout_reg_nr);
			panic("");
	}
	*inout_reg_nr = res;
	return GEAR_ERR_NO_ERROR;
}


enum GEAR_ENGINE_ERR_ENUM translate_dwarf_reg_nr_to_target_reg_nr_win32_context(struct gear_engine_context * ctx, int * inout_reg_nr)
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
int res;

	switch (res = *inout_reg_nr)
	{
		case 0:
			res = 9;
			break;
		case 1:
			res = 8;
			break;
		case 2:
			res = 7;
			break;
		case 3:
			res = 6;
			break;
		case 4:
			res = 14;
			break;
		case 5:
			res = 10;
			break;
		case 6:
			res = 5;
			break;
		case 7:
			res = 4;
			break;
		case 8:
			panic("");
			break;
		case 9:
			res = 13;
			break;
		case 10:
			panic("");
			break;
		case 11:
			panic("");
			break;
		case 12:
			panic("");
			break;
		case 13:
			panic("");
			break;
		case 14:
			panic("");
			break;
		case 15:
			panic("");
			break;
		case 16:
			panic("");
			break;
		default:
			gprintf("%s(): unknown register number %i\n", __func__, * inout_reg_nr);
			panic("");
	}
	*inout_reg_nr = res;
	return GEAR_ERR_NO_ERROR;
}

static const char * translate_target_core_reg_nr_to_human_readable_win32(struct gear_engine_context * ctx,
		unsigned int target_core_reg_nr)
{
static const char * reg_names[16] =
{
	[0] = "xgs",
	[1] = "xfs",
	[2] = "xes",
	[3] = "xds",
	[4] = "edi",
	[5] = "esi",
	[6] = "ebx",
	[7] = "edx",
	[8] = "ecx",
	[9] = "eax",
	[10] = "ebp",
	[11] = "eip",
	[12] = "xcs",
	[13] = "eflags",
	[14] = "esp",
	[15] = "xss",
};

	return (target_core_reg_nr < 16) ? reg_names[target_core_reg_nr] : 0;
}




























/* this is a callback function called by the udis86
 * library to fetch bytes to disassemble; if
 * no data can be supplied, this function should
 * return UD_EOI */
static int udis86_input_hook(ud_t * ud)
{
struct gear_engine_context * ctx;
struct tdesc_private_data * p;

	gprintf("%s(): invoked\n", __func__);
	/* this is a hack - for explanation of it, read
	 * the comments about the 'udis86_obj' data field
	 * in 'struct tdesc_private_data' in this file */
	ctx = (struct gear_engine_context *) ud->inp_file;
	p = ctx->tdesc->p;
	/* see if the target memory buffer should be repopulated
	 * from target core */
	if (!p->is_membuf_valid
			|| (p->membuf_idx == p->membuf_byte_size))
	{
		unsigned int nbytes;
		nbytes = p->membuf_byte_size;
		if (p->membuf_idx == p->membuf_byte_size)
		{
			/* advance address */
			p->membuf_start_addr += p->membuf_byte_size;
		}
		if (ctx->cc->core_mem_read(ctx, p->membuf, p->membuf_start_addr, &nbytes) !=
				GEAR_ERR_NO_ERROR || nbytes != p->membuf_byte_size)
		{
			p->is_membuf_valid = false;
			return UD_EOI;
		}
		p->membuf_idx = 0;
		p->is_membuf_valid = true;
	}

	return p->membuf[p->membuf_idx++];
}

/* i386 instruction format:
 *
 * prefix - upto four prefixes, 1 byte each (optional)
 * opcode - 1 byte
 * ModR/M - 1 byte (if required) - bits[6-7] - Mod, bits[3-5] - Reg/Opcode, bits[0-2] - R/M
 * SIB - 1 byte (if required) - bits[6-7] - Scale, bits[3-5] - Index, bits[0-2] - Base
 * Displacement - address displacement of 1, 2, or 4 bytes or none
 * Immediate data of 1, 2, 4 bytes or none
 */

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
			if (mod != 0 || idx == 4 || base != 5)
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

static int i386_insn_decode(struct gear_engine_context * ctx, ARM_CORE_WORD addr, ARM_CORE_WORD * next_insn_addr, bool * is_probably_call_insn)
{
int insn_len;
ARM_CORE_WORD eflags, pc, new_pc, core_word;
/*! the maximum byte size of an instruction that can change the program counter */
unsigned char opcode[MAX_CONTROL_XFER_INSN_SIZE_IN_BYTES];
bool is_call_insn;
int nbytes;
struct tdesc_private_data * p;

	p = ctx->tdesc->p;
	if (!p->is_membuf_valid
			|| !(p->membuf_start_addr <= addr && addr < p->membuf_start_addr + p->membuf_byte_size))
	{
		/* force repopulation of the target memory buffer
		 * by the callback (udis86_input_hook()) called 
		 * by the udis86 library */
		p->is_membuf_valid = false;
		p->membuf_start_addr = addr;
		ud_set_input_hook(&p->udis86_obj, udis86_input_hook);
	}
	/* adjust the membuf index */
	p->membuf_idx = addr - p->membuf_start_addr;
	/* this is a hack - for explanation of it, read
	 * the comments about the 'udis86_obj' data field
	 * in 'struct tdesc_private_data' in this file */
	p->udis86_obj.inp_file = (FILE *) ctx;

	/* obtain instruction length */
	ud_set_pc(&p->udis86_obj, (uint64_t) addr);
	insn_len = ud_disassemble(&p->udis86_obj);

	if (!next_insn_addr && !is_probably_call_insn)
		/* only the instruction length is being requested */
		return insn_len;

	if (!insn_len)
		panic("");

	pc = addr;

	/* by default - the instruction does not change the program counter */
	new_pc = *next_insn_addr = pc + insn_len;
	if (is_probably_call_insn)
		*is_probably_call_insn = false;
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


static int i386_print_disassembly(struct gear_engine_context * ctx, ARM_CORE_WORD addr, int (*print_fn)(const char * format, ...))
{
int insn_len;
struct tdesc_private_data * p;

	p = ctx->tdesc->p;
	if (!p->is_membuf_valid
			|| !(p->membuf_start_addr <= addr && addr < p->membuf_start_addr + p->membuf_byte_size))
	{
		/* force repopulation of the target memory buffer
		 * by the callback (udis86_input_hook()) called 
		 * by the udis86 library */
		p->is_membuf_valid = false;
		p->membuf_start_addr = addr;
		ud_set_input_hook(&p->udis86_obj, udis86_input_hook);
	}
	/* adjust the membuf index */
	p->membuf_idx = addr - p->membuf_start_addr;
	/* this is a hack - for explanation of it, read
	 * the comments about the 'udis86_obj' data field
	 * in 'struct tdesc_private_data' in this file */
	p->udis86_obj.inp_file = (FILE *) ctx;

	/* obtain instruction length */
	ud_set_pc(&p->udis86_obj, (uint64_t) addr);
	insn_len = ud_disassemble(&p->udis86_obj);

	if (insn_len == 0)
	{
		/* disassembling failed */
		print_fn("<<< cannot disassemble at address 0x%08x >>>", (int) addr);
		return 0;
	}

	print_fn("%s\t%s", ud_insn_hex(&p->udis86_obj), ud_insn_asm(&p->udis86_obj));
	return insn_len;

}

#define MAX_RELOCATED_INSN_BYTE_SIZE		32
void x86_relocate_insn_to_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr_to_reloc_to, int * insn_byte_len, 
		/*static*/ unsigned char reloc_buf[MAX_RELOCATED_INSN_BYTE_SIZE])
{
int insn_len, patched_insn_len;
ARM_CORE_WORD pc, pc0;
int nbytes;
struct tdesc_private_data * p;

	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&pc)
			!= GEAR_ERR_NO_ERROR)
		panic("");

	p = ctx->tdesc->p;
	if (!p->is_membuf_valid
			|| !(p->membuf_start_addr <= pc && pc < p->membuf_start_addr + p->membuf_byte_size))
	{
		/* force repopulation of the target memory buffer
		 * by the callback (udis86_input_hook()) called 
		 * by the udis86 library */
		p->is_membuf_valid = false;
		p->membuf_start_addr = pc;
		ud_set_input_hook(&p->udis86_obj, udis86_input_hook);
	}
	/* adjust the membuf index */
	p->membuf_idx = pc - p->membuf_start_addr;
	/* this is a hack - for explanation of it, read
	 * the comments about the 'udis86_obj' data field
	 * in 'struct tdesc_private_data' in this file */
	p->udis86_obj.inp_file = (FILE *) ctx;

	/* obtain instruction length */
	ud_set_pc(&p->udis86_obj, (uint64_t) pc);
	insn_len = ud_disassemble(&p->udis86_obj);

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







































static struct target_desc_struct i386_desc_struct =
{
	.get_nr_target_core_regs = get_nr_target_core_regs,
	.get_target_pc_reg_nr = get_target_pc_reg_nr_win32_context,
	.get_target_sp_reg_nr = get_target_sp_reg_nr_win32_context,
	.get_target_pstat_reg_nr = get_target_pstat_reg_nr_win32_context,
	.is_dwarf_reg_nr_callee_saved = is_dwarf_reg_nr_callee_saved,
	.translate_dwarf_reg_nr_to_target_reg_nr = translate_dwarf_reg_nr_to_target_reg_nr_win32_context,
	.translate_target_core_reg_nr_to_human_readable	= translate_target_core_reg_nr_to_human_readable_win32,
	.decode_insn = i386_insn_decode,
	.print_disassembled_insn = i386_print_disassembly,
};

void init_i386_target_desc(struct gear_engine_context * ctx)
{
struct tdesc_private_data * p;

	ctx->tdesc = &i386_desc_struct;
	if (!(p = ctx->tdesc->p = calloc(1, sizeof * ctx->tdesc->p)))
		panic("");
	p->membuf_byte_size = MEMBUF_BYTE_SIZE;
	if (!(p->membuf = calloc(p->membuf_byte_size, sizeof * p->membuf)))
		panic("");
	p->is_membuf_valid = false;

	/* initialize the udis86 disassembler library */
	ud_init(&p->udis86_obj);
	ud_set_syntax(&p->udis86_obj, UD_SYN_ATT);
	ud_set_mode(&p->udis86_obj, 32);
}

