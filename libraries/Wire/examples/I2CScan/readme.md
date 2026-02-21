# I2CScan

Scans the I2C bus for connected devices and reports their addresses.

## Expected Output

The MXChip AZ3166 has several onboard I2C sensors, so you should see devices at addresses like:
- `0x5F` — HTS221 (temperature/humidity)
- `0x5D` — LPS22HB (pressure)
- `0x6A` — LSM6DSL (accelerometer/gyroscope)
- `0x1E` — LIS2MDL (magnetometer)

## Running

1. Open the Serial Monitor at 115200 baud.
2. The scan runs once at startup. Reset the board to scan again.
