#include "RFIDController.h"

RFIDController::RFIDController()
{
#ifdef ESP32C3_BOARD
    ssPin = 7;
    resetPin = 0;
#endif

#ifdef ESP32_BOARD
    ssPin = 5;
    resetPin = 4;
#endif

    nfc = nullptr;
    isNFCPowered = false;
}

void RFIDController::begin()
{
    pinMode(resetPin, OUTPUT);
    // Start with NFC powered down for power optimization
    powerDownNFC();

    nfc = new Adafruit_PN532(ssPin);
}

bool RFIDController::powerUpNFC()
{
    if (isNFCPowered)
        return true;

    // Bring RSTPDN pin HIGH to power up the PN532
    digitalWrite(resetPin, HIGH);
    delay(100); // Give time for the module to power up

    isNFCPowered = true;

    // Initialize the NFC module
    return initializeNFC();
}

void RFIDController::powerDownNFC()
{
    // Set RSTPDN pin LOW to power down the PN532
    digitalWrite(resetPin, LOW);
    isNFCPowered = false;
}

bool RFIDController::initializeNFC()
{
    if (!nfc)
        return false;

    nfc->begin();

    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata)
    {
        return false;
    }

    // Set the max number of retry attempts to read from a card
    nfc->setPassiveActivationRetries(0xFF);

    return true;
}

String RFIDController::scanUID()
{
    if (!nfc)
        return "";

    // Power up NFC module for operation
    if (!powerUpNFC())
        return "";

    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

    String result = "";
    if (success)
    {
        result = bytesToHex(uid, uidLength);
    }

    // Power down NFC module to save power
    powerDownNFC();

    return result;
}

String RFIDController::readData(const String &key)
{
    if (!nfc)
        return "";

    // Power up NFC module for operation
    if (!powerUpNFC())
        return "";

    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    // First, find a card
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        powerDownNFC();
        return "";
    }

    // Convert key from hex string to bytes
    uint8_t keyBytes[6];
    hexToBytes(key, keyBytes);

    String result = "";
    // Try to authenticate and read from block 4 (first data block)
    if (authenticateBlock(4, keyBytes))
    {
        uint8_t data[16];
        success = nfc->mifareclassic_ReadDataBlock(4, data);
        if (success)
        {
            result = bytesToHex(data, 16);
        }
    }

    // Power down NFC module to save power
    powerDownNFC();

    return result;
}

bool RFIDController::writeData(const String &key, const String &data)
{
    if (!nfc)
        return false;

    // Power up NFC module for operation
    if (!powerUpNFC())
        return false;

    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    // First, find a card
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        powerDownNFC();
        return false;
    }

    // Convert key from hex string to bytes
    uint8_t keyBytes[6];
    hexToBytes(key, keyBytes);

    // Convert data from hex string to bytes
    uint8_t dataBytes[16] = {0};
    hexToBytes(data, dataBytes);

    bool result = false;
    // Try to authenticate and write to block 4 (first data block)
    if (authenticateBlock(4, keyBytes))
    {
        success = nfc->mifareclassic_WriteDataBlock(4, dataBytes);
        result = success;
    }

    // Power down NFC module to save power
    powerDownNFC();

    return result;
}

String RFIDController::getVersion()
{
    return "1.0.0";
}

String RFIDController::bytesToHex(uint8_t *data, uint8_t length)
{
    String result = "";
    for (uint8_t i = 0; i < length; i++)
    {
        if (data[i] < 0x10)
            result += "0";
        result += String(data[i], HEX);
    }
    result.toUpperCase();
    return result;
}

void RFIDController::hexToBytes(const String &hex, uint8_t *bytes)
{
    for (int i = 0; i < hex.length(); i += 2)
    {
        String byteString = hex.substring(i, i + 2);
        bytes[i / 2] = (uint8_t)strtol(byteString.c_str(), NULL, 16);
    }
}

bool RFIDController::authenticateBlock(uint8_t blockNumber, uint8_t *key)
{
    if (!nfc)
        return false;

    // Try to authenticate using key A
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    // We need to get the UID first for authentication
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
        return false;

    return nfc->mifareclassic_AuthenticateBlock(uid, uidLength, blockNumber, 0, key);
}
