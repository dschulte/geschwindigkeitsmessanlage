#ifndef MEASUREMENTSERVICE_H
#define MEASUREMENTSERVICE_H

#include <Arduino.h>

// TODO als Parameter im setup uebergeben
#define SWITCH_A_PIN 18
#define SWITCH_B_PIN 19

volatile unsigned long switchLastInteruptionMicros = 0; // Letze Aktion
volatile unsigned long switchALastStartMicros = 0; // Erste Aktivierung seit timeout
volatile unsigned long switchBLastStartMicros = 0; // Erste Aktivierung seit timeout

    boolean isInterrupted(uint8_t pin) {
        return digitalRead(pin) == HIGH;
    }

void switchAISR()
{
    switchLastInteruptionMicros = micros();
    if(isInterrupted(SWITCH_A_PIN)) {
        if (switchALastStartMicros == 0)
            switchALastStartMicros = switchLastInteruptionMicros;
    }
}

void switchBISR()
{
    switchLastInteruptionMicros = micros();
    if(isInterrupted(SWITCH_B_PIN)) {
        if (switchBLastStartMicros == 0)
            switchBLastStartMicros = switchLastInteruptionMicros;
    }
}

// ========= ========= ========= ========= =========

typedef void (*cbNewSpeed_T)(double);

class MeasurementService
{
private:
    boolean armed;

    cbNewSpeed_T cbNewSpeed;

    void rearm()
    {
        // Serial.println("rearm");
        switchALastStartMicros = 0;
        switchBLastStartMicros = 0;
        armed = true;
    }

public:
    void serviceSetup(cbNewSpeed_T _cbNewSpeed)
    {
        cbNewSpeed = _cbNewSpeed;

        // TODO andere Richtung (Strecke wieder frei) auch beruecksichtigen
        pinMode(SWITCH_A_PIN, INPUT);
        pinMode(SWITCH_B_PIN, INPUT);
        delay(100);

        attachInterrupt(digitalPinToInterrupt(SWITCH_A_PIN), switchAISR, CHANGE);
        attachInterrupt(digitalPinToInterrupt(SWITCH_B_PIN), switchBISR, CHANGE);

        delay(50);
        rearm();
    }

    void serviceLoop()
    {
        unsigned long currentMicros = micros();

        if(isInterrupted(SWITCH_A_PIN) || isInterrupted(SWITCH_B_PIN)) {
            switchLastInteruptionMicros = currentMicros;
        }


        unsigned long timeout = 2000000; // us // TODO nach oben
        // wenn LS fuer timeout nicht mehr unterbrochen, zuruecksetzen
        unsigned long d = currentMicros - switchLastInteruptionMicros;


        // Serial.print("currentMicros,switchLastInteruptionMicros,diff: ");
        // Serial.print(currentMicros);
        // Serial.print(",");
        // Serial.print(switchLastInteruptionMicros);
        // Serial.print(",");
        // Serial.print(d);
        // Serial.println();

        if (d >= timeout) {
            // Serial.println("timeout");
            rearm();
        }


        // Serial.print("armed,switchALastStartMicros,switchBLastStartMicros: ");
        // Serial.print(armed);
        // Serial.print(",");
        // Serial.print(switchALastStartMicros);
        // Serial.print(",");
        // Serial.print(switchBLastStartMicros);
        // Serial.println();

        // wenn beide Lichtschranken aktiviert wurden
        if (armed && switchALastStartMicros && switchBLastStartMicros)
        {
            armed = false;

            long diff = switchALastStartMicros - switchBLastStartMicros;

            double measureWidth = 1000; // mm // TODO nach oben
            double ms = (((double)diff) / 1000.0);
            double speed = ms == 0 ? 0 : measureWidth * (60.0 * 60.0 * 1000.0) / ms / 1000000.0;

            cbNewSpeed(speed);
        }
    }
};

#endif