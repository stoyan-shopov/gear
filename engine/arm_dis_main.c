#include <stdarg.h>

#include "../../binutils-2.17-include/dis-asm.h"
#include "armdefs.h"
#include "dbg_rdi.h"
#include "core-access.h"
#include "arm-dis.h"

#define ROT(_x)	((_x >> 24) & 0xff) | ((_x >> 8) & 0xff00) | ((_x << 8) & 0xff0000) | ((_x << 24) & 0xff000000)

static struct disassemble_info dis_info;
static unsigned char arm_dis_buf[4];

int main_(void)
{
struct disassemble_info info;
int buf[1024] =
{
 ROT(0x18f09fe5), ROT(0x08f05ee2), ROT(0x18f09fe5), ROT(0x18f09fe5),
 ROT(0x08f05ee2), ROT(0xf826f20b), ROT(0x14f09fe5), ROT(0x14f09fe5),
};
int i;

	init_disassemble_info(&info, stdout, fprintf);
	info.buffer = buf;
	info.buffer_vma = 0;
	info.buffer_length = sizeof(buf);
	//memset(buf, 0, sizeof(buf));
	for (i = 0; i < 8; i++)
	{
		print_insn_little_arm(i * 4, &info);
		printf("\n");
	}
	return 0;
}

int (*printf_func)(const char * format, va_list ap);
int my_disasm_fprintf(void * unused, const char * format, ...)
{
va_list	ap;
int res;

	va_start(ap, format);
	res = printf_func(format, ap);
	va_end(ap);
	return res;
}

/*! \todo	add mode selection (arm/thumb) */
void arm_disassemble_insn(struct core_control * cc, ARMword addr, int (*print_fn)(const char * format, va_list ap))
{
unsigned int nbytes;

	nbytes = 4;
	printf_func = print_fn;
	dis_info.buffer_vma = addr;
	cc->core_mem_read(arm_dis_buf, addr, &nbytes);
	print_insn_little_arm(addr, &dis_info);
}

void init_arm_disassemble(void)
{
	init_disassemble_info(&dis_info, stdout, my_disasm_fprintf);
	dis_info.buffer = arm_dis_buf;
	dis_info.buffer_vma = 0;
	dis_info.buffer_length = sizeof(arm_dis_buf);
}
