#pragma once
#include <Arduino.h>
#include "Response.h"

class LEDController
{
public:
    LEDController(uint8_t pin);
    void begin();
    void on();
    void off();

private:
    uint8_t pin;
};
