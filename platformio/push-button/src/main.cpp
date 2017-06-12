#include <Arduino.h>

void setup() {
    Serial.begin(9600);
    Serial.println(F("Starting"));

    // Set button pin as input.
    pinMode( 6, INPUT_PULLUP );

    // Set LED pin as output.
    pinMode( 13, OUTPUT );
}

void loop() {
    if(digitalRead( 6 )) {
        digitalWrite( 13 , 0 );
    } else {
        digitalWrite( 13 , 1 );
    }
}