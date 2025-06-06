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
    void toggle();
    bool isOn() const;

private:
    uint8_t pin;
    bool state; // true = ON
};
