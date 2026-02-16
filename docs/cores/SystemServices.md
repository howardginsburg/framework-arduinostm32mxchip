# System Services

Boot sequence, WiFi management, time synchronization, OTA firmware updates, logging, DNS, and board identity functions.

> **Source:** [cores/arduino/system/](../../cores/arduino/system/)

---

## System Control

```cpp
#include "system/SystemFunc.h"
```

| Function | Description |
|----------|-------------|
| `void SystemReboot(void)` | Reboot the system |
| `void SystemStandby(int timeout)` | Enter sleep mode, wake after `timeout` seconds |
| `void __sys_setup(void)` | Weak function called before Arduino `setup()` — override for early init |

---

## SDK Version

```cpp
#include "system/SystemVersion.h"
```

| Function | Description |
|----------|-------------|
| `const char* getDevkitVersion()` | Full version string (e.g., "2.0.0") |
| `uint32_t getMajorVersion()` | Major version (2) |
| `uint32_t getMinorVersion()` | Minor version (0) |
| `uint32_t getPatchVersion()` | Patch version (0) |

| Constant | Value |
|----------|-------|
| `DEVKIT_MAJOR_VERSION` | 2 |
| `DEVKIT_MINOR_VERSION` | 0 |
| `DEVKIT_PATCH_VERSION` | 0 |
| `DEVKIT_SDK_VERSION` | 20000 |

---

## Tick Counter

```cpp
#include "system/SystemTickCounter.h"
```

| Function | Description |
|----------|-------------|
| `void SystemTickCounterInit(void)` | Initialize 64-bit tick counter |
| `uint64_t SystemTickCounterRead(void)` | Read 64-bit tick counter (microsecond resolution) |

---

## Board Identity

```cpp
#include "system/SystemVariables.h"
```

| Function | Description |
|----------|-------------|
| `int GetMACWithoutColon(char *buff)` | Get MAC address string without colons |
| `const char* GetBoardID(void)` | Get unique board ID (e.g., "az-XXXX") |
| `const char* GetBoardAPName(void)` | Get board AP name (e.g., "AZ-XXXX") |

| Constant | Value |
|----------|-------|
| `BOARD_NAME` | `"AZ3166"` |
| `BOARD_FULL_NAME` | `"MXChip IoT DevKit"` |
| `BOARD_MCU` | `"STM32F412"` |
| `BOARD_ID_LENGTH` | 15 |
| `BOARD_AP_LENGTH` | 15 |

---

## WiFi Management

```cpp
#include "system/SystemWiFi.h"
```

| Function | Description |
|----------|-------------|
| `bool InitSystemWiFi(void)` | Initialize WiFi subsystem |
| `bool SystemWiFiConnect(void)` | Connect using saved credentials |
| `int SystemWiFiRSSI(void)` | Get current RSSI (signal strength) |
| `const char* SystemWiFiSSID(void)` | Get connected SSID |
| `NetworkInterface* WiFiInterface(void)` | Get station-mode NetworkInterface pointer |
| `bool SystemWiFiAPStart(const char *ssid, const char *passphrase)` | Start WiFi AP mode |
| `NetworkInterface* WiFiAPInterface(void)` | Get AP-mode NetworkInterface pointer |
| `int WiFiScan(WiFiAccessPoint *res, unsigned count)` | Scan for WiFi networks |

---

## Time Synchronization

```cpp
#include "system/SystemTime.h"
```

| Function | Description |
|----------|-------------|
| `int SetTimeServer(const char *tsList)` | Set NTP server list (comma-separated). Returns 0 on success. |
| `void SyncTime(void)` | Sync local time from NTP |
| `int IsTimeSynced(void)` | 0 if synced, non-zero if not |

---

## Local DNS

```cpp
#include "system/SystemDns.h"
```

| Function | Description |
|----------|-------------|
| `int SystemDnsLocalAddHost(const char *hostname, const char *addr)` | Add hostname/IP to local DNS. Returns 0 on success. |
| `int SystemDnsLocalRemoveHost(const char *hostname, const char *addr)` | Remove entry from local DNS. Returns count removed. |

---

## Web Configuration Server

```cpp
#include "system/SystemWeb.h"
```

| Function | Description |
|----------|-------------|
| `void EnableSystemWeb(void)` | Enable the web configuration UI |
| `void StartupSystemWeb(void)` | Start the web configuration server |

---

## OTA Firmware Update

```cpp
#include "system/OTAFirmwareUpdate.h"
```

| Function | Description |
|----------|-------------|
| `int OTADownloadFirmware(const char *url, uint16_t *crc16Checksum, const char *ssl_ca_pem = NULL)` | Download firmware from URL. Returns firmware size, -1 for network error, -2 for flash error. |
| `int OTAApplyNewFirmware(int fwSize, uint16_t crc16Checksum)` | Apply downloaded firmware. Returns 0 on success, -1 on fail. Device reboots. |

---

## Serial Logging

```cpp
#include "system/SerialLog.h"
```

| Function | Description |
|----------|-------------|
| `void serial_log(const char *msg)` | Log a plain message to serial |
| `void serial_xlog(const char *format, ...)` | Log a formatted message (printf-style) |

---

## Serial CLI

```cpp
#include "cli/console_cli.h"
```

| Function | Description |
|----------|-------------|
| `void cli_main(void)` | Enter the serial CLI main loop (blocking) |

---

## See Also

- [DeviceConfig](DeviceConfig.md) — Configuration profiles and settings
- [NTP Client](NTPClient.md) — Lower-level NTP API
- [EEPROM](EEPROM.md) — WiFi credentials storage
- [HTTP Server](HTTPServer.md) — Web server implementation
