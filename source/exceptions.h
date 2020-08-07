#pragma once

#include <string>
#include <iostream>

using namespace std;

/* BASE CLASSES */

class MyException
{
public:
	MyException(string fileName, int lineNum);
	virtual string name() const = 0;
	virtual string message() const = 0;
	virtual string location() const;
	friend ostream& operator<<(ostream& out, const MyException& me);
private:
	string fileName;
	int lineNum;
};

class MyExceptionWithToken : public MyException
{
public:
	MyExceptionWithToken(string fileName, int lineNum, string token);
protected:
	string token;
};

class MessageException
{
public:
	MessageException(string message);
	string getMessage() const;
	friend ostream& operator<<(ostream& out, const MessageException& me);
private:
	string message;
};

/* DERIVED CLASSES */

class UnrecognizableTokenException : public MyExceptionWithToken
{
public:
	UnrecognizableTokenException(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
};

class UnexpectedTokenException : public MyExceptionWithToken
{
public:
	UnexpectedTokenException(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
};

class InvalidOperandForInstruction : public MyExceptionWithToken
{
public:
	InvalidOperandForInstruction(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
};

class DuplicateDefinition : public MyExceptionWithToken
{
public:
	DuplicateDefinition(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
};

class MissingDefinition : public MyExceptionWithToken
{
public:
	MissingDefinition(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
};

class NonRelocatableExpression : public MyExceptionWithToken
{
public:
	NonRelocatableExpression(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
};

class CircularDependencies : public MyExceptionWithToken
{
public:
	CircularDependencies(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
};