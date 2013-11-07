/* target specific code goes her */

#include <stdio.h>
#include <string.h>

#include "typedefs.h"
#include "util.h"
#include "engine-err.h"
#include "target.h"
#include "constants.h"

enum CORTEX_M3_REG_ADDR_ENUM
{
	/*! control/status word register */
	CORTEX_M3_REG_ADDR_CSW = 0x00,
	/*! transfer address register */
	CORTEX_M3_REG_ADDR_TAR = 0x04,
	/*! data read/write register */
	CORTEX_M3_REG_ADDR_DRW = 0x0c,

};
/* core debug registers addresses */
#define CORE_DBG_REG_DHCSR			0xe000edf0
#define CORE_DBG_REG_SELECTOR_REG		0xe000edf4
#define CORE_DBG_REG_DATA_REG			0xe000edf8
#define CORE_DBG_EXCEPTION_MONITOR_CTRL_REG	0xe000edfc

static ARM_CORE_WORD read_cortex_mem(ARM_CORE_WORD addr)
{
ARM_CORE_WORD data;
ARM_CORE_WORD t;

	if (adiv5_write_mem_ap(addr, CORTEX_M3_REG_ADDR_TAR)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (adiv5_read_ctrl_stat(&t) != GEAR_ERR_NO_ERROR)
		panic("");
	if (adiv5_read_mem_ap(&data, CORTEX_M3_REG_ADDR_DRW)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (adiv5_read_ctrl_stat(&t) != GEAR_ERR_NO_ERROR)
		panic("");
	return data;

}

static void write_cortex_mem(ARM_CORE_WORD addr, ARM_CORE_WORD data)
{
ARM_CORE_WORD t;

	if (adiv5_write_mem_ap(addr, CORTEX_M3_REG_ADDR_TAR)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (adiv5_read_ctrl_stat(&t) != GEAR_ERR_NO_ERROR)
		panic("");
	if (adiv5_write_mem_ap(data, CORTEX_M3_REG_ADDR_DRW)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (adiv5_read_ctrl_stat(&t) != GEAR_ERR_NO_ERROR)
		panic("");
}

static enum GEAR_ENGINE_ERR_ENUM core_reg_read(unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[]);
static enum GEAR_ENGINE_ERR_ENUM core_open(void)
{
ARM_CORE_WORD t;

	init_adiv5();
	if (adiv5_read_ctrl_stat(&t) != GEAR_ERR_NO_ERROR)
		panic("");
	/*! \todo	validate the idcode here */

	/* select the memory access port (mem-ap) */
	adiv5_ap_select(0);
	if (adiv5_read_ctrl_stat(&t) != GEAR_ERR_NO_ERROR)
		panic("");

	if (adiv5_read_mem_ap(&t, CORTEX_M3_REG_ADDR_CSW)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	printf("ahb-ap csw = 0x%08x\n", t);
	/* program the ap control/status word (csw) register */
	/* word access, no increment */
	if (adiv5_write_mem_ap(BIT29 | BIT25 | BIT6 | (2 << 0), CORTEX_M3_REG_ADDR_CSW)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (adiv5_read_ctrl_stat(&t) != GEAR_ERR_NO_ERROR)
		panic("");
	/* inspect the core status */
	t = read_cortex_mem(CORE_DBG_REG_DHCSR);
	printf("dhcsr = 0x%08x\n", t);
	if (t & BIT17)
		panic("core already halted");
	/* attempt to halt the core */
	write_cortex_mem(CORE_DBG_REG_DHCSR, 0xa05f0000 | BIT0 | BIT1 | BIT5);
	t = read_cortex_mem(CORE_DBG_REG_DHCSR);
	if (!(t & BIT17))
		panic("failed to halt core");
	printf("ok, core successfully halted\n");
	{
		ARM_CORE_WORD regs[16];
		int i;
		core_reg_read(0, (1 << 16) - 1, regs);
		for (i = 0; i < 16; i++)
			printf("reg%i\t 0x%08x\n", i, regs[i]);

	}
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_close(void)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_read(void *dest, ARM_CORE_WORD source, unsigned *nbytes)
{
	if (target_is_core_running())
		panic("");
	memset(dest, 0, *nbytes);
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_mem_write(ARM_CORE_WORD dest, const void *source, unsigned *nbytes)
{
	if (target_is_core_running())
		panic("");
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_read(unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i, j;

	if (target_is_core_running())
		panic("");
	if (!mask)
		panic("");
	/* wait for any register transfers to complete */
	/*! \todo	add timeouts here */
	while (!(read_cortex_mem(CORE_DBG_REG_DHCSR)
				& BIT16))
		;
	for (i = j = 0; i < 16 && mask; i++, mask >>= 1)
		if (mask & 1)
		{
			write_cortex_mem(CORE_DBG_REG_SELECTOR_REG, i);
			/* wait for register transfer to complete */
			/*! \todo	add timeouts here */
			while (!(read_cortex_mem(CORE_DBG_REG_DHCSR)
						& BIT16))
				;
			buffer[j++] = read_cortex_mem(CORE_DBG_REG_DATA_REG);
		}
	return GEAR_ERR_NO_ERROR;
}
/*! \todo	handle mode here */
static enum GEAR_ENGINE_ERR_ENUM core_reg_write(unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i, j;

	if (target_is_core_running())
		panic("");
	if (!mask)
		panic("");
	/* wait for any register transfers to complete */
	/*! \todo	add timeouts here */
	while (!(read_cortex_mem(CORE_DBG_REG_DHCSR)
				& BIT16))
		;
	for (i = j = 0; i < 16 && mask; i++, mask >>= 1)
		if (mask & 1)
		{
			write_cortex_mem(CORE_DBG_REG_DATA_REG, buffer[j++]);
			write_cortex_mem(CORE_DBG_REG_SELECTOR_REG, i | BIT16);
			/* wait for register transfer to complete */
			/*! \todo	add timeouts here */
			while (!(read_cortex_mem(CORE_DBG_REG_DHCSR)
						& BIT16))
				;
		}
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM core_cop_read(unsigned CPnum, unsigned long mask, ARM_CORE_WORD buffer[])
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_cop_write(unsigned CPnum, unsigned long mask, ARM_CORE_WORD buffer[])
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_set_break(ARM_CORE_WORD address, unsigned long * handle)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_clear_break(unsigned long handle)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_step(void)
{
	panic("");
}
static enum GEAR_ENGINE_ERR_ENUM core_run(void/*ARM_CORE_WORD * halt_addr*/)
{
	panic("");
}

static enum GEAR_ENGINE_ERR_ENUM core_get_status(enum TARGET_CORE_STATE_ENUM * status)
{
	* status = TARGET_CORE_STATE_HALTED;
	return GEAR_ERR_NO_ERROR;
}

static struct core_access_struct core_funcs =
{
	/* core_open */
	core_open,
	/* core_close */
	core_close,
	/* core_mem_read */
	core_mem_read,
	/* core_mem_write */
	core_mem_write,
	/* core_reg_read */
	core_reg_read,
	/* core_reg_write */
	core_reg_write,
	/* core_cop_read */
	core_cop_read,
	/* core_cop_write */
	core_cop_write,
	/* core_set_break */
	core_set_break,
	/* core_step */
	core_step,
	/* core_run */
	core_run,
	/* core_get_status */
	core_get_status,
};

int target_is_core_running(void)
{
	return 0;
}

enum GEAR_ENGINE_ERR_ENUM target_get_core_access(struct core_access_struct * cc)
{
	* cc = core_funcs;
	return GEAR_ERR_NO_ERROR;
}

