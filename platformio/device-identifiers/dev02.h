#include <lmic.h>
#include <hal/hal.h>

#define OTAA

// DEVEUI: Unique device ID (LSBF)
static const u1_t DEVEUI[8] PROGMEM = { 0x02, 0x02, 0x67, 0x44, 0x24, 0x59, 0x34, 0x12 };

// APPEUI: Application ID (LSBF)
static const u1_t APPEUI[8] PROGMEM = { 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };

// APPKEY: Device-specific AES key.
static const u1_t APPKEY[16] PROGMEM = { 0x19, 0x4B, 0x89, 0xCE, 0x08, 0x40, 0x06, 0x91, 0x4B, 0xDA, 0x51, 0x0D, 0x18, 0xDC, 0x32, 0x81 };

// DEVADDR: Unique device ID
static const u4_t DEVADDR = 0;

// NWKSKEY: network specific session key
static const u1_t NWKSKEY[16] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// APPSKEY: Application specific session key
static const u1_t APPSKEY[16] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
x00, 0x00, 0x00, 0x00 };
