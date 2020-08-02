#pragma once

class LittleEndian
{
public:
	static unsigned char intToLittleEndianChar(int x);
	static void intToLittleEndianChar(int x, int size, unsigned char *bytes);
};