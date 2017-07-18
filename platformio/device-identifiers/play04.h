#include <lmic.h>
#include <hal/hal.h>

#define OTAA

// DEVEUI: Unique device ID (LSBF)
static const u1_t DEVEUI[8] PROGMEM = { 0x04, 0x01, 0x4F, 0x44, 0x24, 0x59, 0x34, 0x12 };

// APPEUI: Application ID (LSBF)
static const u1_t APPEUI[8] PROGMEM = { 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };

// APPKEY: Device-specific AES key.
static const u1_t APPKEY[16] PROGMEM = { 0x0C, 0x93, 0xF6, 0xDE, 0xFE, 0xC5, 0xB6, 0xFF, 0x88, 0x53, 0x77, 0xEC, 0xE3, 0x8E, 0x1A, 0x71 };

// DEVADDR: Unique device ID
static const u4_t DEVADDR = 0;

// NWKSKEY: network specific session key
static const u1_t NWKSKEY[16] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// APPSKEY: Application specific session key
static const u1_t APPSKEY[16] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
x00, 0x00, 0x00, 0x00 };
