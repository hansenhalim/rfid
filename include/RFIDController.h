#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "Response.h"

// Debug Configuration
// Uncomment the line below to enable debug messages for development/debugging
// Comment it out for production builds to remove all debug output
// #define RFID_DEBUG_ENABLED

#ifdef RFID_DEBUG_ENABLED
#define RFID_DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define RFID_DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define RFID_DEBUG_PRINT(...)
#define RFID_DEBUG_PRINTLN(...)
#endif

class RFIDController
{
public:
    RFIDController();
    void begin();
    String scanUID();
    String readData(const String &key);
    bool writeData(const String &key, const String &data);
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
    bool authenticateBlock(uint8_t blockNumber, uint8_t *key);
};
