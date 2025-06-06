#include "Response.h"

String Response::statusToString(ResponseStatus r)
{
    switch (r)
    {
    case ResponseStatus::OK:
        return "OK";
    case ResponseStatus::ERR:
        return "ERR";
    case ResponseStatus::INFO:
        return "INFO";
    case ResponseStatus::TIMEOUT:
        return "TIMEOUT";
    default:
        return "UNKNOWN";
    }
}

void Response::send(const String &message, ResponseStatus status)
{
    Serial.println(statusToString(status) + ": " + message);
}
