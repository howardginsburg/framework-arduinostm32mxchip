# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## How to Use a Specific Version

Pin a release in your `platformio.ini` by referencing its tag:

```ini
platform_packages =
    framework-arduinostm32mxchip @ https://github.com/howardginsburg/framework-arduinostm32mxchip/archive/refs/tags/v2.2.1.zip
```

Replace `v2.2.1` with any tag listed below.

> For instructions on creating a new release, see [RELEASING.md](RELEASING.md).

---

## [2.2.1] - 2026-02-21

### Changed
- `cores/arduino/system/_main_sys.cpp` — replaced the hard `SensorManager::init()` call with a weak-symbol hook (`sensor_framework_init`), making the Sensors library genuinely optional at link time
- `libraries/Sensors/src/SensorManager.cpp` — provides the strong `sensor_framework_init` override that calls `Sensors.init()` when the library is linked

---

## [2.1.0] - 2026-02-21

### Added
- `WiFiClientSecure` — Arduino-compatible TLS client for mutual TLS (mTLS) MQTT connections
- `DeviceConfig` system — unified configuration storage using the STSAFE-A100 secure element with multi-zone support for large certificates
- `SensorManager` — simple unified API for all onboard sensors (temperature, humidity, pressure, accelerometer, gyroscope, magnetometer)
- `AzureIoT` library — complete Azure IoT Hub client with DPS provisioning, SAS token generation, HMAC-SHA256 group key derivation, and Device Twin support
- `PubSubClient` library (Nick O'Leary, MIT) — bundled MQTT client; no external `lib_deps` required
- Connection Profiles — pre-defined profiles (`PROFILE_MQTT_MTLS`, `PROFILE_IOTHUB_SAS`, `PROFILE_DPS_CERT`, etc.) set via `build_flags`
- `PROFILE_CUSTOM` support — user-defined profiles via `custom_profile.h` (see [CustomProfile.md](docs/CustomProfile.md))
- Updated browser-based Web Configuration UI served over WiFi AP mode
- Input validation helpers for URLs, certificates, and connection strings
- Extensible CLI serial commands for all configuration settings
- Revamped TLS socket layer with bug fixes for MQTT mTLS (see [TLSPATCH.md](docs/TLSPATCH.md))

### Removed
- Azure IoT Hub built-in libraries (deprecated; devices can no longer connect using the original libraries)
- Board telemetry collector (defunct Microsoft telemetry service)
- Original Paho MQTT library (did not support mTLS connections)

[2.2.1]: https://github.com/howardginsburg/framework-arduinostm32mxchip/releases/tag/v2.2.1
[2.1.0]: https://github.com/howardginsburg/framework-arduinostm32mxchip/releases/tag/v2.1.0
