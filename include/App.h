#pragma once
#include "LEDController.h"
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
    void handleCommand(const String &cmd);
};
