#pragma once
#include "LEDController.h"
#include "RFIDController.h"
#include "CommandParser.h"
#include "Response.h"

class App
{
public:
    App();
    void setup();
    void loop();

private:
    LEDController led;
    RFIDController rfid;
    const uint8_t buttonPin = 9;
    bool lastButtonState = HIGH;

    void handleCommand(const String &cmd);
    void handleButton();
};
