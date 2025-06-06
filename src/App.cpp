#include "App.h"

App::App() : led(LED_BUILTIN) {}

void App::setup()
{
    Serial.begin(9600);
    while (!Serial)
        delay(10);

    led.begin();
    Response::send("System Ready", ResponseStatus::INFO);
}

void App::loop()
{
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
    switch (CommandParser::parse(cmd))
    {
    case CommandCode::LED_ON:
        led.on();
        break;
    case CommandCode::LED_OFF:
        led.off();
        break;
    default:
        Response::send("Unknown command", ResponseStatus::ERR);
        break;
    }
}