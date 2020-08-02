#include "parser.h"

using namespace std;

/* STATIC FIELDS */

char Parser::DOLLAR = 1;
char Parser::STAR = 2;
char Parser::PARENTHESES = 4;
char Parser::COMMA = 8;
char Parser::NEW_LINE = 128;
char Parser::INS_SET0 = 1;					// 0 operand instructions
char Parser::INS_SET1 = 2;					// 1 operand branch instructions
char Parser::INS_SET2 = 4;					// 1 operand non-branch instructions
char Parser::INS_SET3 = 8;					// 2 operand non-branch instructions (normal)
char Parser::INS_SET4 = 16;					// 2 operand non-branch no-target instructions (result not stored - only psw indicators)
char Parser::INS_SET5 = 32;					// 2 reverse operand non-branch instruction (first operand is the destination, second is the source)
char Parser::INSTRUCTION_MOD = 64;

unordered_set<string> Parser::directiveSet
{
	".global", ".extern", ".section", ".end", ".byte", ".word", ".skip", ".equ"
};

// unordered_set<string> Parser::instructionSet
// {
// 	"halt", "iret", "ret", "int", "call", "jmp", "jeq", "jne", "jgt", "push", "pop",
// 	"xchg", "mov", "add", "sub", "mul", "div", "cmp", "not", "and", "or", "xor", "test", "shl", "shr"
// };

unordered_set<string> Parser::instructionSet0	// 0 operand instructions
{
	"halt", "iret", "ret"
};

unordered_set<string> Parser::instructionSet1	// 1 operand branch instructions
{
	"call", "jmp", "jeq", "jne", "jgt"
};

unordered_set<string> Parser::instructionSet2	// 1 operand non-branch instructions
{
	"int", "push", "pop"
};

unordered_set<string> Parser::instructionSet3	// 2 operand non-branch instructions (normal)
{
	"xchg", "mov", "add", "sub", "mul", "div", "not", "and", "or", "xor", "shl"
};

unordered_set<string> Parser::instructionSet4	// 2 operand non-branch no-target instructions (result not stored - only psw indicators)
{
	"cmp", "test"
};

unordered_set<string> Parser::instructionSet5	// 2 reverse operand non-branch instruction (first operand is the destination, second is the source)
{
	"shr"
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
	TokenType ret = crudeAnalysis(token, firstGroup, secondGroup, flags);

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
TokenType Parser::crudeAnalysis(string token, string* firstGroup, string* secondGroup, char* flags)
{
	// regex directivePattern("\\.[a-zA-Z_][a-zA-Z_0-9]*");
	regex arithmeticPattern("[+-]");						// might evolve into something more complex - recognizing whole expressions
	regex commentPattern(";.*");

	if (directiveSet.find(token) != directiveSet.end())
	{
		return DIRECTIVE;
	}

	// supporting arrays to make going through the 6 different instruction types easier
	unordered_set<string> *instructionSets[] =
	{
		&instructionSet0,	&instructionSet1,	&instructionSet2,	&instructionSet3,	&instructionSet4,	&instructionSet5
	};
	char flagsArray[] =
	{
		INS_SET0, 			INS_SET1, 			INS_SET2, 			INS_SET3, 			INS_SET4, 			INS_SET5
	};

	// setup for checking the 'b' or 'w' instruction suffix
	bool modPossible = token[token.length() - 1] == 'b' || token[token.length() - 1] == 'w';
	string tokenMod = token.substr(0, token.length() - 1);

	// check for the 6 instruction types
	for (int i = 0; i < 6; i++)
	{
		if (instructionSets[i]->find(token) != instructionSets[i]->end())
		{
			*firstGroup = token;
			*flags |= flagsArray[i];
			return INSTRUCTION;
		}
		else if (modPossible && instructionSets[i]->find(tokenMod) != instructionSets[i]->end())
		{
			*firstGroup = tokenMod;
			*flags |= flagsArray[i] | INSTRUCTION_MOD;
			return INSTRUCTION;
		}
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
	string literalPatternString = "(0|-?[1-9]\\d*)";
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
		(char)(STAR | PARENTHESES),		PARENTHESES, 				PARENTHESES, 				(char)(STAR | PARENTHESES),		(char)(STAR | PARENTHESES)
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