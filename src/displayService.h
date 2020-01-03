#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeMono24pt7b.h>

#define MINPRESSURE 200
#define MAXPRESSURE 1000
#define TFT_ROTATION 1

typedef void (*cbButtonPress_T)();

struct LAP
{
    double currentSpeed = -1;
    unsigned long lapTime;
};


class DisplayService
{
private:
    char buf[10];

    double speed = 0;

    const int XP = 8, XM = A2, YP = A3, YM = 9; //320x480 ID=0x9486
    const int TS_LEFT = 142, TS_RT = 883, TS_TOP = 944, TS_BOT = 101;
    TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
    Adafruit_GFX_Button buttonPlus, buttonMinus;

    cbButtonPress_T cbButtonPlusPressed;
    cbButtonPress_T cbButtonMinusPressed;

    LAP laps[5];

    int pixelX, pixelY;
    bool getTouchXY(void)
    {
        TSPoint p = ts.getPoint();
        pinMode(YP, OUTPUT); //restore shared pins
        pinMode(XM, OUTPUT);
        digitalWrite(YP, HIGH); //because TFT control pins
        digitalWrite(XM, HIGH);
        bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
        if (pressed)
        {
            switch (TFT_ROTATION)
            {
            case 0:
                pixelX = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
                pixelY = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
                break;

            case 1:
                pixelX = map(p.y, TS_TOP, TS_BOT, 0, tft.width());
                pixelY = map(p.x, TS_RT, TS_LEFT, 0, tft.height());
                break;

            default:
                Serial.println("Invalid TFT rotation!");
                break;
            }
            // Serial.print("x,y: ");
            // Serial.print(pixelX);
            // Serial.print(",");
            // Serial.print(pixelY);
            // Serial.println();
        }
        return pressed;
    }

    void paintSpeed(uint16_t textColor, double speed)
    {
        tft.setFont(&FreeSans24pt7b);
        tft.setTextSize(5);
        tft.setCursor(10, 220);
        tft.setTextColor(textColor);
        tft.setTextWrap(false);

        dtostrf(speed, 5, 1, buf);

        for (int i = 0; buf[i] != '\0'; ++i)
        {
            if (buf[i] == '.')
                tft.setTextSize(1);

            tft.print(buf[i]);
        }
    }

public:
    void serviceSetup(cbButtonPress_T _cbButtonPlusPressed, cbButtonPress_T _cbButtonMinusPressed)
    {
        cbButtonPlusPressed = _cbButtonPlusPressed;
        cbButtonMinusPressed = _cbButtonMinusPressed;

        uint16_t ID = tft.readID();
        Serial.print("TFT ID = 0x");
        Serial.println(ID, HEX);
        if (ID == 0xD3D3)
            ID = 0x9486; // write-only shield
        tft.begin(ID);
        tft.setRotation(TFT_ROTATION);
        tft.fillScreen(TFT_BLACK);
        buttonPlus.initButton(&tft, 420, 300, 30, 30, TFT_WHITE, TFT_CYAN, TFT_BLACK, (char*)"+", 2);
        buttonMinus.initButton(&tft, 460, 300, 30, 30, TFT_WHITE, TFT_CYAN, TFT_BLACK, (char*)"-", 2);
        buttonPlus.drawButton(false);
        buttonMinus.drawButton(false);
    }

    void serviceLoop()
    {
        tft.setFont(NULL);
        bool down = getTouchXY();
        buttonPlus.press(down && buttonPlus.contains(pixelX, pixelY));
        buttonMinus.press(down && buttonMinus.contains(pixelX, pixelY));
        if (buttonPlus.justReleased())
            buttonPlus.drawButton();
        if (buttonMinus.justReleased())
            buttonMinus.drawButton();
        if (buttonPlus.justPressed())
        {
            buttonPlus.drawButton(true);
            cbButtonPlusPressed();
        }
        if (buttonMinus.justPressed())
        {
            buttonMinus.drawButton(true);
            cbButtonMinusPressed();
        }
    }

    void addLap(LAP lap) {
        Serial.println(lap.currentSpeed);
        setSpeed(lap.currentSpeed);
    }

    void setSpeed(double newSpeed)
    {
        if (newSpeed != speed)
        {
            // alten Speed loeschen
            paintSpeed(TFT_BLACK, speed);

            speed = newSpeed;
            // neuen Speed schreiben
            paintSpeed(TFT_WHITE, speed);
        }
    }

    void setSpeedLimit(double speedLimit)
    {
        tft.fillRect(410, 250, 70, 25, TFT_DARKGREY);
        tft.setFont(&FreeSans9pt7b);
        tft.setCursor(420, 270);
        tft.setTextColor(TFT_RED);
        tft.setTextSize(1);
        tft.print(dtostrf(speedLimit, 5, 1, buf));
        tft.setFont(NULL);
    }
};
#endif