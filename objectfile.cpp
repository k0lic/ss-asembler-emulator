#include "objectfile.h"

using namespace std;

ObjectFile::ObjectFile(string name)
{
	this->name = name;
}

string ObjectFile::getName()
{
	return name;
}

SymbolTable& ObjectFile::getSymbolTable()
{
	return symbolTable;
}

vector<SymbolAddendum>& ObjectFile::getAddendums()
{
	return addendums;
}