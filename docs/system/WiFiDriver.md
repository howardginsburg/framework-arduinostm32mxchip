# WiFi Driver

EMW10xx WiFi interface for the Cypress BCM43362 chipset, providing mbed `WiFiInterface` compatibility and lwIP network stack integration.

> **Source:** [system/az3166-driver/EMW10xxInterface.h](../../system/az3166-driver/EMW10xxInterface.h)

---

## EMW10xxInterface Class

Extends mbed `WiFiInterface`.

### Methods

| Method | Description |
|--------|-------------|
| `int connect()` | Connect using previously set credentials |
| `int connect(const char *ssid, const char *pass, nsapi_security_t security = NSAPI_SECURITY_NONE, uint8_t channel = 0)` | Connect to a specific network |
| `int set_credentials(const char *ssid, const char *pass, nsapi_security_t security = NSAPI_SECURITY_NONE)` | Set WiFi credentials |
| `int set_channel(uint8_t channel)` | **Not supported** — returns `NSAPI_ERROR_UNSUPPORTED` |
| `int set_interface(wlan_if_t interface)` | Select station or SoftAP mode |
| `int disconnect()` | Disconnect from WiFi |
| `const char* get_ip_address()` | Get current IP address |
| `const char* get_mac_address()` | Get MAC address |
| `const char* get_gateway()` | Get gateway address |
| `const char* get_netmask()` | Get subnet mask |
| `int8_t get_rssi()` | Get signal strength (RSSI) |
| `int scan(WiFiAccessPoint *res, unsigned count)` | Scan for available networks |

---

## lwIP Stack

```cpp
#include "emw10xx_lwip_stack.h"
```

| Function | Description |
|----------|-------------|
| `nsapi_error_t mbed_lwip_init(void)` | Initialize the lwIP stack |
| `nsapi_error_t mbed_lwip_bringup(void)` | Bring up the network interface |

### Thread Safety

```cpp
#include "lwip_lock.h"
```

| Function | Description |
|----------|-------------|
| `void init_lwip_lock(void)` | Initialize lwIP mutex |
| `void lwip_lock(void)` | Lock lwIP core |
| `void lwip_unlock(void)` | Unlock lwIP core |

---

## MiCO WiFi Configuration

From [mico_config.h](../../system/az3166-driver/mico_config.h):

| Constant | Value | Description |
|----------|-------|-------------|
| `MICO_WLAN_CONFIG_MODE` | `CONFIG_MODE_EASYLINK` | WiFi provisioning mode |
| `EasyLink_TimeOut` | 60000 | EasyLink timeout (ms) |
| `EasyLink_ConnectWlan_Timeout` | 20000 | WiFi connection timeout (ms) |
| `MICO_CLI_ENABLE` | defined | Serial CLI enabled |

---

## WiFi Firmware

The WiFi chipset firmware binary is stored in `system/az3166-driver/libwlan/`. The NVRAM configuration for the BCM43362 is in [wifi_nvram_image.h](../../system/az3166-driver/TARGET_AZ3166/wifi_nvram_image.h):

- Device ID: `0x4343`
- Crystal frequency: 26 MHz
- Default MAC: `C8:93:46:00:00:01`
- Band: 2.4 GHz only

---

## See Also

- [mbed OS](MbedOS.md) — NSAPI networking framework
- [Board Hardware](BoardHardware.md) — SDIO pin connections to WiFi chipset
