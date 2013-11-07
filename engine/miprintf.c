#include <stdarg.h>
#include <stdlib.h>
#include "miprintf.h"
#include "util.h"


static char mibuf[4 * 1024];
static int mi_idx;

//extern ssize_t write_to_frontends(const void *buf, size_t count);
extern int write_to_frontends(const void *buf, size_t count);


/*! \todo	remove these, debug only */
static bool is_print_enabled = true;
void miprintf_switch_debug(bool is_debug_print_enabled)
{
	is_print_enabled = is_debug_print_enabled;
}

int miprintf(const char * format, ...)
{
va_list ap;
int res;
int i;
//char buf[1024];
	
	va_start(ap, format);
	res = vsnprintf(mibuf + mi_idx, sizeof mibuf - mi_idx, format, ap);
	/* see if the buffer should be flushed */
	for (i = mi_idx; i < mi_idx + res; i++)
		if (mibuf[i] == '\n')
		{
			i = -1;
			break;
		}
	/*! \todo	remove this, debugging only */	 
	if (is_print_enabled)
		printf("%s", mibuf + mi_idx);
	mi_idx += res;
	if (i == -1 || mi_idx >= ((sizeof mibuf * 3) >> 2))
	{
		if (write_to_frontends(mibuf, mi_idx) == -1)
			panic("");
		mi_idx = 0;
	}	
	va_end(ap);
	return res;
}

int vmiprintf(const char * format, va_list ap)
{
int res;
char buf[1024];

	res = vsnprintf(buf, sizeof buf, format, ap);
	if (res >= sizeof buf)
		panic("");
	if (write_to_frontends(buf, res) == -1)
		panic("");
	/*! \todo	remove this, debugging only */	 
	printf("%s", buf);
	return res;
}

