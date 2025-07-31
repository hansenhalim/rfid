# RFID Reader Serial API Implementation

This project implements the RFID Reader Serial API Specification (v1.0) using an ESP32 microcontroller and Adafruit PN532 NFC/RFID module.

## Hardware Requirements

- ESP32 development board (ESP32-C3 Super Mini or ESP32 DevKit v1)
- Adafruit PN532 NFC/RFID Breakout Board
- MIFARE Classic RFID cards/tags

## Pin Configuration

### ESP32-C3 Super Mini

- PN532_SS: GPIO 7
- PN532_RESET: GPIO 0

### ESP32 DevKit v1

- PN532_SS: GPIO 5
- PN532_RESET: GPIO 4

## Serial Configuration

- Baudrate: 115200
- Data Bits: 8
- Parity: None
- Stop Bits: 1
- Flow Control: None
- Line Ending: LF (\n) or CRLF (\r\n)

## Supported Commands

### SCAN_UID

Scan and return the UID of a nearby RFID tag.

**Request:** `SCAN_UID`

**Response:**

- Success: `OK UID <hex_uid>`
- Error: `ERR NO_TAG`

**Example:**

```
> SCAN_UID
< OK UID A1B2C3D4
```

### READ <KEY>

Read data from an RFID tag using a given key.

**Request:** `READ <key>`

- `<key>`: 192-character hex string (96 bytes - 16 sectors x 6 bytes each)

**Response:**

- Success: `OK DATA <hex_data>`
- Error: `ERR AUTH_FAILED` or `ERR NO_TAG`

**Example:**

```
> READ A0A1A2A3A4A5B0B1B2B3B4B5C0C1C2C3C4C5D0D1D2D3D4D5E0E1E2E3E4E5F0F1F2F3F4F5000102030405101112131415202122232425303132333435404142434445505152535455606162636465707172737475808182838485909192939495A0A1A2A3A4A5B0B1B2B3B4B5
< OK DATA [1024 hex characters representing 512 bytes of payload data from blocks 1 and 2 of each sector 0-15]
```

### WRITE <KEY> <DATA>

Write data to an RFID tag using a given key.

**Request:** `WRITE <key> <hex_data>`

- `<key>`: 192-character hex string (96 bytes - 16 sectors x 6 bytes each)
- `<hex_data>`: 1024 hex characters (512 bytes - 16 sectors x 2 blocks x 16 bytes each)

**Response:**

- Success: `OK WRITE_DONE`
- Error: `ERR AUTH_FAILED`, `ERR WRITE_FAIL`, or `ERR NO_TAG`

**Example:**

```
> WRITE A0A1A2A3A4A5B0B1B2B3B4B5C0C1C2C3C4C5D0D1D2D3D4D5E0E1E2E3E4E5F0F1F2F3F4F5000102030405101112131415202122232425303132333435404142434445505152535455606162636465707172737475808182838485909192939495A0A1A2A3A4A5B0B1B2B3B4B5 DEADBEEFCAFEBABE1234567890ABCDEF[...continues for 1024 hex characters total...]
< OK WRITE_DONE
```

### VERSION

Return the firmware version of the RFID reader.

**Request:** `VERSION`

**Response:** `OK VERSION <version_number>`

**Example:**

```
> VERSION
< OK VERSION 1.0.0
```

## Building and Uploading

1. Install PlatformIO Core or use PlatformIO IDE
2. Connect your ESP32 board
3. Build and upload:
   ```bash
   platformio run --target upload
   ```
4. Open serial monitor:
   ```bash
   platformio device monitor
   ```

## Project Structure

- `src/main.cpp` - Main application entry point
- `src/App.cpp` - Main application logic and command handling
- `src/CommandParser.cpp` - Command parsing and validation
- `src/RFIDController.cpp` - RFID hardware interface
- `src/Response.cpp` - Serial response formatting
- `include/` - Header files for all classes
- `platformio.ini` - PlatformIO configuration with library dependencies

## Power Optimization

The implementation includes power optimization features:

- **Automatic Power Management**: The PN532 module is powered down (RSTPDN pin LOW) when not in use
- **On-Demand Power Up**: The module is only powered up when executing RFID commands
- **Immediate Power Down**: After each operation, the module is immediately powered down to save power
- **100ms Power-Up Delay**: Ensures stable operation when powering up the module

This approach significantly reduces power consumption, making it suitable for battery-powered applications.

## Notes

- Commands are case-insensitive
- All hex values in responses are uppercase
- The implementation uses MIFARE Classic authentication with Key A
- Data is read/written from blocks 1 and 2 of each sector (sectors 0-15)
- Block 0 of each sector is typically reserved for sector headers/keys
- Key size: 96 bytes (192 hex chars) for 16 sectors x 6 bytes each
- Payload size: 512 bytes (1024 hex chars) for 16 sectors x 2 blocks x 16 bytes each
- Error handling includes proper response codes as per specification
- Power optimization automatically manages PN532 power state for minimal consumption
