#include "symboladdendum.h"

using namespace std;

SymbolAddendum::SymbolAddendum(int remapIndex, int offset)
{
	this->remapIndex = remapIndex;
	this->offset = offset;
}

void SymbolAddendum::setRemapIndex(int remapIndex)
{
	this->remapIndex = remapIndex;
}

void SymbolAddendum::setOffset(int offset)
{
	this->offset = offset;
}

int SymbolAddendum::getRemapIndex()
{
	return remapIndex;
}

int SymbolAddendum::getOffset()
{
	return offset;
}