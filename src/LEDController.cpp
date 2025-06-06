#include "LEDController.h"

LEDController::LEDController(uint8_t pin) : pin(pin) {}

void LEDController::begin()
{
    pinMode(pin, OUTPUT);
    off(); // Default state
}

void LEDController::on()
{
    digitalWrite(pin, LOW); // LED ON
    Response::send("LED is ON", ResponseStatus::OK);
}

void LEDController::off()
{
    digitalWrite(pin, HIGH); // LED OFF
    Response::send("LED is OFF", ResponseStatus::OK);
}
