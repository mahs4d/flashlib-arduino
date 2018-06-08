#include "processor.h"

Processor::Processor()
{
	reset();
}

int Processor::setup(extEEPROM* _eeprom)
{
	eeprom = _eeprom;

	if (eeprom->begin(twiClock400kHz) != 0)
	{
		return EEPROM_ERROR;
	}

	return OK;
}

void Processor::reset()
{
	currentEepromAddress = EEPROM_OFFSET_ADDRESS;
	maxEepromAddress = EEPROM_OFFSET_ADDRESS;
}

uint8_t Processor::process(const uint8_t *buf, size_t sz)
{
	if (eeprom->write(currentEepromAddress, buf, sz) != 0)
	{
		return -1;
	}

	currentEepromAddress += sz;

	if (currentEepromAddress > maxEepromAddress)
	{
		maxEepromAddress = currentEepromAddress;
	}

	return OK;
}
