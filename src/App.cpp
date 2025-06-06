#include "App.h"

App::App() : led(LED_BUILTIN) {}

void App::setup()
{
    Serial.begin(9600);
    while (!Serial)
        delay(10);

    led.begin();

    pinMode(buttonPin, INPUT_PULLUP); // Button on GPIO0, active LOW

    Response::send("System Ready", ResponseStatus::OK);
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

    // Handle Button Press
    handleButton();
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

void App::handleButton()
{
    bool buttonState = digitalRead(buttonPin);

    // Detect falling edge (button press)
    if (lastButtonState == HIGH && buttonState == LOW)
    {
        led.toggle();
    }

    lastButtonState = buttonState;
}
