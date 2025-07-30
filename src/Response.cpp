#include "Response.h"

void Response::sendOK(const String &message)
{
    Serial.println("OK " + message);
}

void Response::sendError(const String &message)
{
    Serial.println("ERR " + message);
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
