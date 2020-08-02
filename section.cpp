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

void Section::addByte(unsigned char byte)
{
	code.push_back(byte);
}

void Section::addInstruction(int instructionLength, unsigned char *bytes)
{
	for (int i=0;i<instructionLength;i++)
		code.push_back(bytes[i]);
}

int Section::size()
{
	return code.size();
}

ostream& operator<<(ostream& out, const Section& s)
{
	// setup for hexadecimal format
	out << hex;

	for (unsigned int i = 0; i < s.code.size(); i++)
	{
		out << setw(2) << setfill('0') << +s.code[i];

		if (i % 16 == 15 || i + 1 == s.code.size())
			out << endl;								// new line every 16 bytes
		else if (i % 4 == 3)
			out << "\t\t";								// double tab every 4 bytes
		else
			out << " ";									// space after every byte
	}

	// return to decimal format
	out << dec;

	return out;
}