# NTP Client

UDP-based NTP client for synchronizing the device RTC with a network time server.

> **Source:** [cores/arduino/NTPClient/NTPClient.h](../../cores/arduino/NTPClient/NTPClient.h)

---

## Usage

```cpp
#include "NTPClient/NTPClient.h"

NTPClient ntp(WiFiInterface());
NTPResult result = ntp.setTime("pool.ntp.org");
```

---

## Constructor

```cpp
NTPClient(NetworkInterface *networkInterface);
```

---

## Methods

| Method | Description |
|--------|-------------|
| `NTPResult setTime(const char *host, uint16_t port = NTP_DEFAULT_PORT, uint32_t timeout = NTP_DEFAULT_TIMEOUT)` | Sync RTC from NTP server (blocking) |

---

## NTPResult Enum

| Value | Description |
|-------|-------------|
| `NTP_OK` | Success — RTC updated |
| `NTP_DNS` | DNS resolution failed |
| `NTP_PRTCL` | NTP protocol error |
| `NTP_TIMEOUT` | Connection timeout |
| `NTP_CONN` | Connection error |

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `NTP_DEFAULT_PORT` | 123 | Standard NTP port |
| `NTP_DEFAULT_TIMEOUT` | 1000 | Default timeout in ms |

---

## See Also

- [System Services](SystemServices.md) — `SyncTime()` and `SetTimeServer()` for system-level time sync
