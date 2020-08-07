#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

class Section
{
public:
	Section();
	Section(const Section& other);									// copy constructor
	~Section();
	void addByte(unsigned char byte);
	void addInstruction(int instructionLength, unsigned char *bytes);
	void readBytes(int startAt, int numOfBytes, unsigned char *bytes);
	void overwriteBytes(int startAt, int numOfBytes, unsigned char *bytes);
	int size();
	void write(ofstream& out);
	void read(ifstream& in);
	friend ostream& operator<<(ostream& out, const Section& s);
private:
	vector<unsigned char> code;
};