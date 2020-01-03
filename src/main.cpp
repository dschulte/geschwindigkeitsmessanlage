#include <Arduino.h>

#include <speedControlService.h>
#include <measurementService.h>
#include <displayService.h>

SpeedControlService speedControlService;
MeasurementService measurementService;
DisplayService displayService;

void buttonPlusPressedCB()
{
    Serial.println("buttonPlusPressedCB");
    speedControlService.setSpeedLimit(speedControlService.getSpeedLimit() + 1);
}

void buttonMinusPressedCB()
{
    Serial.println("buttonMinusPressedCB");
    speedControlService.setSpeedLimit(speedControlService.getSpeedLimit() - 1);
}

void newSpeedCB(double newSpeed)
{
    Serial.println("newSpeedCB");
    speedControlService.setNewSpeed(newSpeed);

    LAP lap;
    lap.currentSpeed = abs(newSpeed);
    lap.lapTime = 83000; // FIXME
    displayService.addLap(lap);
}

void newSpeedLimitCB(double newSpeedLimit)
{
    Serial.println("newSpeedLimitCB");
    displayService.setSpeedLimit(newSpeedLimit);
}

void setup(void)
{
    Serial.begin(9600);
    delay(100);

    speedControlService.serviceSetup(newSpeedLimitCB);
    measurementService.serviceSetup(newSpeedCB);
    displayService.serviceSetup(buttonPlusPressedCB, buttonMinusPressedCB);

    speedControlService.setSpeedLimit(42);
}

void loop(void)
{
    measurementService.serviceLoop();
    displayService.serviceLoop();
}
