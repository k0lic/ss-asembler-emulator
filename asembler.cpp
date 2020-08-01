#include <iostream>
#include "parser.h"
#include "symboltable.h"
#include "exceptions.h"


using namespace std;

int main(int argc, char *argv[])
{
	// check argument validity
	if (argc < 2)
	{
		cout << "One argument is necessarry: Input file name" << endl;
		return 1;
	}

	string fileName = argv[1];
	Parser *parser = new Parser(fileName);
	SymbolTable symbolTable;

	string token, firstGroup, secondGroup;
	char flags;

	TokenType tokenType = OTHER;
	int currentSection = -1;
	bool sectionNameExpected = false;
	bool newLineExpected = true;
	bool instructionUnfinished;
	bool twoOperandInstruction = false;
	bool yetAnotherFlag = true;
	bool iDontThinkImGoingToStickWithThis = true;

	try
	{
		while (tokenType != ENDOFFILE)
		{
			tokenType = parser->getNextToken(&token, &firstGroup, &secondGroup, &flags);

			if (tokenType == JUNK)
				throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);

			if (tokenType == COMMENT)
				continue;

			if (tokenType == ENDOFFILE || (tokenType == DIRECTIVE && token == ".end"))
			{
				tokenType = ENDOFFILE;
			}
			else if (tokenType == DIRECTIVE)
			{
				
			}




			/*
			if (tokenType == DIRECTIVE || tokenType == INSTRUCTION)
				cout << endl;
			cout << token << " ";

			if (tokenType == SYMBOL || tokenType == SYMBOL_AND_REGISTER || tokenType == LABEL)
			{
				symbolTable.getSymbolIndex(firstGroup);
			}
			*/
		}
	}
	catch (MyException& e)
	{
		//collect garbage
		delete parser;
		parser = nullptr;

		cout << e;

		return 1;
	}

	// collect garbage
	delete parser;
	parser = nullptr;

	cout << endl;

	symbolTable.printAllSymbols();

	return 0;
}