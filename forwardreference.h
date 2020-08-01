#pragma once

using namespace std;

// TODO: this class is not needed, DELETE it
// everything that would be stored as a forward reference can be stored as a relocation record - forward references are pointless
// will need to backup the BPAction enum since it is used outside this class

enum BPAction { PLUS, MINUS };

class ForwardReference
{
public:
	ForwardReference(int sectionNum, int address, BPAction action);
	int getSectionNumber();
	int getAddress();
	BPAction getAction();
private:
	int sectionNum;
	int address;
	BPAction action;
};