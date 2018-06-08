#include "processor.h"
#include <extEEPROM.h>
#include <UsbFat.h>
#include <masstorage.h>

Processor processor;
extEEPROM eeprom = extEEPROM(kbits_256, 1, 64);
USB usb;
BulkOnly bulk(&usb);
UsbFat key(&bulk);

uint8_t packet[PACKET_SIZE];

void setup()
{
  Serial.begin(OPTIBOOT_BAUD_RATE);
  Serial.println("Setup");
  
  setupBasics();
  setupEeprom();
  setupUsb();
  setupProcessor();
  startProcessing("a.hex");
  
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

void setupUsb()
{
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
}

void setupProcessor()
{
  processor.setup(&eeprom);
}

// Main --------------------------------------------------------

void loop() 
{
  
}

void startProcessing(const char* filename)
{
  processor.reset();
  
  if (!key.exists(filename))
  {
    Serial.println("File doesn't exist");
    return;
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
      return;
    }
  }
  file.close();

  Serial.println(processor.maxEepromAddress);
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

