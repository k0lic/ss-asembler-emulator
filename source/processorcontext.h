#pragma once

class ProcessorContext
{
public:
	ProcessorContext();

	short getRegister(int regNum);
	short getRegisterLow(int regNum);
	short getRegisterHigh(int regNum);
	unsigned short getPC();
	unsigned short getSP();
	unsigned short getPSW();
	bool getZero();
	bool getOverflow();
	bool getCarry();
	bool getNegative();
	bool getTimer();
	bool getTerminal();
	bool getInterrupt();
	bool interruptSignal(int interruptLineNum);
	short getInterruptTableEntry(int num);

	void setRegister(int regNum, short value);
	void setRegisterLow(int regNum, short value);
	void setRegisterHigh(int regNum, short value);
	void setPC(unsigned short value);
	void incPC();
	void setSP(unsigned short value);
	void incSP();
	void decSP();
	void setPSW(unsigned short value);
	void setZero(bool zero = true);
	void setOverflow(bool overflow = true);
	void setCarry(bool carry = true);
	void setNegative(bool negative = true);
	void setTimer(bool timer = true);
	void setTerminal(bool terminal = true);
	void setInterrupt(bool interrupt = true);
	void setInterruptSignal(int num, bool value = true);

	static const unsigned short ZERO, OVERFLOW, CARRY, NEGATIVE, TIMER, TERMINAL, INTERRUPT, ALL_FLAGS;
private:
	short reg[8];
	short psw;
	bool interruptLines[2];
	short interruptTableEntries[2];
};