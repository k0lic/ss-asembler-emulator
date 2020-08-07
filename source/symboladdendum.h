#pragma once

using namespace std;

class SymbolAddendum
{
public:
	SymbolAddendum(int remapIndex, int offset);
	void setRemapIndex(int remapIndex);
	void setOffset(int offset);
	int getRemapIndex();
	int getOffset();
private:
	int remapIndex;
	int offset;
};