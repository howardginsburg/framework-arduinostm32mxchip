# AzureIoT Library

Arduino library for connecting the MXChip AZ3166 to Azure IoT Hub and the Azure Device Provisioning Service (DPS) over MQTT.

## Features

- Direct connection to Azure IoT Hub (SAS token or X.509 certificate)
- Device provisioning via Azure DPS (individual SAS, group SAS, or X.509)
- Device-to-cloud (D2C) telemetry with optional message properties
- Cloud-to-device (C2D) message reception
- Device Twin: read full twin, receive desired property updates, update reported properties
- SAS token generation, HMAC-SHA256, and group key derivation

## Connection Profiles

The library supports five connection profiles, selected at compile time via the `CONNECTION_PROFILE` define:

| Profile | Macro | Description |
|---|---|---|
| IoT Hub + SAS | `PROFILE_IOTHUB_SAS` | Direct connection using a device connection string with a shared access key |
| IoT Hub + X.509 | `PROFILE_IOTHUB_CERT` | Direct connection using a device certificate and private key |
| DPS + Individual SAS | `PROFILE_DPS_SAS` | Provision via DPS using an individual symmetric key |
| DPS + Group SAS | `PROFILE_DPS_SAS_GROUP` | Provision via DPS using a group enrollment master key (device key is derived automatically) |
| DPS + X.509 | `PROFILE_DPS_CERT` | Provision via DPS using a device certificate |

## Source Files

| File | Purpose |
|---|---|
| `src/AzureIoTHub.h / .cpp` | Main API — initialization, MQTT connection, telemetry, C2D, and Device Twin operations |
| `src/AzureIoTDPS.h / .cpp` | DPS registration over MQTT (SAS and X.509) |
| `src/AzureIoTCrypto.h / .cpp` | SAS token generation, HMAC-SHA256, URL encoding, group key derivation |
| `src/AzureIoTConfig.h` | Protocol constants (API versions, MQTT port, SAS TTL) and the Azure root CA certificate |

## Usage

### Initialization and Connection

```cpp
#include "AzureIoTHub.h"

void setup() {
    // Connect to WiFi first, then:
    if (azureIoTInit()) {
        azureIoTConnect();
    }
}

void loop() {
    azureIoTLoop();  // process incoming MQTT messages
}
```

### Sending Telemetry

```cpp
azureIoTSendTelemetry("{\"temperature\":22.5}");

// With message properties (URL-encoded key=value pairs separated by &)
azureIoTSendTelemetry("{\"temperature\":22.5}", "sensor=dht22&location=lab");
```

### Receiving Cloud-to-Device Messages

```cpp
void onC2D(const char* topic, const char* payload, unsigned int length) {
    Serial.println(payload);
}

azureIoTSetC2DCallback(onC2D);
```

### Device Twin

```cpp
// Receive full twin
void onTwinReceived(const char* payload) {
    Serial.println(payload);
}
azureIoTSetTwinReceivedCallback(onTwinReceived);
azureIoTRequestTwin();

// React to desired property changes
void onDesired(const char* payload, int version) {
    Serial.println(payload);
}
azureIoTSetDesiredPropertiesCallback(onDesired);

// Update reported properties
azureIoTUpdateReportedProperties("{\"firmwareVersion\":\"1.0.0\"}");
```

## Device Configuration

Credentials are stored in EEPROM and managed through the serial CLI (see `DeviceConfig`). The required settings depend on the active connection profile:

| Setting | CLI Command | Profiles |
|---|---|---|
| Connection string | `set_az_iothub` | IoT Hub SAS, IoT Hub Cert |
| DPS endpoint | `set_dps_endpoint` | DPS SAS, DPS Group SAS, DPS Cert |
| Scope ID | `set_scopeid` | DPS SAS, DPS Group SAS, DPS Cert |
| Registration ID | `set_regid` | DPS SAS, DPS Group SAS, DPS Cert |
| Symmetric key | `set_symkey` | DPS SAS, DPS Group SAS |
| Device certificate + key | `set_devicecert` | IoT Hub Cert, DPS Cert |

## Dependencies

- **PubSubClient** — MQTT client
- **WiFi** — `WiFiClientSecure` for TLS connections
- **DeviceConfig** — EEPROM credential storage
