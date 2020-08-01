#pragma once

#include <string>
#include <vector>
#include <iostream>
// #include "forwardreference.h"
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
	// bool popReference(int *sectionNum, int *address, BPAction *action);
	// void pushReference(int sectionNum, int address, BPAction action);
	string getName();
	bool isDefined();
	int getSection();
	int getValue();
	bool isGlobal();
	int getIndex();
	Section* getSectionCode();
	vector<RelocationRecord>* getRelocationRecords(bool autoCreate = true);
	void setDefined(bool defined);
	void setSection(int section);
	void setValue(int value);
	void setGlobal(bool global);
	void newSectionCode();
	vector<RelocationRecord>* newRelocationRecords();
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
};