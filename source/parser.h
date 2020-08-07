#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_set>

using namespace std;

// enum WordType {DIRECTIVE, INSTRUCTION, LABEL, SYMBOL, LITERAL, ARITHMETIC, COMMENT, UNEXPECTED, ENDOFFILE};
enum TokenType {ENDOFFILE, DIRECTIVE, INSTRUCTION, ARITHMETIC, COMMENT, OTHER, LABEL, SYMBOL, LITERAL, REGISTER, SYMBOL_AND_REGISTER, LITERAL_AND_REGISTER, JUNK};

class Parser
{
public:
	Parser(string fileName);
	~Parser();
	TokenType getNextToken(string* result, string *firstGroup, string *secondGroup, char *flags);
	int getLineNumber();

	static char DOLLAR, STAR, PARENTHESES, COMMA, NEW_LINE, INS_SET0, INS_SET1, INS_SET2, INS_SET3, INS_SET4, INS_SET5, INSTRUCTION_MOD;
private:
	ifstream *file;
	istringstream *line;
	int lineNum;

	static unordered_set<string> directiveSet;
	static unordered_set<string> instructionSet0;						// 0 operand instructions
	static unordered_set<string> instructionSet1;						// 1 operand branch instructions
	static unordered_set<string> instructionSet2;						// 1 operand non-branch instructions
	static unordered_set<string> instructionSet3;						// 2 operand non-branch instructions (normal)
	static unordered_set<string> instructionSet4;						// 2 operand non-branch no-target instructions (result not stored - only psw indicators)
	static unordered_set<string> instructionSet5;						// 2 reverse operand non-branch instruction (first operand is the destination, second is the source)

	bool getNextLine();
	TokenType crudeAnalysis(string token, string* firstGroup, string *secondGroup, char* flags);
	TokenType moreAnalysis(string token, string* firstGroup, string* secondGroup, char* flags);
};