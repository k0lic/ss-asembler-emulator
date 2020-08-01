#include "forwardreference.h"

using namespace std;

/* CONSTRUCTOR */

ForwardReference::ForwardReference(int sectionNum, int address, BPAction action)
{
	this->sectionNum = sectionNum;
	this->address = address;
	this->action =action;
}

/* GETTERS */

int ForwardReference::getSectionNumber()
{
	return sectionNum;
}

int ForwardReference::getAddress()
{
	return address;
}

BPAction ForwardReference::getAction()
{
	return action;
}