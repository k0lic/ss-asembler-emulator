#pragma once

#include <iostream>
#include "processorcontext.h"
#include "littleendian.h"

using namespace std;

enum AddressingMode { IMMEDIATE, REGISTER_DIRECT, REGISTER_INDIRECT, REGISTER_INDIRECT_WITH_OFFSET, MEMORY };

class ProcessorLogic
{
public:
	ProcessorLogic(ProcessorContext *context, unsigned char *memory);

	bool executeNextInstruction();
	bool isHalted();
	void interrupt(unsigned int entryNum);

	static const int MIN_ADDRESS, MAX_ADDRESS;
	// static bool (*instructions[])(int);
private:
	ProcessorContext *context;
	unsigned char *memory;
	bool halted;

	char nextByte();
	unsigned char nextByteu();
	int nextWord();
	unsigned int nextWordu();
	bool nextAddressingMode(AddressingMode *mode, unsigned char *regNum, bool *high);
	int nextOperand(AddressingMode mode, int operandSize);
	void processOperand(int *operand, int *address, int rawOperand, AddressingMode mode, unsigned char regNum, bool high, int operandSize);
	// void saveResult(int result, AddressingMode mode, unsigned char regNum, bool high, int resultSize);
	void saveResult(int result, AddressingMode mode, unsigned char regNum, bool high, int address, int resultSize);
	void outputCheck(AddressingMode mode, int address);

	void push(int size, int value);
	unsigned int pop(int size);

	bool ins_halt(int sizeOfOperands);
	bool ins_iret(int sizeOfOperands);
	bool ins_ret(int sizeOfOperands);
	bool ins_int(int sizeOfOperands);
	bool ins_call(int sizeOfOperands);
	bool ins_jmp(int sizeOfOperands);
	bool ins_jeq(int sizeOfOperands);
	bool ins_jne(int sizeOfOperands);
	bool ins_jgt(int sizeOfOperands);
	bool ins_push(int sizeOfOperands);
	bool ins_pop(int sizeOfOperands);
	bool ins_xchg(int sizeOfOperands);
	bool ins_mov(int sizeOfOperands);
	bool ins_add(int sizeOfOperands);
	bool ins_sub(int sizeOfOperands);
	bool ins_mul(int sizeOfOperands);
	bool ins_div(int sizeOfOperands);
	bool ins_cmp(int sizeOfOperands);
	bool ins_not(int sizeOfOperands);
	bool ins_and(int sizeOfOperands);
	bool ins_or(int sizeOfOperands); 
	bool ins_xor(int sizeOfOperands);
	bool ins_test(int sizeOfOperands);
	bool ins_shl(int sizeOfOperands);
	bool ins_shr(int sizeOfOperands);
};