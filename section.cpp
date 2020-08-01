#include "section.h"

using namespace std;

/* CONSTRUCTORS */

Section::Section() {}

Section::Section(const Section& other)
{
	code.reserve(other.code.capacity());

	for (unsigned int i=0;i<other.code.size();i++)
		code.push_back(other.code[i]);
}

/* DESTRUCTOR */

Section::~Section()
{
	code.clear();
}

/* PUBLIC METHODS */

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