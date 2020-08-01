#include "globalsymbol.h"

using namespace std;

GlobalSymbol::GlobalSymbol(string name, GlobalType type)
{
	this->name = name;
	this->type = type;
}

string GlobalSymbol::getName()
{
	return name;
}

GlobalType GlobalSymbol::getType()
{
	return type;
}