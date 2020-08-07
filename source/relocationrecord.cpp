#include "relocationrecord.h"

using namespace std;

map<RelocationType, string> RelocationRecord::relTypeMap =
{
	{ R_386_16,		"R_386_16"},
	{ R_386_PC16,	"R_386_PC16"},
	{ R_386_8,		"R_386_8"},
	{ R_386_PC8,	"R_386_PC8"}
};

RelocationRecord::RelocationRecord(int offset, RelocationType type, int symbolIndex)
{
	this->offset = offset;
	this->type = type;
	this->symbolIndex = symbolIndex;
}

int RelocationRecord::getOffset()
{
	return offset;
}

RelocationType RelocationRecord::getType()
{
	return type;
}

int RelocationRecord::getSymbolIndex()
{
	return symbolIndex;
}

void RelocationRecord::setSymbolIndex(int symbolIndex)
{
	this->symbolIndex = symbolIndex;
}

string RelocationRecord::relocationTypeToString(RelocationType type)
{
	return relTypeMap[type];
}