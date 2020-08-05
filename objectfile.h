#pragma once

#include <string>
#include <vector>
#include "symboltable.h"
#include "symboladdendum.h"

using namespace std;

class ObjectFile
{
public:
	ObjectFile(string name);
	string getName();
	SymbolTable& getSymbolTable();
	vector<SymbolAddendum>& getAddendums();
private:
	string name;
	SymbolTable symbolTable;
	vector<SymbolAddendum> addendums;
};