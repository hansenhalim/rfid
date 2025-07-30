#pragma once
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
    RFIDController rfid;

    void handleCommand(const String &cmd);
};
