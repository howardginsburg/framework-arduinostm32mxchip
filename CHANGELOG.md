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

## [2.3.1] - 2026-03-29

### Changed
- **TLS/crypto migrated from mbedTLS to wolfSSL 5.7.6** — the `TLSSocket` layer, `AzureIoTCrypto` (HMAC-SHA256, Base64), and X.509 certificate parsing in `DeviceConfigRuntime` now use wolfSSL/wolfCrypt exclusively; adds TLS 1.3 support alongside TLS 1.2
- **`DeviceConfigRuntime.cpp`** — `extractCNFromCert()` rewritten to use wolfCrypt (`wc_CertPemToDer`, `wc_InitDecodedCert`, `wc_ParseCert`, `DecodedCert.subjectCN`) instead of mbedTLS `mbedtls_x509_crt_parse`
- **`console_cli.cpp`** — version command now prints wolfSSL version (`LIBWOLFSSL_VERSION_STRING`) instead of mbed TLS version

### Fixed
- **`TLSSocket.cpp`** — PEM buffer length passed to `wolfSSL_CTX_load_verify_buffer()`, `wolfSSL_CTX_use_certificate_buffer()`, and `wolfSSL_CTX_use_PrivateKey_buffer()` no longer includes the null terminator (`strlen()` instead of `strlen() + 1`); the extra byte could cause PEM parsing failures with certain certificates
- **`user_settings.h`** — added `WOLFSSL_ALT_CERT_CHAINS` to enable flexible certificate chain verification; required for Azure endpoints that present intermediate certificates in non-strict order or when the loaded CA is an intermediate rather than the ultimate root

### Notes
- mbed OS 5.4 remains in use for RTOS, HAL, drivers, networking sockets (TCPSocket/UDPSocket), filesystem, and lwIP
- Pre-compiled binaries (`libdevkit-sdk-core-lib.a`, `libstsafe.a`) still link mbedTLS internally; the mbedTLS headers in `system/mbed-os/features/mbedtls/` are retained for those dependencies
- No changes to sketch-level APIs — existing projects require no code changes

---

## [2.3.0] - 2026-02-23

### Added
- **Config-file backed settings** — New `DeviceConfigFile` module stores settings that don't fit in STSAFE EEPROM zones in a key=value text file at `/fs/device.cfg` on the SFlash FAT filesystem
- **`SystemFileSystem`** — Framework-owned FAT filesystem mount at `/fs/`; mounted automatically before `DeviceConfig_LoadAll()` so file-backed settings are available from first boot; auto-formats the flash partition on first use
- **Three new settings** available across MQTT and IoT Hub profiles:
  - `SETTING_SEND_INTERVAL` — message send interval in seconds (default 30), CLI command `set_interval`, stored in config file
  - `SETTING_PUBLISH_TOPIC` — MQTT publish topic, CLI command `set_pubtopic`, stored in config file
  - `SETTING_SUBSCRIBE_TOPIC` — MQTT subscribe topic, CLI command `set_subtopic`, stored in config file
- **`SETTING_DEVICE_PASSWORD`** added to `PROFILE_MQTT_USERPASS` and `PROFILE_MQTT_USERPASS_TLS` profiles with EEPROM zone backing
- **New `DeviceConfig` accessor functions**: `DeviceConfig_GetSendInterval()`, `DeviceConfig_GetPublishTopic()`, `DeviceConfig_GetSubscribeTopic()`, `DeviceConfig_GetDevicePassword()`
- **Complete DPS accessor functions**: `DeviceConfig_GetDpsEndpoint()`, `DeviceConfig_GetScopeId()`, `DeviceConfig_GetRegistrationId()`, `DeviceConfig_GetSymmetricKey()` — previously missing from the runtime layer
- **`FILE_ZONE(s)` macro** in `DeviceConfigZones.h` — designates a setting as file-backed (uses `FILE_ZONE_MARKER = 0xFE` sentinel in `zones[0]`) with max-size constants for each file setting
- **PR build verification** — GitHub Actions workflow (`.github/workflows/pr-build-verification.yml`) and `tests/build_check.cpp` sketch that validates the framework compiles cleanly on every pull request

### Changed
- **FileSystem library refactor** — `SFlashBlockDevice` implementation consolidated from `libraries/FileSystem/src/` into `cores/arduino/FileSystem/`; `fatfs_exfuns` also moved into the core; the framework core now owns the singleton FAT filesystem instance
- **`SystemFileSystem.cpp`** trimmed to a thin wrapper — block device and FAT mount logic lives in the new `cores/arduino/FileSystem/SFlashBlockDevice.cpp`
- **`AzureIoTHub.cpp`** connection string handling updated to use the new `DeviceConfig_GetDpsEndpoint()` / `DeviceConfig_GetScopeId()` / `DeviceConfig_GetRegistrationId()` / `DeviceConfig_GetSymmetricKey()` getters instead of parsing the connection string inline
- `SettingUI.h` and `SettingValidator.h` updated with UI metadata and validation rules for the three new file-backed settings

### Breaking Changes
- Do **not** create a `FATFileSystem("fs")` instance in sketch code or via the FileSystem library — the framework now owns this mount point. Using the `SFlashBlockDevice` class from the FileSystem library for low-level block access is still supported.

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

[2.4.0]: https://github.com/howardginsburg/framework-arduinostm32mxchip/releases/tag/v2.4.0
[2.3.0]: https://github.com/howardginsburg/framework-arduinostm32mxchip/releases/tag/v2.3.0
[2.2.1]: https://github.com/howardginsburg/framework-arduinostm32mxchip/releases/tag/v2.2.1
[2.1.0]: https://github.com/howardginsburg/framework-arduinostm32mxchip/releases/tag/v2.1.0
