#include "exceptions.h"

using namespace std;

/* MyException */

MyException::MyException(string fileName, int lineNum)
{ 
	this->fileName = fileName;
	this->lineNum = lineNum;
}

string MyException::location() const
{
	if (lineNum < 0)
		return fileName;
	return fileName + " at line " + to_string(lineNum);
}

ostream& operator<<(ostream& out, const MyException& me)
{
	out << endl << "\033[1;31m" << endl;
	out << me.name() << " thrown from " << me.location() << endl;
	out << me.message() << endl;
	out << "\033[0m" << endl;
	return out;
}

/* MyExceptionWithToken */

MyExceptionWithToken::MyExceptionWithToken(string fileName, int lineNum, string token) : MyException(fileName, lineNum)
{
	this->token = token;
}

/* UnrecognizableTokenException */

UnrecognizableTokenException::UnrecognizableTokenException(string fileName, int lineNum, string token) : MyExceptionWithToken(fileName, lineNum, token) {}

string UnrecognizableTokenException::name() const
{
	return "UnrecognizableTokenException";
}

string UnrecognizableTokenException::message() const
{
	return "Unrecognizable token found: '" + token + "'. Can not parse!";
}

/* UnexpectedTokenException */


UnexpectedTokenException::UnexpectedTokenException(string fileName, int lineNum, string token) : MyExceptionWithToken(fileName, lineNum, token) {}

string UnexpectedTokenException::name() const
{
	return "UnexpectedTokenException";
}

string UnexpectedTokenException::message() const
{
	return "Unexpected token found: '" + token + "'. Was expecting something else.";
}

/* InvalidOperandForInstruction */

InvalidOperandForInstruction::InvalidOperandForInstruction(string fileName, int lineNum, string token) : MyExceptionWithToken(fileName, lineNum, token) {}

string InvalidOperandForInstruction::name() const
{
	return "InvalidOperandForInstruction";
}

string InvalidOperandForInstruction::message() const
{
	return "Invalid instruction and operand combination! Could not parse: '" + token + "' with current instruction.";
}

/* DuplicateDefinition */

DuplicateDefinition::DuplicateDefinition(string fileName, int lineNum, string token) : MyExceptionWithToken(fileName, lineNum, token) {}

string DuplicateDefinition::name() const
{
	return "DuplicateDefinition";
}

string DuplicateDefinition::message() const
{
	return "Duplicate definition of symbol: '" + token + "'.";
}

/* MissingDefinition */

MissingDefinition::MissingDefinition(string fileName, int lineNum, string token) : MyExceptionWithToken(fileName, lineNum, token) {}

string MissingDefinition::name() const
{
	return "MissingDefinition";
}

string MissingDefinition::message() const
{
	return "Missing definition of symbol: '" + token + "'.";
}

/* NonRelocatableExpression */

NonRelocatableExpression::NonRelocatableExpression(string fileName, int lineNum, string token) : MyExceptionWithToken(fileName, lineNum, token) {}

string NonRelocatableExpression::name() const
{
	return "NonRelocatableExpression";
}

string NonRelocatableExpression::message() const
{
	return "Expression used for calculation of symbol: '" + token + "' is invalid! The expression must resolve to either an absolute or relocatable result.";
}

/* CircularDependencies */

CircularDependencies::CircularDependencies(string fileName, int lineNum, string token) : MyExceptionWithToken(fileName, lineNum, token) {}

string CircularDependencies::name() const
{
	return "CircularDependencies";
}

string CircularDependencies::message() const
{
	return "Circular dependencies detected between the following symbols: '" + token + "'. Circular dependencies are not permitted!";
}