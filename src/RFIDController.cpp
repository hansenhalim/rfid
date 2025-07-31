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

    // Convert keys from hex string to bytes (96 bytes = 16 sectors x 6 bytes each)
    uint8_t keyBytes[96];
    hexToBytes(key, keyBytes);

    String result = "";
    uint8_t allData[512]; // 16 sectors x 2 blocks x 16 bytes = 512 bytes
    int dataIndex = 0;
    bool allSuccess = true;

    // Read from sectors 0-15, blocks 1 and 2 of each sector
    for (int sector = 0; sector < 16; sector++)
    {
        // Get the key for this sector (6 bytes per sector)
        uint8_t sectorKey[6];
        for (int i = 0; i < 6; i++)
        {
            sectorKey[i] = keyBytes[sector * 6 + i];
        }

        // Calculate block numbers for this sector
        int block1 = sector * 4 + 1; // Block 1 of sector
        int block2 = sector * 4 + 2; // Block 2 of sector

        // Authenticate and read block 1
        if (authenticateBlock(block1, sectorKey))
        {
            uint8_t blockData[16];
            success = nfc->mifareclassic_ReadDataBlock(block1, blockData);
            if (success)
            {
                // Copy block data to result array
                for (int i = 0; i < 16; i++)
                {
                    allData[dataIndex++] = blockData[i];
                }
            }
            else
            {
                allSuccess = false;
                break;
            }
        }
        else
        {
            allSuccess = false;
            break;
        }

        // Authenticate and read block 2
        if (authenticateBlock(block2, sectorKey))
        {
            uint8_t blockData[16];
            success = nfc->mifareclassic_ReadDataBlock(block2, blockData);
            if (success)
            {
                // Copy block data to result array
                for (int i = 0; i < 16; i++)
                {
                    allData[dataIndex++] = blockData[i];
                }
            }
            else
            {
                allSuccess = false;
                break;
            }
        }
        else
        {
            allSuccess = false;
            break;
        }
    }

    if (allSuccess)
    {
        result = bytesToHex(allData, 512);
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

    // Convert keys from hex string to bytes (96 bytes = 16 sectors x 6 bytes each)
    uint8_t keyBytes[96];
    hexToBytes(key, keyBytes);

    // Convert data from hex string to bytes (512 bytes = 16 sectors x 2 blocks x 16 bytes)
    uint8_t dataBytes[512];
    hexToBytes(data, dataBytes);

    bool allSuccess = true;
    int dataIndex = 0;

    // Write to sectors 0-15, blocks 1 and 2 of each sector
    for (int sector = 0; sector < 16; sector++)
    {
        // Get the key for this sector (6 bytes per sector)
        uint8_t sectorKey[6];
        for (int i = 0; i < 6; i++)
        {
            sectorKey[i] = keyBytes[sector * 6 + i];
        }

        // Calculate block numbers for this sector
        int block1 = sector * 4 + 1; // Block 1 of sector
        int block2 = sector * 4 + 2; // Block 2 of sector

        // Prepare data for block 1 (16 bytes)
        uint8_t block1Data[16];
        for (int i = 0; i < 16; i++)
        {
            block1Data[i] = dataBytes[dataIndex++];
        }

        // Authenticate and write block 1
        if (authenticateBlock(block1, sectorKey))
        {
            success = nfc->mifareclassic_WriteDataBlock(block1, block1Data);
            if (!success)
            {
                allSuccess = false;
                break;
            }
        }
        else
        {
            allSuccess = false;
            break;
        }

        // Prepare data for block 2 (16 bytes)
        uint8_t block2Data[16];
        for (int i = 0; i < 16; i++)
        {
            block2Data[i] = dataBytes[dataIndex++];
        }

        // Authenticate and write block 2
        if (authenticateBlock(block2, sectorKey))
        {
            success = nfc->mifareclassic_WriteDataBlock(block2, block2Data);
            if (!success)
            {
                allSuccess = false;
                break;
            }
        }
        else
        {
            allSuccess = false;
            break;
        }
    }

    // Power down NFC module to save power
    powerDownNFC();

    return allSuccess;
}

String RFIDController::getVersion()
{
    return "1.0.0";
}

String RFIDController::bytesToHex(uint8_t *data, uint16_t length)
{
    String result = "";
    for (uint16_t i = 0; i < length; i++)
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

    // Try to authenticate using key B
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    // We need to get the UID first for authentication
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
        return false;

    return nfc->mifareclassic_AuthenticateBlock(uid, uidLength, blockNumber, 1, key);
}
