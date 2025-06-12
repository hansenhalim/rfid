#pragma once
#include <Arduino.h>

enum class CommandCode
{
    LED_ON,
    LED_OFF,
    RFID_ON,
    RFID_OFF,
    UNKNOWN
};

class CommandParser
{
public:
    static CommandCode parse(const String &cmd);
};
