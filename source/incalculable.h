#pragma once

#include <string>

using namespace std;

enum BPAction { PLUS, MINUS };

class Incalculable
{
public:
	Incalculable(string name, BPAction action);
	string getName();
	BPAction getAction();
private:
	string name;
	BPAction action;
};