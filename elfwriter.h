#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include "symboltable.h"
#include "relocationrecord.h"

using namespace std;

class ElfWriter
{
public:
	ElfWriter(SymbolTable& symTab);
	ostream& write(ostream& out) const;
	friend ostream& operator<<(ostream& out, const ElfWriter& elf);
private:
	SymbolTable &symbolTable;
};