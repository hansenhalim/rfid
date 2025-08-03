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
    RFID_DEBUG_PRINTLN("[DEBUG] begin: Initializing RFID Controller");

    RFID_DEBUG_PRINT("[DEBUG] begin: Setting reset pin ");
    RFID_DEBUG_PRINT(resetPin);
    RFID_DEBUG_PRINTLN(" as OUTPUT");
    pinMode(resetPin, OUTPUT);

    RFID_DEBUG_PRINTLN("[DEBUG] begin: Powering down NFC module for optimization");
    // Start with NFC powered down for power optimization
    powerDownNFC();

    RFID_DEBUG_PRINT("[DEBUG] begin: Creating PN532 instance with SS pin ");
    RFID_DEBUG_PRINTLN(ssPin);
    nfc = new Adafruit_PN532(ssPin);

    RFID_DEBUG_PRINTLN("[DEBUG] begin: RFID Controller initialization complete");
}

bool RFIDController::powerUpNFC()
{
    RFID_DEBUG_PRINTLN("[DEBUG] powerUpNFC: Attempting to power up NFC module");

    if (isNFCPowered)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] powerUpNFC: NFC module already powered up");
        return true;
    }

    RFID_DEBUG_PRINT("[DEBUG] powerUpNFC: Setting reset pin ");
    RFID_DEBUG_PRINT(resetPin);
    RFID_DEBUG_PRINTLN(" HIGH to power up PN532");
    // Bring RSTPDN pin HIGH to power up the PN532
    digitalWrite(resetPin, HIGH);

    RFID_DEBUG_PRINTLN("[DEBUG] powerUpNFC: Waiting 100ms for module to power up");
    delay(100); // Give time for the module to power up

    isNFCPowered = true;
    RFID_DEBUG_PRINTLN("[DEBUG] powerUpNFC: Power state updated to ON");

    RFID_DEBUG_PRINTLN("[DEBUG] powerUpNFC: Initializing NFC module");
    // Initialize the NFC module
    bool initResult = initializeNFC();

    RFID_DEBUG_PRINT("[DEBUG] powerUpNFC: Initialization ");
    RFID_DEBUG_PRINTLN(initResult ? "successful" : "failed");

    return initResult;
}

void RFIDController::powerDownNFC()
{
    RFID_DEBUG_PRINTLN("[DEBUG] powerDownNFC: Powering down NFC module");

    RFID_DEBUG_PRINT("[DEBUG] powerDownNFC: Setting reset pin ");
    RFID_DEBUG_PRINT(resetPin);
    RFID_DEBUG_PRINTLN(" LOW to power down PN532");
    // Set RSTPDN pin LOW to power down the PN532
    digitalWrite(resetPin, LOW);

    isNFCPowered = false;
    RFID_DEBUG_PRINTLN("[DEBUG] powerDownNFC: Power state updated to OFF");
}

bool RFIDController::initializeNFC()
{
    RFID_DEBUG_PRINTLN("[DEBUG] initializeNFC: Starting NFC module initialization");

    if (!nfc)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] initializeNFC: ERROR - NFC object is null");
        return false;
    }

    RFID_DEBUG_PRINTLN("[DEBUG] initializeNFC: Calling nfc->begin()");
    nfc->begin();

    RFID_DEBUG_PRINTLN("[DEBUG] initializeNFC: Getting firmware version");
    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] initializeNFC: ERROR - Failed to get firmware version");
        return false;
    }

    RFID_DEBUG_PRINT("[DEBUG] initializeNFC: Firmware version: 0x");
    RFID_DEBUG_PRINTLN(versiondata, HEX);

    RFID_DEBUG_PRINTLN("[DEBUG] initializeNFC: Setting passive activation retries to 0xFE");
    // Set the max number of retry attempts to read from a card
    nfc->setPassiveActivationRetries(0xFE);

    RFID_DEBUG_PRINTLN("[DEBUG] initializeNFC: NFC module initialization successful");
    return true;
}

String RFIDController::scanUID()
{
    RFID_DEBUG_PRINTLN("[DEBUG] scanUID: Starting UID scan operation");

    if (!nfc)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] scanUID: ERROR - NFC object is null");
        return "";
    }

    RFID_DEBUG_PRINTLN("[DEBUG] scanUID: NFC object is valid, powering up NFC module");
    // Power up NFC module for operation
    if (!powerUpNFC())
    {
        RFID_DEBUG_PRINTLN("[DEBUG] scanUID: ERROR - Failed to power up NFC module");
        return "";
    }

    RFID_DEBUG_PRINTLN("[DEBUG] scanUID: NFC module powered up, attempting to find RFID card");
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

    String result = "";
    if (success)
    {
        RFID_DEBUG_PRINT("[DEBUG] scanUID: Card found! UID Length: ");
        RFID_DEBUG_PRINT(uidLength);
        RFID_DEBUG_PRINT(", UID: ");
        for (uint8_t i = 0; i < uidLength; i++)
        {
            if (uid[i] < 0x10)
                RFID_DEBUG_PRINT("0");
            RFID_DEBUG_PRINT(uid[i], HEX);
            RFID_DEBUG_PRINT(" ");
        }
        RFID_DEBUG_PRINTLN();

        result = bytesToHex(uid, uidLength);
        RFID_DEBUG_PRINT("[DEBUG] scanUID: UID converted to hex string: ");
        RFID_DEBUG_PRINTLN(result);
    }
    else
    {
        RFID_DEBUG_PRINTLN("[DEBUG] scanUID: No RFID card found");
    }

    RFID_DEBUG_PRINTLN("[DEBUG] scanUID: Powering down NFC module");
    // Power down NFC module to save power
    powerDownNFC();

    RFID_DEBUG_PRINT("[DEBUG] scanUID: Operation completed, returning result: ");
    RFID_DEBUG_PRINTLN(result);
    return result;
}

String RFIDController::readData(const String &key)
{
    RFID_DEBUG_PRINTLN("[DEBUG] readData: Starting data read operation");

    if (!nfc)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] readData: ERROR - NFC object is null");
        return "";
    }

    RFID_DEBUG_PRINTLN("[DEBUG] readData: NFC object is valid, powering up NFC module");

    // Power up NFC module for operation
    if (!powerUpNFC())
    {
        RFID_DEBUG_PRINTLN("[DEBUG] readData: ERROR - Failed to power up NFC module");
        return "";
    }

    RFID_DEBUG_PRINTLN("[DEBUG] readData: NFC module powered up successfully");

    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    RFID_DEBUG_PRINTLN("[DEBUG] readData: Attempting to find RFID card...");

    // First, find a card
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] readData: ERROR - No RFID card found");
        powerDownNFC();
        return "";
    }

    RFID_DEBUG_PRINT("[DEBUG] readData: Card found! UID Length: ");
    RFID_DEBUG_PRINT(uidLength);
    RFID_DEBUG_PRINT(", UID: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
        if (uid[i] < 0x10)
            RFID_DEBUG_PRINT("0");
        RFID_DEBUG_PRINT(uid[i], HEX);
        RFID_DEBUG_PRINT(" ");
    }
    RFID_DEBUG_PRINTLN();

    RFID_DEBUG_PRINT("[DEBUG] readData: Key length: ");
    RFID_DEBUG_PRINT(key.length());
    RFID_DEBUG_PRINTLN(" characters");

    // Convert keys from hex string to bytes (96 bytes = 16 sectors x 6 bytes each)
    uint8_t keyBytes[96];
    hexToBytes(key, keyBytes);

    RFID_DEBUG_PRINTLN("[DEBUG] readData: Keys converted from hex to bytes");

    String result = "";
    uint8_t allData[512]; // 16 sectors x 2 blocks x 16 bytes = 512 bytes
    int dataIndex = 0;
    bool allSuccess = true;

    RFID_DEBUG_PRINTLN("[DEBUG] readData: Starting to read from 16 sectors (blocks 1 and 2 of each sector)");

    // Read from sectors 0-15, blocks 1 and 2 of each sector
    for (int sector = 0; sector < 16; sector++)
    {
        RFID_DEBUG_PRINT("[DEBUG] readData: Processing sector ");
        RFID_DEBUG_PRINTLN(sector);

        // Get the key for this sector (6 bytes per sector)
        uint8_t sectorKey[6];
        for (int i = 0; i < 6; i++)
        {
            sectorKey[i] = keyBytes[sector * 6 + i];
        }

        RFID_DEBUG_PRINT("[DEBUG] readData: Sector ");
        RFID_DEBUG_PRINT(sector);
        RFID_DEBUG_PRINT(" key: ");
        for (int i = 0; i < 6; i++)
        {
            if (sectorKey[i] < 0x10)
                RFID_DEBUG_PRINT("0");
            RFID_DEBUG_PRINT(sectorKey[i], HEX);
            RFID_DEBUG_PRINT(" ");
        }
        RFID_DEBUG_PRINTLN();

        // Calculate block numbers for this sector
        int block1 = sector * 4 + 1; // Block 1 of sector
        int block2 = sector * 4 + 2; // Block 2 of sector

        RFID_DEBUG_PRINT("[DEBUG] readData: Reading blocks ");
        RFID_DEBUG_PRINT(block1);
        RFID_DEBUG_PRINT(" and ");
        RFID_DEBUG_PRINTLN(block2);

        // Authenticate and read block 1
        RFID_DEBUG_PRINT("[DEBUG] readData: Authenticating block ");
        RFID_DEBUG_PRINTLN(block1);

        if (authenticateBlock(block1, sectorKey))
        {
            RFID_DEBUG_PRINT("[DEBUG] readData: Authentication successful for block ");
            RFID_DEBUG_PRINTLN(block1);

            uint8_t blockData[16];
            success = nfc->mifareclassic_ReadDataBlock(block1, blockData);
            if (success)
            {
                RFID_DEBUG_PRINT("[DEBUG] readData: Successfully read block ");
                RFID_DEBUG_PRINT(block1);
                RFID_DEBUG_PRINT(", data: ");
                for (int i = 0; i < 16; i++)
                {
                    if (blockData[i] < 0x10)
                        RFID_DEBUG_PRINT("0");
                    RFID_DEBUG_PRINT(blockData[i], HEX);
                    RFID_DEBUG_PRINT(" ");
                }
                RFID_DEBUG_PRINTLN();

                // Copy block data to result array
                for (int i = 0; i < 16; i++)
                {
                    allData[dataIndex++] = blockData[i];
                }
            }
            else
            {
                RFID_DEBUG_PRINT("[DEBUG] readData: ERROR - Failed to read block ");
                RFID_DEBUG_PRINTLN(block1);
                allSuccess = false;
                break;
            }
        }
        else
        {
            RFID_DEBUG_PRINT("[DEBUG] readData: ERROR - Authentication failed for block ");
            RFID_DEBUG_PRINTLN(block1);
            allSuccess = false;
            break;
        }

        // Authenticate and read block 2
        RFID_DEBUG_PRINT("[DEBUG] readData: Authenticating block ");
        RFID_DEBUG_PRINTLN(block2);

        if (authenticateBlock(block2, sectorKey))
        {
            RFID_DEBUG_PRINT("[DEBUG] readData: Authentication successful for block ");
            RFID_DEBUG_PRINTLN(block2);

            uint8_t blockData[16];
            success = nfc->mifareclassic_ReadDataBlock(block2, blockData);
            if (success)
            {
                RFID_DEBUG_PRINT("[DEBUG] readData: Successfully read block ");
                RFID_DEBUG_PRINT(block2);
                RFID_DEBUG_PRINT(", data: ");
                for (int i = 0; i < 16; i++)
                {
                    if (blockData[i] < 0x10)
                        RFID_DEBUG_PRINT("0");
                    RFID_DEBUG_PRINT(blockData[i], HEX);
                    RFID_DEBUG_PRINT(" ");
                }
                RFID_DEBUG_PRINTLN();

                // Copy block data to result array
                for (int i = 0; i < 16; i++)
                {
                    allData[dataIndex++] = blockData[i];
                }
            }
            else
            {
                RFID_DEBUG_PRINT("[DEBUG] readData: ERROR - Failed to read block ");
                RFID_DEBUG_PRINTLN(block2);
                allSuccess = false;
                break;
            }
        }
        else
        {
            RFID_DEBUG_PRINT("[DEBUG] readData: ERROR - Authentication failed for block ");
            RFID_DEBUG_PRINTLN(block2);
            allSuccess = false;
            break;
        }

        RFID_DEBUG_PRINT("[DEBUG] readData: Completed sector ");
        RFID_DEBUG_PRINT(sector);
        RFID_DEBUG_PRINT(", total bytes read so far: ");
        RFID_DEBUG_PRINTLN(dataIndex);
    }

    if (allSuccess)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] readData: All sectors read successfully, converting to hex string");
        result = bytesToHex(allData, 512);
        RFID_DEBUG_PRINT("[DEBUG] readData: Final result length: ");
        RFID_DEBUG_PRINT(result.length());
        RFID_DEBUG_PRINTLN(" characters");
    }
    else
    {
        RFID_DEBUG_PRINTLN("[DEBUG] readData: ERROR - Failed to read all sectors");
    }

    RFID_DEBUG_PRINTLN("[DEBUG] readData: Powering down NFC module");

    // Power down NFC module to save power
    powerDownNFC();

    RFID_DEBUG_PRINT("[DEBUG] readData: Operation completed, returning result (length: ");
    RFID_DEBUG_PRINT(result.length());
    RFID_DEBUG_PRINTLN(")");

    return result;
}

bool RFIDController::writeData(const String &key, const String &data)
{
    RFID_DEBUG_PRINTLN("[DEBUG] writeData: Starting data write operation");

    if (!nfc)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] writeData: ERROR - NFC object is null");
        return false;
    }

    RFID_DEBUG_PRINTLN("[DEBUG] writeData: NFC object is valid, powering up NFC module");
    // Power up NFC module for operation
    if (!powerUpNFC())
    {
        RFID_DEBUG_PRINTLN("[DEBUG] writeData: ERROR - Failed to power up NFC module");
        return false;
    }

    RFID_DEBUG_PRINTLN("[DEBUG] writeData: NFC module powered up successfully");
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    RFID_DEBUG_PRINTLN("[DEBUG] writeData: Attempting to find RFID card...");
    // First, find a card
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] writeData: ERROR - No RFID card found");
        powerDownNFC();
        return false;
    }

    RFID_DEBUG_PRINT("[DEBUG] writeData: Card found! UID Length: ");
    RFID_DEBUG_PRINT(uidLength);
    RFID_DEBUG_PRINT(", UID: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
        if (uid[i] < 0x10)
            RFID_DEBUG_PRINT("0");
        RFID_DEBUG_PRINT(uid[i], HEX);
        RFID_DEBUG_PRINT(" ");
    }
    RFID_DEBUG_PRINTLN();

    RFID_DEBUG_PRINT("[DEBUG] writeData: Key length: ");
    RFID_DEBUG_PRINT(key.length());
    RFID_DEBUG_PRINTLN(" characters");

    RFID_DEBUG_PRINT("[DEBUG] writeData: Data length: ");
    RFID_DEBUG_PRINT(data.length());
    RFID_DEBUG_PRINTLN(" characters");

    // Convert keys from hex string to bytes (96 bytes = 16 sectors x 6 bytes each)
    uint8_t keyBytes[96];
    hexToBytes(key, keyBytes);

    // Convert data from hex string to bytes (512 bytes = 16 sectors x 2 blocks x 16 bytes)
    uint8_t dataBytes[512];
    hexToBytes(data, dataBytes);

    RFID_DEBUG_PRINTLN("[DEBUG] writeData: Keys and data converted from hex to bytes");

    bool allSuccess = true;
    int dataIndex = 0;

    RFID_DEBUG_PRINTLN("[DEBUG] writeData: Starting to write to 16 sectors (blocks 1 and 2 of each sector)");

    // Write to sectors 0-15, blocks 1 and 2 of each sector
    for (int sector = 0; sector < 16; sector++)
    {
        RFID_DEBUG_PRINT("[DEBUG] writeData: Processing sector ");
        RFID_DEBUG_PRINTLN(sector);

        // Get the key for this sector (6 bytes per sector)
        uint8_t sectorKey[6];
        for (int i = 0; i < 6; i++)
        {
            sectorKey[i] = keyBytes[sector * 6 + i];
        }

        RFID_DEBUG_PRINT("[DEBUG] writeData: Sector ");
        RFID_DEBUG_PRINT(sector);
        RFID_DEBUG_PRINT(" key: ");
        for (int i = 0; i < 6; i++)
        {
            if (sectorKey[i] < 0x10)
                RFID_DEBUG_PRINT("0");
            RFID_DEBUG_PRINT(sectorKey[i], HEX);
            RFID_DEBUG_PRINT(" ");
        }
        RFID_DEBUG_PRINTLN();

        // Calculate block numbers for this sector
        int block1 = sector * 4 + 1; // Block 1 of sector
        int block2 = sector * 4 + 2; // Block 2 of sector

        RFID_DEBUG_PRINT("[DEBUG] writeData: Writing to blocks ");
        RFID_DEBUG_PRINT(block1);
        RFID_DEBUG_PRINT(" and ");
        RFID_DEBUG_PRINTLN(block2);

        // Prepare data for block 1 (16 bytes)
        uint8_t block1Data[16];
        for (int i = 0; i < 16; i++)
        {
            block1Data[i] = dataBytes[dataIndex++];
        }

        RFID_DEBUG_PRINT("[DEBUG] writeData: Block ");
        RFID_DEBUG_PRINT(block1);
        RFID_DEBUG_PRINT(" data: ");
        for (int i = 0; i < 16; i++)
        {
            if (block1Data[i] < 0x10)
                RFID_DEBUG_PRINT("0");
            RFID_DEBUG_PRINT(block1Data[i], HEX);
            RFID_DEBUG_PRINT(" ");
        }
        RFID_DEBUG_PRINTLN();

        // Authenticate and write block 1
        RFID_DEBUG_PRINT("[DEBUG] writeData: Authenticating block ");
        RFID_DEBUG_PRINTLN(block1);

        if (authenticateBlock(block1, sectorKey))
        {
            RFID_DEBUG_PRINT("[DEBUG] writeData: Authentication successful for block ");
            RFID_DEBUG_PRINTLN(block1);

            success = nfc->mifareclassic_WriteDataBlock(block1, block1Data);
            if (!success)
            {
                RFID_DEBUG_PRINT("[DEBUG] writeData: ERROR - Failed to write block ");
                RFID_DEBUG_PRINTLN(block1);
                allSuccess = false;
                break;
            }
            else
            {
                RFID_DEBUG_PRINT("[DEBUG] writeData: Successfully wrote block ");
                RFID_DEBUG_PRINTLN(block1);
            }
        }
        else
        {
            RFID_DEBUG_PRINT("[DEBUG] writeData: ERROR - Authentication failed for block ");
            RFID_DEBUG_PRINTLN(block1);
            allSuccess = false;
            break;
        }

        // Prepare data for block 2 (16 bytes)
        uint8_t block2Data[16];
        for (int i = 0; i < 16; i++)
        {
            block2Data[i] = dataBytes[dataIndex++];
        }

        RFID_DEBUG_PRINT("[DEBUG] writeData: Block ");
        RFID_DEBUG_PRINT(block2);
        RFID_DEBUG_PRINT(" data: ");
        for (int i = 0; i < 16; i++)
        {
            if (block2Data[i] < 0x10)
                RFID_DEBUG_PRINT("0");
            RFID_DEBUG_PRINT(block2Data[i], HEX);
            RFID_DEBUG_PRINT(" ");
        }
        RFID_DEBUG_PRINTLN();

        // Authenticate and write block 2
        RFID_DEBUG_PRINT("[DEBUG] writeData: Authenticating block ");
        RFID_DEBUG_PRINTLN(block2);

        if (authenticateBlock(block2, sectorKey))
        {
            RFID_DEBUG_PRINT("[DEBUG] writeData: Authentication successful for block ");
            RFID_DEBUG_PRINTLN(block2);

            success = nfc->mifareclassic_WriteDataBlock(block2, block2Data);
            if (!success)
            {
                RFID_DEBUG_PRINT("[DEBUG] writeData: ERROR - Failed to write block ");
                RFID_DEBUG_PRINTLN(block2);
                allSuccess = false;
                break;
            }
            else
            {
                RFID_DEBUG_PRINT("[DEBUG] writeData: Successfully wrote block ");
                RFID_DEBUG_PRINTLN(block2);
            }
        }
        else
        {
            RFID_DEBUG_PRINT("[DEBUG] writeData: ERROR - Authentication failed for block ");
            RFID_DEBUG_PRINTLN(block2);
            allSuccess = false;
            break;
        }

        RFID_DEBUG_PRINT("[DEBUG] writeData: Completed sector ");
        RFID_DEBUG_PRINT(sector);
        RFID_DEBUG_PRINT(", total bytes written so far: ");
        RFID_DEBUG_PRINTLN(dataIndex);
    }

    if (allSuccess)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] writeData: All sectors written successfully");
    }
    else
    {
        RFID_DEBUG_PRINTLN("[DEBUG] writeData: ERROR - Failed to write all sectors");
    }

    RFID_DEBUG_PRINTLN("[DEBUG] writeData: Powering down NFC module");
    // Power down NFC module to save power
    powerDownNFC();

    RFID_DEBUG_PRINT("[DEBUG] writeData: Operation completed, returning ");
    RFID_DEBUG_PRINTLN(allSuccess ? "true" : "false");
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
    RFID_DEBUG_PRINT("[DEBUG] authenticateBlock: Authenticating block ");
    RFID_DEBUG_PRINTLN(blockNumber);

    if (!nfc)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] authenticateBlock: ERROR - NFC object is null");
        return false;
    }

    RFID_DEBUG_PRINT("[DEBUG] authenticateBlock: Using key: ");
    for (int i = 0; i < 6; i++)
    {
        if (key[i] < 0x10)
            RFID_DEBUG_PRINT("0");
        RFID_DEBUG_PRINT(key[i], HEX);
        RFID_DEBUG_PRINT(" ");
    }
    RFID_DEBUG_PRINTLN();

    // Try to authenticate using key B
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    RFID_DEBUG_PRINTLN("[DEBUG] authenticateBlock: Getting card UID for authentication");
    // We need to get the UID first for authentication
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (!success)
    {
        RFID_DEBUG_PRINTLN("[DEBUG] authenticateBlock: ERROR - Failed to get card UID");
        return false;
    }

    RFID_DEBUG_PRINT("[DEBUG] authenticateBlock: Got UID (length ");
    RFID_DEBUG_PRINT(uidLength);
    RFID_DEBUG_PRINT("): ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
        if (uid[i] < 0x10)
            RFID_DEBUG_PRINT("0");
        RFID_DEBUG_PRINT(uid[i], HEX);
        RFID_DEBUG_PRINT(" ");
    }
    RFID_DEBUG_PRINTLN();

    RFID_DEBUG_PRINT("[DEBUG] authenticateBlock: Attempting authentication with key B for block ");
    RFID_DEBUG_PRINTLN(blockNumber);

    bool authResult = nfc->mifareclassic_AuthenticateBlock(uid, uidLength, blockNumber, 1, key);

    RFID_DEBUG_PRINT("[DEBUG] authenticateBlock: Authentication ");
    RFID_DEBUG_PRINT(authResult ? "successful" : "failed");
    RFID_DEBUG_PRINT(" for block ");
    RFID_DEBUG_PRINTLN(blockNumber);

    return authResult;
}
