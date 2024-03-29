#pragma once

class LittleEndian
{
public:
	static unsigned char intToLittleEndianChar(int x);
	static void intToLittleEndianChar(int x, int size, unsigned char *bytes);
	static int charLittleEndianToInt(int size, unsigned char *bytes);
	static unsigned int charLittleEndianToUInt(int size, unsigned char *bytes);
};