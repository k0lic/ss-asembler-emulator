#pragma once

#include <string>
#include <iostream>

using namespace std;

/* BASE CLASS */

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

/* DERIVED CLASSES */

class UnrecognizableTokenException : public MyException
{
public:
	UnrecognizableTokenException(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
private:
	string token;
};

class UnexpectedTokenException : public MyException
{
public:
	UnexpectedTokenException(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
private:
	string token;
};

class InvalidOperandForInstruction : public MyException
{
public:
	InvalidOperandForInstruction(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
private:
	string token;
};

class DuplicateDefinition : public MyException
{
public:
	DuplicateDefinition(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
private:
	string token;
};

class MissingDefinition : public MyException
{
public:
	MissingDefinition(string fileName, int lineNum, string token);
	string name() const;
	string message() const;
private:
	string token;
};