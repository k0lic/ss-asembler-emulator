#pragma once

#include <vector>

using namespace std;

class Section
{
public:
	void addByte(char byte);
	void addInstruction(int instructionLength, char *bytes);
	int size();
private:
	vector<char> code;
};