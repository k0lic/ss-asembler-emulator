#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "incalculable.h"
#include "symboltable.h"

using namespace std;

class Expression
{
public:
	Expression();
	void add(string name, BPAction action);
	void add(int number, BPAction action = PLUS);
	// 0 - success
	// 1 - undefined symbol(s)
	// 2 - non-relocatable expression
	char tryToCalculate(SymbolTable &symTab, int *result, int *sectionNum);
private:
	vector<Incalculable> unknowns;
	int known;
};