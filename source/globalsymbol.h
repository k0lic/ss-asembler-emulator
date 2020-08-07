#pragma once

#include <string>

using namespace std;

enum GlobalType { GLOBAL, EXTERN };

class GlobalSymbol
{
public:
	GlobalSymbol(string name, GlobalType type);
	string getName();
	GlobalType getType();
private:
	string name;
	GlobalType type;
};