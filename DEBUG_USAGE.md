# RFID Debug System Usage

## Overview

The RFID Controller now includes a comprehensive debug system that can be easily toggled for development and production environments.

## How to Enable/Disable Debug Messages

### For Development (Enable Debug)

1. Open `include/RFIDController.h`
2. Uncomment the line: `#define RFID_DEBUG_ENABLED`
3. Compile and upload your code
4. Open Serial Monitor to see detailed debug output

### For Production (Disable Debug)

1. Open `include/RFIDController.h`
2. Comment out the line: `// #define RFID_DEBUG_ENABLED`
3. Compile and upload your code
4. No debug messages will be output, saving memory and processing time

## Debug Output Example

When debug is enabled, you'll see detailed output like:

```
[DEBUG] readData: Starting data read operation
[DEBUG] readData: NFC object is valid, powering up NFC module
[DEBUG] readData: NFC module powered up successfully
[DEBUG] readData: Attempting to find RFID card...
[DEBUG] readData: Card found! UID Length: 4, UID: DE AD BE EF
[DEBUG] readData: Key length: 192 characters
[DEBUG] readData: Keys converted from hex to bytes
[DEBUG] readData: Starting to read from 16 sectors (blocks 1 and 2 of each sector)
[DEBUG] readData: Processing sector 0
[DEBUG] readData: Sector 0 key: C0 FF EE C0 FF EE
[DEBUG] readData: Reading blocks 1 and 2
[DEBUG] readData: Authenticating block 1
[DEBUG] readData: Authentication successful for block 1
[DEBUG] readData: Successfully read block 1, data: DE AD BE EF DE AD BE EF DE AD BE EF DE AD BE EF
...
```

## What Gets Debugged

The debug system now covers all major RFID Controller functions with detailed logging:

### `begin()` Function

- RFID Controller initialization
- Pin configuration (reset pin setup)
- NFC module power management setup
- PN532 instance creation

### `powerUpNFC()` Function

- Power-up attempts and status
- Reset pin state changes
- Power-up timing and delays
- NFC module initialization calls
- Success/failure status

### `powerDownNFC()` Function

- Power-down operations
- Reset pin state changes
- Power state updates

### `initializeNFC()` Function

- NFC module initialization steps
- Firmware version detection and display
- Passive activation retry configuration
- Initialization success/failure status

### `scanUID()` Function

- UID scanning operations
- Card detection attempts
- UID data display in hex format
- UID conversion to hex string
- Power management during scanning

### `readData()` Function

- Data read operation start/completion
- Card detection and UID display
- Key processing and validation
- Sector-by-sector reading progress (16 sectors)
- Authentication attempts for each block
- Block data reading and hex display
- Error conditions and failure points
- Total bytes read tracking

### `writeData()` Function

- Data write operation start/completion
- Card detection and UID display
- Key and data length validation
- Data conversion from hex to bytes
- Sector-by-sector writing progress (16 sectors)
- Block data preparation and hex display
- Authentication attempts for each block
- Write operation success/failure for each block
- Total bytes written tracking

### `authenticateBlock()` Function

- Block authentication attempts
- Authentication key display
- UID retrieval for authentication
- Authentication method (Key B)
- Authentication success/failure results

## Benefits

### Development Mode (Debug Enabled)

- Detailed troubleshooting information
- Step-by-step execution tracking
- Easy identification of failure points
- Hex data visualization

### Production Mode (Debug Disabled)

- No performance overhead
- Reduced memory usage
- Clean serial output
- Professional deployment ready

## Implementation Details

The system uses preprocessor macros:

- `RFID_DEBUG_PRINT(...)` - Prints without newline
- `RFID_DEBUG_PRINTLN(...)` - Prints with newline

These macros expand to `Serial.print()` calls when debug is enabled, or to nothing when disabled, ensuring zero overhead in production builds.
