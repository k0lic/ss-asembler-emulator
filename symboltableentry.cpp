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
}

/* DESTRUCTOR */

SymbolTableEntry::~SymbolTableEntry()
{
	references.clear();
	delete sectionCode;
	sectionCode = nullptr;
}

/* FORWARD REFERENCE MANIPULATION */

bool SymbolTableEntry::popReference(int *sectionNum, int *address, BPAction *action)
{
	if (references.empty())
		return false;

	ForwardReference &ref = references.back();
	*sectionNum = ref.getSectionNumber();
	*address = ref.getAddress();
	*action = ref.getAction();
	references.pop_back();
	return true;
}

void SymbolTableEntry::pushReference(int sectionNum, int address, BPAction action)
{
	references.emplace_back(sectionNum, address, action);
}

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

void SymbolTableEntry::newSectionCode()
{
	sectionCode = new Section();
}