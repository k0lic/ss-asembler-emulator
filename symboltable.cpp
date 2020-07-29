#include "symboltable.h"

using namespace std;

/* CONSTRUCTOR */

SymbolTable::SymbolTable() {}

/* PUBLIC METHODS */

int SymbolTable::getSymbolIndex(string name)
{
	for (int i = 0; i < symbols.size(); i++)
	{
		if (symbols[i].getName() == name)
			return i + 1;
	}

	symbols.emplace_back(name, symbols.size() + 1);

	return symbols.size();
}

bool SymbolTable::setSymbolValue(string name, int section, int value)
{
	for (int i = 0; i < symbols.size(); i++)
	{
		if (symbols[i].getName() == name)
		{
			if (symbols[i].isDefined())
				return false;
			symbols[i].setSection(section);
			symbols[i].setValue(value);
			symbols[i].setDefined(true);
			return true;
		}
	}

	symbols.emplace_back(name, symbols.size() + 1, section, value);
	
	return true;
}

void SymbolTable::setSymbolGlobal(string name)
{
	for (int i = 0; i < symbols.size(); i++)
	{
		if (symbols[i].getName() == name)
		{
			symbols[i].setGlobal(true);
			return;
		}
	}

	symbols.emplace_back(name, symbols.size() + 1, true);
}

void SymbolTable::printAllSymbols()
{
	for (int i = 0; i < symbols.size(); i++)
	{
		cout << symbols[i].getName();
		if (i + 1 < symbols.size())
			cout << ", ";
	}
	cout << endl;
}