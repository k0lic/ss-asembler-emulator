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

	static char DOLLAR, STAR, PARENTHESES, COMMA, NEW_LINE;
private:
	ifstream *file;
	istringstream *line;
	int lineNum;

	static unordered_set<string> directiveSet;
	static unordered_set<string> instructionSet;

	bool getNextLine();
	TokenType crudeAnalysis(string token);
	TokenType moreAnalysis(string token, string* firstGroup, string* secondGroup, char* flags);
};