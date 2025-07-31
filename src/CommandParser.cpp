#include "CommandParser.h"

ParsedCommand CommandParser::parse(const String &cmd)
{
    ParsedCommand result;
    result.code = CommandCode::UNKNOWN;
    result.arg1 = "";
    result.arg2 = "";

    // Split command into parts
    int firstSpace = cmd.indexOf(' ');
    String command;
    String args = "";

    if (firstSpace == -1)
    {
        command = cmd;
    }
    else
    {
        command = cmd.substring(0, firstSpace);
        args = cmd.substring(firstSpace + 1);
        args.trim();
    }

    // Parse commands
    if (command == "SCAN_UID")
    {
        result.code = CommandCode::SCAN_UID;
    }
    else if (command == "READ")
    {
        if (args.length() == 192 && isValidHexString(args, 192))
        {
            result.code = CommandCode::READ;
            result.arg1 = args;
        }
    }
    else if (command == "WRITE")
    {
        int spacePos = args.indexOf(' ');
        if (spacePos != -1)
        {
            String key = args.substring(0, spacePos);
            String data = args.substring(spacePos + 1);
            key.trim();
            data.trim();

            if (key.length() == 192 && isValidHexString(key, 192) &&
                data.length() == 1024 && isValidHexString(data, 1024))
            {
                result.code = CommandCode::WRITE;
                result.arg1 = key;
                result.arg2 = data;
            }
        }
    }
    else if (command == "VERSION")
    {
        result.code = CommandCode::VERSION;
    }

    return result;
}

bool CommandParser::isValidHexString(const String &str, int expectedLength)
{
    if (str.length() != expectedLength)
        return false;

    for (int i = 0; i < str.length(); i++)
    {
        char c = str.charAt(i);
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
        {
            return false;
        }
    }
    return true;
}
