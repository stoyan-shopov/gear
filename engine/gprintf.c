#include <stdarg.h>
#include <stdbool.h>

#include "util.h"

/*! \todo	remove these, debug only */
static bool is_print_enabled = true;
void gprintf_switch_on(bool on)
{
	is_print_enabled = on;
}

int gprintf(const char * format, ...)
{
va_list ap;
int res;

	/*! \todo	remove this, debugging only */	 
	if (!is_print_enabled)
		return 0;
	va_start(ap, format);
	res = vprintf(format, ap);
	va_end(ap);
	fflush(stdout);
	return res;
}

int vgprintf(const char * format, va_list ap)
{
int res;	

	/*! \todo	remove this, debugging only */	 
	if (!is_print_enabled)
		return 0;
	res = vprintf(format, ap);
	fflush(stdout);
	return res;
}
