
#include <stdio.h>

#include "engine-err.h"
#include "typedefs.h"
#include "util.h"

#define ADIV5_JTAG_IR_LEN	4
#define ADIV5_JTAG_IDCODE_LEN	32
#define ADIV5_JTAG_AP_LEN	35
#define ADIV5_JTAG_DP_LEN	35
#define ADIV5_JTAG_SELECT_LEN	32

/* current scanchain selected - this is actually the value
 * loaded in the jtag tap controller instruction register (ir) */
static enum ADIV5_JTAG_SCANCHAIN_ENUM
{
	ADIV5_JTAG_SCANCHAIN_INAVLID = 0,
	ADIV5_JTAG_SCANCHAIN_ABORT = 0x8,
	ADIV5_JTAG_SCANCHAIN_DPACC = 0xa,
	ADIV5_JTAG_SCANCHAIN_APACC = 0xb,
	ADIV5_JTAG_SCANCHAIN_IDCODE = 0xe,
	ADIV5_JTAG_SCANCHAIN_BYPASS = 0xf,
}
cur_sc;
/* current access port register bank selected in the APBANKSEL
 * field in the AP SELECT debug port register */
static int cur_ap_bank;

/* current access port selected in the APSEL
 * field in the AP SELECT debug port register */
static int cur_ap;


static void adiv5_connect_scanchain(enum ADIV5_JTAG_SCANCHAIN_ENUM sc)
{
int ir;	

	if (cur_sc == sc)
		return;
	cur_sc = ir = sc;
	jtagdrv_ireg_cmd(&ir, ADIV5_JTAG_IR_LEN);
}

static enum GEAR_ENGINE_ERR_ENUM adiv5_ap_bank_select(int ap_bank_nr)
{
ARM_CORE_WORD t;
char buf[5];
	
	if (cur_ap_bank == ap_bank_nr)
		return GEAR_ERR_NO_ERROR;
	cur_ap_bank = ap_bank_nr & 0xff;
	t = ((cur_ap & 0xff) << 24) | ((cur_ap_bank & 0xf) << 4);
	*(int *) buf = (t << 3) | (2 << 1);
	buf[4] = t >> 29;
	/* write the apselect register in the debug port */
	adiv5_connect_scanchain(ADIV5_JTAG_SCANCHAIN_DPACC);
	jtagdrv_dreg_cmd(buf, ADIV5_JTAG_DP_LEN);
	/* check the ack field of the response */
	switch (buf[0] & 7)
	{
		case 1:	/* wait */
			panic("");
			break;
		case 2: /* ok/fault */
			break;
		default:
			panic("");
	}
	return GEAR_ERR_NO_ERROR;
}

enum GEAR_ENGINE_ERR_ENUM adiv5_read_ctrl_stat(ARM_CORE_WORD * ctrl_stat)
{
ARM_CORE_WORD t;
int prev_scanchain;
char buf[5];

	prev_scanchain = cur_sc;
	/* read the debug ctrl/stat register */
	adiv5_connect_scanchain(ADIV5_JTAG_SCANCHAIN_DPACC);
	*buf = (1 << 1) | 1;
	jtagdrv_dreg_cmd(buf, ADIV5_JTAG_DP_LEN);

	/* validate the ack field of the response */
	switch (buf[0] & 7)
	{
		case 1:	/* wait */
			panic("");
			break;
		case 2: /* ok/fault */
			break;
		default:
			panic("");
	}
	/* issue a dummy rdbuff register read so that the value
	 * of the ctrl/stat register requested above appears for
         * inspection */	 
	*buf = (3 << 1) | 1;
	jtagdrv_dreg_cmd(buf, ADIV5_JTAG_DP_LEN);
	/* here, in buf is the result of the previous read operation, i.e.
	 * the ctrl/stat register value */

	/* but first - again, validate the ack field of the response */
	switch (buf[0] & 7)
	{
		case 1:	/* wait */
			panic("");
			break;
		case 2: /* ok/fault */
			break;
		default:
			panic("");
	}
	t = *(ARM_CORE_WORD *) buf;
	t >>= 3;
	t |= buf[4] << 29;
	/* ok, here t holds the ctrl/stat register value - examine
	 * error flags (namely, sticky error and sticky overrun flags) */

	*ctrl_stat = t;
	printf("%s(): ctrl/stat = 0x%08x\n", __func__, t);
	if (t & BIT5)
	{
		t = 0;
	}
	adiv5_connect_scanchain(prev_scanchain);
	return GEAR_ERR_NO_ERROR;

}

enum GEAR_ENGINE_ERR_ENUM adiv5_get_idcode(ARM_CORE_WORD * idcode)
{
	adiv5_connect_scanchain(ADIV5_JTAG_SCANCHAIN_IDCODE);
	jtagdrv_dreg_cmd(idcode, ADIV5_JTAG_IDCODE_LEN);
	return GEAR_ERR_NO_ERROR;
}

enum GEAR_ENGINE_ERR_ENUM adiv5_read_mem_ap(ARM_CORE_WORD * data_out, int reg_addr)
{
char buf[5];
ARM_CORE_WORD t;
ARM_CORE_WORD t1;

	if (reg_addr & 3)
		panic("");
	reg_addr >>= 2;
	if (adiv5_ap_bank_select(reg_addr >> 2) != GEAR_ERR_NO_ERROR)
		panic("");
	*(int *) buf = ((reg_addr & 3) << 1) | 1;
	adiv5_connect_scanchain(ADIV5_JTAG_SCANCHAIN_APACC);
	jtagdrv_dreg_cmd(buf, ADIV5_JTAG_AP_LEN);
	/* check the ack field of the response */
	switch (buf[0] & 7)
	{
		case 1:	/* wait */
			panic("");
			break;
		case 2: /* ok/fault */
			break;
		default:
			panic("");
	}
	/* read the debug ctrl/stat register to determine if everything
	 * is ok */
	adiv5_connect_scanchain(ADIV5_JTAG_SCANCHAIN_DPACC);
	*buf = (1 << 1) | 1;
	jtagdrv_dreg_cmd(buf, ADIV5_JTAG_DP_LEN);
	/* here, in buf is the result of the previous read operation, i.e.
	 * the mem ap read result - store it back for the caller */

	/* but first - again, validate the ack field of the response */
	switch (buf[0] & 7)
	{
		case 1:	/* wait */
			panic("");
			break;
		case 2: /* ok/fault */
			break;
		default:
			panic("");
	}
	t = *(ARM_CORE_WORD *) buf;
	t >>= 3;
	t |= buf[4] << 29;
	*data_out = t;
	printf("%s(), %i : read 0x%08x\n", __func__, __LINE__, t);
	/* issue a dummy rdbuff register read so that the value
	 * of the ctrl/stat register requested above appears for
         * inspection */	 
	*buf = (3 << 1) | 1;
	jtagdrv_dreg_cmd(buf, ADIV5_JTAG_DP_LEN);
	/* here, in buf is the result of the previous read operation, i.e.
	 * the ctrl/stat register value */

	/* but first - again, validate the ack field of the response */
	switch (buf[0] & 7)
	{
		case 1:	/* wait */
			panic("");
			break;
		case 2: /* ok/fault */
			break;
		default:
			panic("");
	}
	t = *(ARM_CORE_WORD *) buf;
	t >>= 3;
	t |= buf[4] << 29;
	/* ok, here t holds the ctrl/stat register value - examine
	 * error flags (namely, sticky error and sticky overrun flags) */
	if (t & ((1 << 5) | (1 << 1)))
	{
		printf("ctrl/stat = 0x%08x\n", t);
		panic("");
	}

	return GEAR_ERR_NO_ERROR;
}

enum GEAR_ENGINE_ERR_ENUM adiv5_write_mem_ap(ARM_CORE_WORD data_in, int reg_addr)
{
char buf[5];

	if (reg_addr & 3)
		panic("");
	reg_addr >>= 2;
	if (adiv5_ap_bank_select(reg_addr >> 2) != GEAR_ERR_NO_ERROR)
		panic("");
	*(int *) buf = (data_in << 3) | ((reg_addr & 3) << 1);
	buf[4] = data_in >> 29;
	adiv5_connect_scanchain(ADIV5_JTAG_SCANCHAIN_APACC);
	jtagdrv_dreg_cmd(buf, ADIV5_JTAG_AP_LEN);
	/* check the ack field of the response */
	switch (buf[0] & 7)
	{
		case 1:	/* wait */
			panic("");
			break;
		case 2: /* ok/fault */
			break;
		default:
			panic("");
	}
	return GEAR_ERR_NO_ERROR;
}

enum GEAR_ENGINE_ERR_ENUM adiv5_ap_select(int ap_nr)
{
	
	cur_ap = ap_nr & 0xff;
	/* put an invalid value in the cur_ap_bank variable
	 * so that adiv5_ap_bank_select() always executes
         * when called */
	cur_ap_bank = -1;
	return GEAR_ERR_NO_ERROR;
}

enum GEAR_ENGINE_ERR_ENUM init_adiv5(void)
{
char buf[5];
ARM_CORE_WORD t;

	cur_ap = 0;
	/* put an invalid value in the cur_ap_bank variable
	 * so that adiv5_ap_bank_select() always executes
         * when called */
	cur_ap_bank = -1;
	cur_sc = ADIV5_JTAG_SCANCHAIN_INAVLID;
	init_jtagdrv(4, 0, 1 + 1, 0, 4 + 5);
	/* clear the stickyerr bit in the ctrl/stat register */
	adiv5_connect_scanchain(ADIV5_JTAG_SCANCHAIN_DPACC);
	t = BIT5 | BIT28 | BIT30;
	*(int *) buf = (t << 3) | (1 << 1);
	buf[4] = t >> 29;
	jtagdrv_dreg_cmd(buf, ADIV5_JTAG_DP_LEN);
	/* check the ack field of the response */
	switch (buf[0] & 7)
	{
		case 1:	/* wait */
			panic("");
			break;
		case 2: /* ok/fault */
			break;
		default:
			panic("");
	}
	return GEAR_ERR_NO_ERROR;
}

