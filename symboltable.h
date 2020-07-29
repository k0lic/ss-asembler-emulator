#include <iostream>
#include <vector>
#include "symboltableentry.h"

using namespace std;

class SymbolTable
{
public:
	SymbolTable();
	int getSymbolIndex(string name);
	bool setSymbolValue(string name, int section, int value);
	void setSymbolGlobal(string name);
	void printAllSymbols();
private:
	vector<SymbolTableEntry> symbols;
};