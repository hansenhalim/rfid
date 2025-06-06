#include "CommandParser.h"

CommandCode CommandParser::parse(const String &cmd)
{
    if (cmd == "LED ON")
        return CommandCode::LED_ON;
    if (cmd == "LED OFF")
        return CommandCode::LED_OFF;
    return CommandCode::UNKNOWN;
}
