#include "flasher.h"

#define RESET_PIN 10

Flasher flasher;
extEEPROM eeprom = extEEPROM(kbits_256, 1, 64);

void setup()
{
	setupEEPROM();
	setupFlasher();
	Serial.println("Setup Complete");
  if (flasher.flash(EEPROM_OFFSET_ADDRESS, 1936) != OK)
  {
    Serial.println("Flash Failed");
    halt();
  }
  Serial.println("Flash Completed");
}

void setupEEPROM()
{
	if (eeprom.begin(twiClock400kHz) != 0)
	{
		Serial.println("Eeprom Setup Failed");
		halt();
	}
}

void setupFlasher()
{
	if (flasher.setup(&Serial, &eeprom, RESET_PIN) != OK)
	{
		Serial.println("FlashLib Setup Failed");
		halt();
	}
}

void loop()
{

}

void halt()
{
	while(true)
	{
		digitalWrite(LED_BUILTIN, HIGH);
		delay(300);
		digitalWrite(LED_BUILTIN, LOW);
		delay(300);
	}
}
