# DeviceTwin

Demonstrates Azure IoT Hub Device Twin operations on the MXChip AZ3166.

## Features

- Requests the full device twin on startup
- Listens for desired property updates pushed from the cloud
- Reports properties (firmware version, device model) back to IoT Hub
- Acknowledges desired property changes by reporting `lastDesiredVersion`

## Prerequisites

1. Provision your device in Azure IoT Hub.
2. Store connection credentials in EEPROM using the DeviceConfig system.
3. Set WiFi credentials in the sketch.

## Running

1. Open the Serial Monitor at 115200 baud.
2. The device connects and fetches its twin.
3. Update desired properties in the Azure portal to see them arrive on the device.
