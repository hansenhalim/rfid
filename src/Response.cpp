#include "Response.h"

void Response::sendOK(const String &message)
{
    Serial.println("OK " + message);
}

void Response::sendError(const String &message)
{
    Serial.println("ERR " + message);
}

void Response::sendVerboseError(const String &errorCode, const String &description)
{
    Serial.println("ERR " + errorCode + " - " + description);
}

void Response::sendVerboseError(const String &errorCode, const String &description, const String &context)
{
    Serial.println("ERR " + errorCode + " - " + description + " (" + context + ")");
}

void Response::send(const String &message, ResponseStatus status)
{
    if (status == ResponseStatus::OK)
    {
        sendOK(message);
    }
    else
    {
        sendError(message);
    }
}
