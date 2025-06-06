#include "LEDController.h"

LEDController::LEDController(uint8_t pin) : pin(pin), state(false) {}

void LEDController::begin()
{
    pinMode(pin, OUTPUT);
    off(); // Default state
}

void LEDController::on()
{
    digitalWrite(pin, LOW); // Active LOW
    state = true;
    Response::send("LED is ON", ResponseStatus::OK);
}

void LEDController::off()
{
    digitalWrite(pin, HIGH); // OFF
    state = false;
    Response::send("LED is OFF", ResponseStatus::OK);
}

void LEDController::toggle()
{
    state ? off() : on();
}

bool LEDController::isOn() const
{
    return state;
}
