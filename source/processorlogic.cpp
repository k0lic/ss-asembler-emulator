#include "processorlogic.h"

using namespace std;

/* STATIC CONSTANTS */

const int ProcessorLogic::MIN_ADDRESS = 0;
const int ProcessorLogic::MAX_ADDRESS = 65'535;

/* CONSTRUCTOR */

ProcessorLogic::ProcessorLogic(ProcessorContext *context, unsigned char *memory)
{
	this->context = context;
	this->memory = memory;
	this->halted = false;
}

/* PUBLIC METHODS */

bool ProcessorLogic::executeNextInstruction()
{
	// get next instruction
	unsigned char instruction = nextByteu();
	unsigned char opCode = instruction >> 3;
	unsigned char sizeOfOperands = instruction & 4 ? 2 : 1;

	// check if the lowest 2 bits are 0 - they are not used and always set to 0
	if (instruction & 3)
		return false;

	// check if valid op code
	if (opCode > 24)
		return false;

	bool (ProcessorLogic::*insss[])(int) =
	{
		&ProcessorLogic::ins_halt,
		&ProcessorLogic::ins_iret,
		&ProcessorLogic::ins_ret,
		&ProcessorLogic::ins_int,
		&ProcessorLogic::ins_call,
		&ProcessorLogic::ins_jmp,
		&ProcessorLogic::ins_jeq,
		&ProcessorLogic::ins_jne,
		&ProcessorLogic::ins_jgt,
		&ProcessorLogic::ins_push,
		&ProcessorLogic::ins_pop,
		&ProcessorLogic::ins_xchg,
		&ProcessorLogic::ins_mov,
		&ProcessorLogic::ins_add,
		&ProcessorLogic::ins_sub,
		&ProcessorLogic::ins_mul,
		&ProcessorLogic::ins_div,
		&ProcessorLogic::ins_cmp,
		&ProcessorLogic::ins_not,
		&ProcessorLogic::ins_and,
		&ProcessorLogic::ins_or,
		&ProcessorLogic::ins_xor,
		&ProcessorLogic::ins_test,
		&ProcessorLogic::ins_shl,
		&ProcessorLogic::ins_shr
	};

	// call the appropriate instruction method
	if (!(this->*insss[opCode])(sizeOfOperands))
		return false;

	return true;
}

bool ProcessorLogic::isHalted()
{
	return halted;
}

void ProcessorLogic::interrupt(unsigned int entryNum)
{
	// push pc
	push(2, context->getPC());

	// push psw
	push(2, context->getPSW());
	
	// pc = mem[(entryNum % 8) * 2]
	unsigned int dest = (entryNum & 0x0007) << 1;
	dest = LittleEndian::charLittleEndianToUInt(2, memory + dest);
	context->setPC(dest);
}

/* PRIVATE METHODS */

char ProcessorLogic::nextByte()
{
	char byte = memory[context->getPC()];
	context->incPC();
	return byte;
}

unsigned char ProcessorLogic::nextByteu()
{
	unsigned char byte = memory[context->getPC()];
	context->incPC();
	return byte;
}

int ProcessorLogic::nextWord()
{
	int word = LittleEndian::charLittleEndianToInt(2, memory + context->getPC());
	context->incPC();
	context->incPC();
	return word;
}

unsigned int ProcessorLogic::nextWordu()
{
	unsigned int word = LittleEndian::charLittleEndianToUInt(2, memory + context->getPC());
	context->incPC();
	context->incPC();
	return word;
}

bool ProcessorLogic::nextAddressingMode(AddressingMode *mode, unsigned char *regNum, bool *high)
{
	unsigned char opDescr = nextByteu();
	unsigned char amNum = opDescr >> 5;
	*regNum = (opDescr & 0x1e) >> 1;
	*high = opDescr & 1;

	if (amNum == 0)
		*mode = IMMEDIATE;
	else if (amNum == 1)
		*mode = REGISTER_DIRECT;
	else if (amNum == 2)
		*mode = REGISTER_INDIRECT;
	else if (amNum == 3)
		*mode = REGISTER_INDIRECT_WITH_OFFSET;
	else if (amNum == 4)
		*mode = MEMORY;
	else
		return false;

	if ((*mode == REGISTER_DIRECT || *mode == REGISTER_INDIRECT || *mode == REGISTER_INDIRECT_WITH_OFFSET) && *regNum > 7 && *regNum < 15)
		return false;

	return true;
}

int ProcessorLogic::nextOperand(AddressingMode mode, int operandSize)
{
	int operand;

	if (mode == IMMEDIATE)
	{
		if (operandSize == 1)
			operand = nextByte();
		else
			operand = nextWord();
	}
	else if (mode == REGISTER_INDIRECT_WITH_OFFSET)
		operand = nextWord();
	else if (mode == MEMORY)
		operand = nextWordu();
	else
		operand = 0;

	return operand;
}

void ProcessorLogic::processOperand(int *operand, int *address, int rawOperand, AddressingMode mode, unsigned char regNum, bool high, int operandSize)
{
	*operand = 0;
	*address = 0;

	// check if immediate mode was used
	if (mode == IMMEDIATE)
		*operand = rawOperand;

	// check if a register value is part of the operand
	if (mode == REGISTER_DIRECT)
	{
		if (operandSize == 1)
		{
			if (high)
				*operand = context->getRegisterHigh(regNum);
			else
				*operand = context->getRegisterLow(regNum);
		}
		else
			*operand = context->getRegister(regNum);
	}
	else if (mode == REGISTER_INDIRECT || mode == REGISTER_INDIRECT_WITH_OFFSET)
		*address = (unsigned short)context->getRegister(regNum);

	// check if offset is part of the operand
	if (mode == REGISTER_INDIRECT_WITH_OFFSET || mode == MEMORY)
		*address += rawOperand;

	// check if operand is located in memory
	if (mode == REGISTER_INDIRECT || mode == REGISTER_INDIRECT_WITH_OFFSET || mode == MEMORY)
		*operand = LittleEndian::charLittleEndianToInt(operandSize, memory + *address);
}

void ProcessorLogic::saveResult(int result, AddressingMode mode, unsigned char regNum, bool high, int address, int resultSize)
{
	if (mode == REGISTER_DIRECT)
	{
		if (resultSize == 1)
		{
			if (high)
				context->setRegisterHigh(regNum, result);
			else
				context->setRegisterLow(regNum, result);
		}
		else
			context->setRegister(regNum, result);
	}
	else
		LittleEndian::intToLittleEndianChar(result, resultSize, memory + address);
}

void ProcessorLogic::outputCheck(AddressingMode mode, int address)
{
	if ((mode == REGISTER_INDIRECT || mode == REGISTER_INDIRECT_WITH_OFFSET || mode == MEMORY) && address == 0xff00)
		cout << memory[0xff00];
}

void ProcessorLogic::push(int size, int value)
{
	for (int i = 0; i < size; i++)
		context->decSP();
	LittleEndian::intToLittleEndianChar(value, size, memory + (unsigned short)context->getSP());
}

unsigned int ProcessorLogic::pop(int size)
{
	unsigned int ret = LittleEndian::charLittleEndianToUInt(size, memory + (unsigned short)context->getSP());
	for (int i = 0; i < size; i++)
		context->incSP();

	return ret;
}

/* INSTRUCTION METHODS */

// 0 operand instructions

bool ProcessorLogic::ins_halt(int sizeOfOperands)
{
	halted = true;

	return true;
}

bool ProcessorLogic::ins_iret(int sizeOfOperands)
{
	// pop psw
	unsigned int tmp = pop(2);
	context->setPSW(tmp);

	// pop pc
	tmp = pop(2);
	context->setPC(tmp);

	return true;
}

bool ProcessorLogic::ins_ret(int sizeOfOperands)
{
	// pop pc
	unsigned int tmp = pop(2);
	context->setPC(tmp);
	
	return true;
}

// 1 operand instructions

bool ProcessorLogic::ins_int(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode;
	unsigned char regNum;
	bool high;

	// read operand descriptor - if format is invalid - bail
	if (!nextAddressingMode(&mode, &regNum, &high))
		return false;

	// read operand content (can represent the whole operand, can represent the address, or the offset)
	int rawOperand = nextOperand(mode, sizeOfOperands);

	// calculate operand value and address (where applicaple)
	int operandValue, operandAddress;
	processOperand(&operandValue, &operandAddress, rawOperand, mode, regNum, high, sizeOfOperands);

	// EXE phase:
	// push pc
	push(2, context->getPC());

	// push psw
	push(2, context->getPSW());
	
	// pc = mem[(op1 % 8) * 2]
	unsigned int dest = (operandValue & 0x0007) << 1;
	dest = LittleEndian::charLittleEndianToUInt(2, memory + dest);
	context->setPC(dest);

	return true;
}

bool ProcessorLogic::ins_call(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode;
	unsigned char regNum;
	bool high;

	// read operand descriptor - if format is invalid - bail
	if (!nextAddressingMode(&mode, &regNum, &high))
		return false;

	// read operand content (can represent the whole operand, can represent the address, or the offset)
	int rawOperand = nextOperand(mode, sizeOfOperands);

	// calculate operand value and address (where applicaple)
	int operandValue, operandAddress;
	processOperand(&operandValue, &operandAddress, rawOperand, mode, regNum, high, sizeOfOperands);

	// EXE phase:
	// push pc
	push(2, context->getPC());

	// 'pc += op1' or 'pc = op1'
	if (mode == IMMEDIATE && high)
		context->setPC((unsigned int)operandValue + context->getPC());
	else
		context->setPC((unsigned int)operandValue);

	return true;
}

bool ProcessorLogic::ins_jmp(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode;
	unsigned char regNum;
	bool high;

	// read operand descriptor - if format is invalid - bail
	if (!nextAddressingMode(&mode, &regNum, &high))
		return false;

	// read operand content (can represent the whole operand, can represent the address, or the offset)
	int rawOperand = nextOperand(mode, sizeOfOperands);

	// calculate operand value and address (where applicaple)
	int operandValue, operandAddress;
	processOperand(&operandValue, &operandAddress, rawOperand, mode, regNum, high, sizeOfOperands);

	// EXE phase:
	// 'pc += op1' or 'pc = op1'
	if (mode == IMMEDIATE && high)
		context->setPC((unsigned int)operandValue + context->getPC());
	else
		context->setPC((unsigned int)operandValue);

	return true;
}

bool ProcessorLogic::ins_jeq(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode;
	unsigned char regNum;
	bool high;

	// read operand descriptor - if format is invalid - bail
	if (!nextAddressingMode(&mode, &regNum, &high))
		return false;

	// read operand content (can represent the whole operand, can represent the address, or the offset)
	int rawOperand = nextOperand(mode, sizeOfOperands);

	// calculate operand value and address (where applicaple)
	int operandValue, operandAddress;
	processOperand(&operandValue, &operandAddress, rawOperand, mode, regNum, high, sizeOfOperands);

	// EXE phase:
	// if ZERO then jump
	if (context->getZero())
	{
		// 'pc += op1' or 'pc = op1'
		if (mode == IMMEDIATE && high)
			context->setPC((unsigned int)operandValue + context->getPC());
		else
			context->setPC((unsigned int)operandValue);
	}

	return true;
}

bool ProcessorLogic::ins_jne(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode;
	unsigned char regNum;
	bool high;

	// read operand descriptor - if format is invalid - bail
	if (!nextAddressingMode(&mode, &regNum, &high))
		return false;

	// read operand content (can represent the whole operand, can represent the address, or the offset)
	int rawOperand = nextOperand(mode, sizeOfOperands);

	// calculate operand value and address (where applicaple)
	int operandValue, operandAddress;
	processOperand(&operandValue, &operandAddress, rawOperand, mode, regNum, high, sizeOfOperands);

	// EXE phase:
	// if !ZERO then jump
	if (!context->getZero())
	{
		// 'pc += op1' or 'pc = op1'
		if (mode == IMMEDIATE && high)
			context->setPC((unsigned int)operandValue + context->getPC());
		else
			context->setPC((unsigned int)operandValue);
	}

	return true;
}

bool ProcessorLogic::ins_jgt(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode;
	unsigned char regNum;
	bool high;

	// read operand descriptor - if format is invalid - bail
	if (!nextAddressingMode(&mode, &regNum, &high))
		return false;

	// read operand content (can represent the whole operand, can represent the address, or the offset)
	int rawOperand = nextOperand(mode, sizeOfOperands);

	// calculate operand value and address (where applicaple)
	int operandValue, operandAddress;
	processOperand(&operandValue, &operandAddress, rawOperand, mode, regNum, high, sizeOfOperands);

	// EXE phase:
	// if !ZERO && !NEGATIVE && !OVERFLOW then jump
	if (!context->getZero() && !context->getNegative() && !context->getOverflow())
	{
		// 'pc += op1' or 'pc = op1'
		if (mode == IMMEDIATE && high)
			context->setPC((unsigned int)operandValue + context->getPC());
		else
			context->setPC((unsigned int)operandValue);
	}

	return true;
}

bool ProcessorLogic::ins_push(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode;
	unsigned char regNum;
	bool high;

	// read operand descriptor - if format is invalid - bail
	if (!nextAddressingMode(&mode, &regNum, &high))
		return false;

	// read operand content (can represent the whole operand, can represent the address, or the offset)
	int rawOperand = nextOperand(mode, sizeOfOperands);

	// calculate operand value and address (where applicaple)
	int operandValue, operandAddress;
	processOperand(&operandValue, &operandAddress, rawOperand, mode, regNum, high, sizeOfOperands);

	// EXE phase:
	// push op1
	push(sizeOfOperands, operandValue);

	return true;
}

bool ProcessorLogic::ins_pop(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode;
	unsigned char regNum;
	bool high;

	// read operand descriptor - if format is invalid - bail
	if (!nextAddressingMode(&mode, &regNum, &high))
		return false;

	// this instruction is NOT compatible with immediate addressing!
	if (mode == IMMEDIATE)
		return false;

	// read operand content (can represent the whole operand, can represent the address, or the offset)
	int rawOperand = nextOperand(mode, sizeOfOperands);

	// calculate operand value and address (where applicaple)
	int operandValue, operandAddress;
	processOperand(&operandValue, &operandAddress, rawOperand, mode, regNum, high, sizeOfOperands);

	// EXE phase:
	// pop result
	unsigned int result = pop(sizeOfOperands);

	// save result - either into memory or into a register - depending on the addressing mode used
	saveResult(result, mode, regNum, high, operandAddress, sizeOfOperands);

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode, operandAddress);

	return true;
}

// 2 operand instructions

bool ProcessorLogic::ins_xchg(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on either operand!
	if (mode[0] == IMMEDIATE || mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);

	// EXE phase:
	// save both operand values to the other operand address (or register)
	for (int i = 0; i < 2; i++)
		saveResult(operandValue[(i + 1) % 2], mode[i], regNum[i], high[i], operandAddress[i], sizeOfOperands);

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	for (int i = 0; i < 2; i++)
		outputCheck(mode[i], operandAddress[i]);

	return true;
}

bool ProcessorLogic::ins_mov(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);

	// EXE phase:
	// save the first operand value to the second operand address (or register)
	saveResult(operandValue[0], mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO and NEGATIVE bits
	context->setZero((operandValue[0] & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setNegative(operandValue[0] & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_add(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// save the sum of the operands to the second operand address (or register)
	int result = operandValue[0] + operandValue[1];
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO, OVERFLOW, CARRY and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setOverflow((operandValue[0] > 0 && operandValue[1] > 0 && result & (sizeOfOperands == 1 ? 0x80 : 0x8000)) || (operandValue[0] < 0 && operandValue[1] < 0 && !(result & (sizeOfOperands == 1 ? 0x80 : 0x8000))));
	context->setCarry((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) != result);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_sub(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// save the difference of the operands to the second operand address (or register)
	int result = operandValue[1] - operandValue[0];
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO, OVERFLOW, CARRY and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setOverflow((operandValue[1] > 0 && operandValue[0] < 0 && result & (sizeOfOperands == 1 ? 0x80 : 0x8000)) || (operandValue[1] < 0 && operandValue[0] > 0 && !(result & (sizeOfOperands == 1 ? 0x80 : 0x8000))));
	context->setCarry((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) != result);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_mul(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// save the product of the operands to the second operand address (or register)
	int result = operandValue[1] * operandValue[0];
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_div(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	int negativeNum = 0;
	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		
		// turn negative into positive so division is simple
		if (operandValue[i] < 0)
		{
			operandValue[i] = -operandValue[i];
			negativeNum++;
		}
	}

	// EXE phase:
	// save the product of the operands to the second operand address (or register)
	int result = operandValue[1] / operandValue[0];
	if (negativeNum == 1)
		result = -result;
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_cmp(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// DO NOT SAVE the difference of the operands to the second operand address (or register), only UPDATE psw indicators
	int result = operandValue[1] - operandValue[0];

	// update ZERO, OVERFLOW, CARRY and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setOverflow((operandValue[1] > 0 && operandValue[0] < 0 && result & (sizeOfOperands == 1 ? 0x80 : 0x8000)) || (operandValue[1] < 0 && operandValue[0] > 0 && !(result & (sizeOfOperands == 1 ? 0x80 : 0x8000))));
	context->setCarry((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) != result);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	return true;
}

bool ProcessorLogic::ins_not(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// save the complement (not) of the first operand to the second operand address (or register)
	int result = ~operandValue[0];
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_and(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// save the bitwise and of the operands to the second operand address (or register)
	int result = operandValue[1] & operandValue[0];
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_or(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// save the bitwise or of the operands to the second operand address (or register)
	int result = operandValue[1] | operandValue[0];
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_xor(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// save the bitwise xor of the operands to the second operand address (or register)
	int result = operandValue[1] ^ operandValue[0];
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_test(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// DO NOT SAVE the bitwise and of the operands to the second operand address (or register), only UPDATE psw indicators
	int result = operandValue[1] & operandValue[0];

	// update ZERO and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	return true;
}

bool ProcessorLogic::ins_shl(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the second operand!
	if (mode[1] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
	{
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);
		// easier on my brain if I do this
		operandValue[i] = operandValue[i] & (sizeOfOperands == 1 ? 0xff : 0xffff);
	}

	// EXE phase:
	// save the second operand shifted to the left first operand times to the second operand address (or register)
	int result = operandValue[1] << operandValue[0];
	saveResult(result, mode[1], regNum[1], high[1], operandAddress[1], sizeOfOperands);

	// update ZERO, CARRY and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setCarry((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) != result);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[1], operandAddress[1]);

	return true;
}

bool ProcessorLogic::ins_shr(int sizeOfOperands)
{
	// ID phase:
	// operand description variables
	AddressingMode mode[2];
	unsigned char regNum[2];
	bool high[2];

	int rawOperand[2], operandValue[2], operandAddress[2];

	for (int i = 0; i < 2; i++)
	{
		// read operand%i descriptor - if format is invalid - bail
		if (!nextAddressingMode(mode + i, regNum + i, high + i))
			return false;

		// read operand%i content (can represent the whole operand, can represent the address, or the offset)
		rawOperand[i] = nextOperand(mode[i], sizeOfOperands);
	}

	// this instruction is NOT compatible with immediate addressing on the first operand!
	if (mode[0] == IMMEDIATE)
		return false;

	// calculate the operand values and addresses (where applicable)
	for (int i = 0; i < 2; i++)
		processOperand(operandValue + i, operandAddress + i, rawOperand[i], mode[i], regNum[i], high[i], sizeOfOperands);

	// EXE phase:
	// save the first operand shifted to the right second operand times to the first operand address (or register)
	int result = operandValue[0] >> operandValue[1];
	saveResult(result, mode[0], regNum[0], high[0], operandAddress[0], sizeOfOperands);

	// update ZERO, CARRY and NEGATIVE bits
	context->setZero((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) == 0);
	context->setCarry((result & (sizeOfOperands == 1 ? 0xff : 0xffff)) != result);
	context->setNegative(result & (sizeOfOperands == 1 ? 0x80 : 0x8000));

	// check if data_out memory mapped register was overwritten - if it was overwritten then output the new value to the terminal
	outputCheck(mode[0], operandAddress[0]);

	return true;
}