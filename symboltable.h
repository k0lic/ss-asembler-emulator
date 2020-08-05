#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "symboltableentry.h"

using namespace std;

class SymbolTable
{
public:
	SymbolTable();
	~SymbolTable();
	SymbolTableEntry* getSymbol(string name);
	SymbolTableEntry& operator[](int index);
	SymbolTableEntry& back();
	void pop_back();
	int getSymbolIndex(string name);
	int size();
	bool setSymbolValue(string name, int section, int value);
	void setSymbolGlobal(string name);
	void printAllSymbols();
	// bool popReference(string name, int *sectionNum, int *address, BPAction *action);
	// void pushReference(string name, int sectionNum, int address, BPAction action);
	void write(ofstream& out);
	void read(ifstream& in);
private:
	vector<SymbolTableEntry> symbols;
};