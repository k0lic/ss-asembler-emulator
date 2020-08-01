#pragma once

#include <vector>

using namespace std;

class Section
{
public:
	Section();
	Section(const Section& other);									// copy constructor
	~Section();
	void addByte(char byte);
	void addInstruction(int instructionLength, char *bytes);
	int size();
private:
	vector<char> code;
};