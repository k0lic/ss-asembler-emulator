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
	this->size = 0;
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
	this->size = 0;
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
	this->size = other.size;

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
	this->size = other.size;

	this->sectionCode = other.sectionCode;
	other.sectionCode = nullptr;

	this->relocationRecords = other.relocationRecords;
	other.relocationRecords = nullptr;
}

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
	this->size = other.size;

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

/* BINARY INPUT AND OUTPUT */

void SymbolTableEntry::write(ofstream& out)
{
	// write the name
	int nameLength = name.length();
	out.write((char*)&nameLength, sizeof(nameLength));
	out.write((char*)name.c_str(), nameLength * sizeof(char));

	// write the rest of the basic fields
	out.write((char*)&defined, sizeof(defined));
	out.write((char*)&section, sizeof(section));
	out.write((char*)&value, sizeof(value));
	out.write((char*)&global, sizeof(global));
	out.write((char*)&index, sizeof(index));
	out.write((char*)&size, sizeof(size));

	// write whether there is code
	bool sectionCodeIsNull = sectionCode == nullptr;
	out.write((char*)&sectionCodeIsNull, sizeof(sectionCodeIsNull));
	// if there is code - write it
	if (!sectionCodeIsNull)
		sectionCode->write(out);

	// write whether there are relocation records
	bool relocationRecordsAreNull = relocationRecords == nullptr;
	out.write((char*)&relocationRecordsAreNull, sizeof(relocationRecordsAreNull));
	// if there are relocation records - write them
	if (!relocationRecordsAreNull)
	{
		// write the number of records
		unsigned int relocationRecordsSize = relocationRecords->size();
		out.write((char*)&relocationRecordsSize, sizeof(relocationRecordsSize));

		// write every record
		for (unsigned int i = 0; i < relocationRecordsSize; i++)
		{
			// extract record info
			int offset = relocationRecords->at(i).getOffset();
			RelocationType type = relocationRecords->at(i).getType();
			int symbolIndex = relocationRecords->at(i).getSymbolIndex();

			// write record info
			out.write((char*)&offset, sizeof(offset));
			out.write((char*)&type, sizeof(type));
			out.write((char*)&symbolIndex, sizeof(symbolIndex));
		}
	}
}

void SymbolTableEntry::read(ifstream& in)
{
	// read the name
	int nameLength;
	in.read((char*)&nameLength, sizeof(nameLength));
	char nameTemp[nameLength + 1];
	in.read((char*)nameTemp, nameLength * sizeof(char));
	nameTemp[nameLength] = 0;
	name = nameTemp;

	// read the rest of the basic fields
	in.read((char*)&defined, sizeof(defined));
	in.read((char*)&section, sizeof(section));
	in.read((char*)&value, sizeof(value));
	in.read((char*)&global, sizeof(global));
	in.read((char*)&index, sizeof(index));
	in.read((char*)&size, sizeof(size));

	// read whether there is code
	bool sectionCodeIsNull;
	in.read((char*)&sectionCodeIsNull, sizeof(sectionCodeIsNull));
	// if there is code - read it
	if (!sectionCodeIsNull)
	{
		sectionCode = new Section();
		sectionCode->read(in);
	}
	else
		sectionCode = nullptr;

	// read whether there are relocation records
	bool relocationRecordsAreNull;
	in.read((char*)&relocationRecordsAreNull, sizeof(relocationRecordsAreNull));
	// if there are relocation records - read them
	if (!relocationRecordsAreNull)
	{
		// initialize empty relocation records vector
		relocationRecords = new vector<RelocationRecord>();

		// read the number of records
		unsigned int relocationRecordsSize;
		in.read((char*)&relocationRecordsSize, sizeof(relocationRecordsSize));

		// read every record
		for (unsigned int i = 0; i < relocationRecordsSize; i++)
		{
			// setup temporary variables for the data
			int offset;
			RelocationType type;
			int symbolIndex;

			// read the record data
			in.read((char*)&offset, sizeof(offset));
			in.read((char*)&type, sizeof(type));
			in.read((char*)&symbolIndex, sizeof(symbolIndex));

			// add the record
			relocationRecords->emplace_back(offset, type, symbolIndex);
		}
	}
	else
		relocationRecords = nullptr;
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

int SymbolTableEntry::getSize()
{
	return size;
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

void SymbolTableEntry::setSize(int size)
{
	this->size = size;
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