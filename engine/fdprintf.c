
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef __LINUX__

#else
#include <winsock2.h>
#endif

#include "util.h"

int fdprintf(int fd, const char * format, ...)
{
va_list ap;
char buf[256];
int i;

	va_start(ap, format);
	i = vsnprintf(buf, sizeof buf, format, ap);
	if (write(fd, buf, i) != i)
		panic("");
	va_end(ap);
	return i;
}

int sockprintf(int sock_fd, const char * format, ...)
{
va_list ap;
char buf[256];
int i;

	va_start(ap, format);
	i = vsnprintf(buf, sizeof buf, format, ap);
	if (send(sock_fd, buf, i, 0) != i)
		panic("");
	va_end(ap);
	return i;
}

