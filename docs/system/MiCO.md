# MiCO Framework

MiCO (MXCHIP IoT Communication Operating System) provides the WiFi stack, RTOS primitives, BSD sockets, hardware drivers, and system services underneath the Arduino layer.

> **Source:** [system/az3166-driver/mico/](../../system/az3166-driver/mico/)

---

## Architecture

```
mico/
├── include/          — Public headers
│   ├── mico.h        — Top-level include
│   ├── mico_common.h — Types, status codes, utility macros
│   ├── mico_wlan.h   — WiFi management
│   ├── mico_rtos.h   — RTOS primitives
│   ├── mico_socket.h — BSD socket API
│   ├── mico_system.h — System framework
│   ├── mico_platform.h — Platform HAL
│   ├── mico_debug.h  — Debug output
│   └── mico_drivers/  — Hardware driver headers
├── net/LwIP/         — lwIP networking integration
├── platform/         — Toolchain/compiler support
├── rtos/             — RTOS common headers
└── system/           — System services and CLI
```

---

## Key Modules

### System (`mico.h`)

| Function | Description |
|----------|-------------|
| `MicoInit()` | Initialize MiCO system |
| `MicoGetVer()` | Get MiCO version string |
| `mico_generate_cid()` | Generate unique chip ID |

### WiFi (`mico_wlan.h`)

WiFi management including:
- Station connect/disconnect
- Network scanning
- EasyLink provisioning
- WPS and Airkiss support
- Monitor mode (raw packet capture)
- Power save modes
- SoftAP mode

### RTOS (`mico_rtos.h`)

| Primitive | Description |
|-----------|-------------|
| Threads | Create, delete, join, yield, sleep |
| Semaphores | Init, get, set, destroy |
| Mutexes | Init, lock, unlock, destroy |
| Queues | Init, push, pop (front/back), destroy |
| Timers | One-shot and periodic timers |
| Events | Event flags for thread synchronization |

### Sockets (`mico_socket.h`)

BSD socket API: `socket()`, `bind()`, `connect()`, `listen()`, `accept()`, `send()`, `recv()`, `select()`, `close()`, `setsockopt()`, `getsockopt()`

### Hardware Drivers (`mico_drivers/`)

| Driver | Description |
|--------|-------------|
| `mico_driver_gpio.h` | GPIO init/finalize, digital read/write, interrupt enable/disable |
| `mico_driver_i2c.h` | I2C init, probe, transfer, message builders (Tx/Rx/Combined) |
| `mico_driver_flash.h` | Flash get_info, erase, write, read, enable_security |

### Platform (`mico_platform.h`)

| Function | Description |
|----------|-------------|
| Reboot | System restart |
| Power save | Enter low-power modes |
| LED control | Status LED management |
| Bootloader | Boot mode control |

### System Services (`mico_system.h`)

- System context and configuration management
- OTA firmware updates
- mDNS service discovery
- System monitors (watchdog-like)
- Notification framework
- Time management
- CLI (command line interface)

---

## See Also

- [WiFi Driver](WiFiDriver.md) — mbed-compatible WiFi interface built on MiCO
- [Board Hardware](BoardHardware.md) — GPIO and peripheral assignments
- [mbed OS](MbedOS.md) — mbed RTOS and driver layer
