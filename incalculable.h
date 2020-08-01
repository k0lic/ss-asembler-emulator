#pragma once

#include <string>
#include "forwardreference.h"

using namespace std;

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