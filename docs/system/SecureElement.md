# Secure Element — STSAFE-A100

Hardware Abstraction Layer for the STSAFE-A100 I2C secure element. Provides low-level zone read/write, encrypted envelopes, and personalization.

> **Source:** [system/az3166-driver/HAL_STSAFE-A100.h](../../system/az3166-driver/HAL_STSAFE-A100.h)

---

## API

All functions return 0 on success, 1 on error (except `HAL_Version`).

### Initialization

| Function | Description |
|----------|-------------|
| `uint8_t Init_HAL(uint8_t i2c_address, void **handle_se)` | Initialize the secure element. Returns a handle. |
| `uint8_t Free_HAL(void *handle_se)` | Release the secure element handle. |
| `uint8_t Init_Perso(void *handle_se, int perso_type, int pcrop_enable, uint8_t *buf)` | Personalize the secure element. |

### Personalization Types

| `perso_type` | Description |
|--------------|-------------|
| 0 | Default keys |
| 1 | Random keys |
| 2 | Custom keys (`buf` must contain key data) |

### Plain Data Access

| Function | Description |
|----------|-------------|
| `uint8_t HAL_Store_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint8_t *in_Data, uint16_t offset)` | Write data to a zone (plain) |
| `uint8_t HAL_Get_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint8_t *buf, uint16_t offset)` | Read data from a zone (plain, max 800 bytes) |
| `uint8_t HAL_Erase_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint16_t offset)` | Erase data in a zone |

### Encrypted Envelope Access

| Function | Description |
|----------|-------------|
| `uint8_t HAL_Store_Data_WithinEnvelop(void *handle_se, uint8_t zone, uint16_t size, uint8_t *in_Data, uint16_t offset)` | Write data with encryption (max 480 bytes; 8-byte overhead) |
| `uint8_t HAL_Get_Data_WithinEnvelop(void *handle_se, uint8_t zone, uint16_t size, uint8_t *buf, uint16_t offset)` | Read data with encryption (max 480 bytes) |

### Version

| Function | Description |
|----------|-------------|
| `void HAL_Version(char *string)` | Get HAL version string |

---

## Notes

- The secure element communicates over I2C (I2C_1 on the AZ3166).
- The pre-built library is at `system/az3166-driver/libstsafe.a`.
- Higher-level access is provided by [EEPROMInterface](../cores/EEPROM.md) in the Arduino core.

---

## See Also

- [EEPROM](../cores/EEPROM.md) — Arduino-level secure storage API
- [DeviceConfig](../cores/DeviceConfig.md) — Configuration system built on EEPROM zones
