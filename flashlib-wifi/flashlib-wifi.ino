#include "flasher.h"
#include "processor.h"
#include <extEEPROM.h>
#include "SoftwareSerial.h"
#include <WiFiEsp.h>

#define RESET_PIN 10
#define SOFT_RX 8
#define SOFT_TX 9

#define PORT 8890
#define MAGIC_START "flashme"

#define ESP_PACKET_SIZE 32

extEEPROM eeprom = extEEPROM(kbits_256, 1, 64);
SoftwareSerial wifiSerial(SOFT_RX, SOFT_TX);
uint8_t packet[ESP_PACKET_SIZE];

WiFiEspServer server(PORT);
RingBuffer buf(8);

void setup()
{
	Serial.begin(OPTIBOOT_BAUD_RATE);
	Serial.println("Setup");

	setupBasics();
	setupEeprom();
	setupWifi();

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

void setupWifi()
{
	wifiSerial.begin(9600);
	WiFi.init(&wifiSerial);

	WiFi.beginAP("FlashLib", 10, "flashlib123", ENC_TYPE_WPA2_PSK);

	server.begin();

	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);
	Serial.print("Update Server running on port ");
	Serial.println(PORT);
}

// Main --------------------------------------------------------
union ArrayToInteger
{
	byte array[2];
	int integer;
};

void loop()
{
	WiFiEspClient client = server.available();  // listen for incoming clients

	if (client)
	{                               // if you get a client,
		Serial.println("New Client!");             // print a message out the serial port
		buf.init();

		// read first 6 bytes, should be magic numbers
		int bytesread = 0;
		while (client.connected() && bytesread < 7)
		{
			if (client.available())
			{
				char c = client.read();
				buf.push(c);
				bytesread++;
			}
		}

		if (bytesread != 7 || !buf.endsWith(MAGIC_START))
		{
			client.stop();
      Serial.println("Error in Magic Number Phase");
			return;
		}
    
    Serial.println("Magic Word Captured");

		// read size of program
		ArrayToInteger computedsize;
		bytesread = 0;
		while (client.connected() && bytesread < 2)
		{
			if (client.available())
			{
				bytesread += client.read(computedsize.array, 2);
			}
		}

		if (bytesread != 2)
		{
			client.stop();
      Serial.println("Error in Size Phase");
			return;
		}

    Serial.print("Size: ");
    Serial.println(computedsize.integer);
    
    Serial.println("Processing ...");

		int progsize = startProcessing(&client, computedsize.integer);

		if (progsize < 0)
		{
			client.stop();
      Serial.println("Error in Processing Phase");
			return;
		}

    Serial.println("Flashing ...");
		int result = startFlashing(progsize);

    if (result == OK)
    {
      client.print("OK");
    }
    else
    {
      client.print("NO");
    }
		delay(10);

		// close the connection
		client.stop();
		Serial.println("Client disconnected");
	}
}

int startProcessing(WiFiEspClient* client, int size)
{
	Processor processor;
  processor.setup(&eeprom);
  processor.reset();

	size_t readbytes;
	int readBytesTotal = 0;
	while (readBytesTotal < size)
	{
		if (client->available())
		{
			readbytes = client->read(packet, ESP_PACKET_SIZE);
			readBytesTotal += readbytes;
      if (readbytes == 0)
        continue;
      
      Serial.write(packet, readbytes);
      Serial.println("");
      client->print("OK");
			if (processor.process(packet, readbytes) != OK)
			{
				Serial.println("[Processing] Error in writing to EEPROM!");
				return -1;
			}
		}
    else if (!client->connected())
    {
      break;
    }
	}

	if (readBytesTotal != size)
	{
    Serial.print("[Processing] Error in number of received bytes: ");
    Serial.print(readBytesTotal);
    Serial.print("/");
    Serial.println(size);
		return -1;
	}

	return processor.maxEepromAddress;
}

int startFlashing(int size)
{
	Flasher flasher;
	if (flasher.setup(&Serial, &eeprom, RESET_PIN) != OK)
  {
    Serial.println("Error in flasher setup");
    return -1;
  }
	return flasher.flash(EEPROM_OFFSET_ADDRESS, size - EEPROM_OFFSET_ADDRESS);
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
