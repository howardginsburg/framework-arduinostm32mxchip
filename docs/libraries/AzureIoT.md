# AzureIoT Library

**Author:** Microsoft

Connects the MXChip AZ3166 to Azure IoT Hub and Azure Device Provisioning Service (DPS) over MQTT. Supports multiple authentication methods: SAS tokens (individual and group enrollment), X.509 certificates, and connection strings.

---

## Quick Start

```cpp
#include <AZ3166WiFi.h>
#include <AzureIoTHub.h>
#include <DeviceConfig.h>

void onC2D(const char* topic, const char* payload, unsigned int len) {
    Serial.println(payload);
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(DeviceConfig_GetWifiSsid(), DeviceConfig_GetWifiPassword());

    azureIoTInit();
    azureIoTConnect();
    azureIoTSetC2DCallback(onC2D);
}

void loop() {
    azureIoTLoop();
    azureIoTSendTelemetry("{\"temp\":25.3}");
    delay(10000);
}
```

---

## Supported Profiles

| Profile | Authentication | Description |
|---------|---------------|-------------|
| `PROFILE_IOTHUB_SAS` | Connection string | Direct IoT Hub connection with SAS token |
| `PROFILE_IOTHUB_CERT` | X.509 certificate | Direct IoT Hub connection with device certificate |
| `PROFILE_DPS_SAS` | Symmetric key | Individual DPS enrollment |
| `PROFILE_DPS_SAS_GROUP` | Group symmetric key | Group DPS enrollment with key derivation |
| `PROFILE_DPS_CERT` | X.509 certificate | DPS enrollment with device certificate |

Set the profile in `platformio.ini`:

```ini
build_flags = -DCONNECTION_PROFILE=PROFILE_DPS_SAS
```

---

## API Reference

### Modules

| File | Purpose |
|------|---------|
| `AzureIoTHub.h/.cpp` | Main API — init, connect, telemetry, C2D, Device Twin |
| `AzureIoTDPS.h/.cpp` | DPS registration over MQTT |
| `AzureIoTCrypto.h/.cpp` | SAS tokens, HMAC-SHA256, URL encoding, group key derivation |
| `AzureIoTConfig.h` | Protocol constants and Azure root CA certificate |

### Initialization & Connection

| Function | Signature | Description |
|----------|-----------|-------------|
| `azureIoTInit` | `bool azureIoTInit()` | Load config, provision via DPS if needed |
| `azureIoTConnect` | `bool azureIoTConnect()` | Connect to IoT Hub over MQTT |
| `azureIoTIsConnected` | `bool azureIoTIsConnected()` | Check connection status |
| `azureIoTLoop` | `void azureIoTLoop()` | Process incoming MQTT messages — call in `loop()` |

### Telemetry

| Function | Signature | Description |
|----------|-----------|-------------|
| `azureIoTSendTelemetry` | `bool azureIoTSendTelemetry(const char* payload, const char* properties = NULL)` | Send D2C telemetry message |

### Cloud-to-Device Messages

```cpp
void myC2DHandler(const char* topic, const char* payload, unsigned int length) {
    // Handle incoming message
}
azureIoTSetC2DCallback(myC2DHandler);
```

### Device Twin

| Function | Signature | Description |
|----------|-----------|-------------|
| `azureIoTRequestTwin` | `void azureIoTRequestTwin()` | Request full twin document |
| `azureIoTUpdateReportedProperties` | `void azureIoTUpdateReportedProperties(const char* jsonPayload)` | Update reported properties |
| `azureIoTSetDesiredPropertiesCallback` | `void azureIoTSetDesiredPropertiesCallback(callback)` | Handle desired property changes |
| `azureIoTSetTwinReceivedCallback` | `void azureIoTSetTwinReceivedCallback(callback)` | Handle full twin response |

### Accessors

| Function | Signature | Description |
|----------|-----------|-------------|
| `azureIoTGetDeviceId` | `const char* azureIoTGetDeviceId()` | Get the device ID |
| `azureIoTGetHostname` | `const char* azureIoTGetHostname()` | Get the IoT Hub hostname |

### DPS Registration (AzureIoTDPS.h)

```cpp
bool AzureIoT_DPSRegister(
    WiFiClientSecure& wifiClient,
    const char* endpoint,
    const char* scopeId,
    const char* registrationId,
    const char* password,
    char* assignedHub, size_t hubSize,
    char* assignedDeviceId, size_t deviceIdSize
);
```

### Cryptographic Utilities (AzureIoTCrypto.h)

| Function | Description |
|----------|-------------|
| `AzureIoT_UrlEncode(input, output, outputSize)` | RFC 3986 URL encoding |
| `AzureIoT_HmacSHA256(key, keyLen, data, dataLen, output, outputSize)` | HMAC-SHA256 (output >= 32 bytes) |
| `AzureIoT_GenerateSasToken(host, key, keyLen, expiry, output, outputSize)` | Generate SAS authentication token |
| `AzureIoT_DeriveGroupKey(masterKey, masterKeyLen, registrationId, output, outputSize)` | Derive device key from group master key |

---

## Protocol Constants (AzureIoTConfig.h)

| Constant | Value | Description |
|----------|-------|-------------|
| `IOT_HUB_API_VERSION` | `"2021-04-12"` | IoT Hub MQTT API version |
| `MQTT_PORT` | 8883 | TLS MQTT port |
| `SAS_TOKEN_DURATION` | 86400 | SAS token validity (seconds) |
| `DPS_API_VERSION` | `"2021-06-01"` | DPS API version |
| `DPS_POLL_INTERVAL` | 3000 | DPS registration poll interval (ms) |
| `DPS_MAX_RETRIES` | 10 | DPS registration max retries |

---

## Dependencies

- [PubSubClient](PubSubClient.md) — MQTT transport
- [WiFi](WiFi.md) — `WiFiClientSecure` for TLS connections
- DeviceConfig (core) — EEPROM credential storage
