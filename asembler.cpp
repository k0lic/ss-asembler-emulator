#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include "parser.h"
#include "symboltable.h"
#include "symboltableentry.h"
#include "expression.h"
#include "globalsymbol.h"
#include "littleendian.h"
#include "elfwriter.h"
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

	if (argc == 3 || argc > 4)
	{
		cout << "Invalid number of arguments, expected either 2 or 4" << endl;
		return 2;
	}

	string srcFileName = "";
	string dstFileName = "a.out";

	// parse console arguments
	bool nextIsDstFileName = false;
	for (int i = 1; i < argc; i++)
	{
		string tmp = argv[i];

		if (nextIsDstFileName)
		{
			dstFileName = tmp;
			nextIsDstFileName = false;
		}
		else if (tmp == "-o")
			nextIsDstFileName = true;
		else
			srcFileName = tmp;
	}

	if (nextIsDstFileName)
	{
		cout << "Could not parse console arguments" << endl;
		return 3;
	}

	Parser *parser = new Parser(srcFileName);
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
	unsigned char expressionStatus = 0;
	bool twoMoreOperands = false;
	bool oneMoreOperand = false;
	bool oneByteOperands = false;
	bool nonImediateOperand = false;
	bool reverseOrderOperands = false;

	try
	{
		// STEP1: single pass through the source file
		while (tokenType != ENDOFFILE)
		{
			// get a single token - array of non-whitespace characters
			tokenType = parser->getNextToken(&token, &firstGroup, &secondGroup, &flags);

			// check if token is JUNK - a token that doesn't conform to the assembly syntax
			if (tokenType == JUNK)
				throw UnrecognizableTokenException(srcFileName, parser->getLineNumber(), token);

			// skip this token if it is a comment - ignore all comments
			if (tokenType == COMMENT)
				continue;

			// special case: expresssions - they can end unexpectedly - deal with it here: check if the expression was in a stable state when a new line appears
			if (expressionExpected && (flags & Parser::NEW_LINE))
			{
				// if expression was in a stable state (last token wasn't an operation) end the sentence
				if (expressionStatus == 3)
				{
					sentenceStarted = false;
					currentDirective = "";

					expressionExpected = false;
					expressionStatus = 0;

					newLineExpected = true;
				}
			}

			// check if missing expected new line
			if (newLineExpected && !(flags & Parser::NEW_LINE))
				throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
			// check if unexpected new line
			if (sentenceStarted && (flags & Parser::NEW_LINE))
				throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
			newLineExpected = false;

			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if end of source file
			if (tokenType == ENDOFFILE || (tokenType == DIRECTIVE && token == ".end"))
			{
				if (sentenceStarted || !labelWaitingQueue.empty())
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), ".end");

				tokenType = ENDOFFILE;
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is an assembly directive
			else if (tokenType == DIRECTIVE)
			{
				if (sentenceStarted == true)
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

				sentenceStarted = true;
				currentDirective = token;

				if (token == ".section")
				{
					sectionNameExpected = true;
				}
				else if (token == ".byte")
				{
					if (currentSection == -1)
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					atLeastOneMoreSymbolOrLiteralExpected = true;
				}
				else if (token == ".word")
				{
					if (currentSection == -1)
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					atLeastOneMoreSymbolOrLiteralExpected = true;	
				}
				else if (token == ".skip")
				{
					if (currentSection == -1)
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

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
			// check if token is an assembly instruction
			else if (tokenType == INSTRUCTION)
			{
				if (sentenceStarted == true || currentSection == -1)
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

				// fulfill the waiting labels
				for (unsigned int i = 0; i < labelWaitingQueue.size(); i++)
					if (!symbolTable.setSymbolValue(labelWaitingQueue[i], currentSection, locationCounter))
						throw DuplicateDefinition(srcFileName, parser->getLineNumber(), labelWaitingQueue[i]);
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
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					newLineExpected = true;
				}
				else if (flags & Parser::INS_SET1)												// 1 operand branch instructions
				{
					// these instructions do not have 'b' or 'w' suffix variants
					if (flags & Parser::INSTRUCTION_MOD)
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					sentenceStarted = true;
					branchOperandExpected = true;
					oneMoreOperand = true;
				}
				else if (flags & Parser::INS_SET2)												// 1 operand non-branch instructions
				{
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
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is + or -
			else if (tokenType == ARITHMETIC)
			{
				// the sentence was started by .equ directive - an expression is expected as the second operand
				if (expressionExpected && expressionStatus == 3)
				{
					if (token == "+")
						expressionStatus = 1;
					else
						expressionStatus = 2;
				}
				else
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is a label - <symbol>:
			else if (tokenType == LABEL)
			{
				if (sentenceStarted && !sectionNameExpected)
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

				int index = symbolTable.getSymbolIndex(firstGroup);

				if (sectionNameExpected)
				{
					if (symbolTable[index].isDefined())
						throw DuplicateDefinition(srcFileName, parser->getLineNumber(), token);

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
				// if one of these flags is set - means the sentence was started by a directive that requires one or more symbols - .global, .extern, .equ, .byte, .word
				if (atLeastOneMoreSymbolExpected || atLeastOneMoreSymbolOrLiteralExpected)
				{
					// for .global, .extern, .byte, .word and .equ only 'pure' symbols are allowed - NO STARS/DOLLARS/PARENTHESES
					if (flags & (Parser::DOLLAR | Parser::STAR | Parser::PARENTHESES))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// check which directive was used - different directives apply different logic
					// if .equ was used it means we prepare for an expression for this symbol
					if (currentDirective == ".equ")
					{
						// a comma must be included since an expression is a mandatory second operand
						if (!(flags & Parser::COMMA))
							throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

						// declare symbol
						symbolTable.getSymbolIndex(firstGroup);

						// setup for upcoming expression
						incalculableSymbols.push_back(make_pair(firstGroup, Expression()));
						
						atLeastOneMoreSymbolExpected = false;
						expressionExpected = true;
						expressionStatus = 0;			// expression empty
					}
					// if .global or .extern were used it means we add the symbol to the array of global symbols
					else if (currentDirective == ".global" || currentDirective == ".extern")
					{
						// add the symbol to the array of global symbols
						globalSymbols.emplace_back(firstGroup, currentDirective == ".global" ? GLOBAL : EXTERN);

						// check if a comma was used - if it was not used it means that the sentence is over
						if (!(flags & Parser::COMMA))
						{
							atLeastOneMoreSymbolExpected = false;
							sentenceStarted = false;
							newLineExpected = true;
							currentDirective = "";
						}
					}
					// if .byte or .word were used it means we need to add a number of 'all 0' bytes to the current section and mark the location with a relocation record
					else if (currentDirective == ".byte" || currentDirective == ".word")
					{
						// fulfill the waiting labels
						for (unsigned int i = 0; i < labelWaitingQueue.size(); i++)
							if (!symbolTable.setSymbolValue(labelWaitingQueue[i], currentSection, locationCounter))
								throw DuplicateDefinition(srcFileName, parser->getLineNumber(), labelWaitingQueue[i]);
						labelWaitingQueue.clear();

						int numOfBytes = currentDirective == ".byte" ? 1 : 2;
						// get the index of the used symbol
						int index = symbolTable.getSymbolIndex(firstGroup);

						// add a relocation record for the current section using the index of the used symbol
						vector<RelocationRecord> *rel = symbolTable[currentSection].getRelocationRecords();
						rel->emplace_back(locationCounter, (numOfBytes == 1 ? R_386_8 : R_386_16), index);

						// add a number of 'all 0' bytes to the current sections code
						for (int i=0;i<numOfBytes;i++)
						{
							symbolTable[currentSection].getSectionCode()->addByte(0);
							locationCounter++;
						}

						// check if a comma was used - if it was not used it means that the sentence is over
						if (!(flags & Parser::COMMA))
						{
							atLeastOneMoreSymbolExpected = atLeastOneMoreSymbolOrLiteralExpected = false;
							sentenceStarted = false;
							newLineExpected = true;
							currentDirective = "";
						}
					}
				}
				// the sentence was started with an instruction that requires one or two operands (symbols/literals/registers/combo)
				else if (oneMoreOperand || twoMoreOperands)
				{
					// this is the last operand, no ',' needed
					if (oneMoreOperand && (flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// expecting one more operand, need a ','
					if (twoMoreOperands && !(flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// check if operand is compatible with a branch instruction or non-branch instruction
					if (branchOperandExpected && (flags & Parser::DOLLAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);
					if (!branchOperandExpected && (flags & Parser::STAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);

					// check for unexpected immediate operand - only non-branch instructions have this requirement
					if (nonImediateOperand && (reverseOrderOperands ? twoMoreOperands : oneMoreOperand) && (flags & Parser::DOLLAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);

					// calculate the operand description
					unsigned char operandDescription = 0;
					if (branchOperandExpected ? !(flags & (Parser::DOLLAR | Parser::STAR | Parser::PARENTHESES)) : (flags & Parser::DOLLAR))	// check the addressing mode
						operandDescription |= 0 | 1;	// immediate operand - or 1 to indicate it is a relative jump (to differantiate from branch instruction with immediate literal operand)
					else
						operandDescription |= 128;		// direct memory operand

					// write operand description to section code
					symbolTable[currentSection].getSectionCode()->addByte(operandDescription);
					locationCounter++;

					// determine number of bytes needed for operand address/value
					int numOfBytes = oneByteOperands ? 1 : 2;

					// add a relocation record
					int index = symbolTable.getSymbolIndex(firstGroup);
					// go for pc-relative operand addressing only when using a branch instruction with immediate operand
					bool pcRelativeAddressing = branchOperandExpected && !(flags & Parser::STAR);
					RelocationType relType = (numOfBytes == 1 ? (pcRelativeAddressing ? R_386_PC8 : R_386_8) : (pcRelativeAddressing ? R_386_PC16 : R_386_16));
					symbolTable[currentSection].getRelocationRecords()->emplace_back(locationCounter, relType, index);

					// write the operand addresss/value - 0 ('-numOfBytes' for pc-relative) for now, real value will be written when relocation records are processed
					unsigned char bytes[numOfBytes];
					LittleEndian::intToLittleEndianChar((pcRelativeAddressing ? -numOfBytes : 0), numOfBytes, bytes);
					symbolTable[currentSection].getSectionCode()->addInstruction(numOfBytes, bytes);
					locationCounter += numOfBytes;

					// prepare for next operand
					if (twoMoreOperands)
					{
						twoMoreOperands = false;
						oneMoreOperand = true;
					}
					// prepare for next sentence
					else if (oneMoreOperand)
					{
						oneMoreOperand = false;
						branchOperandExpected = false;
						nonImediateOperand = false;
						reverseOrderOperands = false;
						oneByteOperands = false;

						sentenceStarted = false;
						newLineExpected = true;
					}
				}
				// the sentence was started by .equ directive - an expression is expected as the second operand
				else if (expressionExpected && (expressionStatus == 0 || expressionStatus == 1 || expressionStatus == 2))
				{
					BPAction action = (expressionStatus == 2) ? MINUS : PLUS;
					incalculableSymbols.back().second.add(firstGroup, action);
					expressionStatus = 3;
				}
				// if this branch is reached - no one requires a symbol
				else
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is a literal operand - multiple addressing modes possible - denoted by flags
			else if (tokenType == LITERAL)
			{
				// sentence was started by a directive that requires either one or one or more literals - .byte, .word or .skip
				if (oneLiteralExpected || atLeastOneMoreSymbolOrLiteralExpected)
				{
					// only 'pure' literals are expected after .byte, .word or .skip
					if (flags & (Parser::DOLLAR | Parser::STAR | Parser::PARENTHESES))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// no more operands are expected, no need for ','
					if (oneLiteralExpected && (flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// fulfill the waiting labels
					for (unsigned int i = 0; i < labelWaitingQueue.size(); i++)
						if (!symbolTable.setSymbolValue(labelWaitingQueue[i], currentSection, locationCounter))
							throw DuplicateDefinition(srcFileName, parser->getLineNumber(), labelWaitingQueue[i]);
					labelWaitingQueue.clear();

					// cast literal into integer, determine number of new bytes
					int intToken = stoi(firstGroup);
					int numOfBytes = currentDirective == ".byte" ? 1 : (currentDirective == ".word" ? 2 : intToken);

					// write selected number of 0s
					if (currentDirective == ".skip")
					{
						// write 0s into section code
						for (int i = 0; i < numOfBytes; i++)
							symbolTable[currentSection].getSectionCode()->addByte(0);
						locationCounter += numOfBytes;
					}
					// write selected number in litte-endian format
					else
					{
						// format number into little-endian
						unsigned char bytes[numOfBytes];
						LittleEndian::intToLittleEndianChar(intToken, numOfBytes, bytes);

						//write into section code
						symbolTable[currentSection].getSectionCode()->addInstruction(numOfBytes, bytes);
						locationCounter += numOfBytes;
					}

					// check if a comma was used - if it was not used it means that the sentence is over
					if (!(flags & Parser::COMMA))
					{
						oneLiteralExpected = atLeastOneMoreSymbolOrLiteralExpected = false;
						sentenceStarted = false;
						newLineExpected = true;
						currentDirective = "";
					}
				}
				// sentence was started by an instruction that requires one or two operands (symbols/literals/registers/combos)
				else if (oneMoreOperand || twoMoreOperands)
				{
					
					// this is the last operand, no ',' needed
					if (oneMoreOperand && (flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// expecting one more operand, need a ','
					if (twoMoreOperands && !(flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// check if operand is compatible with a branch instruction or non-branch instruction
					if (branchOperandExpected && (flags & Parser::DOLLAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);
					if (!branchOperandExpected && (flags & Parser::STAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);

					// check for unexpected immediate operand - only non-branch instructions have this requirement
					if (nonImediateOperand && (reverseOrderOperands ? twoMoreOperands : oneMoreOperand) && (flags & Parser::DOLLAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);

					// calculate the operand description
					unsigned char operandDescription = 0;
					if (branchOperandExpected ? !(flags & (Parser::DOLLAR | Parser::STAR | Parser::PARENTHESES)) : (flags & Parser::DOLLAR))	// check the addressing mode
						operandDescription |= 0;	// immediate operand
					else
						operandDescription |= 128;	// direct memory operand

					// write operand description to section code
					symbolTable[currentSection].getSectionCode()->addByte(operandDescription);
					locationCounter++;

					// determine number of bytes needed for operand address/value
					int numOfBytes = oneByteOperands ? 1 : 2;

					// cast literal into little-endian
					unsigned char bytes[numOfBytes];
					int intToken = stoi(firstGroup);
					LittleEndian::intToLittleEndianChar(intToken, numOfBytes, bytes);

					// write the operand address/value into section code
					symbolTable[currentSection].getSectionCode()->addInstruction(numOfBytes, bytes);
					locationCounter += numOfBytes;

					// prepare for next operand
					if (twoMoreOperands)
					{
						twoMoreOperands = false;
						oneMoreOperand = true;
					}
					// prepare for next sentence
					else if (oneMoreOperand)
					{
						oneMoreOperand = false;
						branchOperandExpected = false;
						nonImediateOperand = false;
						reverseOrderOperands = false;
						oneByteOperands = false;

						sentenceStarted = false;
						newLineExpected = true;
					}
				}
				// the sentence was started by .equ directive - an expression is expected as the second operand
				else if (expressionExpected && (expressionStatus == 0 || expressionStatus == 1 || expressionStatus == 2))
				{
					BPAction action = (expressionStatus == 2) ? MINUS : PLUS;
					int intToken = stoi(firstGroup);
					incalculableSymbols.back().second.add(intToken, action);
					expressionStatus = 3;
				}
				// if this branch is reached - no one requires a literal
				else
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is a register operand - multiple addressing modes possible - denoted by flags
			else if (tokenType == REGISTER)
			{
				// sentence was started by an instruction that requires one or two operands (symbols/literals/registers/combos)
				if (oneMoreOperand || twoMoreOperands)
				{
					// check if comma presence meets requirement
					if (oneMoreOperand && (flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
					if (twoMoreOperands && !(flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// check if operand is compatible with a branch instruction or non-branch instruction
					if (branchOperandExpected && !(flags & Parser::STAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);
					if (!branchOperandExpected && (flags & Parser::STAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);

					// check if operand of requested size was provided
					if (oneByteOperands && !(flags & Parser::PARENTHESES) && !(firstGroup[firstGroup.length() - 1] == 'l' || firstGroup[firstGroup.length() - 1] == 'h'))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);
					if (!(oneByteOperands && !(flags & Parser::PARENTHESES)) && (firstGroup[firstGroup.length() - 1] == 'l' || firstGroup[firstGroup.length() - 1] == 'h'))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);

					// extract register number [0-7,15]
					unsigned char regNum = 0;
					if (firstGroup[0] == 'r')
						regNum = firstGroup[1] - '0';
					else if (firstGroup == "pc")
						regNum = 7;
					else if (firstGroup == "psw")
						regNum = 15;
					regNum <<= 1;

					// extract l/h info
					unsigned char isHigh = 0;
					if (firstGroup[firstGroup.length() - 1] == 'h')
						isHigh = 1;

					// calculate the operand description
					unsigned char operandDescription = regNum | isHigh;
					if (flags & Parser::PARENTHESES)	// check the addressing mode
						operandDescription |= 64;	// register indirect
					else
						operandDescription |= 32;	// register direct

					// write operand description to section code
					symbolTable[currentSection].getSectionCode()->addByte(operandDescription);
					locationCounter++;

					// prepare for next operand
					if (twoMoreOperands)
					{
						twoMoreOperands = false;
						oneMoreOperand = true;
					}
					// prepare for next sentence
					else if (oneMoreOperand)
					{
						oneMoreOperand = false;
						branchOperandExpected = false;
						nonImediateOperand = false;
						reverseOrderOperands = false;
						oneByteOperands = false;

						sentenceStarted = false;
						newLineExpected = true;
					}
				}
				// if this branch is reached - no one requires a register
				else
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is operand with indirect-offset addressing - address = symbol + register
			else if (tokenType == SYMBOL_AND_REGISTER)
			{
				// sentence was started by an instruction that requires one or two operands (symbols/literals/registers/combos)
				if (oneMoreOperand || twoMoreOperands)
				{
					// check if comma presence meets requirement
					if (oneMoreOperand && (flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
					if (twoMoreOperands && !(flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// check if operand is compatible with a branch instruction or non-branch instruction
					if (branchOperandExpected && !(flags & Parser::STAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);
					if (!branchOperandExpected && (flags & Parser::STAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);

					// extract register number [0-7,15]
					unsigned char regNum = 0;
					if (secondGroup[0] == 'r')
						regNum = secondGroup[1] - '0';
					else if (secondGroup == "pc")
						regNum = 7;
					else if (secondGroup == "psw")
						regNum = 15;
					regNum <<= 1;

					// calculate the operand description
					unsigned char operandDescription = 96 | regNum;		// register indirect with offset

					// write operand description to section code
					symbolTable[currentSection].getSectionCode()->addByte(operandDescription);
					locationCounter++;

					// add a relocation record
					int index = symbolTable.getSymbolIndex(firstGroup);
					symbolTable[currentSection].getRelocationRecords()->emplace_back(locationCounter, R_386_16, index);

					// write the offset address - 0 for now, real value will be written when relocation records are processed
					symbolTable[currentSection].getSectionCode()->addByte(0);
					symbolTable[currentSection].getSectionCode()->addByte(0);
					locationCounter += 2;

					// prepare for next operand
					if (twoMoreOperands)
					{
						twoMoreOperands = false;
						oneMoreOperand = true;
					}
					// prepare for next sentence
					else if (oneMoreOperand)
					{
						oneMoreOperand = false;
						branchOperandExpected = false;
						nonImediateOperand = false;
						reverseOrderOperands = false;
						oneByteOperands = false;

						sentenceStarted = false;
						newLineExpected = true;
					}
				}
				// if this branch is reached - no one requires a symbol-register combo
				else
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// check if token is operand with indirect-offset addressing - address = literal + register
			else if (tokenType == LITERAL_AND_REGISTER)
			{
				// sentence was started by an instruction that requires one or two operands (symbols/literals/registers/combos)
				if (oneMoreOperand || twoMoreOperands)
				{
					// check if comma presence meets requirement
					if (oneMoreOperand && (flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
					if (twoMoreOperands && !(flags & Parser::COMMA))
						throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);

					// check if operand is compatible with a branch instruction or non-branch instruction
					if (branchOperandExpected && !(flags & Parser::STAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);
					if (!branchOperandExpected && (flags & Parser::STAR))
						throw InvalidOperandForInstruction(srcFileName, parser->getLineNumber(), token);

					// extract register number [0-7,15]
					unsigned char regNum = 0;
					if (secondGroup[0] == 'r')
						regNum = secondGroup[1] - '0';
					else if (secondGroup == "pc")
						regNum = 7;
					else if (secondGroup == "psw")
						regNum = 15;
					regNum <<= 1;

					// calculate the operand description
					unsigned char operandDescription = 96 | regNum;		// register indirect with offset

					// write operand description to section code
					symbolTable[currentSection].getSectionCode()->addByte(operandDescription);
					locationCounter++;

					// cast literal into little-endian
					unsigned char bytes[2];
					int intToken = stoi(firstGroup);
					LittleEndian::intToLittleEndianChar(intToken, 2, bytes);

					// write the offset address into section code
					symbolTable[currentSection].getSectionCode()->addInstruction(2, bytes);
					locationCounter += 2;

					// prepare for next operand
					if (twoMoreOperands)
					{
						twoMoreOperands = false;
						oneMoreOperand = true;
					}
					// prepare for next sentence
					else if (oneMoreOperand)
					{
						oneMoreOperand = false;
						branchOperandExpected = false;
						nonImediateOperand = false;
						reverseOrderOperands = false;
						oneByteOperands = false;

						sentenceStarted = false;
						newLineExpected = true;
					}
				}
				// if this branch is reached - no one requires a symbol-register combo
				else
					throw UnexpectedTokenException(srcFileName, parser->getLineNumber(), token);
			}
			/*  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  **** ****  */
			// this should never trigger as I have covered all of the cases
			else
				throw UnrecognizableTokenException(srcFileName, parser->getLineNumber(), token);
		}
		// END OF SINGLE PASS

		// STEP2: try to calculate expressions (.equ directive)
		int prevExCount = -1, exCount = incalculableSymbols.size();
		// repeat while expressions are getting solved
		while (exCount > 0 && exCount != prevExCount)
		{
			prevExCount = exCount;

			int symbolValue, symbolSectionNum;
			char outcome;

			// go through all of the expressions and try to calculate at least one
			for (unsigned int i = 0; i < incalculableSymbols.size();)
			{
				// try to calculate the expression
				outcome = incalculableSymbols[i].second.tryToCalculate(symbolTable, &symbolValue, &symbolSectionNum);

				// successful calculation
				if (outcome == 0)
				{
					// get symbol table entry for this symbol
					int index = symbolTable.getSymbolIndex(incalculableSymbols[i].first);

					// check if the symbol was already defined
					if (symbolTable[index].isDefined())
						throw DuplicateDefinition(srcFileName, -1, incalculableSymbols[i].first);

					// define the symbol
					symbolTable[index].setDefined(true);
					symbolTable[index].setValue(symbolValue);
					symbolTable[index].setSection(symbolSectionNum);

					// remove the now solved expression
					exCount--;
					incalculableSymbols.erase(incalculableSymbols.begin() + i);
				}
				// undefined symbol(s) - leave this expression for later
				else if (outcome == 1)
				{
					i++;
				}
				// result is non-relocatable - invalid combination of symbols from multiple sections used
				else if (outcome == 2)
					throw NonRelocatableExpression(srcFileName, -1, incalculableSymbols[i].first);
			}
		}

		// circular dependencies detected
		if (exCount > 0)
		{
			// itemize all symbols that have circular dependencies
			string circularDependentSymbols = "{ ";
			for (unsigned int i = 0; i < incalculableSymbols.size() - 1; i++)
				circularDependentSymbols += incalculableSymbols[i].first + ", ";
			circularDependentSymbols += incalculableSymbols.back().first + " }";

			throw CircularDependencies(srcFileName, -1, circularDependentSymbols);
		}

		// STEP3: resolve global symbols table
		// go through all the exported symbols and check if they are defined - or for imported symbols, check if they are NOT defined
		for (unsigned int i = 0; i < globalSymbols.size(); i++)
		{
			int index = symbolTable.getSymbolIndex(globalSymbols[i].getName());
			GlobalType gt = globalSymbols[i].getType();

			if (gt == GLOBAL)
			{
				if (!symbolTable[index].isDefined())
					throw MissingDefinition(srcFileName, -1, symbolTable[index].getName());
			}
			else
			{
				if (symbolTable[index].isDefined())
					throw DuplicateDefinition(srcFileName, -1, symbolTable[index].getName());
			}

			symbolTable[index].setGlobal(true);
		}

		// STEP4: update relocation records - remove same-section jumps, process local symbol records
		for (int i = 0; i < symbolTable.size(); i++)
		{
			Section *sec = symbolTable[i].getSectionCode();
			vector<RelocationRecord> *relRecords = symbolTable[i].getRelocationRecords(false);

			// check if symbol is a section and has relocation records (sometimes a section doesn't)
			if (relRecords != nullptr)
			{
				// go through all the relocation records and process each accordingly
				for (unsigned int j = 0; j < relRecords->size();)
				{
					// extract data for easier use
					int relOffset = relRecords->at(j).getOffset();
					RelocationType relType = relRecords->at(j).getType();
					int relSymbolIndex = relRecords->at(j).getSymbolIndex();

					// if same-section pc relative - write it into code
					if ((relType == R_386_PC16 || relType == R_386_PC8) && symbolTable[relSymbolIndex].getSection() == i)
					{
						// determine number of bytes that are used
						int numOfBytes = (relType == R_386_PC16 ? 2 : 1);

						// read the old value
						unsigned char bytes[numOfBytes];
						sec->readBytes(relOffset, numOfBytes, bytes);

						// cast the old value to integer
						int oldValue = LittleEndian::charLittleEndianToInt(numOfBytes, bytes);

						// calculate new pc relative value
						int newValue = oldValue + symbolTable[relSymbolIndex].getValue() - relOffset;

						// cast the new value back to little endian char array
						LittleEndian::intToLittleEndianChar(newValue, numOfBytes, bytes);

						// write new value into code
						sec->overwriteBytes(relOffset, numOfBytes, bytes);

						// delete the relocation record
						relRecords->erase(relRecords->begin() + j);
					}
					// if symbol is global - leave the record as is
					else if (symbolTable[relSymbolIndex].isGlobal())
						j++;
					// if symbol is absolute - write it into code (only works for absolute relocation records)
					else if ((relType == R_386_16 || relType == R_386_8) && symbolTable[relSymbolIndex].isDefined() && symbolTable[relSymbolIndex].getSection() == 0)
					{
						// determine number of bytes that are used
						int numOfBytes = (relType == R_386_16 ? 2 : 1);

						// read the old value
						unsigned char bytes[numOfBytes];
						sec->readBytes(relOffset, numOfBytes, bytes);

						// cast the old value to integer
						int oldValue = LittleEndian::charLittleEndianToInt(numOfBytes, bytes);

						// calculate the new value
						int newValue = oldValue + symbolTable[relSymbolIndex].getValue();

						// cast the new value back to little endian char array
						LittleEndian::intToLittleEndianChar(newValue, numOfBytes, bytes);

						// write new value into code
						sec->overwriteBytes(relOffset, numOfBytes, bytes);

						// delete the relocation record
						relRecords->erase(relRecords->begin() + j);
					}
					// if symbol is local - replace with section symbol
					else
					{
						// determine number of bytes that are used
						int numOfBytes = (relType == R_386_16 || relType == R_386_PC16 ? 2 : 1);

						// read the old value
						unsigned char bytes[numOfBytes];
						sec->readBytes(relOffset, numOfBytes, bytes);

						// cast the old value to integer
						int oldValue = LittleEndian::charLittleEndianToInt(numOfBytes, bytes);

						// calculate the new value
						int newValue = oldValue + symbolTable[relSymbolIndex].getValue();

						// cast the new value back to little endian char array
						LittleEndian::intToLittleEndianChar(newValue, numOfBytes, bytes);

						// write new value into code
						sec->overwriteBytes(relOffset, numOfBytes, bytes);

						// update the relocation record
						relRecords->at(j).setSymbolIndex(symbolTable[relSymbolIndex].getSection());

						// proceed to the next one
						j++;
					}
				}
			}
		}

		// STEP5: reorder symbol table so section symbols are at the beginning of the table
		map<int, int> symTabRemap;
		symTabRemap[0] = 0;
		int remapNextSpot = 1;

		// remap all sections to the beginning
		for (int i = 1; i < symbolTable.size(); i++)
			if (symbolTable[i].getSectionCode() != nullptr)
				symTabRemap[i] = remapNextSpot++;

		// remap the rest of the symbols
		for (int i = 1; i < symbolTable.size(); i++)
			if (symbolTable[i].getSectionCode() == nullptr)
				symTabRemap[i] = remapNextSpot++;

		// apply new order to relocation records
		for (int i = 0; i < symbolTable.size(); i++)
		{
			vector<RelocationRecord> *relRecords = symbolTable[i].getRelocationRecords(false);

			if (relRecords != nullptr)
			{
				for (unsigned int j = 0; j < relRecords->size(); j++)
					(*relRecords)[j].setSymbolIndex(symTabRemap[(*relRecords)[j].getSymbolIndex()]);
			}
		}

		// apply new order to symbol table fields
		for (int i = 0; i < symbolTable.size(); i++)
		{
			symbolTable[i].setIndex(symTabRemap[symbolTable[i].getIndex()]);
			symbolTable[i].setSection(symTabRemap[symbolTable[i].getSection()]);
		}

		// reorder symbol table 
		for (int i = 0; i < symbolTable.size();)
		{
			// check if symbol needs moving
			if (symbolTable[i].getIndex() != i)
			{
				// swap the symbol in the current spot with the symbol in the target spot
				int targetIndex = symbolTable[i].getIndex();

				SymbolTableEntry tmp = symbolTable[targetIndex];
				symbolTable[targetIndex] = symbolTable[i];
				symbolTable[i] = tmp;
			}
			// symbol does not need moving, onto the next one
			else
				i++;
		}

		// STEP6: output to dst file
		// TODO: maybe a binary form for the emulator
		ofstream out(dstFileName);

		ElfWriter *elf = new ElfWriter(symbolTable);
		out << *elf;
	}
	catch (MyException& e)
	{
		//collect garbage
		delete parser;
		parser = nullptr;

		cout << e;

		return 4;
	}

	// collect garbage
	delete parser;
	parser = nullptr;

	return 0;
}