#include "relocationrecord.h"

using namespace std;

RelocationRecord::RelocationRecord(int offset, int targetSize, RelocationType type, int symbolIndex)
{
	this->offset = offset;
	this->targetSize = targetSize;
	this->type = type;
	this->symbolIndex = symbolIndex;
}

int RelocationRecord::getOffset()
{
	return offset;
}

int RelocationRecord::getTargetSize()
{
	return targetSize;
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