#include "parser.h"

using namespace std;

/* STATIC FIELDS */

char Parser::DOLLAR = 1;
char Parser::STAR = 2;
char Parser::PARENTHESES = 4;
char Parser::COMMA = 8;
char Parser::NEW_LINE = 128;

unordered_set<string> Parser::directiveSet
{
	".global", ".extern", ".section", ".end", ".byte", ".word", ".skip", ".equ"
};

unordered_set<string> Parser::instructionSet
{
	"halt", "iret", "ret", "int", "call", "jmp", "jeq", "jne", "jgt", "push", "pop",
	"xchg", "mov", "add", "sub", "mul", "div", "cmp", "not", "and", "or", "xor", "test", "shl", "shr"
};

/* 	CONSTRUCTOR */

Parser::Parser(string fileName)
{
	this->file = new ifstream(fileName);
	this->line = nullptr;
	this->lineNum = 0;
}

/* DESTRUCTOR */

Parser::~Parser()
{
	delete line;
	line = nullptr;
	file->close();
	delete file;
	file = nullptr;
}

/* PUBLIC METHODS */

TokenType Parser::getNextToken(string *result, string *firstGroup, string *secondGroup, char *flags)
{
	// reset all flags
	*flags = 0;

	// get next line if no line is loaded
	if (line == nullptr)
	{
		if (!getNextLine())
			return ENDOFFILE;
		*flags |= NEW_LINE;
	}

	// get next token - possibly scanning through multiple empty lines
	string token;
	while (!(*line >> token))
	{
		if (!getNextLine())
			return ENDOFFILE;
		*flags |= NEW_LINE;
	}

	// perform crude analysis of token
	TokenType ret = crudeAnalysis(token);

	// toss the rest of the line if token is a comment
	if (ret == COMMENT)
	{
		delete line;
		line = nullptr;
	}
	// further analysis needed
	else if (ret == OTHER)
	{
		// perform more analysis
		ret = moreAnalysis(token, firstGroup, secondGroup, flags);
	}

	*result = token;
	return ret;
}

int Parser::getLineNumber()
{
	return lineNum;
}

/* PRIVATE METHODS */

bool Parser::getNextLine()
{
	delete line;
	line = nullptr;

	string newLine;
	if (getline(*file, newLine))
	{
		line = new istringstream(newLine);
		lineNum++;
		return true;
	}

	return false;
}

/* 
 *	Analyzes the token and assigns it into one of the following groups:
 *		DIRECTIVE
 *		INSTRUCTION
 *		ARITHMETIC
 *		COMMENT
 *		OTHER
 */
TokenType Parser::crudeAnalysis(string token)
{
	// regex directivePattern("\\.[a-zA-Z_][a-zA-Z_0-9]*");
	regex arithmeticPattern("[+-]");						// might evolve into something more complex - recognizing whole expressions
	regex commentPattern(";.*");

	if (directiveSet.find(token) != directiveSet.end())
	{
		return DIRECTIVE;
	}

	if (instructionSet.find(token) != instructionSet.end()
		|| ((token[token.length()-1] == 'b' || token[token.length()-1] == 'w') && instructionSet.find(token.substr(0, token.length()-1)) != instructionSet.end()))
	{
		return INSTRUCTION;
	}

	if (regex_match(token, arithmeticPattern))
	{
		return ARITHMETIC;
	}

	if (regex_match(token, commentPattern))
	{
		return COMMENT;
	}

	/* might be
		- a symbol
		- a literal
		- a register
		- some combination of previous items
		- junk
	*/
	return OTHER;
}

/* 
 *	Analyzes the token and recognizes symbols, literals and registers. Assigns the token to one of the following groups:
 *		LABEL
 *		SYMBOL
 *		LITERAL
 *		REGISTER
 *		SYMBOL_AND_REGISTER
 *		LITERAL_AND_REGISTER
 *		JUNK
 */
TokenType Parser::moreAnalysis(string token, string* firstGroup, string* secondGroup, char* flags)
{
	// regex setup
	string symbolPatternString = "([a-zA-Z_][a-zA-Z_0-9]*)";
	string literalPatternString = "(0|[1-9]\\d*)";
	string registerPatternString = "((?:r[0-7]|pc|psw)[lh]?)";
	
	regex labelPattern(symbolPatternString + ":");
	regex symbolPattern(symbolPatternString);
	regex literalPattern(literalPatternString);

	regex dollarSymbolPattern("\\$" + symbolPatternString);
	regex dollarLiteralPattern("\\$" + literalPatternString);
	regex starSymbolPattern("\\*" + symbolPatternString);
	regex starLiteralPattern("\\*" + literalPatternString);
	
	regex registerPattern("%" + registerPatternString);
	regex indirectRegisterPattern("\\(%" + registerPatternString + "\\)");
	regex starRegisterPattern("\\*%" + registerPatternString);
	regex starIndirectRegisterPattern("\\*\\(%" + registerPatternString + "\\)");

	regex literalAndRegisterPattern(literalPatternString + "\\(%" + registerPatternString + "\\)");
	regex symbolAndRegisterPattern(symbolPatternString + "\\(%" + registerPatternString + "\\)");
	regex starLiteralAndRegisterPattern("\\*" + literalPatternString + "\\(%" + registerPatternString + "\\)");
	regex starSymbolAndRegisterPattern("\\*" + symbolPatternString + "\\(%" + registerPatternString + "\\)");

	smatch matches;

	// remove ','
	if (token[token.length() - 1] == ',')
	{
		*flags |= COMMA;
		token = token.substr(0, token.length()-1);
	}

	// some arrays to make automating these ~15 checks easier
	regex *patternArray[] = 
	{
		&labelPattern, 					&symbolPattern, 			&literalPattern, 			&dollarSymbolPattern, 			&dollarLiteralPattern,
		&starSymbolPattern, 			&starLiteralPattern, 		&registerPattern,			&indirectRegisterPattern,		&starRegisterPattern,
		&starIndirectRegisterPattern, 	&literalAndRegisterPattern,	&symbolAndRegisterPattern,	&starLiteralAndRegisterPattern,	&starSymbolAndRegisterPattern
	};
	bool twoGroups[] =
	{
		false, 							false,						false,						false,							false,
		false, 							false,						false,						false,							false,
		false, 							true, 						true, 						true, 							true
	};
	char flagsArray[] =
	{
		0,								0, 							0, 							DOLLAR,							DOLLAR,
		STAR, 							STAR, 						0,							PARENTHESES,					STAR,
		STAR | PARENTHESES, 			PARENTHESES, 				PARENTHESES, 				STAR | PARENTHESES,				STAR | PARENTHESES
	};
	TokenType tokenTypeArray[] =
	{
		LABEL,							SYMBOL, 					LITERAL, 					SYMBOL,							LITERAL,
		SYMBOL, 						LITERAL, 					REGISTER,					REGISTER,						REGISTER,
		REGISTER, 						LITERAL_AND_REGISTER, 		SYMBOL_AND_REGISTER, 		LITERAL_AND_REGISTER,			SYMBOL_AND_REGISTER
	};

	// perform checks until format is identified
	for (int i=0;i<15;i++)
	{
		if (regex_match(token, matches, *patternArray[i]))
		{
			*firstGroup = matches[1].str();

			if (twoGroups[i])
				*secondGroup = matches[2].str();
			else
				*secondGroup = "";

			*flags |= flagsArray[i];

			return tokenTypeArray[i];
		}
	}

	return JUNK;
}