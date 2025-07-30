#pragma once
#include <Arduino.h>

enum class ResponseStatus
{
    OK,
    ERR
};

class Response
{
public:
    static void sendOK(const String &message);
    static void sendError(const String &message);
    static void send(const String &message, ResponseStatus status);
};
