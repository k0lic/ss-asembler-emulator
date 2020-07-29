#include <string>

using namespace std;

class SymbolTableEntry
{
public:
	SymbolTableEntry(string name, int index, bool global = false);
	SymbolTableEntry(string name, int index, int section, int value);
	string getName();
	bool isDefined();
	int getSection();
	int getValue();
	bool isGlobal();
	int getIndex();
	void setDefined(bool defined);
	void setSection(int section);
	void setValue(int value);
	void setGlobal(bool global);
private:
	string name;
	bool defined;
	int section;
	int value;
	bool global;
	int index;
};