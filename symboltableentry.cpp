#include "symboltableentry.h"

using namespace std;

/* CONSTRUCTORS */

SymbolTableEntry::SymbolTableEntry(string name, int index, bool global)
{
	this->name = name;
	this->defined = false;
	this->section = 0;
	this->value = 0;
	this->global = global;
	this->index = index;
	this->sectionCode = nullptr;
	this->relocationRecords = nullptr;
}

SymbolTableEntry::SymbolTableEntry(string name, int index, int section, int value)
{
	this->name = name;
	this->defined = true;
	this->section = section;
	this->value = value;
	this->global = false;
	this->index = index;
	this->sectionCode = nullptr;
	this->relocationRecords = nullptr;
}

SymbolTableEntry::SymbolTableEntry(const SymbolTableEntry& other)												// copy constructor
{
	this->name = other.name;
	this->defined = other.defined;
	this->section = other.section;
	this->value = other.value;
	this->global = other.global;
	this->index = other.index;

	if (other.sectionCode == nullptr)
		this->sectionCode = nullptr;
	else
		this->sectionCode = new Section(*other.sectionCode);

	if (other.relocationRecords == nullptr)
		this->relocationRecords = nullptr;
	else
		this->relocationRecords = new vector<RelocationRecord>(*other.relocationRecords);
}

SymbolTableEntry::SymbolTableEntry(SymbolTableEntry&& other)													// move constructor
{
	this->name = other.name;
	this->defined = other.defined;
	this->section = other.section;
	this->value = other.value;
	this->global = other.global;
	this->index = other.index;

	this->sectionCode = other.sectionCode;
	other.sectionCode = nullptr;

	this->relocationRecords = other.relocationRecords;
	other.relocationRecords = nullptr;
}

/*
private:
	string name;
	bool defined;
	int section;
	int value;
	bool global;
	int index;
	// vector<ForwardReference> references;
	Section *sectionCode;
	vector<RelocationRecord> *relocationRecords;
*/

/* DESTRUCTOR */

SymbolTableEntry::~SymbolTableEntry()
{
	// references.clear();
	delete sectionCode;
	sectionCode = nullptr;
	delete relocationRecords;
	relocationRecords = nullptr;
}

/* ASSIGN OPERATOR */

SymbolTableEntry& SymbolTableEntry::operator=(const SymbolTableEntry& other)
{
	this->name = other.name;
	this->defined = other.defined;
	this->section = other.section;
	this->value = other.value;
	this->global = other.global;
	this->index = other.index;

	delete this->sectionCode;
	if (other.sectionCode == nullptr)
		this->sectionCode = nullptr;
	else
		this->sectionCode = new Section(*other.sectionCode);

	delete this->relocationRecords;
	if (other.relocationRecords == nullptr)
		this->relocationRecords = nullptr;
	else
		this->relocationRecords = new vector<RelocationRecord>(*other.relocationRecords);

	return *this;
}

/* FORWARD REFERENCE MANIPULATION */

// bool SymbolTableEntry::popReference(int *sectionNum, int *address, BPAction *action)
// {
// 	if (references.empty())
// 		return false;

// 	ForwardReference &ref = references.back();
// 	*sectionNum = ref.getSectionNumber();
// 	*address = ref.getAddress();
// 	*action = ref.getAction();
// 	references.pop_back();
// 	return true;
// }

// void SymbolTableEntry::pushReference(int sectionNum, int address, BPAction action)
// {
// 	references.emplace_back(sectionNum, address, action);
// }

/* GETTERS */

string SymbolTableEntry::getName()
{
	return name;
}

bool SymbolTableEntry::isDefined()
{
	return defined;
}

int SymbolTableEntry::getSection()
{
	return section;
}

int SymbolTableEntry::getValue()
{
	return value;
}

bool SymbolTableEntry::isGlobal()
{
	return global;
}

int SymbolTableEntry::getIndex()
{
	return index;
}

Section* SymbolTableEntry::getSectionCode()
{
	return sectionCode;
}

vector<RelocationRecord>* SymbolTableEntry::getRelocationRecords(bool autoCreate)
{
	if (autoCreate && relocationRecords == nullptr)
		relocationRecords = new vector<RelocationRecord>();
	return relocationRecords;
}

/* SETTERS */

void SymbolTableEntry::setDefined(bool defined)
{
	this->defined = defined;
}

void SymbolTableEntry::setSection(int section)
{
	this->section = section;
}

void SymbolTableEntry::setValue(int value)
{
	this->value = value;
}

void SymbolTableEntry::setGlobal(bool global)
{
	this->global = global;
}

void SymbolTableEntry::setIndex(int index)
{
	this->index = index;
}

void SymbolTableEntry::newSectionCode()
{
	delete sectionCode;
	sectionCode = new Section();
}

vector<RelocationRecord>* SymbolTableEntry::newRelocationRecords()
{
	relocationRecords = new vector<RelocationRecord>();
	return relocationRecords;
}