# Software Stack Deep Dive

This document traces the complete software stack of the MXChip AZ3166, from bare silicon up through mbed OS, the MiCO WiFi driver, and the Arduino framework layer — and shows exactly how each Arduino API call maps to the underlying hardware.

> **Source references:** All paths below are relative to the repository root.

---

## Table of Contents

- [Layer 0 — Hardware](#layer-0--hardware)
- [Layer 1 — mbed OS 5.4](#layer-1--mbed-os-54)
- [Layer 2 — MiCO / EMW10xx WiFi Stack](#layer-2--mico--emw10xx-wifi-stack)
- [Layer 3 — STSAFE-A100 Secure Element](#layer-3--stsafe-a100-secure-element)
- [Layer 4 — TLS Socket](#layer-4--tls-socket)
- [Layer 5 — `main()` Boot Sequence](#layer-5--main-boot-sequence)
- [Layer 6 — Arduino Core](#layer-6--arduino-core)
- [End-to-End Call Chains](#end-to-end-call-chains)

---

## Layer 0 — Hardware

The **STM32F412** is an ARM Cortex-M4 running at 96 MHz with an FPU. It provides the peripheral buses (APB1/APB2) and the hardware blocks the rest of the stack depends on:

| Peripheral | Role in the stack |
|---|---|
| **IWDG** | Independent Watchdog Timer, clocked by the 32 kHz LSI oscillator — entirely independent of the main clock and the RTOS. Programmed directly by `Watchdog.cpp` via register writes (`IWDG->KR`, `IWDG->PR`, `IWDG->RLR`). If the CPU hangs, IWDG resets the chip regardless of the OS state. |
| **us_ticker** | High-resolution hardware counter exposed by mbed. `millis()` and `micros()` call `us_ticker_read()` directly — no RTOS scheduler involved. |
| **USART6** (PC6/PC7) | `Serial`. Driven by mbed's `BufferedSerial` — an interrupt-based ring buffer draining to the USART shift register. |
| **I2C1** (PB8/PB9) | Shared bus for the OLED (SSD1306), all six onboard sensors (HTS221, LPS22HB, LSM6DSL, LIS2MDL), and the STSAFE-A100 secure element. Confirmed by `mbed_config.h`: `OLED_I2C_PORT = MICO_I2C_1`, `LPS25HB_I2C_PORT = MICO_I2C_1`. |
| **SDIO** | 4-bit high-speed interface to the BCM43362 WiFi chip. Managed entirely by the binary MiCO/EMW10xx driver. |
| **SPI flash (QSPI)** | Onboard SPI NOR flash. Exposed as an mbed `BlockDevice` by `SFlashBlockDevice`, mounted as a FATFileSystem at `/fs/`. |
| **TIM3/TIM4** | PWM channels for the RGB LED (PB4→TIM3\_CH1, PB3→TIM3\_CH2, PC7→TIM3\_CH2). `_main_sys.cpp` drives them directly via mbed `PwmOut`. |
| **ADC** | Available on PA4, PA5, PB0 (Arduino analog pins). |
| **NVIC/EXTI** | External interrupt controller. `attachInterrupt()` routes through mbed `InterruptIn` → EXTI line → NVIC vector. |

### Watchdog internals

`Watchdog::configure()` in [`cores/arduino/Watchdog.cpp`](../../cores/arduino/Watchdog.cpp) programs IWDG directly:

```cpp
IWDG->KR  = 0x5555;   // unlock prescaler/reload registers
IWDG->PR  = prescalerCode;
IWDG->RLR = reloadValue;
IWDG->KR  = 0xAAAA;   // reload counter from RLR
IWDG->KR  = 0xCCCC;   // start watchdog
```

Prescaler selection spans `/4` (0.125 ms min) to `/256` (32,768 ms max), derived from the 32 kHz LSI oscillator. The LSI frequency is not calibrated — expect ±20% accuracy in the actual timeout.

---

## Layer 1 — mbed OS 5.4

mbed OS 5.4 is the RTOS and HAL foundation. Its presence is confirmed in `system/mbed_config.h`:

```c
#define MBED_CONF_RTOS_PRESENT       1
#define MBED_CONF_EVENTS_PRESENT     1
#define MBED_CONF_NSAPI_PRESENT      1
#define MBED_CONF_FILESYSTEM_PRESENT 1
```

### RTOS (RTX5 / CMSIS-RTOS2)

mbed OS uses the RTX5 preemptive RTOS kernel (ARM CMSIS-RTOS2 API). The scheduler runs at the SysTick interrupt rate and preemptively time-slices threads by priority.

The mbed startup sequence (`mbed_start`) runs in a small initial thread with only **512 bytes** of stack — too small for a sketch. So the framework immediately spawns a dedicated Arduino thread in [`cores/arduino/system/_main_arduino.cpp`](../../cores/arduino/system/_main_arduino.cpp):

```cpp
static Thread arduino_thread(osPriorityNormal, 0x2000, NULL);  // 8 KB stack

void start_arduino(void) {
    arduino_thread.start(arduino_main);
}

static void arduino_main(void) {
    setup();
    for (;;) { loop(); }
}
```

When your `loop()` calls `delay(ms)`, it calls mbed's `wait_ms()`, which calls `rtos::Thread::wait()`. The RTOS context-switches to another runnable thread — the CPU is not busy-spinning. Other threads running concurrently include the lwIP timer thread, the MiCO WiFi driver thread, and any threads your sketch creates.

### mbed HAL — The Arduino ↔ mbed mapping

Every Arduino I/O primitive is a thin wrapper over a mbed HAL object:

| Arduino API | Source file | mbed underneath | Hardware |
|---|---|---|---|
| `pinMode()` | `wiring_digital.cpp` | `mbed::DigitalIn::mode()` | GPIO MODER/PUPDR registers |
| `digitalWrite()` | `wiring_digital.cpp` | `mbed::DigitalOut::write()` | GPIO ODR register |
| `digitalRead()` | `wiring_digital.cpp` | `mbed::DigitalIn::read()` | GPIO IDR register |
| `analogRead()` | `wiring_analog.cpp` | `mbed::AnalogIn::read_u16()` | ADC peripheral |
| `analogWrite()` | `wiring_analog.cpp` | `mbed::PwmOut::write()` | TIM CCR register |
| `attachInterrupt()` | `AttachInterrupt.cpp` | `mbed::InterruptIn::rise()/fall()` | NVIC/EXTI |
| `millis()` | `wiring.cpp` | `us_ticker_read() / 1000` | us_ticker hardware counter |
| `micros()` | `wiring.cpp` | `us_ticker_read()` | us_ticker hardware counter |
| `delay()` | `wiring.cpp` | `wait_ms()` → RTOS sleep | SysTick / RTX scheduler |

**Important:** `pinMode()`, `digitalWrite()`, and `digitalRead()` each construct a **stack-local** mbed object, use it once, and destroy it. There is no persistent Arduino pin state. This is slightly less efficient than a register-caching implementation but means any mbed code can freely reconfigure pins outside of Arduino's awareness.

`wiring_analog.cpp` holds a `pinsDescription[]` lookup table mapping Arduino pin numbers to STM32 `PinName` values and supported peripheral modes (ADC, PWM, SPI, I2C, UART):

```cpp
static const PinDescription pinsDescription[] = {
  { PB_0,  GPIO_PIN_IO|GPIO_PIN_ADC|GPIO_PIN_SPI_CS },  // Pin 0
  { PA_5,  GPIO_PIN_IO|GPIO_PIN_ADC },                   // Pin 4
  { PB_4,  GPIO_PIN_IO|GPIO_PIN_PWM },                   // Pin 6
  // ...
};
```

### Interrupts

`AttachInterrupt.cpp` maintains a static array of 16 `mbed::InterruptIn` objects, pre-constructed at program startup:

```cpp
static InterruptIn event[16] = {
    InterruptIn(PB_0), InterruptIn(NC), InterruptIn(PB_2), ...
};
```

The index into this array is `pin & 0x0F` — the GPIO pin number within the port. This means pin PB3, PC3, and PA3 all map to index 3 and would conflict; only one EXTI line per pin number can be active on STM32.

### NSAPI (Network Socket API)

mbed's portable network abstraction layer. `TCPSocket`, `UDPSocket`, `WiFiInterface`, `nsapi_error_t` — all NSAPI types. The WiFi driver, lwIP, the TLS socket, and all application code speak NSAPI, enabling the stack to be swapped without touching application code.

### mbedTLS

Compiled into `system/libdevkit-sdk-core-lib.a` with the algorithm set selected in `system/mbed_config.h`:

```c
#define MBEDTLS_SHA256_C           // HMAC-SHA256 for SAS tokens
#define MBEDTLS_SHA1_C             // TLS certificate fingerprints
#define MBEDTLS_MD5_C              // Legacy compatibility
#define MBEDTLS_SSL_PROTO_TLS1_1   // TLS 1.1 support
#define MBEDTLS_SSL_PROTO_TLS1_2   // TLS 1.2 support
#define MBEDTLS_MPI_MAX_SIZE  512  // RSA modulus up to 4096 bits
#define MBEDTLS_MPI_WINDOW_SIZE 1  // RAM-conserving sliding window
```

`USE_MBED_TLS` is always defined — the TLS stack is unconditionally present. The `MBEDTLS_MPI_WINDOW_SIZE 1` setting trades execution speed for ~2–4 KB less peak stack during RSA operations, important on a 256 KB RAM device.

### FATFileSystem and BlockDevice

mbed provides a portable storage abstraction:

- `mbed::BlockDevice` — pure virtual interface (read/program/erase/size)
- `mbed::FATFileSystem` — ChaN FatFS wrapped as a mbed filesystem object

`cores/arduino/FileSystem/SFlashBlockDevice` implements `BlockDevice` for the onboard SPI NOR flash. `SystemFileSystem` (`cores/arduino/config/SystemFileSystem.h/cpp`) constructs a singleton `FATFileSystem("fs")` on top of it and mounts it at `/fs/`. Auto-formatting occurs if the partition has never been initialised.

### tickcounter

The RTOS provides `tickcounter_create()` / `tickcounter_get_current_ms()` — used by `SystemTickCounter.cpp` to provide a monotonic millisecond counter for the Azure IoT C SDK's timeout and retry logic.

---

## Layer 2 — MiCO / EMW10xx WiFi Stack

The BCM43362 WiFi chip connects to the STM32 over 4-bit SDIO. Its driver is a **closed-source binary** (`system/az3166-driver/libwlan/`) provided by MXCHIP under the MiCO (Micro-controller Internet Connectivity Operating system) brand.

### Interface headers

- `system/az3166-driver/EMW10xxInterface.h` — extends mbed `WiFiInterface` (NSAPI). Provides `connect()`, `disconnect()`, `scan()`, `get_rssi()`, and `set_interface(Station | Soft_AP)`.
- `system/az3166-driver/emw10xx_lwip_stack.h` — exposes `mbed_lwip_init()`, which wires lwIP's network interface callbacks to the MiCO SDIO driver.

### Data path

```
BCM43362 hardware
    └── SDIO (4-bit, ~25 MHz)
         └── MiCO EMW10xx driver (binary libwlan)
              └── lwIP TCP/IP stack  ← mbed_lwip_init()
                   └── NSAPI (TCPSocket, UDPSocket)
                        └── TLSSocket / WiFiClientSecure
                             └── PubSubClient / HTTPClient / NTPClient
```

`mbed_lwip_init()` is the very first call inside `Initialization()` in `_main_sys.cpp` — before any device config, filesystem, or Arduino setup. It starts the lwIP timer thread and registers the network interface.

### Station vs AP mode

`SystemWiFi.cpp` shows the mode switch:

```cpp
// Station mode
((EMW10xxInterface*)_defaultSystemNetwork)->set_interface(Station);
((EMW10xxInterface*)_defaultSystemNetwork)->connect(ssid, pass, NSAPI_SECURITY_WPA_WPA2, 0);

// AP mode
((EMW10xxInterface*)_defaultSystemNetwork)->set_interface(Soft_AP);
((EMW10xxInterface*)_defaultSystemNetwork)->connect(ap_name, passphrase, NSAPI_SECURITY_WPA_WPA2, 0);
```

In AP mode the device gets the static IP `192.168.0.1` and the HTTP configuration server runs on top of a `TCPSocket` bound to port 80.

After station connect, `SyncTime()` fires a `UDPSocket` NTP request to `pool.ntp.org` (and fallbacks). A successful response sets the mbed RTC, which backs POSIX `time()` for the rest of the session.

### Pin assignment (BCM43362 ↔ STM32)

From `system/az3166-driver/TARGET_AZ3166/mico_board.h`, the BCM43362 uses the SDIO peripheral:
- **PB15** → SDIO MOSI / SPI2_MOSI (also usable as Arduino SPI MOSI)
- **PB13** → SDIO CLK / SPI2_SCK
- **PB14** → SDIO MISO / SPI2_MISO
- The SDIO D0–D3 lines are routed internally to the module

---

## Layer 3 — STSAFE-A100 Secure Element

The STSAFE-A100 is a dedicated security co-processor from STMicroelectronics, sitting on the I2C bus. It provides:

- **Tamper-resistant zone storage** — 11 configurable data zones; reads and writes are gated by hardware-enforced access control
- **Cryptographic operations** — ECC key generation, ECDSA sign/verify, key agreement
- **Anti-cloning** — the root keys never leave the chip

Its driver is a **binary library** (`system/az3166-driver/libstsafe.a`) with `HAL_STSAFE-A100.h` as the C interface:

```c
uint8_t Init_HAL(uint8_t i2c_address, void **handle_se);  // open I2C session
uint8_t StSafe_Read(void *handle, uint8_t zone, uint16_t offset, uint8_t *buf, uint16_t len);
uint8_t StSafe_Write(void *handle, uint8_t zone, uint16_t offset, const uint8_t *buf, uint16_t len);
```

### Zone layout (hardware constants)

These sizes are fixed by the personalization that was written to the chip at the factory. They cannot be changed post-manufacture:

| Zone | Size (bytes) | mbed_config define | Conventional use |
|------|-------------|-------------------|-----------------|
| 0 | 976 | `STSAFE_ZONE_0_SIZE` | Large certificates (part 1) |
| 1 | 0 | — | Reserved / unusable |
| 2 | 192 | `STSAFE_ZONE_2_SIZE` | Short strings, IDs |
| 3 | 120 | `STSAFE_ZONE_3_SIZE` | WiFi SSID |
| 4 | 0 | — | Reserved / unusable |
| 5 | 584 | `STSAFE_ZONE_5_SIZE` | URLs, connection strings |
| 6 | 680 | `STSAFE_ZONE_6_SIZE` | Device ID, medium certs |
| 7 | 784 | `STSAFE_ZONE_7_SIZE` | Large certs (part 2) |
| 8 | 880 | `STSAFE_ZONE_8_SIZE` | Large certs (part 3), keys |
| 9 | 0 | — | Reserved / unusable |
| 10 | 88 | `STSAFE_ZONE_10_SIZE` | WiFi password |

**Total accessible storage: 4,304 bytes** across 8 usable zones.

`EEPROMInterface` (`cores/arduino/EEPROMInterface.h/cpp`) wraps `libstsafe.a` into an Arduino-friendly `read(zone, buf, len)` / `write(zone, buf, len)` API. `DeviceConfig` sits on top of `EEPROMInterface`, adding the profile/zone-mapping abstraction and runtime buffers.

---

## Layer 4 — TLS Socket

[`cores/arduino/TLSSocket.cpp`](../../cores/arduino/TLSSocket.cpp) implements a TLS session on top of an NSAPI `TCPSocket`, using mbedTLS directly.

### Key design choice: IoT Hub SDK–style polling recv

During the TLS handshake, the mbedTLS engine calls `ssl_recv()` repeatedly. The original implementation blocked the RTOS thread, causing timeouts with some MQTT brokers. This fork reimplements `ssl_recv()` to match the approach used by the Azure IoT C SDK's `tlsio_mbedtls.c`:

```cpp
static int ssl_recv(void *ctx, unsigned char *buf, size_t len) {
    TLSSocket *tls = static_cast<TLSSocket *>(ctx);
    // During handshake: poll with wait_ms() between attempts, timeout at HANDSHAKE_TIMEOUT_MS
    // After handshake: return MBEDTLS_ERR_SSL_WANT_READ immediately if no data
    while (tls->_recv_buffer_count == 0) {
        int recv_result = socket->recv(temp_buf, sizeof(temp_buf));
        if (recv_result > 0) { /* buffer it */ break; }
        else if (recv_result == NSAPI_ERROR_WOULD_BLOCK) {
            if (tls->_handshake_complete) break;  // non-blocking post-handshake
            if (pending++ >= HANDSHAKE_TIMEOUT_MS / HANDSHAKE_WAIT_INTERVAL_MS)
                return MBEDTLS_ERR_SSL_TIMEOUT;
            wait_ms(HANDSHAKE_WAIT_INTERVAL_MS);  // yield to RTOS during handshake
        }
    }
    // return from internal buffer
}
```

Received data is accumulated in a heap-allocated `_recv_buffer`, which grows with `realloc()` as chunks arrive. This avoids the mbedTLS internal buffer size restrictions during the handshake record exchange.

### WiFiClientSecure

`WiFiClientSecure` (`cores/arduino/TLSSocket.h/cpp`) is an Arduino-API wrapper over `TLSSocket`, presenting the `setCACert()` / `setCertificate()` / `setPrivateKey()` interface that `PubSubClient` and other Arduino libraries expect:

```cpp
wifiClient.setCACert(DeviceConfig_GetCACert());
wifiClient.setCertificate(DeviceConfig_GetClientCert());
wifiClient.setPrivateKey(DeviceConfig_GetClientKey());
PubSubClient mqtt(wifiClient);  // PubSubClient sees only the Client* interface
```

---

## Layer 5 — `main()` Boot Sequence

[`cores/arduino/system/_main_sys.cpp`](../../cores/arduino/system/_main_sys.cpp) contains the actual `main()`. It runs in the mbed startup thread and never returns:

```
mbed RTOS starts → main() enters
  │
  ├─ Initialization()
  │     ├─ mbed_lwip_init()            ← stand up lwIP against MiCO SDIO driver
  │     ├─ SystemTickCounterInit()     ← monotonic ms counter for Azure SDK
  │     ├─ Screen.init()               ← I2C → SSD1306 OLED
  │     ├─ DigitalOut LEDs = 0         ← GPIO via mbed HAL (WiFi, Azure, User LEDs)
  │     ├─ PwmOut RGB = 0.0f           ← TIM via mbed HAL (PB4, PB3, PC7)
  │     └─ sensor_framework_init()     ← WEAK symbol; no-op unless Sensors lib linked
  │                                       SensorManager.cpp provides the strong override
  │
  ├─ DeviceConfig_Init(CONNECTION_PROFILE)   ← profile selected at compile time via -D flag
  ├─ SystemFileSystem_Mount()               ← FATFileSystem on SFlash at /fs/; auto-formats
  ├─ DeviceConfig_LoadAll()                 ← EEPROM zones + /fs/device.cfg → static RAM buffers
  │
  ├─ __sys_setup()    ← WEAK; default starts the HTTP config web server
  │                      sketch can override to suppress it
  │
  ├─ 2-second window — display button hints, then check:
  │     ├─ Button A held → cli_main()         ← serial CLI; blocks forever
  │     ├─ Button B held → EnterAPMode()      ← WiFi AP + HTTP server; blocks forever
  │     └─ Neither       → EnterUserMode()
  │
  └─ EnterUserMode()
        ├─ start_arduino()
        │     └─ arduino_thread.start(arduino_main)
        │               └─ setup();  for(;;) loop();
        └─ for(;;) wait_ms(60000);   ← supervisor loop; main thread never exits
```

The `sensor_framework_init` weak/strong symbol pattern allows the Sensors library to be genuinely optional at link time. If you don't include `<SensorManager.h>` in your project, the linker keeps the weak no-op stub and the sensor I2C buses are never initialised — saving both flash and runtime.

---

## Layer 6 — Arduino Core

With the boot sequence complete, the Arduino layer maps the familiar API onto mbed objects:

### Serial

`UARTClass` wraps `mbed::BufferedSerial(STDIO_UART1_TX, STDIO_UART1_RX, UART_RCV_SIZE)`. The `BufferedSerial` uses a USART interrupt to drain received bytes into a ring buffer. `Serial.read()` dequeues from the ring buffer; `Serial.write()` queues to the transmit side (also interrupt-driven).

### Screen

`OLEDDisplay` drives the SSD1306 via `mbed::I2C(PB8, PB9)`. Display commands are assembled into I2C transaction sequences — no DMA, purely interrupt-driven I2C.

### Interrupts

```cpp
// Static array of InterruptIn objects, one per EXTI line (by pin number 0–15)
static InterruptIn event[16] = { InterruptIn(PB_0), InterruptIn(NC), ... };

int attachInterrupt(PinName pin, Callback<void()> ISR, int mode) {
    if (mode == RISING)  event[pin & 0x0F].rise(ISR);
    if (mode == FALLING) event[pin & 0x0F].fall(ISR);
    if (mode == CHANGE)  { event[pin & 0x0F].rise(ISR); event[pin & 0x0F].fall(ISR); }
}
```

The index is `pin & 0x0F` — the lower nibble of the `PinName` enum value, which encodes the pin number within the port. **PA3, PB3, and PC3 all map to index 3 and cannot all be active interrupt sources simultaneously** (STM32 EXTI multiplexes all ports through a single line per pin number).

### Timing constants

```cpp
#define F_CPU SystemCoreClock   // ARM CMSIS variable, updated from RCC at startup
#define clockCyclesPerMicrosecond()    (SystemCoreClock / 1000000L)
#define microsecondsToClockCycles(a)   ((a) * (SystemCoreClock / 1000000L))
```

`SystemCoreClock` is an ARM CMSIS global updated by the mbed clock init code from the RCC registers — it reflects the actual configured frequency (96 MHz), not a compile-time constant.

---

## End-to-End Call Chains

### `mqtt.publish("topic", payload)`

```
PubSubClient::publish()
  └─ WiFiClientSecure::write()           // Arduino Client* interface
       └─ TLSSocket::send()              // mbedTLS: encrypt record
            └─ ssl_send() callback
                 └─ TCPSocket::send()    // NSAPI write to socket
                      └─ lwIP tcp_write() / tcp_output()
                           └─ EMW10xxInterface  // MiCO SDIO driver (binary)
                                └─ BCM43362 WiFi chip → RF → network
```

### `DeviceConfig_GetCACert()` returning a certificate

```
DeviceConfig_GetCACert()
  └─ returns s_caCert[]                    // static RAM buffer
       └─ loaded by DeviceConfig_LoadAll()
            └─ EEPROMInterface::read(zone0, zone7, zone8)
                 └─ libstsafe.a HAL_Read() (binary)
                      └─ mbed::I2C(PB8, PB9)
                           └─ I2C peripheral → STSAFE-A100 chip
```

### `ConfigFile_Read(SETTING_PUBLISH_TOPIC, buf, size)`

```
ConfigFile_Read()
  └─ SystemFileSystem_GetFS()     // returns mounted FATFileSystem*
       └─ mbed::File::open(fs, "/device.cfg", O_RDONLY)
            └─ FATFileSystem (ChaN FatFS)
                 └─ SFlashBlockDevice::read()
                      └─ SPI flash hardware
```

### `attachInterrupt(PB_3, myISR, RISING)`

```
attachInterrupt(PB_3, myISR, RISING)
  └─ event[3].rise(myISR)              // static InterruptIn[pin & 0x0F]
       └─ mbed::InterruptIn::rise()
            └─ NVIC_SetVector(EXTI3_IRQn, ...)
                 └─ EXTI line 3 → NVIC → CPU exception → myISR()
```

### `delay(100)`

```
delay(100)
  └─ wait_ms(100)
       └─ rtos::Thread::wait(100)
            └─ RTX5 scheduler: block arduino_thread for 100 ms
                 └─ switch to another runnable thread (lwIP timer, WiFi driver, ...)
                      └─ SysTick interrupt fires after 100 ms
                           └─ RTX5 scheduler: resume arduino_thread
```

---

## See Also

- [mbed OS](MbedOS.md) — mbed OS 5.4.3 configuration details
- [WiFi Driver](WiFiDriver.md) — EMW10xx interface and lwIP stack
- [Secure Element](SecureElement.md) — STSAFE-A100 zone storage details
- [Board Hardware](BoardHardware.md) — GPIO pin mapping and peripheral assignments
- [MiCO Framework](MiCO.md) — MiCO OS abstraction layer
- [TLS Socket](../cores/TLSSocket.md) — TLS layer modifications
- [DeviceConfig](../cores/DeviceConfig.md) — Profile-based configuration system
