# Azure AZ3166 MXChip IoT DevKit SDK Update

This project is a fork of the original [Azure IoT SDK](https://github.com/microsoft/devkit-sdk) for the MXChip AZ3166 IoT DevKit for PlatformIO. The original SDK is no longer maintained, and this fork aims to provide an updated version of the SDK with the latest features.

## Features

- Removal of the board telemetry collector code as it is now defunct.
- Removal of the outdated Azure IoT Hub code.
- Removal of the MQTT libary that did not support TLS.  Updated WiFi libaries to support WiFiClientSecure so that any MQTT Library that supports WiFiClientSecure can be used.
- Updated the EEPromInterface to add new functions to store broker address, device ID, and device password as an alternative to using the Azure IoT Hub.
- Updated the CLI that is triggered when pressing Reset + A to have commands to set the broker address, device ID, and device password.

## TLS/SSL Limitations

### Publish-Only MQTT Operation

The `WiFiClientSecure` classes have been modified to support stable MQTT publish operations over TLS. However, there is an important limitation:

**MQTT subscriptions and message receiving are NOT supported** when using `WiFiClientSecure` with libraries like PubSubClient.

### Technical Details

The underlying mbedTLS implementation (version from 2020) has the following behavior:

1. When `PubSubClient::loop()` calls `available()` and `read()` during idle periods, the TLS socket either:
   - Receives `MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY` (-30848) from the server
   - Times out waiting for data

2. These error conditions were previously treated as fatal disconnections, causing:
   - Unnecessary socket teardown
   - Reconnection cycles every few seconds
   - Unstable connection even when publishing works fine

### Workaround for Applications

When using `WiFiClientSecure` with PubSubClient for Azure Event Grid:

```cpp
void loop() {
    // Do NOT call mqttClient.loop() - it triggers read timeouts
    
    if (!wifiClient.connected()) {
        reconnect();
    }
    
    // Just publish periodically
    if (millis() - lastPublish >= 5000) {
        mqttClient.publish(topic, payload);
        lastPublish = millis();
    }
}
```

## Usage

1. Make sure you have the latest firmware on your AZ3166 board. You can find the latest firmware [here](/firmware/devkit-firmware-2.0.0.bin).  Just copy the firmware file to the root of the AZ3166 board and it will automatically flash the firmware.  
    - This is especially the case if you have tried running the Eclipse RTOS sample as it appears to change the bootloader.
1. Install VSCode and the PlatformIO extension.
1. Create a new PlatformIO project using the AZ3166 board.
1. Update the `platformio.ini` file to point to this repository.
    ```
    platform_packages =
        framework-arduinostm32mxchip@https://github.com/howardginsburg/framework-arduinostm32mxchip.git
    ```