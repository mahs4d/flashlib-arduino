#ifndef FLASHLIB_H
#define FLASHLIB_H

#define OPTIBOOT_BAUD_RATE 115200
#define OPTIBOOT_READ_TIMEOUT 1000

// EEPROM
#define EEPROM_OFFSET_ADDRESS 16
#define PROGRAM_START_ADDRESS 0

// Page and Buffers
#define PACKET_SIZE 32
#define PROG_PAGE_SIZE 128
#define BUFFER_SIZE PROG_PAGE_SIZE + 3
#define READ_BUFFER_SIZE PROG_PAGE_SIZE + 3

// STK CONSTANTS
#define STK_OK              0x10
#define STK_INSYNC          0x14  // ' '
#define CRC_EOP             0x20  // 'SPACE'
#define STK_GET_PARAMETER   0x41  // 'A'
#define STK_ENTER_PROGMODE  0x50  // 'P'
#define STK_LEAVE_PROGMODE  0x51  // 'Q'
#define STK_LOAD_ADDRESS    0x55  // 'U'
#define STK_PROG_PAGE       0x64  // 'd'
#define STK_READ_PAGE       0x74  // 't'
#define STK_READ_SIGN       0x75  // 'u'

// Codes
#define OK 1
#define START_OVER 2
#define TIMEOUT 3
#define FLASH_ERROR 4
#define EEPROM_ERROR 5
#define EEPROM_WRITE_ERROR 6
#define EEPROM_READ_ERROR 7

#endif
