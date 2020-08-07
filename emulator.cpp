#include <regex>
#include "objectfile.h"
#include "elfwriter.h"
#include "processorlogic.h"
#include "exceptions.h"

using namespace std;

#define MEM_SIZE (65'536)

int main(int argc, char *argv[])
{
	vector<ObjectFile> inputFiles;
	map<string, int> sectionPlacements;
	SymbolTable mainTable;
	unsigned char memory[MEM_SIZE];

	try
	{
		/*** STAGE1: LINKER STAGE ***/
		// parse console arguments
		regex sectionPlacementPattern("-place=([^@]+)@0x([0-9a-fA-F]{4})");
		smatch matches;
		for (int i = 1; i < argc; i++)
		{
			string tmp = argv[i];

			if (regex_match(tmp, matches, sectionPlacementPattern))
			{
				string sectionName = matches[1].str();
				string placementString = matches[2].str();

				int placement = stol(placementString, nullptr, 16);
				if (placement < 0 || placement >= MEM_SIZE)
					throw MessageException("Parsing console arguments FAILED!\nInvalid placement argument: '" + tmp + "'. Placement must be a positive integer less than " + to_string(MEM_SIZE) + "!");

				sectionPlacements[sectionName] = placement;
			}
			else if (tmp[0] == '-')
				throw MessageException("Parsing console arguments FAILED!\nCould not parse argument: '" + tmp + "'.");
			else
				inputFiles.emplace_back(tmp);
		}

		if (inputFiles.size() == 0)
			throw MessageException("Parsing console arguments FAILED!\nOne input object file is necessarry.");

		// read the contents of the object files
		for (unsigned int i = 0; i < inputFiles.size(); i++)
		{
			ifstream bf(inputFiles[i].getName(), ios::out | ios::binary);
			inputFiles[i].getSymbolTable().read(bf);

			if (bf.fail())
				throw MessageException("Linker FAILED!\nCould not open or could not parse data from file: '" + inputFiles[i].getName() + "'.");

			// set addendums
			for (int j = 0; j < inputFiles[i].getSymbolTable().size(); j++)
				inputFiles[i].getAddendums().emplace_back(0, 0);
		}

		// ElfWriter *elf = nullptr;
		// for (unsigned int i = 0; i < inputFiles.size(); i++)
		// {
		// 	elf = new ElfWriter(inputFiles[i].getSymbolTable());
		// 	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << inputFiles[i].getName() << endl;
		// 	cout << *elf << endl;
		// 	delete elf;
		// 	elf = nullptr;
		// }

		// add sections from every file to main table
		for (unsigned int i = 0; i < inputFiles.size(); i++)
		{
			SymbolTable& symtab = inputFiles[i].getSymbolTable();
			vector<SymbolAddendum>& addendums = inputFiles[i].getAddendums();

			// get every section from this file
			for (int j = 0; j < symtab.size(); j++)
				if (symtab[j].getSectionCode() != nullptr)
				{
					// get index of the aggregated section in main table - add to main table if not present yet
					int index = mainTable.getSymbolIndex(symtab[j].getName());

					// set remap index and offset for input file section - does not take into account the aggregated sections offset (which is still 0)
					addendums[j].setRemapIndex(index);
					addendums[j].setOffset(mainTable[index].getSize());

					// add input file section size to the aggregated section size
					int newSize = mainTable[index].getSize() + symtab[j].getSectionCode()->size();
					mainTable[index].setSize(newSize);
				}
		}

		// place aggregated sections following placement console arguments
		int nextSlot = 0;
		for (int i = 1; i < mainTable.size(); i++)
		{
			string sectionName = mainTable[i].getName();

			if (sectionPlacements.find(sectionName) != sectionPlacements.end())
			{
				mainTable[i].setValue(sectionPlacements[sectionName]);
				mainTable[i].setDefined(true);
				mainTable[i].setSection(i);

				int potentialNextSlot = mainTable[i].getValue() + mainTable[i].getSize();
				if (potentialNextSlot > MEM_SIZE)
				{
					stringstream stream;
					stream << hex << mainTable[i].getValue();
					throw MessageException("Linker FAILED!\nRequested placement of section '" + mainTable[i].getName() + "' at 0x" + stream.str() + " does not fit size of said section.");
				}

				if (potentialNextSlot > nextSlot)
					nextSlot = potentialNextSlot;
			}
		}

		// place the rest of the aggregated sections sequentially
		for (int i = 1; i < mainTable.size(); i++)
		{
			if (mainTable[i].isDefined())
				continue;

			string sectionName = mainTable[i].getName();

			mainTable[i].setValue(nextSlot);
			mainTable[i].setDefined(true);
			mainTable[i].setSection(i);

			nextSlot += mainTable[i].getSize();
		}

		if (nextSlot > MEM_SIZE)
			throw MessageException("Linker FAILED!\nCould not fit all of the sections!");

		// check for section overlap
		map<int, int> sectionEdges;
		for (int i = 1; i < mainTable.size(); i++)
		{
			int sectionStart = mainTable[i].getValue();
			int sectionEnd = sectionStart + mainTable[i].getSize();

			sectionEdges[sectionStart]++;
			sectionEdges[sectionEnd]--;
		}

		int overlappingSections = 0;
		for (auto i = sectionEdges.begin(); i != sectionEdges.end(); i++)
		{
			overlappingSections += i->second;

			if (overlappingSections > 1)
				throw MessageException("Linker FAILED!\nThe requested placement causes section overlap!");
		}

		// import the rest of the symbols from every file into the main symbol table
		for (unsigned int i = 0; i < inputFiles.size(); i++)
		{
			SymbolTable& symtab = inputFiles[i].getSymbolTable();
			vector<SymbolAddendum>& addendums = inputFiles[i].getAddendums();

			// get the non-section symbols from this file
			for (int j = 1; j < symtab.size(); j++)
				if (symtab[j].getSectionCode() == nullptr)
				{
					// get index of the symbol in main table - add to main table if not present yet
					int index = mainTable.getSymbolIndex(symtab[j].getName());

					// check if symbol is defined in this file - if not just remap to the main table entry, it will be defined in another file
					if (symtab[j].isDefined())
					{
						// check for duplicate definition
						if (mainTable[index].isDefined())
							throw DuplicateDefinition(inputFiles[i].getName(), -1, symtab[j].getName());

						// calculate new value - take into account input file section offset from aggregated section start, as well as the aggregated section placement
						int oldSectionIndex = symtab[j].getSection();
						int newSectionIndex = addendums[oldSectionIndex].getRemapIndex();
						int newValue = symtab[j].getValue() + addendums[oldSectionIndex].getOffset() + mainTable[newSectionIndex].getValue();

						// define main table symbol for every file to see
						mainTable[index].setDefined(true);
						mainTable[index].setSection(newSectionIndex);
						mainTable[index].setValue(newValue);
						mainTable[index].setGlobal(true);
					}

					// set remap index for input file symbol
					addendums[j].setRemapIndex(index);
				}
		}

		// check if there are still undefined symbols
		for (int i = 1; i < mainTable.size(); i++)
			if (!mainTable[i].isDefined())
				throw MissingDefinition("NO_INFO", -1, mainTable[i].getName());

		ElfWriter elf (mainTable);
		cout << elf << endl;

		// copy section contents into memory
		for (unsigned int i = 0; i < inputFiles.size(); i++)
		{
			SymbolTable& symtab = inputFiles[i].getSymbolTable();
			vector<SymbolAddendum>& addendums = inputFiles[i].getAddendums();

			for (int j = 0; j < symtab.size(); j++)
				if (symtab[j].getSectionCode() != nullptr)
				{
					int copyTarget = mainTable[addendums[j].getRemapIndex()].getValue() + addendums[j].getOffset();
					symtab[j].getSectionCode()->readBytes(0, symtab[j].getSectionCode()->size(), memory + copyTarget);
				}
		}

		// resolve relocation records
		// go trough every input file
		for (unsigned int i = 0; i < inputFiles.size(); i++)
		{
			SymbolTable& symtab = inputFiles[i].getSymbolTable();
			vector<SymbolAddendum>& addendums = inputFiles[i].getAddendums();

			// go through every section (only sections have relocation records tied to their symbol table entries)
			for (int j = 0; j < symtab.size(); j++)
			{
				vector<RelocationRecord> *relRecords = symtab[j].getRelocationRecords(false);

				if (relRecords == nullptr)
					continue;

				// get the offset for input file section
				int localSectionOffset = addendums[j].getOffset();

				// get the index of the aggregated section in the master table
				int aggregatedSectionIndex = addendums[j].getRemapIndex();

				// go through every relocation record
				for (unsigned int k = 0; k < relRecords->size(); k++)
				{
					// get the offset of the location that needs to be modified - this offset is calculated with input file section as 0
					int relRecordOffset = relRecords->at(k).getOffset();

					// get the input file table index for the referenced symbol
					int targetSymbolLocalIndex = relRecords->at(k).getSymbolIndex();

					// get the master table index for the referenced symbol
					int targetSymbolMainIndex = addendums[targetSymbolLocalIndex].getRemapIndex();

					// get the type of relocation record
					RelocationType relType = relRecords->at(k).getType();

					// get the size of the location that need to be modified
					int locationSize = (relType == R_386_16 || relType == R_386_PC16 ? 2 : 1);

					// calculate the address of the location that needs to be modified
					// address = offset of location from input file section beginning + offset of input file section from aggregated master section beginning + address of aggregated section
					int address = relRecordOffset + localSectionOffset + mainTable[aggregatedSectionIndex].getValue();
					
					// calculate the modified value of the target location - start with the value of the targeted symbol
					// for symbols the offset is 0, so the true value is in the master table; for sections both values carry information
					int value = addendums[targetSymbolLocalIndex].getOffset() + mainTable[targetSymbolMainIndex].getValue();
					// add the existing value of the location
					value += LittleEndian::charLittleEndianToInt(locationSize, memory + address);
					// if PC-relative relocation was used - substract the address of the location
					if (relType == R_386_PC16 || relType == R_386_PC8)
						value -= address;

					// overwrite the targeted location with the calculated value
					LittleEndian::intToLittleEndianChar(value, locationSize, memory + address);
				}
			}
		}

		// TEST OUTPUT
		cout << hex;
		for (int i = 0; i < 160; i++)
		{
			cout << setw(2) << setfill('0') << +memory[i];

			if (i % 16 == 15)
				cout << endl;
			else if (i % 4 == 3)
				cout << "\t\t";
			else
				cout << " ";
		}
		cout << dec;

		/*** STAGE2: EMULATOR STAGE ***/
		ProcessorContext context;
		ProcessorLogic logic (&context, memory);

		// setup starting point - interrupt routine from IVT entry 0
		unsigned int startingAddress = LittleEndian::charLittleEndianToUInt(2, memory + 0);
		context.setPC(startingAddress);

		cout << ">EMULATION STARTED" << endl;

		// loop until you reach the 'halt' instruction
		while (!logic.isHalted())
		{
			// try to execute instruction - care for invalid op-codes and addressing modes
			bool successfulInstruction = logic.executeNextInstruction();

			// TODO: maybe save instruction start PC value, so we can revert to it if there is an error in the instruction code

			// if there was an incorrect instruction - execute interrupt routine from IVT entry 1
			if (!successfulInstruction)
				logic.interrupt(1);

			// TODO: timer
			// TODO: terminal
		}

		cout << ">EMULATION ENDED" << endl;
	}
	catch (MyException& e)
	{
		cout << e;

		return 1;
	}
	catch (MessageException& e)
	{
		cout << e;

		return 2;
	}

	return 0;
}