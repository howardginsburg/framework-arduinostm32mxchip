# EEPROM Interface

Secure persistent storage via the STSAFE-A100 I2C secure element. Provides zone-based read/write and convenience methods for common credentials.

> **Source:** [cores/arduino/EEPROMInterface.h](../../cores/arduino/EEPROMInterface.h)

---

## Usage

```cpp
#include <EEPROMInterface.h>

EEPROMInterface eeprom;
```

---

## Methods

### Low-Level Zone Access

| Method | Description |
|--------|-------------|
| `int write(uint8_t *dataBuff, int buffSize, uint8_t dataZoneIndex)` | Write data to a zone. Returns 0 on success, -1 on fail. |
| `int read(uint8_t *dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)` | Read data from a zone. Returns bytes read or -1. |
| `int enableHostSecureChannel(int level = 1, uint8_t *key = NULL)` | Enable encrypted I2C. Level 1=hardcode, 2=user key (32 bytes), 3=random. |

### Credential Helpers

All return 0 on success, -1 on failure.

| Method | Description |
|--------|-------------|
| `saveWiFiSetting(char *ssid, char *pwd)` | Save WiFi SSID and password |
| `readWiFiSetting(char *ssid, int ssidSize, char *pwd, int pwdSize)` | Read WiFi credentials |
| `saveDeviceConnectionString(char *connStr)` | Save Azure IoT Hub connection string |
| `readDeviceConnectionString(char *buf, int bufSize)` | Read IoT Hub connection string |
| `saveX509Cert(char *cert)` | Save X.509 certificate |
| `readX509Cert(char *cert, int bufSize)` | Read X.509 certificate |
| `saveMQTTAddress(char *addr)` | Save MQTT broker address |
| `readMQTTAddress(char *addr, int bufSize)` | Read MQTT broker address |
| `saveDeviceID(char *id)` | Save device ID |
| `readDeviceID(char *id, int bufSize)` | Read device ID |
| `saveDevicePassword(char *pwd)` | Save device password |
| `readDevicePassword(char *pwd, int bufSize)` | Read device password |
| `saveClientCert(char *cert)` | Save client certificate (≤192 bytes) |
| `readClientCert(char *cert, int bufSize)` | Read client certificate |
| `saveClientKey(char *key)` | Save client private key (≤880 bytes) |
| `readClientKey(char *key, int bufSize)` | Read client private key |

---

## Storage Zones

| Zone | Size (bytes) | Purpose |
|------|-------------|---------|
| 0 | 976 | General / X.509 cert (part 1) |
| 2 | 192 | Client certificate |
| 3 | 120 | WiFi SSID |
| 5 | 584 | Azure IoT Hub connection string |
| 6 | 680 | DPS UDS |
| 7 | 784 | X.509 cert (part 2) |
| 8 | 880 | Client private key |
| 10 | 88 | WiFi password |

Zones 1, 4, and 9 are not usable (size 0).

---

## Size Limits

| Constant | Value | Description |
|----------|-------|-------------|
| `WIFI_SSID_MAX_LEN` | 32 | Max WiFi SSID length |
| `WIFI_PWD_MAX_LEN` | 64 | Max WiFi password length |
| `AZ_IOT_HUB_MAX_LEN` | 512 | Max IoT Hub connection string |
| `DPS_UDS_MAX_LEN` | 64 | Max DPS UDS length |
| `AZ_IOT_X509_MAX_LEN` | 2639 | Max X.509 cert (zones 0+7+8−1) |
| `MQTT_MAX_LEN` | 512 | Max MQTT broker address |
| `DEVICE_ID_MAX_LEN` | 64 | Max device ID |
| `DEVICE_PASSWORD_MAX_LEN` | 64 | Max device password |
| `CLIENT_CERT_MAX_LEN` | 192 | Max client certificate |
| `CLIENT_KEY_MAX_LEN` | 880 | Max client key |
| `EEPROM_DEFAULT_LEN` | 200 | Default buffer length |

---

## See Also

- [DeviceConfig](DeviceConfig.md) — High-level configuration system built on top of EEPROM zones
