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

int LittleEndian::charLittleEndianToInt(int size, unsigned char *bytes)
{
	int x = 0;
	unsigned int shift = 0;

	for (int i = 0; i < size; i++)
	{
		x |= bytes[i] << shift;
		shift += 8;
	}

	// if sign bit is 1 - fill the rest of int with 1s
	if (bytes[size - 1] & 128)
		while (shift < 8 * sizeof(unsigned int))
		{
			x |= 255 << shift;
			shift += 8;
		}

	return x;
}

unsigned int LittleEndian::charLittleEndianToUInt(int size, unsigned char *bytes)
{
	unsigned int x = 0;
	unsigned int shift = 0;

	for (int i = 0; i < size; i++)
	{
		x |= bytes[i] << shift;
		shift += 8;
	}

	return x;
}