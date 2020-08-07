#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "section.h"
#include "relocationrecord.h"

using namespace std;

class SymbolTableEntry
{
public:
	SymbolTableEntry(string name, int index, bool global = false);
	SymbolTableEntry(string name, int index, int section, int value);
	SymbolTableEntry(const SymbolTableEntry& other);											// copy constructor
	SymbolTableEntry(SymbolTableEntry&& other);													// move constructor
	~SymbolTableEntry();
	SymbolTableEntry& operator=(const SymbolTableEntry& other);									// copy assign operator
	void write(ofstream& out);
	void read(ifstream& in);
	string getName();
	bool isDefined();
	int getSection();
	int getValue();
	bool isGlobal();
	int getIndex();
	int getSize();
	Section* getSectionCode();
	vector<RelocationRecord>* getRelocationRecords(bool autoCreate = true);
	void setDefined(bool defined);
	void setSection(int section);
	void setValue(int value);
	void setGlobal(bool global);
	void setIndex(int index);
	void setSize(int size);
	void newSectionCode();
	vector<RelocationRecord>* newRelocationRecords();
private:
	string name;
	bool defined;
	int section;
	int value;
	bool global;
	int index;
	int size;
	Section *sectionCode;
	vector<RelocationRecord> *relocationRecords;
};