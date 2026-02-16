# mbed OS

mbed OS 5.4.3 provides the RTOS kernel, hardware abstraction, networking, filesystem, and TLS for the AZ3166.

> **Source:** [system/mbed-os/](../../system/mbed-os/)

---

## Included Subsystems

| Subsystem | Directory | Description |
|-----------|-----------|-------------|
| CMSIS | `cmsis/` | ARM Cortex-M core headers |
| Drivers | `drivers/` | mbed driver abstractions (DigitalOut, I2C, SPI, Serial, etc.) |
| Events | `events/` | EventQueue for deferred execution |
| Features | `features/` | Storage, networking (lwIP/NSAPI), mbedTLS |
| HAL | `hal/` | Hardware Abstraction Layer |
| Platform | `platform/` | Platform utilities (Callback, CriticalSection, etc.) |
| RTOS | `rtos/` | Thread, Mutex, Semaphore, Queue, EventFlags |
| Targets | `targets/` | TARGET_MXCHIP/TARGET_AZ3166 pin definitions |

The master include is `mbed.h`.

---

## Build Configuration

Key defines from [system/mbed_config.h](../../system/mbed_config.h):

| Define | Value | Description |
|--------|-------|-------------|
| `MBED_CONF_RTOS_PRESENT` | 1 | RTOS enabled |
| `MBED_CONF_NSAPI_PRESENT` | 1 | Networking API enabled |
| `MBED_CONF_FILESYSTEM_PRESENT` | 1 | Filesystem support |
| `MBED_CONF_EVENTS_PRESENT` | 1 | EventQueue enabled |
| `MBED_CONF_PLATFORM_STDIO_BAUD_RATE` | 9600 | Default stdio baud rate |
| `MBED_STACK_STATS_ENABLED` | 1 | Stack usage statistics |
| `MBED_HEAP_STATS_ENABLED` | 1 | Heap usage statistics |

### TLS Configuration

| Define | Description |
|--------|-------------|
| `USE_MBED_TLS` | Enable mbedTLS |
| `MBEDTLS_SSL_PROTO_TLS1_1` | TLS 1.1 support |
| `MBEDTLS_SSL_PROTO_TLS1_2` | TLS 1.2 support |
| `MBEDTLS_SHA256_C` | SHA-256 enabled |
| `MBEDTLS_SHA1_C` | SHA-1 enabled |
| `MBEDTLS_MD5_C` | MD5 enabled |
| `MBEDTLS_MPI_WINDOW_SIZE` | 1 (memory optimization) |
| `MBEDTLS_MPI_MAX_SIZE` | 512 |

### IoT SDK Configuration

| Define | Description |
|--------|-------------|
| `HSM_TYPE_SYMM_KEY` | Symmetric key HSM support |
| `HSM_TYPE_X509` | X.509 HSM support |
| `USE_PROV_MODULE` | Device Provisioning Service support |
| `DONT_USE_UPLOADTOBLOB` | Blob upload disabled |
| `LWIP_TIMEVAL_PRIVATE` | 0 (use system timeval) |

---

## See Also

- [WiFi Driver](WiFiDriver.md) — Network interface built on mbed NSAPI
- [Board Hardware](BoardHardware.md) — Pin definitions from `targets/TARGET_MXCHIP/`
