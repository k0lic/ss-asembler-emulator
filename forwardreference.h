#pragma once

using namespace std;

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