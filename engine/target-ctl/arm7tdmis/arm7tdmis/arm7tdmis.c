/*!
 * \file	arm7tdmis_core_access.c
 * \brief	arm7tdmis low level core access code
 * \author	shopov
 *
 *	this is deliberately not documented; it is dirty; however, effort
 *	has been employed that this module actually does what it is supposed
 *	to accomplish; this module is really not very interesting, and indeed
 *	quite boring at times, but it might nonetheless be worth documenting
 *	and cleaning up
 *
 *	\todo	document and clean this up
 *	\todo	major cleanup and dispersal of data to headers
 *
 * Revision summary:
 *
 * $Log: $
 *
 */

#include <stdio.h>
#include <string.h>

#include "typedefs.h"
#include "util.h"
#include "engine-err.h"
#include "target.h"
#include "constants.h"

#define BIT0		(1 << 0)
#define BIT1		(1 << 1)
#define BIT2		(1 << 2)
#define BIT3		(1 << 3)
#define BIT4		(1 << 4)
#define BIT5		(1 << 5)
#define BIT15		(1 << 15)
#define BIT16		(1 << 16)

#define NR_CORE_REGS		16

enum ARM7TDMIS_CORE_MODE
{
	ARM_MODE_USER = 0,
	ARM_MODE_FIQ,
	ARM_MODE_IRQ,
	ARM_MODE_SVC,
	ARM_MODE_ABORT,
	ARM_MODE_UNDEF,
	ARM_MODE_SYSTEM,
	NR_CORE_MODES,
};

/* psr - enum ARM7TDMIS_CORE_MODE correspondence, the order of these must match the enum ARM7TDMIS_CORE_MODE
 * enumerators order */
static int mode_bits[NR_CORE_MODES] =
{
	0x10,	/* user */
	0x11,	/* fiq */
	0x12,	/* irq */
	0x13,	/* svc */
	0x17,	/* abort */
	0x1b,	/* undef */
	0x1f,	/* system */
};


static inline ARM_CORE_WORD extract_psr_mode_bits(ARM_CORE_WORD psr)
{
	return psr & (BIT5 - 1);
}

static inline int get_bank_for_mode(ARM_CORE_WORD mode)
{
int i;
	for (i = 0; i < NR_CORE_MODES; i++)
		if (mode_bits[i] == mode)
			break;
	if (i == NR_CORE_MODES)
	{
		printf("unknown core mode:\t0x%08x\n", mode);
		panic("");
	}
	return i;
}


enum ARM_OPCODES
{
	ARM_OPC_STR_R0_AT_R15	= 0xe58f0000,
	ARM_OPC_LDR_R0_FROM_R0	= 0xe5900000,
	ARM_OPC_LDR_R1_FROM_R0	= 0xe5901000,
	ARM_OPC_MOV_R15_TO_R0	= 0xe28f0000,
	ARM_OPC_MOV_R0_0	= 0xe3a00000,
	ARM_OPC_NOP		= 0xe2800000,
	ARM_OPC_MRS_R0_CPSR	= 0xe10f0000,
	ARM_OPC_MSR_CPSR_R0	= 0xe129f000,
	ARM_OPC_STMIA_R0_EMPTY_REG_LIST	= 0xe8800000,
	ARM_OPC_LDMIA_R0_EMPTY_REG_LIST	= 0xe8900000,
	ARM_OPC_STRB_R1_AT_R0 = 0xe5c01000,
	ARM_OPC_BRANCH_MINUS_SIX = 0xeafffffa,
};

enum EMBEDDED_ICE_REGISTERS_ENUM
{
	EICE_REG_DBGCTL_ADDR		= 0,
	EICE_REG_DBGCTL_WIDTH		= 6,
	EICE_REG_DBGSTAT_ADDR		= 1,
	EICE_REG_DBGSTAT_WIDTH		= 5,
	EICE_REG_ABORTSTAT_ADDR		= 2,
	EICE_REG_ABORTSTAT_WIDTH	= 1,
	EICE_REG_DBGCOMMCTL_ADDR	= 4,
	EICE_REG_DBGCOMMCTL_WIDTH	= 6,
	EICE_REG_DBGCOMMDATA_ADDR	= 5,
	EICE_REG_DBGCOMMDATA_WIDTH	= 32,
	EICE_REG_WP0ADDRVAL_ADDR	= 8,
	EICE_REG_WP0ADDRVAL_WIDTH	= 32,
	EICE_REG_WP0ADDRMASK_ADDR	= 9,
	EICE_REG_WP0ADDRMASK_WIDTH	= 32,
	EICE_REG_WP0DATAVAL_ADDR	= 10,
	EICE_REG_WP0DATAVAL_WIDTH	= 32,
	EICE_REG_WP0DATAMASK_ADDR	= 11,
	EICE_REG_WP0DATAMASK_WIDTH	= 32,
	EICE_REG_WP0CTLVAL_ADDR		= 12,
	EICE_REG_WP0CTLVAL_WIDTH	= 9,
	EICE_REG_WP0CTLMASK_ADDR	= 13,
	EICE_REG_WP0CTLMASK_WIDTH	= 8,
	EICE_REG_WP1ADDRVAL_ADDR	= 16,
	EICE_REG_WP1ADDRVAL_WIDTH	= 32,
	EICE_REG_WP1ADDRMASK_ADDR	= 17,
	EICE_REG_WP1ADDRMASK_WIDTH	= 32,
	EICE_REG_WP1DATAVAL_ADDR	= 18,
	EICE_REG_WP1DATAVAL_WIDTH	= 32,
	EICE_REG_WP1DATAMASK_ADDR	= 19,
	EICE_REG_WP1DATAMASK_WIDTH	= 32,
	EICE_REG_WP1CTLVAL_ADDR		= 20,
	EICE_REG_WP1CTLVAL_WIDTH	= 9,
	EICE_REG_WP1CTLMASK_ADDR	= 21,
	EICE_REG_WP1CTLMASK_WIDTH	= 8,

	ARM7TDMIS_JTAG_ICEREG_WIDTH	= 38,
	ARM7TDMIS_JTAG_ICEREG_ADDR_BITS	= 5,

	ARM7TDMIS_JTAG_SC1_WIDTH	= 33,
};


enum ARM7TDMIS_JTAG_COMMANDS_ENUM
{
	EXTEST		= 0x0,
	SCAN_N		= 0x2,
	ARM7TDMIS_JTAG_SCANCHAIN_REG_WIDTH	= 4,

	SAMPLE_PRELOAD	= 0x3,
	RESTART		= 0x4,
	CLAMP		= 0x5,
	HIGHZ		= 0x7,
	CLAMPZ		= 0x9,
	INTEST		= 0xC,
	IDCODE		= 0xE,
	BYPASS		= 0xF,

	ARM7TDMIS_JTAG_IREG_WIDTH	= 4,

};


/*! set when dbgrq has been asserted and debug mode entry is expected */
static int is_dbgrq_asserted;

static struct arm7tdmis_core_status
{
	enum
	{
		DBG_RUNNING,
		DBG_HALTED,
		SYS_RUNNING,
	}
	core_state;
	enum
	{
		UNKNOWN_HALTED,
		DBGRQ_HALTED,
		WATCHPOINT_HALTED,
		BREAKPOINT_HALTED,
		WP_WITH_EXCEPTION_HALTED,
	}
	dbg_halt_reason;
	enum
	{
		ENTERED_FROM_THUMB,
		ENTERED_FROM_ARM,
	}
	dbg_entry_arm_mode;
	/*! \todo	properly handle this, this is by far subtle and i dont know how to handle it properly */
	ARM_CORE_WORD		eice_abort_stat_reg;
	/* the plus 2 is for the cpsr and spsr register of the corresponding mode
	 *
	 * cpsr is at index NR_CORE_REGS, spsr - at NR_CORE_REGS + 1 */
	ARM_CORE_WORD		core_regs[NR_CORE_MODES][NR_CORE_REGS + 2];
	/* bitmaps for the cached registers - already read in the array above and available
	 * for access without fetching from the target; this basically operates as a
	 * simple writeback-like cache with registers actually written to the core when the target program
	 * is resumed
	 *
	 * the core registers go here, as usual cpsr is at bit index NR_CORE_REGS, spsr - at NR_CORE_REGS + 1 */
	ARM_CORE_WORD		cached_core_regs[NR_CORE_MODES];
	/* clobbered registers bitmaps */
	ARM_CORE_WORD		clobbered_core_regs[NR_CORE_MODES];
	/* current core mode, may differ from dbgentry_cpsr - identifies also the currently active register bank */
	enum ARM7TDMIS_CORE_MODE	cur_mode;
	ARM_CORE_WORD		dbgentry_pc;
	ARM_CORE_WORD		dbgentry_cpsr;
}
corestat;

static int scan_chain_active;

static enum
{
	JTAG_STATE_UNKNOWN = 0,
	JTAG_STATE_INTEST,
}
current_jtag_state;

/*! \todo	the information kept here is not
 *		really correctly handled everywhere right
 *		now (see for example the check for a
 *		breakpoint hit in the debug mode
 *		reason validation code below) */
static struct eice_wp_bp_res
{
	int	is_in_use;
	enum
	{
		ARM7TDMIS_EICE_NONE = 0,
		ARM7TDMIS_EICE_BREAKPOINT,
		ARM7TDMIS_EICE_WATCHPOINT,
	}
	type;
	ARM_CORE_WORD		data_val;
	ARM_CORE_WORD		data_mask;
	ARM_CORE_WORD		addr_val;
	ARM_CORE_WORD		addr_mask;
	ARM_CORE_WORD		ctl_val;
	ARM_CORE_WORD		ctl_mask;
}
arm7tdmis_eice_resources[2];

static void arm7tdmis_connect_scanchain(int sc_nr)
{
ARM_CORE_WORD buf;

	if (sc_nr != 1 && sc_nr != 2)
		panic("bad scan chain");
/*! \todo	enable this */
#if 0
	if (sc_nr == scan_chain_active)
		return;
#endif
	buf = SCAN_N;
	jtagdrv_ireg_cmd((unsigned char *)&buf, ARM7TDMIS_JTAG_IREG_WIDTH, 0);
	buf = sc_nr;
	jtagdrv_dreg_cmd((unsigned char *)&buf, ARM7TDMIS_JTAG_SCANCHAIN_REG_WIDTH);
	scan_chain_active = sc_nr;
	current_jtag_state = JTAG_STATE_UNKNOWN;
}

static ARM_CORE_WORD arm7tdmis_read_eicereg(int reg_nr)
{
ARM_CORE_WORD buf[2];

	if (scan_chain_active != 2)
		panic("bad scan chain active");
	if (current_jtag_state != JTAG_STATE_INTEST)
		panic("bad jtag state");
	buf[1] = reg_nr & ((1 << ARM7TDMIS_JTAG_ICEREG_ADDR_BITS) - 1);
	jtagdrv_dreg_cmd((unsigned char *)buf, ARM7TDMIS_JTAG_ICEREG_WIDTH);
	buf[1] = reg_nr & ((1 << ARM7TDMIS_JTAG_ICEREG_ADDR_BITS) - 1);
	jtagdrv_dreg_cmd((unsigned char *)buf, ARM7TDMIS_JTAG_ICEREG_WIDTH);
	return *buf;
}

static void arm7tdmis_write_eicereg(int reg_nr, ARM_CORE_WORD  val)
{
ARM_CORE_WORD  buf[2];

	if (scan_chain_active != 2)
		panic("bad scan chain active");
	if (current_jtag_state != JTAG_STATE_INTEST)
		panic("bad jtag state");
	buf[0] = val;
	buf[1] = (reg_nr & ((1 << ARM7TDMIS_JTAG_ICEREG_ADDR_BITS) - 1))
		| (1 << ARM7TDMIS_JTAG_ICEREG_ADDR_BITS);
	jtagdrv_dreg_cmd((unsigned char *)buf, ARM7TDMIS_JTAG_ICEREG_WIDTH);
}

static enum GEAR_ENGINE_ERR_ENUM core_set_break(struct target_ctl_context * ctx, ARM_CORE_WORD address)
{
int i;

	/* see if the address is already breakpointed */
	for (i = 0; i < 2; i++)
		if (arm7tdmis_eice_resources[i].is_in_use
			&& arm7tdmis_eice_resources[i].addr_val == address)
			panic("breakpoint already set\n");
	/* find an unused breakpoint resource */
	for (i = 0; i < 2; i++)
		if (!arm7tdmis_eice_resources[i].is_in_use)
			break;
	if (i == 2)
		panic("no more breakpoint resources");
	arm7tdmis_eice_resources[i].is_in_use = 1;
	arm7tdmis_eice_resources[i].type = ARM7TDMIS_EICE_BREAKPOINT;
	arm7tdmis_eice_resources[i].addr_val = address;

	arm7tdmis_connect_scanchain(2);
	if (!i)
	{
		/* program watchpoint 0 registers */
		arm7tdmis_write_eicereg(EICE_REG_WP0ADDRVAL_ADDR, address);
		/* only arm breakpoints supported right now */
		arm7tdmis_write_eicereg(EICE_REG_WP0ADDRMASK_ADDR, 3);
		arm7tdmis_write_eicereg(EICE_REG_WP0DATAMASK_ADDR, 0xffffffff);
		arm7tdmis_write_eicereg(EICE_REG_WP0CTLMASK_ADDR, 0xf7);
		arm7tdmis_write_eicereg(EICE_REG_WP0CTLVAL_ADDR, 0x108);
	}
	else
	{
		/* program watchpoint 1 registers */
		arm7tdmis_write_eicereg(EICE_REG_WP1ADDRVAL_ADDR, address);
		/* only arm breakpoints supported right now */
		arm7tdmis_write_eicereg(EICE_REG_WP1ADDRMASK_ADDR, 3);
		arm7tdmis_write_eicereg(EICE_REG_WP1DATAMASK_ADDR, 0xffffffff);
		arm7tdmis_write_eicereg(EICE_REG_WP1CTLMASK_ADDR, 0xf7);
		arm7tdmis_write_eicereg(EICE_REG_WP1CTLVAL_ADDR, 0x108);
	}

	return GEAR_ERR_NO_ERROR;

}
static enum GEAR_ENGINE_ERR_ENUM core_clear_break(struct target_ctl_context * ctx, ARM_CORE_WORD address)
{
int i;

	/* make sure the breakpoint is active */
	for (i = 0; i < 2; i++)
		if (arm7tdmis_eice_resources[i].is_in_use
			&& arm7tdmis_eice_resources[i].addr_val == address
			&& arm7tdmis_eice_resources[i].type == ARM7TDMIS_EICE_BREAKPOINT)
			break;
	if (i == 2)
		panic("breakpoint not programmed\n");

	arm7tdmis_connect_scanchain(2);
	if (!i)
	{
		/* program watchpoint 0 registers */
		arm7tdmis_write_eicereg(EICE_REG_WP0CTLVAL_ADDR, 0);
	}
	else
	{
		/* program watchpoint 0 registers */
		arm7tdmis_write_eicereg(EICE_REG_WP1CTLVAL_ADDR, 0);
	}
	arm7tdmis_eice_resources[i].is_in_use = 0;

	return GEAR_ERR_NO_ERROR;
}


/*!
 *	\fn	static ARM_CORE_WORD arm7tdmis_exec_insn(ARM_CORE_WORD insn, int breakpt, int * breakpt_out)
 *	\brief	execute an arm instruction
 *
 *	this function puts an instruction in the pipeline using scan chain 1, bit 33, breakpt, is
 *	used to determine if the instruction is to execute at system (breakpt == 1) or debug
 *	(breakpt == 0) speed; if of interest, the value of the breakpt signal can be stored in
 *	*breakpt_out (useful for determining the debug mode entry reason)
 *
 *	\param	insn	32 bit arm/16 bit thumb instruction opcode to be put in the pipeline;
 *			if a thumb instruction, bits 0-15 must equal bits 16-31 must equal
 *			the desired thumb opcode
 *	\param	breakpt	the value of the breakpt signal to be used for the instruction
 *	\param	breakpt_out	pointer to where to store the captured breakpt signal, can be 0
 *	\return	the data value captured out of the arm core
 */

static ARM_CORE_WORD arm7tdmis_exec_insn(ARM_CORE_WORD insn, int breakpt, int * breakpt_out)
{
ARM_CORE_WORD buf[2];

	if (scan_chain_active != 1)
		panic("bad scan chain active");
	if (current_jtag_state != JTAG_STATE_INTEST)
		panic("bad jtag state");
	/* invert bits */

	insn = ((insn & 0x55555555) << 1) | ((insn & 0xaaaaaaaa) >> 1);
	insn = ((insn & 0x33333333) << 2) | ((insn & 0xcccccccc) >> 2);
	insn = ((insn & 0x0f0f0f0f) << 4) | ((insn & 0xf0f0f0f0) >> 4);
	insn = ((insn & 0x00ff00ff) << 8) | ((insn & 0xff00ff00) >> 8);
	insn = ((insn & 0x0000ffff) << 16) | ((insn & 0xffff0000) >> 16);

	buf[1] = (insn & 0x80000000) ? 1 : 0;
	insn <<= 1;
	insn |= breakpt & 1;
	buf[0] = insn;

	jtagdrv_dreg_cmd((unsigned char *)buf, ARM7TDMIS_JTAG_SC1_WIDTH);
	if (breakpt_out)
		*breakpt_out = *buf & 1;
	insn = *buf;
	insn >>= 1;
	if (buf[1])
		insn |= 0x80000000;

	/* invert back */
	insn = ((insn & 0x55555555) << 1) | ((insn & 0xaaaaaaaa) >> 1);
	insn = ((insn & 0x33333333) << 2) | ((insn & 0xcccccccc) >> 2);
	insn = ((insn & 0x0f0f0f0f) << 4) | ((insn & 0xf0f0f0f0) >> 4);
	insn = ((insn & 0x00ff00ff) << 8) | ((insn & 0xff00ff00) >> 8);
	insn = ((insn & 0x0000ffff) << 16) | ((insn & 0xffff0000) >> 16);

	return insn;
}


void arm7tdmis_force_intest(void)
{
ARM_CORE_WORD buf;

	/*! \todo	enable this */
#if 0
	if (current_jtag_state == JTAG_STATE_INTEST)
		return;
#endif
	buf = INTEST;
	jtagdrv_ireg_cmd((unsigned char *)&buf, ARM7TDMIS_JTAG_IREG_WIDTH, 0);
	current_jtag_state = JTAG_STATE_INTEST;
}

void arm7tdmis_request_target_halt(void)
{
#if 0
	arm7tdmis_connect_scanchain(2);
	arm7tdmis_force_intest();
	arm7tdmis_write_eicereg(EICE_REG_WP0ADDRMASK_ADDR, 0xffffffff);
	arm7tdmis_write_eicereg(EICE_REG_WP0DATAMASK_ADDR, 0xffffffff);
	arm7tdmis_write_eicereg(EICE_REG_WP0CTLMASK_ADDR, 0xf7);
	arm7tdmis_write_eicereg(EICE_REG_WP0CTLVAL_ADDR, 0x1f7);
#else
	arm7tdmis_connect_scanchain(2);
	arm7tdmis_force_intest();
	arm7tdmis_write_eicereg(EICE_REG_DBGCTL_ADDR, BIT1);
	is_dbgrq_asserted = 1;
#endif	
}

static void mark_regs_cached(unsigned long mask)
{
int i;
unsigned long t;

	for (i = 0; i < NR_CORE_MODES; i++)
	{
		if (i != corestat.cur_mode)
		{
			if (i != ARM_MODE_FIQ)
				t = 0x1fff;
			else
				t = 0xff;
		}
		else
			t = 0xffff;
		corestat.cached_core_regs[i] |= mask & t;
	}
}

static void mark_regs_clobbered(unsigned long mask)
{
int i;
unsigned long t;

	for (i = 0; i < NR_CORE_MODES; i++)
	{
		if (i != corestat.cur_mode)
		{
			if (i != ARM_MODE_FIQ)
				t = 0x1fff;
			else
				t = 0xff;
		}
		else
			t = 0xffff;
		/* sanity check */
		if ((corestat.cached_core_regs[i] & (mask & t)) ^ (mask & t))
			panic("tried to clobber a non-cached register\n");
		corestat.clobbered_core_regs[i] |= mask & t;
	}
}


static ARM_CORE_WORD cpsr_read(void)
{
	/* sanity checks */
	if (!(corestat.cached_core_regs[corestat.cur_mode] & 1))
		panic("");
	/* use r0 to read the cpsr */
	mark_regs_clobbered(1);
	arm7tdmis_exec_insn(ARM_OPC_MRS_R0_CPSR, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_STR_R0_AT_R15, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	/* does not enter the pipeline */
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	return arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
}

static void regs_read_core(unsigned long mask, ARM_CORE_WORD * buf)
{
ARM_CORE_WORD opc;
int i;

	/* assemble an stm instruction for the registers not already cached */
	opc = ARM_OPC_STMIA_R0_EMPTY_REG_LIST | (mask & 0xffff);
	/* start feeding the pipeline */
	arm7tdmis_exec_insn(opc, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	/* does not enter the pipeline */
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	for (i = 0; i < NR_CORE_REGS; i++)
	{
		if (!(mask & (1 << i)))
			continue;
		/* does not enter the pipeline */
		buf[i] = arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	}
	/*! \todo	is this really necessary? things seem to work without this,
	 *		i think the last instruction scanned in the loop above
	 *		actually enters the pipeline, but i am not sure... */
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_read(struct target_ctl_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
ARM_CORE_WORD buf[NR_CORE_REGS];
unsigned long t, cmask;
int i, j;

	if (corestat.core_state != DBG_HALTED)
		panic("bad core mode");
	/*
	if (mode != RDIMode_Curr)
		panic("must support mode selection");
		*/
	if (mask & (0xffff0000))
		printf("downgraded from panic to printf, must support all regs");
	



#define RDIReg_R15              (1L << 15)	/* mask values for CPU */
#define RDIReg_PC               (1L << 16)
#define RDIReg_CPSR             (1L << 17)
#define RDIReg_SPSR             (1L << 18)

	/* process base core registers (without psrs) */
	/* read any registers not yet cached */
	t = (mask & corestat.cached_core_regs[corestat.cur_mode]) ^ mask;
	if (t)
	{
		regs_read_core(t, buf);
		/* populate register banks with newly read registers */
		for (j = 0; j < NR_CORE_MODES; j++)
		{
			/* never touch the pc */
			if (j != corestat.cur_mode)
			{
				if (j != ARM_MODE_FIQ)
					cmask = 0x1fff;
				else
					cmask = 0xff;
			}
			else
				cmask = 0x7fff;
			cmask &= t;

			for (i = 0; i < NR_CORE_REGS; i++)
			{
				if (!(cmask & (1 << i)))
					continue;
				corestat.core_regs[j][i] = buf[i];
			}
		}
		mark_regs_cached(t);
	}
	/* populate output buffer */
	for (i = j = 0; i < NR_CORE_REGS - 1; i++)
	{
		if (!(mask & (1 << i)))
			continue;
		buffer[j++] = corestat.core_regs[corestat.cur_mode][i];
	}
	/* special case */
	if (mask & BIT15)
		buffer[j++] = corestat.dbgentry_pc;
	if (mask & RDIReg_PC)
		buffer[j++] = corestat.dbgentry_pc;
	if (mask & RDIReg_CPSR)
		buffer[j++] = corestat.dbgentry_cpsr;
	if (mask & RDIReg_SPSR)
		printf("must handle spsr\n");
	/*! \todo	process psrs here */
	return GEAR_ERR_NO_ERROR;
}


static enum GEAR_ENGINE_ERR_ENUM core_reg_write(struct target_ctl_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i, j, k;

	if (corestat.core_state != DBG_HALTED)
		panic("bad core mode");
	/*! \todo	handle modes and all registers here */
	if (mask & ~(BIT16 - 1))
		panic("todo: support all registers");
	/* store registers and mark them as cached */
	for (i = j = 0; i < 15; i++)
		if (mask & (1 << i))
		{
			for (k = 0; k < NR_CORE_MODES; k++)
				corestat.core_regs[k][i] = buffer[j];
			j++;
		}
	if (mask & BIT15)
		corestat.dbgentry_pc = buffer[j];
	mark_regs_cached(mask);
	mark_regs_clobbered(mask);
	return GEAR_ERR_NO_ERROR;
}


/*! \todo	optimize number of register accesses here */
/*! \todo	use ldmia instructions with writeback here, thus we can 
 *		save reloading r0 each time - might gain
 *		some speed */ 
static enum GEAR_ENGINE_ERR_ENUM core_mem_read(struct target_ctl_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes)
{
ARM_CORE_WORD t;
unsigned char * pdest, * psrc;
unsigned int len, clen;
ARM_CORE_WORD regs[14];
int i;

	if (corestat.core_state != DBG_HALTED)
		panic("bad core mode");
	/* stash all of the registers for the current mode (if not already cached) */
	core_reg_read(ctx, /*RDIMode_Curr*/0, 0x7fff, regs);
	/* also clobber them */
	mark_regs_clobbered(0x7fff);
	if (get_bank_for_mode(extract_psr_mode_bits(cpsr_read())) == ARM_MODE_ABORT)
		panic("must not read memory from within abort mode");
	/* setup copying helpers */
	pdest = (unsigned char *) dest;
	len = *nbytes;
	*nbytes = 0;

	while (len)
	{
		/* start feeding the pipeline */
		arm7tdmis_exec_insn(ARM_OPC_MOV_R0_0, 0, 0);

		arm7tdmis_exec_insn(ARM_OPC_LDR_R0_FROM_R0, 0, 0);
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		/* does not enter the pipeline */
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		/* does not enter the pipeline */
		/* fetch data for the ldr instruction */
		/* force word alignment of target address */
		arm7tdmis_exec_insn(source & (~3)/*0xdaeba*/, 0, 0);
		/*! \todo	i do not understand why, but the sequence below works (found that by trial and error);
		  *		looks like my understanding of the pipeline operation
		  *		of the arm core in debug mode is imperfect; make this clear */

		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		/* there is one additional nop shown in arm
		   * application note 28, before the one that
		   * is scanned with breakpt high; yet, there
		   * are some glaring errors in this datasheet, too */
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		
		/*! \todo	i dont bloody know why this here is necessary,
		 *		but it bloody *is* */ 
		arm7tdmis_exec_insn(ARM_OPC_NOP, 1, 0);

		arm7tdmis_exec_insn(ARM_OPC_LDMIA_R0_EMPTY_REG_LIST | 0x7fff, 0, 0);
		/* restart core at system speed */
		corestat.core_state = DBG_RUNNING;
		t = RESTART;
		/*! \todo	must interrupts be disabled here? i think not,
		 * because the documentation says when dbgack is high, interrupts
		 * are disabled, but make this clear */
		jtagdrv_ireg_cmd((unsigned char *)&t, ARM7TDMIS_JTAG_IREG_WIDTH, 1);

		/* wait for debug mode to be reentered */
		/*! \todo	make the wait loop here time-out driven */
		arm7tdmis_connect_scanchain(2);
		arm7tdmis_force_intest();
		for (i = 0; i < 16; i++)
		{
			t = arm7tdmis_read_eicereg(EICE_REG_DBGSTAT_ADDR);
			/*! \todo	is this correct */
			if ((t & (BIT3 | BIT0)) == (BIT3 | BIT0))
			{
				corestat.core_state = DBG_HALTED;
				if (t & BIT1)
					panic("external debug request sensed, dont know what to do\n");
				/* read abort register */
				t = arm7tdmis_read_eicereg(EICE_REG_ABORTSTAT_ADDR);
				if (t)
					panic("");
				arm7tdmis_connect_scanchain(1);
				arm7tdmis_force_intest();

				/* read registers back */
				regs_read_core(0x7fff, regs);
				/* see if abort was entered and act accordingly */
				if (get_bank_for_mode(extract_psr_mode_bits(cpsr_read())) == ARM_MODE_ABORT)
					panic("must handle abort from system speed debug access");
				break;
			}
		}
		if (i == 16)
			panic("system speed access timed out");
		/* copy the current chunk of data */
		/* this below is nonoptimal but it doesnt make much sense to optimize it, since
		   * the real slow operation is the target memory access above */
		psrc = (unsigned char *) regs;
		psrc += source & 3;
		clen = sizeof(regs);
		clen -= source & 3;
		if (clen > len)
			clen = len;
		memcpy(pdest, psrc, clen);
		len -= clen;
		source += clen;
		pdest += clen;
		*nbytes += clen;
	}
	return GEAR_ERR_NO_ERROR;
}


/*! \todo	optimize number of register accesses here */
/*! \todo	use stmia with writeback instructions here, thus we can 
 *		save reloading r0 each time - might gain
 *		some speed */ 
static enum GEAR_ENGINE_ERR_ENUM core_mem_write(struct target_ctl_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes)
{
ARM_CORE_WORD t;
unsigned char * psrc;
unsigned int len, bytes_this_run;
unsigned int i;
/*! dummy, used just for stashing the registers in the cache
 * (if not already there) */ 
ARM_CORE_WORD regs[14];

	if (corestat.core_state != DBG_HALTED)
		panic("bad core mode");
	/* stash all of the registers for the current mode (if not already cached) */
	core_reg_read(ctx, /*RDIMode_Curr*/0, 0x7fff, regs);
	/* also clobber them */
	/*! \todo	we can do much better here for small memory accesses */
	mark_regs_clobbered(0x7fff);
	if (get_bank_for_mode(extract_psr_mode_bits(cpsr_read())) == ARM_MODE_ABORT)
		panic("must not write memory from within abort mode");
	/* setup copying helpers */
	psrc = (unsigned char *) source;
	len = *nbytes;
	*nbytes = 0;

	while (len)
	{
		/* start feeding the pipeline */
		arm7tdmis_exec_insn(ARM_OPC_MOV_R0_0, 0, 0);

		/* according to destination address alignment and bytes
		 * remaining to write, transfer either single
	         * bytes or one or more word(s); halfwords are
		 * currently not handled */	 
		if ((dest & 3) || len < sizeof(ARM_CORE_WORD))
		{
			/* transfer a single byte */
			bytes_this_run = 1;
			arm7tdmis_exec_insn(ARM_OPC_LDR_R1_FROM_R0, 0, 0);
			arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
			/* does not enter the pipeline */
			arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
			/* does not enter the pipeline */
			/* fetch data for the ldr instruction */
			arm7tdmis_exec_insn(*psrc++, 0, 0);
			t = ARM_OPC_STRB_R1_AT_R0;
		}
		else
		{
			/* transfer word(s) */
			t = len >> 2;
			if (t > NR_CORE_REGS - 1 /* transfer base register excluded (r0) */ - 1
					/* r15 not available as a general purpose register */)
				t = NR_CORE_REGS - 1 - 1;
			bytes_this_run = t * sizeof(ARM_CORE_WORD);
			/* produce register load mask */
			t = 1 << t;
			t--;
			t <<= 1;
			arm7tdmis_exec_insn(ARM_OPC_LDMIA_R0_EMPTY_REG_LIST | t, 0, 0);
			arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
			/* does not enter the pipeline */
			arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
			/* does not enter the pipeline */
			/* feed the pipeline - fetch data for the ldm instruction */
			for (i = t >> 1; i; i >>= 1, psrc += sizeof(ARM_CORE_WORD))
				arm7tdmis_exec_insn(*(ARM_CORE_WORD *)psrc, 0, 0);
			t |= ARM_OPC_STMIA_R0_EMPTY_REG_LIST;
		}
		/*! \todo	i still dont know
		 *	why this nop here is needed, but
		 *	it apparently is */ 
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);

		/* load base address for memory writing */
		arm7tdmis_exec_insn(ARM_OPC_LDR_R0_FROM_R0, 0, 0);
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		/* does not enter the pipeline */
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		/* does not enter the pipeline */
		/* fetch data for the ldr instruction */
		arm7tdmis_exec_insn(dest, 0, 0);

		/*! \todo	i still dont know
		 *	why this nop here is needed, but
		 *	it apparently is */ 
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);

		/*! \todo	i do not understand why, but the sequence below works (found that by trial and error);
		  *		looks like my understanding of the pipeline operation
		  *		of the arm core in debug mode is imperfect; make this clear */

		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		/* there is one additional nop shown in arm
		   * application note 28, before the one that
		   * is scanned with breakpt high; yet, there
		   * are some glaring errors in this datasheet, too */
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	/*

		regs_read_core(0x7fff, regs);
		printf("\n\n\nr0\t0x%08x\n", *regs);
		exit(1);//*/
		
		/*! \todo	i dont bloody know why this here is necessary,
		 *		but it bloody *is* */ 
		arm7tdmis_exec_insn(ARM_OPC_NOP, 1, 0);

		arm7tdmis_exec_insn(t, 0, 0);
		/* restart core at system speed */
		corestat.core_state = DBG_RUNNING;
		t = RESTART;
		/*! \todo	must interrupts be disabled here? i think not,
		 * because the documentation says when dbgack is high, interrupts
		 * are disabled, but make this clear */
		jtagdrv_ireg_cmd((unsigned char *)&t, ARM7TDMIS_JTAG_IREG_WIDTH, 1);
		/* wait for debug mode to be reentered */
		/*! \todo	make the wait loop here time-out driven */
		arm7tdmis_connect_scanchain(2);
		arm7tdmis_force_intest();
		for (i = 0; i < 16; i++)
		{
			t = arm7tdmis_read_eicereg(EICE_REG_DBGSTAT_ADDR);
			/*! \todo	is this correct */
			if ((t & (BIT3 | BIT0)) == (BIT3 | BIT0))
			{
				corestat.core_state = DBG_HALTED;
				if (t & BIT1)
					panic("external debug request sensed, dont know what to do\n");
				/* read abort register */
				t = arm7tdmis_read_eicereg(EICE_REG_ABORTSTAT_ADDR);
				if (t)
					panic("");
				arm7tdmis_connect_scanchain(1);
				arm7tdmis_force_intest();
				/* see if abort was entered and act accordingly */
				if (get_bank_for_mode(extract_psr_mode_bits(cpsr_read())) == ARM_MODE_ABORT)
					panic("must handle abort from system speed debug access");
				break;
			}
		}
		if (i == 16)
			panic("system speed access timed out");
		/* advance and adjust pointers and transfer sizes */
		dest += bytes_this_run;
		*nbytes += bytes_this_run;
		len -= bytes_this_run;
	}
	return GEAR_ERR_NO_ERROR;
}


static int arm7tdmis_is_debug_halted(struct target_ctl_context * ctx)
{
ARM_CORE_WORD t;
ARM_CORE_WORD r0;
ARM_CORE_WORD r15;
ARM_CORE_WORD cpsr;
int breakpt;
int i;

	if (corestat.core_state != SYS_RUNNING)
		panic("should not poll target debug status - already not at system speed");
	/* inspect debug status */
	arm7tdmis_connect_scanchain(2);
	arm7tdmis_force_intest();
	t = arm7tdmis_read_eicereg(EICE_REG_DBGSTAT_ADDR);
	/*! \todo	is this correct */
	if ((t & (BIT3 | BIT0)) == (BIT3 | BIT0))
	{
		corestat.core_state = DBG_HALTED;
		if (t & BIT1)
			panic("external debug request sensed, dont know what to do\n");
		/* debug mode entered */
		printf("dbgack asserted, entering debug domains\n");
		/* deassert dbgrq (if at all asserted) and force dbgack high to acknowledge
		 * we are in debug mode */
		arm7tdmis_write_eicereg(EICE_REG_DBGCTL_ADDR, BIT0);
		/* store the tbit */
		corestat.dbg_entry_arm_mode = (t & BIT4) ? ENTERED_FROM_THUMB : ENTERED_FROM_ARM;
		if (t & BIT4)
			panic("must handle thumb mode entry");
		/* read abort register */
		t = arm7tdmis_read_eicereg(EICE_REG_ABORTSTAT_ADDR);
		corestat.eice_abort_stat_reg = t;
		if (t)
			panic("debug mode entered with abort asserted");
		/* store a minimum of context to be able to resume the target program
		 * properly; it is very important that this be kept to the absolute
		 * minimum needed as the overhead imposed must be kept as small as
		 * possible; this has a huge impact on executor single-stepping which
		 * underpins all executor step and proceed commands and therefore
		 * should be as fast and lightweight as possible */

		/* save r0, r15 and cpsr */
		/*! \todo	the number of nops here can be optimized */
		arm7tdmis_connect_scanchain(1);
		arm7tdmis_force_intest();
		/* scan breakpt first time to determine debug mode entry reason */
		arm7tdmis_exec_insn(ARM_OPC_STR_R0_AT_R15, 0, &breakpt);
		arm7tdmis_exec_insn(ARM_OPC_MOV_R15_TO_R0, 0, 0);
		/* does not enter the pipeline */
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		r0 = arm7tdmis_exec_insn(ARM_OPC_STR_R0_AT_R15, 0, 0);
		arm7tdmis_exec_insn(ARM_OPC_MRS_R0_CPSR, 0, 0);
		/* does not enter the pipeline */
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		r15 = arm7tdmis_exec_insn(ARM_OPC_STR_R0_AT_R15, 0, 0);
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		/* does not enter the pipeline */
		arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		cpsr = arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
		/* fix r15 */
		r15 -= 3 * 4;
		printf("r0 = 0x%08x, r15 = 0x%08x, cpsr = 0x%08x\n", r0, r15, cpsr);
		/* determine debug entry reason
		 * this is subtle; arm7tdmi(s) cores cannot reliably
		 * tell whether debug mode was entered due to a breakpoint
		 * or a debug request; this is documented in this
		 * arm google groups post - copied here in case it falls into oblivion */
		/*

		http://groups.google.com/group/comp.sys.arm/browse_thread/thread/81ef32d310f62c5d/9311286496eb962c?lnk=gst&q=dbgrq#9311286496eb962c

		How can I tell the ARM7TDMI core stop because of DBGRQ or breakpoint/watchpoint
		Options
		There are currently too many topics in this group that display first. To make this topic appear first, remove this option from another topic.
		There was an error processing your request. Please try again.
		Standard view   View as tree
		Proportional text   Fixed text


		[Click the star to watch this topic]
		[Click the envelope to receive email updates]

		flag
				2 messages - Collapse all
		The group you are posting to is a Usenet group. Messages posted to this group will make your email address visible to anyone on the Internet.
		Your reply message has not been sent.
		Your post was successful


		hb guan
		View profile
			 More options Sep 5 2003, 2:16 am
		Newsgroups: comp.sys.arm
		From: guan.hua...@zte.com.cn (hb guan)
		Date: 4 Sep 2003 17:16:57 -0700
		Local: Fri, Sep 5 2003 2:16 am
		Subject: How can I tell the ARM7TDMI core stop because of DBGRQ or breakpoint/watchpoint
		Reply to author | Forward | Print | Individual message | Show original | Report this message | Find messages by this author
		When the ARM7TDMI core is running, I can stop it by writing
		embeddedICE deubg control register bits DBGRQ. But if we also have
		some breakpoint/watchpoint config in embeddedICE, how can I tell the
		ARM7TDMI core stop because of DBGRQ or breakpoint/watchpoint? They
		have different return address caculate formula:
		breakpoint/watchpoint: 4 +N +3S
		Debug request: 3 +N +3S
		(N: instruction running in debug speed,  S: instruction running in
		system speed.)

		Any information is welcome!

		Thanks!

		hb guan
		2003-09-05

		    Reply    Reply to author    Forward       Rate this post: Text for clearing space



		You must Sign in before you can post messages.
		To post a message you must first join this group.
		Please update your nickname on the subscription settings page before posting.
		You do not have the permission required to post.


		Dave Adshead
		View profile
			(1 user)  More options Sep 9 2003, 11:25 am
		Newsgroups: comp.sys.arm
		From: Dave Adshead <Dave.Adsh...@at-debuginnovations-dot.com>
		Date: Tue, 09 Sep 2003 10:25:09 +0100
		Local: Tues, Sep 9 2003 11:25 am
		Subject: Re: How can I tell the ARM7TDMI core stop because of DBGRQ or breakpoint/watchpoint
		Reply to author | Forward | Print | Individual message | Show original | Report this message | Find messages by this author

		- Hide quoted text -
		- Show quoted text -
		hb guan wrote:
		> When the ARM7TDMI core is running, I can stop it by writing
		> embeddedICE deubg control register bits DBGRQ. But if we also have
		> some breakpoint/watchpoint config in embeddedICE, how can I tell the
		> ARM7TDMI core stop because of DBGRQ or breakpoint/watchpoint? They
		> have different return address caculate formula:
		> breakpoint/watchpoint: 4 +N +3S
		> Debug request: 3 +N +3S
		> (N: instruction running in debug speed,  S: instruction running in
		> system speed.)

		> Any information is welcome!

		> Thanks!

		> hb guan
		> 2003-09-05

		You can't tell.  On later cores you *can* tell.  For ARM7 cores, you
		need to do some software trickery.  This is the procedure:

		1. Do a last minute check that the core is running.  If not then it must
		    have hit a breakpoint already, so deal with that in the normal way.
		2. Assert DBGRQ and wait for DBGACK.
		3. Check to see if the address that it stopped at is consistent with an
		    address that you know has an existing breakpoint.  If so, assume that
		    it is a breakpoint and do the address calculation for a breakpoint.
		    If not, assume that it stopped because of the DBGRQ and do that
		    address calculation instead.

		Optimise your code or hardware to minimise the time difference between
		steps 1 and 2.

		This mechanism is used by Multi-ICE (and presumably by all other ICEs
		that have bothered to think about the problem) and very rarely fails
		even under contrived test conditions.  Unfortunately you cannot do
		better because it is a debug architecture limitation.  Later cores
		contain 'reason for stop' bits.

		If you need more help you may be interested in my services.  I worked
		for ARM for 6 years, designed and wrote much of the code for Multi-ICE
		and am now a consultant offering these types of services - see
		www.debuginnovations.com for more info.

		Regards,

		Dave.
		*/
		/* i also wrote a letter to the arm tech support asking the question
		 * of determining the debug mode entry reason accurately; they
		 * told me that checking the dbgrqi signal should help; this, however,
		 * is in scan chain 0 of an arm7tdmi, and there is no scan chain 0
		 * on an arm7tdmis at all; but really, i doubt it is any good at all... */

		/* determine debug entry reason */
		/* if breakpt is set, this is a watchpoint */
		corestat.dbg_halt_reason = UNKNOWN_HALTED;
		if (breakpt)
		{
			corestat.dbg_halt_reason = WATCHPOINT_HALTED;
			panic("must handle a watchpoint with another exception debug mode entry case - check for this here\n");
		}
		else
		{
			if (!is_dbgrq_asserted)
			{
				corestat.dbg_halt_reason = BREAKPOINT_HALTED;
				/* also, locate which exactly breakpoint was hit */
				printf("shopov, determine the breakpoint hit accurately\n");
			}
			else
			{
				/* the hardest case - we should decide if the target
				 * halted because of a breakpoint or a debug request;
				 * in accordance with the posts above, assume that
				 * debug mode was entered due to a breakpoint, calculate
				 * the address to resume to and see if there is an
				 * active breakpoint at the address; if so, decide
				 * the breakpoint caused debug mode entry, otherwise
				 * decide debug request caused the halt */
				struct eice_wp_bp_res * wbp;
				wbp = arm7tdmis_eice_resources;
				/* this is currently quite wrong - take into account watchpoint unit mask values as well */
				printf("shopov, determine the breakpoint hit accurately\n");
				printf("the breakpoint scan loop here is wrong\n");
				for (i = 0; i < 2; i++)
				{
					if (!wbp->is_in_use || wbp->type != ARM7TDMIS_EICE_BREAKPOINT)
						continue;
					if (wbp->addr_val == r15 - 3 * 4)
					{
						corestat.dbg_halt_reason = BREAKPOINT_HALTED;
						/* and also warn the user */
						panic("warning: bad luck sensed\n");
						break;
					}
				}
				if (i == 2)
				{
					corestat.dbg_halt_reason = DBGRQ_HALTED;
				}
			}
		}
		/* print debug entry reason - this may eventually go away */
		printf("debug mode entry reason: ");
		switch (corestat.dbg_halt_reason)
		{
			case BREAKPOINT_HALTED:
				printf("breakpoint");
				break;
			case WATCHPOINT_HALTED:
				printf("watchpoint");
				break;
			case WP_WITH_EXCEPTION_HALTED:
				printf("watchpoint with another exception");
				break;
			case DBGRQ_HALTED:
				printf("debug request");
				break;
		}
		printf("\n");

		/* fixup debug return address depending on the debug entry reason */
		/* pc now contains the address of the first instruction executed in debug state
		 * (the very first 'str r0, [r0]' above */
		switch (corestat.dbg_halt_reason)
		{
			case BREAKPOINT_HALTED:
			case WATCHPOINT_HALTED:
				r15 -= 3 * 4;
				break;
			case WP_WITH_EXCEPTION_HALTED:
			case DBGRQ_HALTED:
				r15 -= 2 * 4;
				break;
			default:
				panic("unknown debug halt reason");
		}
		/* now that the debug entry reason is precisely known,
		 * do check that the breakpoint address indeed agrees
	         * with the address of a breakpoint we know about
		 * (consulting the embedded ice register cache we keep),
		 * if breakpoint was the entry reason */
		if (corestat.dbg_halt_reason == BREAKPOINT_HALTED)
		{
			struct eice_wp_bp_res * wbp;
			wbp = arm7tdmis_eice_resources;
			/*! \todo	wrong; the mask registers must be checked here
			 *		for proper validation */ 
			for (i = 0; i < 2; i++)
			{
				if (!wbp->is_in_use || wbp->type != ARM7TDMIS_EICE_BREAKPOINT)
					continue;
				if (wbp->addr_val == r15)
				{
					break;
				}
			}
			if (i < 2)
			{
				panic("breakpoint hit; not one that i know about, though");
			}
		}
		/* save the core state */
		corestat.dbgentry_cpsr = cpsr;
		corestat.dbgentry_pc = r15;
		for (i = 0; i < NR_CORE_MODES; i++)
		{
			corestat.core_regs[i][0] = r0;
			/* mark only r0 as cached and clobbered */
			corestat.clobbered_core_regs[i] = corestat.cached_core_regs[i] = 1;
		}
		/* compute active register bank */
		i = get_bank_for_mode(extract_psr_mode_bits(cpsr));
		if (i != ARM_MODE_SYSTEM && i != ARM_MODE_USER && i != ARM_MODE_SVC)
			panic("must handle all arm modes\n");
		corestat.cur_mode = i;
	
return 1;		

		printf("debug mode successfully entered\n");
		{
			unsigned char buf[512];
			int len;

			core_reg_read(ctx, /*RDIMode_Curr*/0, 0xffff, (void *) buf);
			for (i = 0; i < NR_CORE_REGS; i++)
				printf("r%i\t0x%08x\n", i, ((ARM_CORE_WORD*)buf)[i]);
			printf("trying to read mem at address 0...\n");
			len = sizeof(buf);
			len = 16;
			core_mem_read(ctx, buf, 0, &len);
			for (i = 0; i < sizeof(buf); i++)
			{
				if (!(i & 15))
					printf("\n");
				printf("0x%02hhx ", buf[i]);
			}
			printf("hope that works, remove the garbage here\n");

		}
		return 1;

	}
	return 0;
}

static enum GEAR_ENGINE_ERR_ENUM core_run(struct target_ctl_context * ctx/*ARM_CORE_WORD * halt_addr*/)
{
int i, bank_nr;
ARM_CORE_WORD t;

	if (corestat.core_state != DBG_HALTED)
		panic("");
	arm7tdmis_exec_insn(ARM_OPC_MOV_R0_0, 0, 0);
	/* restore general purpose registers */
	bank_nr = get_bank_for_mode(extract_psr_mode_bits(corestat.dbgentry_cpsr));

	arm7tdmis_exec_insn(ARM_OPC_MOV_R0_0, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_LDR_R0_FROM_R0, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	arm7tdmis_exec_insn(corestat.dbgentry_cpsr , 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_MSR_CPSR_R0, 0, 0);
	/* now restore all registers, pc included */
	arm7tdmis_exec_insn(ARM_OPC_LDMIA_R0_EMPTY_REG_LIST | 0xffff, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	for (i = 0; i < 15; i++)
		arm7tdmis_exec_insn(corestat.core_regs[bank_nr][i], 0, 0);
	/* load the pc */
	arm7tdmis_exec_insn(corestat.dbgentry_pc, 0, 0);
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	/* enters the pipeline */
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);
	/* enters the pipeline */
	arm7tdmis_exec_insn(ARM_OPC_NOP, 0, 0);

	arm7tdmis_exec_insn(ARM_OPC_NOP, 1, 0);
	arm7tdmis_exec_insn(ARM_OPC_BRANCH_MINUS_SIX, 0, 0);

	t = RESTART;
	/*! \todo	must interrupts be disabled here? i think not,
	 * because the documentation says when dbgack is high, interrupts
	 * are disabled, but make this clear */
	jtagdrv_ireg_cmd((unsigned char *)&t, ARM7TDMIS_JTAG_IREG_WIDTH, 1);

	corestat.core_state = SYS_RUNNING;
	return GEAR_ERR_NO_ERROR;

}

static enum GEAR_ENGINE_ERR_ENUM core_get_status(struct target_ctl_context * ctx, enum TARGET_CORE_STATE_ENUM * status)
{
	if (corestat.core_state != SYS_RUNNING
			|| arm7tdmis_is_debug_halted(ctx))
		* status = TARGET_CORE_STATE_HALTED;
	else
		* status = TARGET_CORE_STATE_RUNNING;
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM core_open(struct target_ctl_context * ctx)
{
	memset(&corestat, 0, sizeof(corestat));

	memset(arm7tdmis_eice_resources, 0, sizeof(arm7tdmis_eice_resources));
	corestat.core_state = SYS_RUNNING;
	current_jtag_state = JTAG_STATE_UNKNOWN;
	is_dbgrq_asserted = 0;

	init_jtagdrv(4, 0, 0, 0, 0);
	arm7tdmis_request_target_halt();

	if (!arm7tdmis_is_debug_halted(ctx))
		panic("");
	return GEAR_ERR_NO_ERROR;
}

static struct core_access_struct core_funcs =
{
	/* core_open */
	core_open,
	/* core_close */
	0,
	/* core_mem_read */
	core_mem_read,
	/* core_mem_write */
	core_mem_write,
	/* core_reg_read */
	core_reg_read,
	/* core_reg_write */
	core_reg_write,
	/* core_cop_read */
	0,
	/* core_cop_write */
	0,
	/* core_set_break */
	core_set_break,
	/* core_clear_break */
	core_clear_break,
	/* core_step */
	0,
	/* core_run */
	core_run,
	/* core_get_status */
	core_get_status,
};

int target_is_core_running(struct target_ctl_context * ctx)
{
	if (corestat.core_state == SYS_RUNNING)
		return 1;
	return 0;
}

enum GEAR_ENGINE_ERR_ENUM target_get_core_access(struct target_ctl_context * ctx)
{
	ctx->cc = &core_funcs;
	return GEAR_ERR_NO_ERROR;
}

