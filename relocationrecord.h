#pragma once

enum RelocationType { R_386_32, R_386_PC32 };				// R_386_32 = ABSOLUTE, R_386_PC32 = PC RELATIVE

class RelocationRecord
{
public:
	RelocationRecord(int offset, int targetSize, RelocationType type, int symbolIndex);
	int getOffset();
	int getTargetSize();
	RelocationType getType();
	int getSymbolIndex();
	void setSymbolIndex(int symbolIndex);
private:
	int offset;
	int targetSize;
	RelocationType type;
	int symbolIndex;
};