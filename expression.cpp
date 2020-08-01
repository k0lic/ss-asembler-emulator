#include "expression.h"

using namespace std;

Expression::Expression()
{
	known = 0;
}

void Expression::add(string name, BPAction action)
{
	unknowns.emplace_back(name, action);
}

void Expression::add(int number, BPAction action)
{
	if (action == PLUS)
		known += number;
	else if (action == MINUS)
		known -= number;
}

// 0 - success
// 1 - undefined symbol(s)
// 2 - non-relocatable expression
char Expression::tryToCalculate(SymbolTable &symTab, int *result, int *sectionNum)
{
	// check for undefined symbols
	for (int i=0;i<unknowns.size();i++)
	{
		SymbolTableEntry *ste = symTab.getSymbol(unknowns[i].getName());
		if (ste == nullptr || !ste->isDefined())
			return 1;
	}

	// used to mark the section of every symbol in the expression,
	// to check if the result is absolute/relocatable/invalid
	map<int, int> symbolSections;
	while (!unknowns.empty())
	{
		string name = unknowns.back().getName();
		SymbolTableEntry *ste = symTab.getSymbol(name);

		if (unknowns.back().getAction() == PLUS)
		{
			known += ste->getValue();
			symbolSections[ste->getSection()]++;
		}
		else if (unknowns.back().getAction() == MINUS)
		{
			known -= ste->getValue();
			symbolSections[ste->getSection()]--;
		}

		unknowns.pop_back();
	}
	*result = known;

	// find which section this expression belongs to
	int sectionCount = 0;
	for (auto i = symbolSections.begin(); i != symbolSections.end(); i++)
		if (i->second != 0)
		{
			*sectionNum = i->first;
			sectionCount++;
		}

	if (sectionCount > 1)
		return 2;
	else if (sectionCount == 0)
		*sectionNum = 0;

	return 0;
}