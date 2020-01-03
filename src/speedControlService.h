#ifndef SPEEDCONTROLSERVICE_H
#define SPEEDCONTROLSERVICE_H

#include <Arduino.h>

typedef void (*cbNewSpeedLimit_T)(double);

class SpeedControlService
{
private:
    double speedLimit = 0;
    char buf[10];
    cbNewSpeedLimit_T cbNewSpeedLimit;

public:
    void serviceSetup(cbNewSpeedLimit_T _cbNewSpeedLimit)
    {
        cbNewSpeedLimit = _cbNewSpeedLimit;
    }

    void setSpeedLimit(double _speedLimit)
    {
        speedLimit = _speedLimit;

        cbNewSpeedLimit(getSpeedLimit());
    }

    double getSpeedLimit() {
        return speedLimit;
    }

    void setNewSpeed(double newSpeed) {
        if(newSpeed > getSpeedLimit()) {
            Serial.println("BLITZ"); // TODO Relais ansteuern
        }
    }
};

#endif