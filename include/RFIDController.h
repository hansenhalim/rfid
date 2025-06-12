#pragma once
#include <Arduino.h>
#include "Response.h"

class RFIDController
{
public:
    RFIDController(uint8_t pin);
    void begin();
    void on();
    void off();

private:
    uint8_t pin;
};
