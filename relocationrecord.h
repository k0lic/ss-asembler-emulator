#pragma once

#include <string>
#include <map>
#include <iostream>

using namespace std;

// R_386_16 = ABSOLUTE 16 BIT, R_386_PC16 = PC RELATIVE 16 BIT, R_386_8 = ABSOLUTE 8 BIT, R_386_PC8 = PC RELATIVE 16 BIT
enum RelocationType { R_386_16, R_386_PC16, R_386_8, R_386_PC8 };

class RelocationRecord
{
public:
	RelocationRecord(int offset, RelocationType type, int symbolIndex);
	int getOffset();
	RelocationType getType();
	int getSymbolIndex();
	void setSymbolIndex(int symbolIndex);

	static string relocationTypeToString(RelocationType type);
private:
	int offset;
	RelocationType type;
	int symbolIndex;

	static map<RelocationType, string> relTypeMap;
};