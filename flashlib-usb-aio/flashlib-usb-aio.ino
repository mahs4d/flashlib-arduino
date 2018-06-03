#include "flasher.h"
#include "processor.h"
#include <extEEPROM.h>
#include <UsbFat.h>
#include <masstorage.h>

#define RESET_PIN 9

extEEPROM eeprom = extEEPROM(kbits_256, 1, 64);

uint8_t packet[PACKET_SIZE];

void setup()
{
  Serial.begin(OPTIBOOT_BAUD_RATE);
  Serial.println("Setup");
  
  setupBasics();
  setupEeprom();
  int size = startProcessing("b.hex");
  startFlashing(size);
  
  Serial.println("Setup Complete");
}

void setupBasics()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void setupEeprom()
{
  if (eeprom.begin(twiClock400kHz) != 0)
  {
    Serial.println("Eeprom Initialization Failed");
    halt();
  }
  
  Serial.println("Eeprom Initialized");
}

// Main --------------------------------------------------------

void loop() 
{
  
}

int startProcessing(const char* filename)
{
  Processor processor;
  USB usb;
  BulkOnly bulk(&usb);
  UsbFat key(&bulk);
  if (!initUSB(&usb))
  {
    Serial.println("Usb Initialization Failed");
    halt();
  }

  if (!key.begin())
  {
    Serial.println("Filesystem Initialization Failed");
    halt();
  }
  
  Serial.println("Usb Initialized");
  
  processor.setup(&eeprom);
  processor.reset();
  
  if (!key.exists(filename))
  {
    Serial.println("File doesn't exist");
    halt();
  }
  
  Serial.println("processing...");
  
  File file(filename, FILE_READ);
  size_t readbytes;
  while (readbytes = file.read(packet, PACKET_SIZE))
  {
    if (processor.process(packet, readbytes) != OK)
    {
      Serial.println("error in writing to eeprom!");
      processor.reset();
      file.close();
      halt();
    }
  }
  file.close();

  return processor.maxEepromAddress;
}

void startFlashing(int size)
{
  Flasher flasher;
  flasher.setup(&Serial, &eeprom, RESET_PIN);
  flasher.flash(EEPROM_OFFSET_ADDRESS, size);
}

// Helpers -----------------------------------------------------

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

