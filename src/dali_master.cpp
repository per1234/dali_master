/* dali_master library by Steve Parker
 */

#include "dali_master.h"

/**
 * Constructor.
 */
Dali_master::Dali_master(uint8_t _address)
{
  address = _address;
}

/**
 * Example method.
 */
void Dali_master::begin()
{
	Wire.begin();
	clearStatusRegister();
    transmitCommand(TERMINATE_C, BLANK_C);
    transmitCommand(RESET_C, BLANK_C);
    transmitCommand(RESET_C, BLANK_C);
}

byte Dali_master::transmitCommand(byte cmd1, byte cmd2, bool &reply, byte &reply1, byte &reply2)
{
	byte a = transmitCommand(cmd1, cmd2);
	reply = false;

	if (bitRead(a, VALIDREPLY_S) == 1) {
		reply1 = 0x00;
		reply2 = 0x00;
		reply = true;

		if (bitRead(a, REPLY1_S) == 1 || bitRead(a, REPLY2_S) == 1) {
			getCommandRegister(reply1, reply2);
		}
	}

	return a;
}

byte Dali_master::transmitCommand(byte cmd1, byte cmd2)
{
	// Make sure the command register is clear before continuing
	clearStatusRegister();

	// Wait until bus is free
	byte b = getStatus();
	while (bitRead(b, BUSY_S) == 1) {
		b = getStatus();
	}

	Wire.beginTransmission(address);
	Wire.write(0x01);
	Wire.write(cmd1);
	Wire.write(cmd2);
	Wire.endTransmission();
	delay(10);

	// Wait until the command has been sent
	b = getStatus();
	while (bitRead(b, BUSY_S) == 1) {
		b = getStatus();
	}

	// Wait until reply timeframe has passed
	b = getStatus();
	while (bitRead(b, REPLYTIMEFRAME_S) == 1) {
		b = getStatus();
	}

	// If we have an error (frameerror or overrun)
	if (bitRead(b, FRAMEERROR_S) == 1 || bitRead(b, OVERRUN_S) == 1) {
		if (bitRead(b, FRAMEERROR_S) == 1) {

		} else {

		}
	}

	// If we have a valid reply, wait for the data to be available in the register
	if (bitRead(b, VALIDREPLY_S) == 1) {
		while (bitRead(b, REPLY1_S) == 0 && bitRead(b, REPLY2_S) == 0) {
			b = getStatus();
		}
	}

	return b;
}

void Dali_master::getCommandRegister(uint8_t &byte1, uint8_t &byte2) 
{
	// Wait until we have the correct number of bytes
	while (!getCommandRegisterRaw()) {}

	byte1 = Wire.read();
	byte2 = Wire.read();
}

byte Dali_master::getStatus()
{
	// Wait until we have the correct number of bytes
	while (!getStatusRaw()) {}

	return Wire.read();
}

// PRIVATE METHODS
void Dali_master::clearStatusRegister()
{
	uint8_t a, b;
	getCommandRegister(a, b);
}

bool Dali_master::getStatusRaw()
{
	Wire.beginTransmission(address);
	Wire.write(0x00);
	Wire.endTransmission();
	delay(10);

	Wire.requestFrom(address, 1);

	return (Wire.available() == 1) ? true : false;
}

bool Dali_master::getCommandRegisterRaw()
{
	Wire.beginTransmission(address);
	Wire.write(0x01);
	Wire.endTransmission();
	delay(10);

	Wire.requestFrom(address, 2);

	return (Wire.available() == 2) ? true : false;
}

void Dali_master::splitAdd(long input, uint8_t &highbyte, uint8_t &middlebyte, uint8_t &lowbyte)
{
	highbyte = input >> 16;
	middlebyte = input >> 8;
	lowbyte = input;
}
