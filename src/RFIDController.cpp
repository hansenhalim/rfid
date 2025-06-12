#include "RFIDController.h"

RFIDController::RFIDController(uint8_t pin) : pin(pin) {}

void RFIDController::begin()
{
    pinMode(pin, OUTPUT);
    off(); // Default state
}

void RFIDController::on()
{
    digitalWrite(pin, HIGH); // Active HIGH
    Response::send("RFID is ON", ResponseStatus::OK);
}

void RFIDController::off()
{
    digitalWrite(pin, LOW); // OFF
    Response::send("RFID is OFF", ResponseStatus::OK);
}
