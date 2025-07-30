#pragma once
#include <Arduino.h>

enum class CommandCode
{
    SCAN_UID,
    READ,
    WRITE,
    VERSION,
    UNKNOWN
};

struct ParsedCommand
{
    CommandCode code;
    String arg1;
    String arg2;
};

class CommandParser
{
public:
    static ParsedCommand parse(const String &cmd);

private:
    static bool isValidHexString(const String &str, int expectedLength);
};
