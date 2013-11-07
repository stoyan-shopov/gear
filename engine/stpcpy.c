#include <string.h>

char * stpcpy(char * dest, char * src)
{
	return strcpy(dest, src) + strlen(src);
}

