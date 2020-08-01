#include "section.h"

using namespace std;

void Section::addByte(char byte)
{
	code.push_back(byte);
}

void Section::addInstruction(int instructionLength, char *bytes)
{
	for (int i=0;i<instructionLength;i++)
		code.push_back(bytes[i]);
}

int Section::size()
{
	return code.size();
}