#include "flasher.h"

Flasher::Flasher()
{
	reset();
}

int Flasher::setup(HardwareSerial* _serial, extEEPROM* _eeprom, uint8_t _resetPin)
{
	eeprom = _eeprom;
	serial = _serial;
	resetPin = _resetPin;

	serial->begin(OPTIBOOT_BAUD_RATE);
	pinMode(resetPin, OUTPUT);

	if (eeprom->begin(twiClock400kHz) != 0)
	{
		return EEPROM_ERROR;
	}

	// only necessary for Leonardo. safe for others
	while (!Serial);

	return OK;
}

void Flasher::reset()
{
}

void Flasher::clearRead()
{
	int count = 0;
	while (serial->available() > 0)
	{
		serial->read();
		count++;
	}
}

void Flasher::bounce()
{
	pinMode(resetPin, OUTPUT);
	digitalWrite(resetPin, LOW);
	delay(200);
	digitalWrite(resetPin, HIGH);
	delay(300);
	pinMode(resetPin, INPUT);
}

int Flasher::flash(int startAddress, int size)
{
	uint8_t result = OK;
	bounce();
	if ((result = flashInit()) != OK)
	{
		return result;
	}

	if ((result = sendToOptiboot(STK_ENTER_PROGMODE, buffer, 0, 0)) == -1)
	{
		return result;
	}

	int currentAddress = startAddress;
	int endAddress = size + EEPROM_OFFSET_ADDRESS;
	while (currentAddress < endAddress)
	{
		int len = 0;
		if (endAddress - currentAddress < PROG_PAGE_SIZE)
		{
			len = endAddress - currentAddress;
		}
		else
		{
			len = PROG_PAGE_SIZE;
		}

		if ((result = eeprom->read(currentAddress, buffer + 3, len)) == -1)
		{
			return result;
		}

		addr[0] = ((currentAddress - startAddress) / 2) & 0xff;
		addr[1] = (((currentAddress - startAddress) / 2) >> 8) & 0xff;

		if ((result = sendPageToOptiboot(addr, buffer, len)) == -1)
		{
			return result;
		}

		currentAddress += len;
	}

	if ((result = sendToOptiboot(STK_LEAVE_PROGMODE, buffer, 0, 0)) == -1)
	{
		return result;
	}

	return OK;
}

int Flasher::flashInit()
{
	clearRead();

  int dataLen = 0;

  cmd_buffer[0] = 0x81;
  if ((dataLen = sendToOptiboot(STK_GET_PARAMETER, cmd_buffer, 1, 1)) == -1)
	{
		return dataLen;
	}

  cmd_buffer[0] = 0x82;
  if ((dataLen = sendToOptiboot(STK_GET_PARAMETER, cmd_buffer, 1, 1)) == -1)
	{
		return dataLen;
	}

    // this not a valid command. optiboot will send back 0x3 for anything it doesn't understand
  cmd_buffer[0] = 0x83;
	if ((dataLen = sendToOptiboot(STK_GET_PARAMETER, cmd_buffer, 1, 1)) == -1)
	{
		return dataLen;
	}
	else if (readBuffer[0] != 0x3)
	{
		return -1;
  }

  dataLen = sendToOptiboot(STK_READ_SIGN, NULL, 0, 3);

  if (dataLen != 3)
	{
    return -1;
  }
	else if (readBuffer[0] == 0x1E && readBuffer[1] == 0x94 && readBuffer[2] == 0x6)
	{
      //atmega168
    }
	else if (readBuffer[0] == 0x1E && readBuffer[1] == 0x95 && readBuffer[2] == 0x0f)
	{
    //atmega328p
  }
	else if (readBuffer[0] == 0x1E && readBuffer[1] == 0x95 && readBuffer[2] == 0x14)
	{
    //atmega328
  }
	else
	{
	  return -1;
  }

  return OK;
}

int Flasher::readOptibootReply(uint8_t len, int timeout)
{
	long start = millis();
	int pos = 0;

	while (millis() - start < timeout)
	{
		if (serial->available() > 0)
		{
			readBuffer[pos] = serial->read();
			pos++;

			if (pos == len)
			{
				// we've read expected len
				break;
			}
		}
	}

	if (millis() - start >= timeout)
	{
		return -2;
	}

	// consume any extra
	clearRead();

	if (pos == len)
	{
		return pos;
	}
	else
	{
		return -1;
	}
}

int Flasher::sendToOptiboot(uint8_t command, uint8_t *arr, uint8_t len, uint8_t responseLength)
{
  serial->write(command);

	if (arr != NULL && len > 0)
	{
		for (int i = 0; i < len; i++)
		{
			serial->write(arr[i]);
		}
	}

	serial->write(CRC_EOP);
	// make it synchronous
	serial->flush();

	// add 2 bytes since we always expect to get back STK_INSYNC + STK_OK
	int replyLen = readOptibootReply(responseLength + 2, OPTIBOOT_READ_TIMEOUT);

	if (replyLen < 2)
	{
		return -1;
	}

	if (readBuffer[0] != STK_INSYNC)
	{
		return -1;
	}

	if (readBuffer[replyLen - 1] != STK_OK)
	{
		return -1;
	}

	uint8_t dataReply = replyLen - 2;

	for (int i = 0; i < dataReply; i++)
	{
		readBuffer[i] = readBuffer[i + 1];
	}

	// zero the ok
	readBuffer[replyLen - 1] = 0;

	// return the data portion of the length
	return dataReply;
}

int Flasher::sendPageToOptiboot(uint8_t *addr, uint8_t *buf, uint8_t dataLen)
{
	if (sendToOptiboot(STK_LOAD_ADDRESS, addr, 2, 0) == -1)
	{
    	return -1;
	}

	buffer[0] = 0;
	buffer[1] = dataLen;
	buffer[2] = 0x46;
	//remaining buffer is data

	// add 3 to dataLen
	if (sendToOptiboot(STK_PROG_PAGE, buffer, dataLen + 3, 0) == -1)
	{
		return -1;
	}

	uint8_t replyLen = sendToOptiboot(STK_READ_PAGE, buffer, 3, dataLen);

	if (replyLen == -1)
	{
		return -1;
	}

	if (replyLen != dataLen)
	{
		return -1;
	}

	bool verified = true;

	// verify each byte written matches what is returned by bootloader
	for (int i = 0; i < replyLen; i++)
	{
		if (readBuffer[i] != buffer[i + 3])
		{
			verified = false;
			break;
		}
	}

	if (!verified)
	{
		return -1;
	}

	return OK;
}
