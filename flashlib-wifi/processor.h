#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "flashlib.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <inttypes.h>
#include <extEEPROM.h>


class Processor
{
private:
	extEEPROM* eeprom;
	
public:
	int currentEepromAddress;
	int maxEepromAddress;

	Processor();
  int setup(extEEPROM* _eeprom);
	void reset();
	uint8_t process(const uint8_t *buf, size_t sz);
};

#endif
