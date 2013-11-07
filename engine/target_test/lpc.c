

#include "lpc21xx.h"

void main(void)
{

	PINSEL2 &=~ (1 << 3);
	IODIR1 |= 0xff << 16;

	while (1)
	{
		int i;
		IO1SET = 0xaa << 16;
		IO1CLR = 0x55 << 16;
		for (i = 0x10000 << 4; i--; );
		IO1SET = 0x55 << 16;
		IO1CLR = 0xaa << 16;
		for (i = 0x10000 << 4; i--; );
	}
}

