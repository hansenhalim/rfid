#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <map>
#include "Response.h"

class RFIDController
{
public:
    RFIDController();
    void begin();
    String scanUID();
    String readData(const String &key);
    bool writeData(const String &key, const String &data);
    bool enrollKey(const String &key);
    String getVersion();

private:
    Adafruit_PN532 *nfc;
    uint8_t ssPin;
    uint8_t resetPin;
    bool isNFCPowered;

    bool powerUpNFC();
    void powerDownNFC();
    bool initializeNFC();
    String bytesToHex(uint8_t *data, uint16_t length);
    void hexToBytes(const String &hex, uint8_t *bytes);
    bool isDataAllZeros(const String &data);
    uint16_t readPayloadLength(const String &key, uint8_t *uid, uint8_t uidLength);
    bool writePayloadLength(const String &key, uint16_t length, uint8_t *uid, uint8_t uidLength);
    uint16_t calculatePayloadLength(const String &data);
};
