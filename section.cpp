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

void Section::readBytes(int startAt, int numOfBytes, unsigned char *bytes)
{
	for (int i = 0; i < numOfBytes; i++)
		bytes[i] = code[startAt + i];
}

void Section::overwriteBytes(int startAt, int numOfBytes, unsigned char *bytes)
{
	for (int i = 0; i < numOfBytes; i++)
		code[startAt + i] = bytes[i];	
}

int Section::size()
{
	return code.size();
}

/* INPUT AND OUTPUT */

void Section::write(ofstream& out)
{
	// write code length
	unsigned int codeSize = code.size();
	out.write((char*)&codeSize, sizeof(codeSize));

	// write all of the bytes
	out.write((char*)&code[0], codeSize * sizeof(code[0]));
}

void Section::read(ifstream& in)
{
	// read code length
	unsigned int codeSize;
	in.read((char*)&codeSize, sizeof(codeSize));

	// read all of the bytes
	unsigned char bytes[codeSize];
	in.read((char*)bytes, codeSize * sizeof(unsigned char));

	// initialize the vector
	code.assign(bytes, bytes + codeSize);
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