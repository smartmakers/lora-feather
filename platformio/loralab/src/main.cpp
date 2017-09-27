#include <Arduino.h>
#include <wave.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

/* LoRa Parameters */
#include "../../device-identifiers/qa05.h"

/* Behavior Parameters */

// Period between two data transmissions (seconds) if there is no change in the inputs.
#define TRANSMIT_PERIOD 30

/* End of Parameters */

void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

const lmic_pinmap lmic_pins = {
        .nss = 8,
        .rxtx = LMIC_UNUSED_PIN,
        .rst = PIN_RADIO_RST,
        .dio = {PIN_RADIO_DIO0, 5, LMIC_UNUSED_PIN},
};

// Status led pin.
#define LED_STATUS 13

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

void sendEmptyMsg(u1_t confirmed)
{
    headerOnly[0] = makeHeader(protocolVersion, emptyMsg);

    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, headerOnly, sizeof(headerOnly), confirmed);
}

void sendZeros(u1_t confirmed)
{
    payload[0] = makeHeader(protocolVersion, zerosMsg);
    payload[1] = (uint8_t)(0);
    payload[2] = (uint8_t)(0);

    // Transmit encoded data.
    LMIC_setTxData2(1, payload, 3, confirmed);
}

void sendHello(u1_t confirmed)
{
    payload[0] = makeHeader(protocolVersion, helloMsg);
    memcpy(payload + 1, hello, sizeof(hello));

    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, payload, sizeof(hello) + 1, confirmed);
}

void sendBatteryAndRSSI(u1_t confirmed)
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
    LMIC_setTxData2(1, payload, 3, confirmed);
}

void sendInt32(u1_t confirmed)
{
    payload[0] = makeHeader(protocolVersion, int32Msg);
    // Copy data to send.
    memcpy(payload + 1, (uint8_t*)&int32Val, sizeof(int32Val));

    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, payload, sizeof(int32Val) + 1, confirmed);
}

void sendFloat64(u1_t confirmed)
{
    payload[0] = makeHeader(protocolVersion, float64Msg);
    // Copy data to send.
    memcpy(payload + 1, (uint8_t*)&float64Val, sizeof(float64Val));

    // Transmit encoded data (confirmed).
    LMIC_setTxData2(1, payload, sizeof(float64Val) + 1, confirmed);
}

void reset()
{
    LMIC_reset();

    // Enable ADR.
    LMIC_setAdrMode(1);
    // Set clock error because of inaccurate clock.
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
}

void printDownlinkData() {
    Serial.print(os_getTime());
    Serial.print(": on port ");
    Serial.print(LMIC.frame[LMIC.dataBeg-1]);
    Serial.print(" received ");
    if (LMIC.dataLen != 0) {
        Serial.print("0x");
        for (int idx = LMIC.dataBeg; idx != LMIC.dataBeg + LMIC.dataLen; ++idx) {
            u1_t ch = LMIC.frame[idx];
            if (ch < 10) {
                Serial.print("0");
                Serial.print(ch, HEX);
            } else {
                Serial.print(ch, HEX);
            }
        }
    }  else {
        Serial.print("nothing");
    }
    Serial.println();
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
                sendEmptyMsg(1);
                break;
            case zerosMsg:
                sendZeros(0);
                break;
            case helloMsg:
                sendHello(1);
                break;
            case batteryAndRSSIMsg:
                sendBatteryAndRSSI(0);
                break;
            case int32Msg:
                sendInt32(1);
                break;
            case float64Msg:
                sendFloat64(0);
                break;
            default:
                job_counter = 0;

#ifdef OTAA
                reset();
#else
                Serial.println("Skip reset (abp device)");
#endif
                os_setCallback( &job_transmit, &jobTransmitCallback );
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

            printDownlinkData();

            // Schedule next transmission.
            os_setTimedCallback( &job_transmit, os_getTime() + sec2osticks(TRANSMIT_PERIOD), &jobTransmitCallback );

            break;

        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));

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

    reset();

#ifdef OTAA
#elif defined ABP
    // set the session and immediately start sending stuff
#ifdef PROGMEM
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
#else
    LMIC_setSession(0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif
#else
#error "Need OTAA or ABP set (or simply include the device's header file)"
#endif
    os_setCallback( &job_transmit, &jobTransmitCallback );
}

void loop() {
    os_runloop_once();
    // Update status/blink leds.
    digitalWrite( LED_STATUS, wave_generator_output( gen ) );
}
