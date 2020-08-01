#include "incalculable.h"

using namespace std;

Incalculable::Incalculable(string name, BPAction action)
{
	this->name = name;
	this->action = action;
}

string Incalculable::getName()
{
	return name;
}

BPAction Incalculable::getAction()
{
	return action;
}