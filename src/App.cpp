#include "App.h"

App::App() {}

void App::setup()
{
    // Serial configuration per API spec: 115200, 8N1, no flow control
    Serial.begin(115200);
    while (!Serial)
        delay(10);

    rfid.begin();
}

void App::loop()
{
    // Handle Serial Commands
    if (Serial.available() > 0)
    {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toUpperCase();
        handleCommand(command);
    }
}

void App::handleCommand(const String &cmd)
{
    ParsedCommand parsed = CommandParser::parse(cmd);

    switch (parsed.code)
    {
    case CommandCode::SCAN_UID:
    {
        String uid = rfid.scanUID();
        if (uid.length() > 0)
        {
            Response::sendOK("UID " + uid);
        }
        else
        {
            Response::sendError("NO_TAG");
        }
    }
    break;

    case CommandCode::READ:
    {
        String data = rfid.readData(parsed.arg1);
        if (data.length() > 0)
        {
            Response::sendOK("DATA " + data);
        }
        else
        {
            Response::sendError("AUTH_FAILED");
        }
    }
    break;

    case CommandCode::WRITE:
    {
        bool success = rfid.writeData(parsed.arg1, parsed.arg2);
        if (success)
        {
            Response::sendOK("WRITE_DONE");
        }
        else
        {
            Response::sendError("WRITE_FAIL");
        }
    }
    break;

    case CommandCode::VERSION:
    {
        String version = rfid.getVersion();
        Response::sendOK("VERSION " + version);
    }
    break;

    default:
        Response::sendError("Unknown command");
        break;
    }
}
