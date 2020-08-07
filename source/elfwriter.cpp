#include "elfwriter.h"

using namespace std;

ElfWriter::ElfWriter(SymbolTable& symTab) : symbolTable(symTab) {}

ostream& ElfWriter::write(ostream& out) const
{
	// setup
	out << hex;

	// write symbol table
	out << "#symbol table" << endl;

	out << "#";
	out << setw(8) << "index";
	out << setw(16) << "name";
	out << setw(8) << "defined";
	out << setw(8) << "section";
	out << setw(8) << "value";
	out << setw(8) << "vis.";
	out << setw(8) << "size";
	out << endl;

	for (int i = 0; i < symbolTable.size(); i++)
	{
		out << " ";
		out << setw(8) << symbolTable[i].getIndex();
		out << setw(16) << symbolTable[i].getName();
		out << setw(8) << symbolTable[i].isDefined();
		out << setw(8) << symbolTable[i].getSection();
		out << setw(8) << symbolTable[i].getValue();
		out << setw(8) << (symbolTable[i].isGlobal() ? "g" : "l");
		if (symbolTable[i].getSectionCode() != nullptr)
			out << setw(8) << symbolTable[i].getSectionCode()->size();
		out << endl;
	}

	// write relocation records
	for (int i = 0; i < symbolTable.size(); i++)
	{
		vector<RelocationRecord> *relRecords = symbolTable[i].getRelocationRecords(false);

		if (relRecords != nullptr)
		{
			out << "#rel " << symbolTable[i].getName() << ":" << endl;
			
			out << "#";
			out << setw(8) << "offset";
			out << setw(16) << "type";
			out << setw(8) << "value";
			out << endl;

			for (unsigned int j = 0; j < relRecords->size(); j++)
			{
				out << " ";
				out << setw(8) << (*relRecords)[j].getOffset();
				out << setw(16) << RelocationRecord::relocationTypeToString((*relRecords)[j].getType());
				out << setw(8) << (*relRecords)[j].getSymbolIndex();
				out << endl;
			}
		}
	}

	// write sections
	for (int i = 0; i < symbolTable.size(); i++)
	{
		if (symbolTable[i].getSectionCode() != nullptr)
		{
			out << "#" << symbolTable[i].getName() << ":" << endl;
			out << *symbolTable[i].getSectionCode();
		}
	}

	// cleanup
	out << dec;

	return out;
}

ostream& operator<<(ostream& out, const ElfWriter& elf)
{
	return elf.write(out);
}