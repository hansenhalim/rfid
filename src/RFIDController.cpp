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
    {
        return true;
    }

    // Bring RSTPDN pin HIGH to power up the PN532
    digitalWrite(resetPin, HIGH);

    delay(100); // Give time for the module to power up

    isNFCPowered = true;

    // Initialize the NFC module
    bool initResult = initializeNFC();

    return initResult;
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
    {
        return false;
    }

    nfc->begin();

    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata)
    {
        return false;
    }

    // Set the max number of retry attempts to read from a card
    nfc->setPassiveActivationRetries(0xFE);

    return true;
}

String RFIDController::scanUID()
{
    if (!nfc)
    {
        return "";
    }

    // Power up NFC module for operation
    if (!powerUpNFC())
    {
        return "";
    }

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
    {
        return "";
    }

    // Power up NFC module for operation
    if (!powerUpNFC())
    {
        return "";
    }

    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    // First, find a card
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        powerDownNFC();
        return "";
    }

    // Optimization: Check RFID tombstone flag first to avoid unnecessary reads
    if (readTombstoneFlag(key))
    {
        powerDownNFC();
        // Return 512 bytes of zeros (1024 hex characters)
        String zeroData = "";
        for (int i = 0; i < 1024; i++)
        {
            zeroData += "0";
        }
        return zeroData;
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
    {
        return false;
    }

    // Power up NFC module for operation
    if (!powerUpNFC())
    {
        return false;
    }

    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    // First, find a card
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        powerDownNFC();
        return false;
    }

    // Check if writing all zeros - if so, only set tombstone flag and skip actual write
    if (isDataAllZeros(data))
    {
        bool tombstoneResult = writeTombstoneFlag(key, true);
        powerDownNFC();
        return tombstoneResult;
    }
    else
    {
        // Clear tombstone flag for non-zero writes
        writeTombstoneFlag(key, false);
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

bool RFIDController::enrollKey(const String &key)
{
    if (!nfc)
    {
        return false;
    }

    // Power up NFC module for operation
    if (!powerUpNFC())
    {
        return false;
    }

    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    // First, find a card
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        powerDownNFC();
        return false;
    }

    // Convert key from hex string to bytes (96 bytes = 16 sectors x 6 bytes each)
    uint8_t keyBytes[96];
    hexToBytes(key, keyBytes);

    bool allSuccess = true;

    // Write to block 3 (4th block) of each sector (0-15)
    for (int sector = 0; sector < 16; sector++)
    {
        // Get the current key for this sector (6 bytes per sector)
        uint8_t currentKey[6];
        for (int i = 0; i < 6; i++)
        {
            currentKey[i] = keyBytes[sector * 6 + i];
        }

        // Calculate block 3 (sector trailer) for this sector
        int block3 = sector * 4 + 3;

        // Use factory default key for enrollment authentication
        uint8_t factoryKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        if (authenticateBlock(block3, factoryKey))
        {
            // Prepare the sector trailer data with the specified format
            uint8_t sectorTrailerData[16];

            // Set Key A based on sector: "A0A1A2A3A4A5" for sector 0, "D3F7D3F7D3F7" for others
            if (sector == 0)
            {
                // Set Key A: A0A1A2A3A4A5 for sector 0
                sectorTrailerData[0] = 0xA0;
                sectorTrailerData[1] = 0xA1;
                sectorTrailerData[2] = 0xA2;
                sectorTrailerData[3] = 0xA3;
                sectorTrailerData[4] = 0xA4;
                sectorTrailerData[5] = 0xA5;
            }
            else
            {
                // Set Key A: D3F7D3F7D3F7 for all other sectors
                sectorTrailerData[0] = 0xD3;
                sectorTrailerData[1] = 0xF7;
                sectorTrailerData[2] = 0xD3;
                sectorTrailerData[3] = 0xF7;
                sectorTrailerData[4] = 0xD3;
                sectorTrailerData[5] = 0xF7;
            }

            // Set access bits: 1F01EE00 (default access bits)
            sectorTrailerData[6] = 0x1F;
            sectorTrailerData[7] = 0x01;
            sectorTrailerData[8] = 0xEE;
            sectorTrailerData[9] = 0x00;

            // Set the 6-byte key for this sector (Key B)
            for (int i = 0; i < 6; i++)
            {
                sectorTrailerData[10 + i] = keyBytes[sector * 6 + i];
            }

            // Write the sector trailer
            success = nfc->mifareclassic_WriteDataBlock(block3, sectorTrailerData);
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
    return "1.2.0";
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
    {
        return false;
    }

    // Try to authenticate using key B
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    // We need to get the UID first for authentication
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        return false;
    }

    bool authResult = nfc->mifareclassic_AuthenticateBlock(uid, uidLength, blockNumber, 1, key);

    return authResult;
}

bool RFIDController::isDataAllZeros(const String &data)
{
    for (int i = 0; i < data.length(); i++)
    {
        if (data.charAt(i) != '0')
        {
            return false;
        }
    }
    return true;
}

bool RFIDController::readTombstoneFlag(const String &key)
{
    if (!nfc)
    {
        return false;
    }

    // Convert keys from hex string to bytes
    uint8_t keyBytes[96];
    hexToBytes(key, keyBytes);

    // Get the key for sector 1 (6 bytes per sector, so sector 1 starts at index 6)
    uint8_t sectorKey[6];
    for (int i = 0; i < 6; i++)
    {
        sectorKey[i] = keyBytes[6 + i]; // sector 1 key
    }

    // Sector 1, block 0 = block number 4
    int tombstoneBlock = 4;

    // Authenticate and read tombstone block
    if (authenticateBlock(tombstoneBlock, sectorKey))
    {
        uint8_t blockData[16];
        bool success = nfc->mifareclassic_ReadDataBlock(tombstoneBlock, blockData);
        if (success)
        {
            // Check first byte for tombstone flag (0xAA = tombstoned, anything else = not tombstoned)
            return (blockData[0] == 0xAA);
        }
    }
    
    return false; // Default to not tombstoned if read fails
}

bool RFIDController::writeTombstoneFlag(const String &key, bool flagValue)
{
    if (!nfc)
    {
        return false;
    }

    // Convert keys from hex string to bytes
    uint8_t keyBytes[96];
    hexToBytes(key, keyBytes);

    // Get the key for sector 1 (6 bytes per sector, so sector 1 starts at index 6)
    uint8_t sectorKey[6];
    for (int i = 0; i < 6; i++)
    {
        sectorKey[i] = keyBytes[6 + i]; // sector 1 key
    }

    // Sector 1, block 0 = block number 4
    int tombstoneBlock = 4;

    // Authenticate and write tombstone block
    if (authenticateBlock(tombstoneBlock, sectorKey))
    {
        uint8_t blockData[16];
        
        // Set first byte as tombstone flag
        blockData[0] = flagValue ? 0xAA : 0x00; // 0xAA = tombstoned, 0x00 = not tombstoned
        
        // Fill rest with zeros
        for (int i = 1; i < 16; i++)
        {
            blockData[i] = 0x00;
        }

        bool success = nfc->mifareclassic_WriteDataBlock(tombstoneBlock, blockData);
        return success;
    }
    
    return false;
}
