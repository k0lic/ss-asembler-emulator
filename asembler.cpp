#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include "parser.h"
#include "symboltable.h"
#include "expression.h"
#include "globalsymbol.h"
#include "exceptions.h"


using namespace std;

map<string, unsigned char> opCodes =
{
	{"halt", 0},
	{"iret", 8},
	{"ret", 16},
	{"int", 24},
	{"call", 32},
	{"jmp", 40},
	{"jeq", 48},
	{"jne", 56},
	{"jgt", 64},
	{"push", 72},
	{"pop", 80},
	{"xchg", 88},
	{"mov", 96},
	{"add", 104},
	{"sub", 112},
	{"mul", 120},
	{"div", 128},
	{"cmp", 136},
	{"not", 144},
	{"and", 152},
	{"or", 160},
	{"xor", 168},
	{"test", 176},
	{"shl", 184},
	{"shr", 192}
};

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
	int locationCounter = -1;
	vector<string> labelWaitingQueue;
	vector<pair<string, Expression>> incalculableSymbols;
	vector<GlobalSymbol> globalSymbols;
	string currentDirective = "";

	bool newLineExpected = true;
	bool sentenceStarted = false;

	bool sectionNameExpected = false;
	bool atLeastOneMoreSymbolExpected = false;
	bool atLeastOneMoreSymbolOrLiteralExpected = false;
	bool oneLiteralExpected = false;

	bool branchOperandExpected = false;
	bool expressionExpected = false;
	bool twoMoreOperands = false;
	bool oneMoreOperand = false;
	bool oneByteOperands = false;
	bool nonImediateOperand = false;
	bool reverseOrderOperands = false;

	try
	{
		// single pass through the source file
		while (tokenType != ENDOFFILE)
		{
			// get a single token - array of non-whitespace characters
			tokenType = parser->getNextToken(&token, &firstGroup, &secondGroup, &flags);

			// check if token is JUNK - a token that doesn't conform to the assembly syntax
			if (tokenType == JUNK)
				throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);

			// skip this token if it is a comment - ignore all comments
			if (tokenType == COMMENT)
				continue;

			// check if missing expected new line
			if (newLineExpected && !(flags & Parser::NEW_LINE))
				throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);
			// check if unexpected new line
			if (sentenceStarted && (flags & Parser::NEW_LINE))
				throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);
			newLineExpected = false;

			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if end of source file
			if (tokenType == ENDOFFILE || (tokenType == DIRECTIVE && token == ".end"))
			{
				if (sentenceStarted || !labelWaitingQueue.empty())
					throw UnexpectedTokenException(fileName, parser->getLineNumber(), ".end");

				tokenType = ENDOFFILE;
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is an assembly directive
			else if (tokenType == DIRECTIVE)
			{
				if (sentenceStarted == true)
					throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

				sentenceStarted = true;
				currentDirective = token;

				if (token == ".section")
				{
					sectionNameExpected = true;
				}
				else if (token == ".byte")
				{
					if (currentSection == -1)
						throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

					atLeastOneMoreSymbolOrLiteralExpected = true;
				}
				else if (token == ".word")
				{
					if (currentSection == -1)
						throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

					atLeastOneMoreSymbolOrLiteralExpected = true;	
				}
				else if (token == ".skip")
				{
					if (currentSection == -1)
						throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

					oneLiteralExpected = true;
				}
				else if (token == ".equ")
				{
					atLeastOneMoreSymbolExpected = true;
				}
				else if (token == ".global")
				{
					atLeastOneMoreSymbolExpected = true;	
				}
				else if (token == ".extern")
				{
					atLeastOneMoreSymbolExpected = true;
				}
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////BEGIN
			// check if token is an assembly instruction
			else if (tokenType == INSTRUCTION)
			{
				if (sentenceStarted == true || currentSection == -1)
					throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

				// fulfill the waiting labels
				for (unsigned int i = 0; i < labelWaitingQueue.size(); i++)
					if (!symbolTable.setSymbolValue(labelWaitingQueue[i], currentSection, locationCounter))
						throw DuplicateDefinition(fileName, parser->getLineNumber(), labelWaitingQueue[i]);
				labelWaitingQueue.clear();

				// write instruction into section code
				unsigned char opCodeMod = ((flags & Parser::INSTRUCTION_MOD) && token[token.length() - 1] == 'b') ? 0 : 4;
				symbolTable[currentSection].getSectionCode()->addByte(opCodes[firstGroup] | opCodeMod);
				locationCounter++;

				// check the different assembly instruction types
				if (flags & Parser::INS_SET0)													// 0 operand instructions
				{
					// these instructions do not have 'b' or 'w' suffix variants
					if (flags & Parser::INSTRUCTION_MOD)
						throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

					newLineExpected = true;
				}
				else if (flags & Parser::INS_SET1)												// 1 operand branch instructions
				{
					// these instructions do not have 'b' or 'w' suffix variants
					if (flags & Parser::INSTRUCTION_MOD)
						throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

					sentenceStarted = true;
					branchOperandExpected = true;
					oneMoreOperand = true;
				}
				else if (flags & Parser::INS_SET2)												// 1 operand non-branch instructions
				{
					// TODO
					// POP - special case - cannot have immediate value
					if (firstGroup == "pop")
						nonImediateOperand = true;

					sentenceStarted = true;
					oneMoreOperand = true;
				}
				else if (flags & Parser::INS_SET3)												// 2 operand non-branch instructions (normal)
				{
					sentenceStarted = true;
					twoMoreOperands = true;
					nonImediateOperand = true;
				}
				else if (flags & Parser::INS_SET4)												// 2 operand non-branch no-target instructions (result not stored - only psw indicators)
				{
					sentenceStarted = true;
					twoMoreOperands = true;
				}
				else if (flags & Parser::INS_SET5)												// 2 reverse operand non-branch instruction (first operand is the destination, second is the source)
				{
					sentenceStarted = true;
					twoMoreOperands = true;
					nonImediateOperand = true;
					reverseOrderOperands = true;
				}

				if (opCodeMod == 0)
					oneByteOperands = true;
			}
			////////////////////////////////////////////////////////////////////////////////////////////////////END
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is + or -
			else if (tokenType == ARITHMETIC)
			{
				throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);
				// TODO
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is a label - <symbol>:
			else if (tokenType == LABEL)
			{
				if (sentenceStarted && !sectionNameExpected)
					throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

				int index = symbolTable.getSymbolIndex(firstGroup);

				if (sectionNameExpected)
				{
					if (symbolTable[index].isDefined())
						throw DuplicateDefinition(fileName, parser->getLineNumber(), token);

					symbolTable[index].setDefined(true);
					symbolTable[index].setSection(index);
					symbolTable[index].setValue(0);
					symbolTable[index].newSectionCode();

					currentSection = index;
					locationCounter = 0;

					sentenceStarted = false;
					sectionNameExpected = false;
					newLineExpected = true;
					currentDirective = "";
				}
				else
				{
					labelWaitingQueue.push_back(firstGroup);
				}
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is a symbol operand - multiple addressing modes possible - denoted by flags
			else if (tokenType == SYMBOL)
			{
				// if one of these flags is set - means only 'pure' symbols are allowed
				if (atLeastOneMoreSymbolExpected || atLeastOneMoreSymbolOrLiteralExpected)
				{
					// for .global, .extern and .equ only 'pure' symbols are allowed - NO STARS/DOLLARS/PARENTHESES
					if (flags & (Parser::DOLLAR | Parser::STAR | Parser::PARENTHESES))
						throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

					// check which directive was used - different directives apply different logic
					// if .equ was used it means we prepare for an expression for this symbol
					if (currentDirective == ".equ")
					{
						// a comma must be included since an expression is a mandatory second operand
						if (!(flags & Parser::COMMA))
							throw UnexpectedTokenException(fileName, parser->getLineNumber(), token);

						// setup for upcoming expression
						incalculableSymbols.push_back(make_pair(firstGroup, Expression()));
						
						atLeastOneMoreSymbolExpected = false;
						expressionExpected = true;
						// TODO: what happens in the next round - when expressionExpected is true
					}
					// if .global or .extern were used it means we add the symbol to the array of global symbols
					else if (currentDirective == ".global" || currentDirective == ".extern")
					{
						// check if a comma was used - if it was not used it means that the sentence is over
						if (!(flags & Parser::COMMA))
						{
							atLeastOneMoreSymbolExpected = false;
							sentenceStarted = false;
							newLineExpected = true;
							currentDirective = "";
						}

						// add the symbol to the array of global symbols
						globalSymbols.emplace_back(firstGroup, currentDirective == ".global" ? GLOBAL : EXTERN);
					}
					// if .byte or .word were used it means we need to add a number of 'all 0' bytes to the current section and mark the location with a relocation record
					else if (currentDirective == ".byte" || currentDirective == ".word")
					{
						// check if a comma was used - if it was not used it means that the sentence is over
						if (!(flags & Parser::COMMA))
						{
							atLeastOneMoreSymbolExpected = atLeastOneMoreSymbolOrLiteralExpected = false;
							sentenceStarted = false;
							newLineExpected = true;
							currentDirective = "";
						}

						// fulfill the waiting labels
						for (unsigned int i = 0; i < labelWaitingQueue.size(); i++)
							if (!symbolTable.setSymbolValue(labelWaitingQueue[i], currentSection, locationCounter))
								throw DuplicateDefinition(fileName, parser->getLineNumber(), labelWaitingQueue[i]);
						labelWaitingQueue.clear();

						int numOfBytes = currentDirective == ".byte" ? 1 : 2;
						// get the index of the used symbol
						int index = symbolTable.getSymbolIndex(firstGroup);

						// add a number of 'all 0' bytes to the current sections code
						for (int i=0;i<numOfBytes;i++)
						{
							symbolTable[currentSection].getSectionCode()->addByte(0);
							locationCounter++;
						}

						// add a relocation record for the current section using the index of the used symbol
						vector<RelocationRecord> *rel = symbolTable[currentSection].getRelocationRecords();
						rel->emplace_back(locationCounter, numOfBytes, R_386_32, index);
					}
				}
				else
					throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);
				// TODO
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is a literal operand - multiple addressing modes possible - denoted by flags
			else if (tokenType == LITERAL)
			{
				throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);
				// TODO
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is a register operand - multiple addressing modes possible - denoted by flags
			else if (tokenType == REGISTER)
			{
				throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);
				// TODO
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is operand with indirect-offset addressing - address = symbol + register
			else if (tokenType == SYMBOL_AND_REGISTER)
			{
				throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);
				// TODO
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is operand with indirect-offset addressing - address = literal + register
			else if (tokenType == LITERAL_AND_REGISTER)
			{
				throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);
				// TODO
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// this should never trigger as I have covered all of the cases
			else
				throw UnrecognizableTokenException(fileName, parser->getLineNumber(), token);

			// TO BE DELETED
			if (tokenType == DIRECTIVE || tokenType == INSTRUCTION)
				cout << endl;
			cout << token << " ";
			// END
		}
	}
	catch (MyException& e)
	{
		cout << endl << endl << "All symbols:";
		symbolTable.printAllSymbols();
		cout << endl;

		//collect garbage
		delete parser;
		parser = nullptr;
		// labelWaitingQueue.clear();
		// incalculableSymbols.clear();
		// globalSymbols.clear();

		cout << e;

		return 1;
	}

	// collect garbage
	delete parser;
	parser = nullptr;

	cout << endl;

	// symbolTable.printAllSymbols();

	return 0;
}