#include "CommandParser.h"

CommandCode CommandParser::parse(const String &cmd)
{
    if (cmd == "LED ON")
        return CommandCode::LED_ON;
    if (cmd == "LED OFF")
        return CommandCode::LED_OFF;
    if (cmd == "RFID ON")
        return CommandCode::RFID_ON;
    if (cmd == "RFID OFF")
        return CommandCode::RFID_OFF;
    return CommandCode::UNKNOWN;
}
