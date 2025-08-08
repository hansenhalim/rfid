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
    static void sendVerboseError(const String &errorCode, const String &description);
    static void sendVerboseError(const String &errorCode, const String &description, const String &context);
    static void send(const String &message, ResponseStatus status);
};
