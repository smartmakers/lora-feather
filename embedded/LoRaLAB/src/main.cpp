#include <wave.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

/* Behavior Parameters */

// Period between two data transmissions (seconds) if there is no change in the inputs.
#define TRANSMIT_PERIOD 5

/* LoRa Parameters */

// DEVEUI: Unique device ID (LSBF)
static const u1_t DEVEUI[8] PROGMEM = { 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// APPEUI: Application ID (LSBF)
static const u1_t APPEUI[8] PROGMEM = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// APPKEY: Device-specific AES key.
static const u1_t APPKEY[16] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

/* End of Parameters */

void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

// Pin mapping (nss and dio0/1 required)
const lmic_pinmap lmic_pins = {
        .nss = 8,
        .rxtx = LMIC_UNUSED_PIN,
        .rst = 11,
        .dio = {7, 5, LMIC_UNUSED_PIN},
};

// Status led pin.
#define LED_STATUS 13

// Battery voltage pin.
#define PIN_BATTERY A9

static wave_generator_t* gen;

uint8_t led_fast_blink(uint32_t t) {
    return wave_pwm( t, 200, 50 );
}

uint8_t led_slow_blink(uint32_t t) {
    return wave_pwm( t, 2000, 1000 );
}

uint8_t led_short_blink(uint32_t t) {
    return wave_pwm( t, 2000, 50 );
}

// uses only 3 bits
static const uint8_t protocolVersion = 0x01;

// uses only 5 bits
static const uint8_t emptyMsg = 0x00;
static const uint8_t zerosMsg = 0x01;
static const uint8_t helloMsg = 0x02;
static const uint8_t batteryAndRSSIMsg = 0x03;
static const uint8_t int32Msg = 0x04;
static const uint8_t float64Msg = 0x05;

static uint8_t hello[] = "hello";
static uint8_t headerOnly[1];
// using 1st byte for protocol version and msg type
static uint8_t payload[10];

static const int32_t int32Val = 123456;
static const double float64Val = 123456.123456;

static osjob_t job_transmit;
static uint8_t job_counter = 0;

uint8_t makeHeader(uint8_t protocolVersion, uint8_t msgType)  {
    return protocolVersion << 5 | msgType;
}

void sendEmptyMsg()
{
    headerOnly[0] = makeHeader(protocolVersion, emptyMsg);
    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, headerOnly, sizeof(headerOnly), 1);
}

void sendZeros()
{
    payload[0] = makeHeader(protocolVersion, zerosMsg);
    payload[1] = (uint8_t)(0);
    payload[2] = (uint8_t)(0);

    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, payload, 3, 1);
}

void sendHello()
{
    payload[0] = makeHeader(protocolVersion, helloMsg);
    memcpy(payload + 1, hello, sizeof(hello));
    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, payload, sizeof(hello) + 1, 1);
}

void sendBatteryAndRSSI()
{
    payload[0] = makeHeader(protocolVersion, batteryAndRSSIMsg);
    // Save battery level.
    // To decode on the server side:
    // Voltage = Value * (2^3) * 3.3 / 1024;
    payload[1] = (uint8_t)(analogRead(PIN_BATTERY) >> 2);

    // Save last RSSI
    // Actual RSSI: (Value - 64)
    // radio.c (l.786), RFM95 (5.5.5, p82)
    payload[2] = (uint8_t)(LMIC.rssi);

    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, payload, 3, 1);
}

void sendInt32()
{
    payload[0] = makeHeader(protocolVersion, int32Msg);
    // Copy data to send.
    memcpy(payload + 1, (uint8_t*)&int32Val, sizeof(int32Val));

    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, payload, sizeof(int32Val) + 1, 1);
}

void sendFloat64()
{
    payload[0] = makeHeader(protocolVersion, float64Msg);
    // Copy data to send.
    memcpy(payload + 1, (uint8_t*)&float64Val, sizeof(float64Val));

    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, payload, sizeof(float64Val) + 1, 1);
}

void jobTransmitCallback(osjob_t* j)
{
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("ERROR: OP_TXRXPEND, not sending"));
    } else {

        Serial.print(os_getTime());
        Serial.println(F(": TXDATA"));

        switch (job_counter) {
            case emptyMsg:
                sendEmptyMsg();
                break;
            case zerosMsg:
                sendZeros();
                break;
            case helloMsg:
                sendHello();
                break;
            case batteryAndRSSIMsg:
                sendBatteryAndRSSI();
                break;
            case int32Msg:
                sendInt32();
                break;
            case float64Msg:
                sendFloat64();
                break;
            default:
                job_counter = 0;

                // Reset the MAC state.
                LMIC_reset();

                // Start join (OTAA).
                LMIC_startJoining();
                return;
        }
        job_counter += 1;

    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {

        case EV_JOINING:
            Serial.println(F("EV_JOINING"));

            // Slow blinking while connecting.
            wave_generator_apply(gen, led_slow_blink);

            break;

        case EV_JOINED:
            Serial.println(F("EV_JOINED"));

            // Once connected blink make short blinks.
            wave_generator_apply(gen, led_short_blink);

            // Set lowest data rate by default.
            LMIC_setDrTxpow(DR_SF12, 14);

            // Disable link check validation.
            LMIC_setLinkCheckMode(0);

            // Manually setup additional channels.
            LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
            LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
            LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
            LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
            LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
            LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

            // Schedule first transmission.
            os_setCallback( &job_transmit, &jobTransmitCallback );

            break;

        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));

            // Fast blinking while trying to send something.
            wave_generator_apply(gen, led_fast_blink);

            break;

        case EV_NORX:
            Serial.println(F("EV_NORX"));

            // Slow blinking until next start.
            wave_generator_apply(gen, led_slow_blink);

            break;

        case EV_TXCOMPLETE:
            Serial.print(F("EV_TXCOMPLETE : "));

            // Check for received acknowledge.
            if (LMIC.txrxFlags & TXRX_ACK) {
                Serial.println(F("ack"));

                // While connected, short blink.
                wave_generator_apply(gen, led_short_blink);
            } else {
                Serial.println(F("no ack"));

                // Slow blinking while disconnected.
                wave_generator_apply(gen, led_slow_blink);
            }

            // Schedule next transmission.
            os_setTimedCallback( &job_transmit, os_getTime() + sec2osticks(TRANSMIT_PERIOD), &jobTransmitCallback );

            break;

        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));

            // Try joining again.
            LMIC_startJoining();
            break;

        default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void setup() {
//    while (!Serial);

    Serial.begin(9600);
    delay(100);

    // Setup status/blink led.
    pinMode( LED_STATUS, OUTPUT );

    // Setup led wave generator.
    gen = wave_new_generator( millis );

    // LMIC init
    os_init();
    // Reset the MAC state.
    LMIC_reset();
    // Set clock error because of inaccurate clock.
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
    // Disable ADR (mobile).
    LMIC_setAdrMode(0);

    // Start join (OTAA).
    LMIC_startJoining();
}

void loop() {
    os_runloop_once();
    // Update status/blink leds.
    digitalWrite( LED_STATUS, wave_generator_output( gen ) );
}
