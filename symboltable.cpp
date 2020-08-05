#include "symboltable.h"

using namespace std;

/* CONSTRUCTOR */

SymbolTable::SymbolTable()
{
	symbols.emplace_back("*UND", 0);
}

/* DESTRUCTOR */

SymbolTable::~SymbolTable()
{
	symbols.clear();
}

/* PUBLIC METHODS */

SymbolTableEntry* SymbolTable::getSymbol(string name)
{
	for (unsigned int i = 0; i < symbols.size(); i++)
	{
		if (symbols[i].getName() == name)
			return &symbols[i];
	}

	return nullptr;
}

SymbolTableEntry& SymbolTable::operator[](int index)
{
	return symbols[index];
}

SymbolTableEntry& SymbolTable::back()
{
	return symbols.back();
}

void SymbolTable::pop_back()
{
	symbols.pop_back();
}

int SymbolTable::getSymbolIndex(string name)
{
	for (unsigned int i = 0; i < symbols.size(); i++)
	{
		if (symbols[i].getName() == name)
			return i;
	}

	symbols.emplace_back(name, symbols.size());

	return symbols.size() - 1;
}

int SymbolTable::size()
{
	return symbols.size();
}

bool SymbolTable::setSymbolValue(string name, int section, int value)
{
	for (unsigned int i = 0; i < symbols.size(); i++)
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

	symbols.emplace_back(name, symbols.size(), section, value);
	
	return true;
}

void SymbolTable::setSymbolGlobal(string name)
{
	for (unsigned int i = 0; i < symbols.size(); i++)
	{
		if (symbols[i].getName() == name)
		{
			symbols[i].setGlobal(true);
			return;
		}
	}

	symbols.emplace_back(name, symbols.size(), true);
}

void SymbolTable::printAllSymbols()
{
	for (unsigned int i = 0; i < symbols.size(); i++)
	{
		cout << symbols[i].getName();
		if (i + 1 < symbols.size())
			cout << ", ";
	}
	cout << endl;
}

void SymbolTable::write(ofstream& out)
{
	// write the number of symbols in the table
	unsigned int symbolsSize = symbols.size();
	out.write((char*)&symbolsSize, sizeof(symbolsSize));

	// write every symbol
	for (unsigned int i = 1; i < symbolsSize; i++)
		symbols[i].write(out);
}

void SymbolTable::read(ifstream& in)
{
	// read the number of symbols in the table
	unsigned int symbolsSize;
	in.read((char*)&symbolsSize, sizeof(symbolsSize));

	// read every symbol
	symbols.reserve(symbolsSize);
	for (unsigned int i = 1; i < symbolsSize; i++)
	{
		symbols.emplace_back("PLACEHOLDER", i);
		symbols.back().read(in);
	}
}