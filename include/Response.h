#pragma once
#include <Arduino.h>

enum class ResponseStatus
{
    OK,
    ERR,
    INFO,
    TIMEOUT,
    UNKNOWN
};

class Response
{
public:
    static String statusToString(ResponseStatus status);
    static void send(const String &message, ResponseStatus status);
};
