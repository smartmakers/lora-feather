#include <wave.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

/* Behavior Parameters */

// Period between two data transmissions (seconds) if there is no change in the inputs.
#define TRANSMIT_PERIOD 10

/* LoRa Parameters */

// APPEUI: Application ID (LSBF)
static const u1_t APPEUI[8] PROGMEM = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// DEVEUI: Unique device ID (LSBF)
static const u1_t DEVEUI[8] PROGMEM = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

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

static uint8_t data[2];
static osjob_t job_transmit;

void jobTransmitCallback(osjob_t* j)
{
    Serial.print(os_getTime());
    Serial.println(F(": transmit"));

    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("ERROR: OP_TXRXPEND, not sending"));
    } else {

        // Save battery level.
        // To decode on the server side:
        // Voltage = Value * (2^3) * 3.3 / 1024;
        data[0] = (uint8_t)(analogRead(PIN_BATTERY) >> 2);

        // Save last RSSI
        // Actual RSSI: (Value - 64)
        // radio.c (l.786), RFM95 (5.5.5, p82)
        data[1] = (uint8_t)(LMIC.rssi);

        // Transmit encoded data (unconfirmed).
        LMIC_setTxData2(1, data, sizeof(data), 1);

        // Fast blinking while trying to send something.
        wave_generator_apply(gen, led_fast_blink);
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

            // Transmit first ping directly.
            os_setCallback( &job_transmit, &jobTransmitCallback );

            break;

        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE"));

            // Check for received acknowledge.
            if (LMIC.txrxFlags & TXRX_ACK) {
                Serial.println(F("Received ack"));

                // While connected, short blink.
                wave_generator_apply(gen, led_short_blink);
            }
            if (LMIC.txrxFlags & TXRX_NACK) {
                Serial.println(F("Ack not received"));

                // Slow blinking while disconnected.
                wave_generator_apply(gen, led_slow_blink);
            }

            // Schedule next transmission (maximum period).
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
