

#include "jtag.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>


#define JTAG_INTERFACE_FTDI_STM32_PERFORMANCE_STICK

#ifdef JTAG_INTERFACE_WIGGLER

/* Connection definition between JTAG & Parallel Port */

/*
 * The following table shows the pin assignment of 25-pin  Parallel Printer Port.
 * please refer to IEEE 1284 standard for detailed description.
 * data port (Out)		    status port (In)
 * bit[7] -- pin9 (Out)		bit[7] -- pin11 (In), busy (Hardware Inverted)
 * bit[6] -- pin8 (Out)		bit[6] -- pin10 (In), Ack
 * bit[5] -- pin7 (Out)		bit[5] -- pin12 (In), Paper out
 * bit[4] -- pin6 (Out)		bit[4] -- pin13 (In), Select
 * bit[3] -- pin5 (Out)		bit[3] -- pin15 (In), Error
 * bit[2] -- pin4 (Out)		bit[2] -- IRQ(Not)
 * bit[1] -- pin3 (Out)		bit[1] -- Reserved
 * bit[0] -- pin2 (Out)		bit[0] -- Reserved
 */


#include <sys/io.h>

/* I/O address of parallet port */
#define PARAL_DATA_PORT 	0x378
#define PARAL_STAT_PORT		0x379
#define PARAL_INIT_DATA		0x0
#define PARAL_PIN_11		0x80	//A special pin (Hardware Inverted)

static void jtag_delay(void)
{
}
static int __tms, __tdi;

#define SET_TMS()	do { __tms = 1; } while(0)
#define CLR_TMS()	do { __tms = 0; } while(0)
#define SET_TDI()	do { __tdi = 1; } while(0)
#define CLR_TDI()	do { __tdi = 0; } while(0)
#define CLR_TCK()	do { unsigned char __pval = 0; if (__tms) __pval |= JTAG_TMS; if (__tdi) __pval |= JTAG_TDI; jtag_delay(); outb(JTAG_UNUSED_MASK | __pval | JTAG_NTRST, 0x378); } while(0)
#define SET_TCK()	do { unsigned char __pval = 0; if (__tms) __pval |= JTAG_TMS; if (__tdi) __pval |= JTAG_TDI; jtag_delay(); outb(JTAG_UNUSED_MASK | __pval | JTAG_NTRST | JTAG_CLK, 0x378); } while(0)
#define PULSE_TCK()	do { unsigned char __pval = 0; if (__tms) __pval |= JTAG_TMS; if (__tdi) __pval |= JTAG_TDI; jtag_delay(); outb(JTAG_UNUSED_MASK | __pval | JTAG_NTRST, 0x378); jtag_delay(); outb(JTAG_UNUSED_MASK | __pval | JTAG_NTRST | JTAG_CLK, 0x378); jtag_delay(); outb(JTAG_UNUSED_MASK | __pval | JTAG_NTRST , 0x378); }  while(0)
#define READ_TDO()	(((inb(0x379) >> 7) & 1) ^ 1)

/*! \todo	maybe dont drive trst here */
#define SET_SRST()	do { jtag_delay(); outb(JTAG_UNUSED_MASK | JTAG_SRST /*| JTAG_NTRST*/, 0x378); } while (0)
#define CLR_SRST()	do { jtag_delay(); outb(JTAG_UNUSED_MASK | JTAG_NTRST, 0x378); } while (0)


/* error signal - directly connected to data6 */
#define JTAG_SRST	0x01	/* pin is hardware inverted */
#define JTAG_TMS	0x02
#define JTAG_CLK	0x04
#define JTAG_TDI	0x08
#define JTAG_NTRST	0x10

#define JTAG_UNUSED_MASK	0xe0

#define JTAG_TDO	0x80	/* remember this signal is inverted */

static void init_jtag_driver_wiggler(void)
{
	/* gain port access rights */
	if (ioperm(0x378, 3, 1))
		panic("cannot access parallel port registers");
}

#elif defined JTAG_INTERFACE_FTDI_STM32_PERFORMANCE_STICK

#include <ftdi.h>

static struct ftdi_context ftdic;

static void jtag_delay(void)
{
}
static int __tms, __tdi;

#define SET_TMS()	do { __tms = 1; } while(0)
#define CLR_TMS()	do { __tms = 0; } while(0)
#define SET_TDI()	do { __tdi = 1; } while(0)
#define CLR_TDI()	do { __tdi = 0; } while(0)
#define CLR_TCK()	do { unsigned char __pval = 0, buf[3]; if (__tms) __pval |= JTAG_TMS; if (__tdi) __pval |= JTAG_TDI; buf[0] = 0x80; buf[1] = __pval; buf[2] = 0x8b; if (ftdi_write_data(&ftdic, buf, 3) != 3) panic(""); } while(0)
#define SET_TCK()	do { unsigned char __pval = 0, buf[3]; if (__tms) __pval |= JTAG_TMS; if (__tdi) __pval |= JTAG_TDI; buf[0] = 0x80; buf[1] = __pval | JTAG_CLK; buf[2] = 0x8b; if (ftdi_write_data(&ftdic, buf, 3) != 3) panic(""); } while(0)
#define PULSE_TCK()	do { SET_TCK(); CLR_TCK(); } while(0)


/* \note	for the stm32 performance stick, trst is bit0
 *		in the high four bits in mpsse mode, and
 *		to drive srst low, which will reset the cores
 *		on the stm32 stick, bit1 in the high four
 *		bits and bit 8 in the low eight bits must
 *		be driven low - any other combination of
 *		these two bits will drive the reset inputs
 *		to the cores high, which effectively deasserts
 *		reset of the cores (this is the normal
 *		operation mode of the cores - out of reset) */
void SET_SRST(void)
{
char buf[3];

	buf[0] = 0x80;
	buf[1] = 0x00;
	buf[2] = 0x8b;
	if (ftdi_write_data(&ftdic, buf, 3) != 3)
		panic("");
	/*! \todo	maybe dont drive trst here */
	/* drive srst low (reset the core) and trst low (reset the jtag tap
	 * controller) */
	buf[0] = 0x82;
	buf[1] = 0x00;
	buf[2] = 0x03;
	if (ftdi_write_data(&ftdic, buf, 3) != 3)
		panic("");
}

#define JTAG_TMS	0x08
#define JTAG_CLK	0x01
#define JTAG_TDI	0x02
#define JTAG_TDO	0x04

void CLR_SRST(void)
{
char buf[3];

	buf[0] = 0x80;
	buf[1] = 0x80 | JTAG_TMS;
	buf[2] = 0x8b;
	if (ftdi_write_data(&ftdic, buf, 3) != 3)
		panic("");
	/*! \todo	maybe dont drive trst here */
	/* drive srst high (release reset of the core)
	 * and trst high (release reset of the jtag tap controller) */
	buf[0] = 0x82;
	buf[1] = 0x03;
	buf[2] = 0x03;
	if (ftdi_write_data(&ftdic, buf, 3) != 3)
		panic("");
}

static int READ_TDO(void)
{
char buf;
int res;

	buf = 0x81;
	if (ftdi_write_data(&ftdic, &buf, 1) != 1)
		panic("");
	while (1)
	{
		res = ftdi_read_data(&ftdic, &buf, 1);
		if (res < 0)
			panic("");
		if (res == 1)
			break;
	}
	if (buf & JTAG_TDO)
		return 1;
	else
		return 0;

}

static void init_jtag_driver_stm32_stick(void)
{
	ftdi_init(&ftdic);
	if (ftdi_usb_open(&ftdic, 0x640, 0x2d) < 0)
		panic("");
	/* select channel a */
	ftdi_set_interface(&ftdic, INTERFACE_A);
	/* enter mpsse mode */
	ftdi_set_bitmode(&ftdic, 0xb, 2);
	if (ftdi_set_latency_timer(&ftdic, 1) < 0)
		panic("");

	printf("ok, usb stm32 performance stick initialized\n");
}
#else
#error "no jtag target interface defined"

#endif


static enum JTAG_TAP_STATE_ENUM jtag_state;

static int nr_append_data_bits;
static int nr_prepend_data_bits;
static int nr_append_ir_bits;
static int nr_prepend_ir_bits;

static void jtagdrv_reg_cmd(unsigned char * buf, int nr_bits, int move_to_rt_idle, int jtag_reg)
{
unsigned char din, dout;
int i, j;
int nr_append_bits, nr_prepend_bits;

	if (!nr_bits)
		panic("bad jtag command");

	switch (jtag_state)
	{
		case RUN_TEST_IDLE:
			/* move to the SELECT_DR_SCAN state */
			SET_TMS();
			PULSE_TCK();
			break;
		case SELECT_DR_SCAN:
			/* already in SELECT_DR_SCAN, do nothing */
			break;
		default:
			panic("bad jtag tap controller state");
	}

	if (jtag_reg)
	{
		/* move to SELECT_IR_SCAN */
		SET_TMS();
		PULSE_TCK();
	}

	/* move to CAPTURE_xR */
	CLR_TMS();
	PULSE_TCK();
	/* move to the SHIFT_xR */
	CLR_TMS();
	SET_TCK();

	/* compute jtag chain append and prepend bit counts */
	if (jtag_reg)
	{
		/* instruction register accessed */
		nr_append_bits = nr_append_ir_bits;
		nr_prepend_bits = nr_prepend_ir_bits;
	}
	else
	{
		/* data register accessed */
		nr_append_bits = nr_append_data_bits;
		nr_prepend_bits = nr_prepend_data_bits;
	}
	/* prepare to shift data in - set tdi to one in case
	 * jtag chain append bits are needed */
	if (nr_append_bits)
		SET_TDI();
	while (nr_append_bits--)
	{
		CLR_TCK();
		SET_TCK();
	}
	/* shift data in */
	for (i = 0; i < nr_bits; i++)
	{
		if (!(i & 7))
			din = *buf;
		if (i == nr_bits - 1 && !nr_prepend_bits)
			/* last bit - move to EXIT1_xR */
			SET_TMS();

		if (din & 1)
			SET_TDI();
		else
			CLR_TDI();

		CLR_TCK();
		SET_TCK();

		din >>= 1;
		dout >>= 1;

		if (READ_TDO())
			dout |= 0x80;

		if ((i & 7) == 7)
			/* skip to next byte */
			*buf++ = dout;
	}
	/* make up for an incomplete last byte */
	if (i & 7)
		*buf = dout >> (8 - i);

	/* prepare to shift data in - set tdi to one in case
	 * jtag chain append bits are needed */
	if (nr_prepend_bits)
		SET_TDI();
	while (nr_prepend_bits--)
	{
		if (!nr_prepend_bits)
			/* last bit - move to EXIT1_xR */
			SET_TMS();
		CLR_TCK();
		SET_TCK();
	}

	CLR_TCK();
	/* move to UPDATE_xR */
	PULSE_TCK();
	/* final step - move to SELECT_DR_SCAN, or RUN_TEST_IDLE,
	 * if move_to_rt_idle is nonzero */
	if (move_to_rt_idle)
	{
		CLR_TMS();
		jtag_state = RUN_TEST_IDLE;
	}
	else
		jtag_state = SELECT_DR_SCAN;
	PULSE_TCK();

}

void jtagdrv_ireg_cmd(unsigned char * buf, int nr_bits, int move_to_rt_idle)
{
	jtagdrv_reg_cmd(buf, nr_bits, move_to_rt_idle, 1);
}

void jtagdrv_dreg_cmd(unsigned char * buf, int nr_bits)
{
	jtagdrv_reg_cmd(buf, nr_bits, 1, 0);
}

void init_jtagdrv(int jtag_ir_len, int jtag_append_data_bits_cnt,
		int jtag_prepend_data_bits_cnt, int jtag_append_ir_bits_cnt,
		int jtag_prepend_ir_bits_cnt)
{
int i;
unsigned int buf[2];

#if defined JTAG_INTERFACE_WIGGLER
	init_jtag_driver_wiggler();
#elif defined JTAG_INTERFACE_FTDI_STM32_PERFORMANCE_STICK
	init_jtag_driver_stm32_stick();
#else
#error no jtag interface defined
#endif

	nr_append_data_bits = jtag_append_data_bits_cnt;
	nr_prepend_data_bits = jtag_prepend_data_bits_cnt;
	nr_append_ir_bits = jtag_append_ir_bits_cnt;
	nr_prepend_ir_bits = jtag_prepend_ir_bits_cnt;


	SET_SRST();
	usleep(100000);
	CLR_SRST();
	usleep(100000);
	
	/* reset the jtag tap controller state machine */
	CLR_TCK();
	SET_TMS();
	for (i = 0; i < 5; i++)
	{
		PULSE_TCK();
	}

	jtag_state = TEST_LOGIC_RESET;
	/* move to run test/idle */
	CLR_TMS();
	PULSE_TCK();
	jtag_state = RUN_TEST_IDLE;

	printf("attempting idcode insn execution...\n");
	*buf = 0xe; /* load idcode public insn */

	jtagdrv_ireg_cmd((unsigned char *)buf, 4, 0);
	jtagdrv_dreg_cmd((unsigned char *)buf, 32);
	printf("idcode: 0x%08x\n", *buf);
	return;

	printf("programming breakpoint...\n");


	printf("attempting to breakpoint core...\n");
	/* scann */
	*buf = 2;
	jtagdrv_ireg_cmd((unsigned char *)buf, 4, 0);
	*buf = 2;
	jtagdrv_dreg_cmd((unsigned char *)buf, 4);
	/* intest */
	*buf = 12;
	jtagdrv_ireg_cmd((unsigned char *)buf, 4, 0);


#if 0
	/* program breakpoint */
	/* program wp0 data mask */
	*buf = -1;
	buf[1] = 0x20 | 11;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* program wp0 address mask */
	*buf = 0;
	buf[1] = 0x20 | 9;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* program wp0 address value */
	*buf = 0x288;
	buf[1] = 0x20 | 8;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* program wp0 control mask */
	*buf = 0xf0;
	buf[1] = 0x20 | 13;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* program wp0 control value */
	*buf = 0x104;
	buf[1] = 0x20 | 12;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
#else
	/* program watchpoint */
	/* program wp0 data mask */
	*buf = -1;
	buf[1] = 0x20 | 11;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* program wp0 address mask */
	*buf = 0;
	buf[1] = 0x20 | 9;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* program wp0 address value */
	*buf = 0xe0028004;
	buf[1] = 0x20 | 8;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* program wp0 control mask */
	*buf = 0xf0;
	buf[1] = 0x20 | 13;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* program wp0 control value */
	*buf = 0x10d;
	buf[1] = 0x20 | 12;
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
#endif

	printf("done programming embedded ice registers, will now wait for a breakpoint...\n");
	/* read dbgstat */
	*buf = 0;
	buf[1] = 0x0 | 1;
	/* read command */
	jtagdrv_dreg_cmd((unsigned char *)buf, 38);
	/* actual read */
	for (i = 0; i < 1000; i++)
	//while (1)
	{
		*buf = 0;
		buf[1] = 0x0 | 1;
		jtagdrv_dreg_cmd((unsigned char *)buf, 38);
		if ((buf[0] & 9) == 9)
			break;
	}
	if (i == 1000)
		panic("bkpt not hit\n");
	printf("ok, check the target\n");
	/* dump bit 0 of sc1 - breakpt */
	/* scann */
	*buf = 2;
	jtagdrv_ireg_cmd((unsigned char *)buf, 4, 0);
	*buf = 1;
	jtagdrv_dreg_cmd((unsigned char *)buf, 4);
	/* intest */
	*buf = 12;
	jtagdrv_ireg_cmd((unsigned char *)buf, 4, 0);

	/* read sc1 */
	jtagdrv_dreg_cmd((unsigned char *)buf, 33);
	printf("BREAKPT:\t%s\n", (*buf & 1) ? "SET" : "CLEAR");

	panic("check idcode");
	printf("jtagdrv initialized\n");
}

