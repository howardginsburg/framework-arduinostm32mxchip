# SendTelemetry

Sends sensor telemetry (temperature, humidity, pressure) to Azure IoT Hub every 10 seconds.

## Prerequisites

1. Provision your device in Azure IoT Hub.
2. Store connection credentials in EEPROM using the DeviceConfig system (see `cores/arduino/config/`).
3. Set WiFi credentials in the sketch.

## Running

1. Open the Serial Monitor at 115200 baud.
2. The device connects to WiFi, then to IoT Hub.
3. Telemetry messages with sensor data are sent every 10 seconds.
