#ifndef FLASHER_H
#define FLASHER_H

#include "flashlib.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <inttypes.h>
#include <SoftwareSerial.h>
#include <extEEPROM.h>


class Flasher
{
private:
	extEEPROM* eeprom;
	HardwareSerial* serial;
	uint8_t resetPin;

	uint8_t buffer[BUFFER_SIZE];
	uint8_t readBuffer[READ_BUFFER_SIZE];
	uint8_t cmd_buffer[1];
	uint8_t addr[2];

	void clearRead();
	void bounce();
	int flashInit();
	int readOptibootReply(uint8_t len, int timeout);
	int sendToOptiboot(uint8_t command, uint8_t *arr, uint8_t len, uint8_t responseLength);
	int sendPageToOptiboot(uint8_t *addr, uint8_t *buf, uint8_t dataLen);
public:
	Flasher();
	void reset();
	int setup(HardwareSerial* _serial, extEEPROM* _eeprom, uint8_t _resetPin);
 	int flash(uint16_t startAddress, int size);
};

#endif
