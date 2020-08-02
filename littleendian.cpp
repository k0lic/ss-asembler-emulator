#include "littleendian.h"

unsigned char LittleEndian::intToLittleEndianChar(int x)
{
	// don't think there is a need for anything else
	return x & 255;
}

void LittleEndian::intToLittleEndianChar(int x, int size, unsigned char *bytes)
{
	for (int i = 0; i < size; i++)
	{
		bytes[i] = x & 255;
		x >>= 8;
	}
}