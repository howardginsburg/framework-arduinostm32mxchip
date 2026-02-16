# Sensors Library

**Version:** 0.0.1 | **Author:** Microsoft | **Category:** Other | **Architecture:** stm32f4

Drivers for all onboard sensors plus a high-level `SensorManager` abstraction. Covers temperature, humidity, pressure, accelerometer, gyroscope, magnetometer, infrared transmitter, and RGB LED.

---

## Quick Start

```cpp
#include <SensorManager.h>

void setup() {
    Serial.begin(115200);
    // Sensors.init() is called automatically at startup
}

void loop() {
    // Individual readings
    float temp = Sensors.getTemperature();
    float humidity = Sensors.getHumidity();
    float pressure = Sensors.getPressure();
    Serial.printf("T=%.1f°C  H=%.1f%%  P=%.1fhPa\n", temp, humidity, pressure);

    // Motion sensors
    int32_t ax, ay, az;
    Sensors.getAccelerometer(ax, ay, az);

    // JSON telemetry
    char json[512];
    Sensors.toJson(json, sizeof(json));
    Serial.println(json);

    delay(2000);
}
```

---

## API Reference

### SensorManager (Global Instance: `Sensors`)

The `Sensors` global is initialized automatically at startup.

| Method | Signature | Description |
|--------|-----------|-------------|
| `init` | `bool init()` | Initialize I2C bus and all sensors (auto-called at boot) |
| `readAll` | `SensorData readAll()` | Snapshot of all sensor readings |
| `getTemperature` | `float getTemperature()` | Temperature in °C |
| `getHumidity` | `float getHumidity()` | Relative humidity in %RH |
| `getPressure` | `float getPressure()` | Barometric pressure in hPa |
| `getAccelerometer` | `void getAccelerometer(int32_t &x, int32_t &y, int32_t &z)` | Acceleration in mg |
| `getGyroscope` | `void getGyroscope(int32_t &x, int32_t &y, int32_t &z)` | Angular velocity in mdps |
| `getMagnetometer` | `void getMagnetometer(int32_t &x, int32_t &y, int32_t &z)` | Magnetic field in mGauss |
| `toJson` | `int toJson(char* buf, size_t bufLen)` | Serialize all readings to JSON string |

#### SensorData Struct

```cpp
struct SensorData {
    float temperature;      // °C
    float humidity;         // %RH
    float pressure;         // hPa
    int32_t accelX, accelY, accelZ;  // mg
    int32_t gyroX, gyroY, gyroZ;     // mdps
    int32_t magX, magY, magZ;        // mGauss
};
```

---

### Onboard Sensors

| Sensor IC | Measurements | Units | Class |
|-----------|-------------|-------|-------|
| HTS221 | Temperature, Humidity | °C, %RH | `HTS221Sensor` |
| LPS22HB | Barometric Pressure, Temperature | hPa, °C | `LPS22HBSensor` |
| LSM6DSL | Accelerometer, Gyroscope | mg, mdps | `LSM6DSLSensor` |
| LIS2MDL | Magnetometer | mGauss | `LIS2MDLSensor` |

---

### HTS221Sensor

Temperature and humidity sensor.

| Method | Description |
|--------|-------------|
| `init(void*)` | Initialize sensor |
| `enable()` / `disable()` / `reset()` | Power control |
| `getTemperature(float*)` | Read temperature (°C) |
| `getHumidity(float*)` | Read humidity (%RH) |
| `getOdr(float*)` / `setOdr(float)` | Get/set output data rate |
| `readId(unsigned char*)` | Read device ID |

---

### LPS22HBSensor

Barometric pressure and temperature sensor.

| Method | Description |
|--------|-------------|
| `init(void*)` | Initialize sensor |
| `getPressure(float*)` | Read pressure (hPa) |
| `getTemperature(float*)` | Read temperature (°C) |
| `readId(unsigned char*)` | Read device ID |
| `deInit()` | Deinitialize sensor |

---

### LSM6DSLSensor

6-axis IMU (accelerometer + gyroscope) with advanced motion detection features.

#### Basic Readings

| Method | Description |
|--------|-------------|
| `getXAxes(int*)` | Accelerometer X/Y/Z (mg) |
| `getGAxes(int*)` | Gyroscope X/Y/Z (mdps) |
| `getXSensitivity(float*)` | Accelerometer sensitivity |
| `getGSensitivity(float*)` | Gyroscope sensitivity |

#### Configuration

| Method | Description |
|--------|-------------|
| `getXOdr(float*)` / `setXOdr(float)` | Accelerometer output data rate |
| `getGOdr(float*)` / `setGOdr(float)` | Gyroscope output data rate |
| `getXFullScale(float*)` / `setXFullScale(float)` | Accelerometer full scale |
| `getGFullScale(float*)` / `setGFullScale(float)` | Gyroscope full scale |
| `enableAccelerator()` / `disableAccelerator()` | Accelerometer power |
| `enableGyroscope()` / `disableGyroscope()` | Gyroscope power |

#### Motion Detection Features

| Feature | Enable/Disable | Additional Methods |
|---------|---------------|-------------------|
| Free-Fall | `enableFreeFallDetection(pin)` / `disableFreeFallDetection()` | — |
| Pedometer | `enablePedometer()` / `disablePedometer()` | `getStepCounter(uint16_t*)`, `resetStepCounter()`, `setPedometerThreshold(uint8_t)` |
| Tilt | `enableTiltDetection(pin)` / `disableTiltDetection()` | — |
| Wake-Up | `enableWakeUpDetection(pin)` / `disableWakeUpDetection()` | `setWakeUpThreshold(uint8_t)` |
| Single Tap | `enableSingleTapDetection(pin)` / `disableSingleTapDetection()` | `setTapThreshold(uint8_t)`, `setTapShockTime(uint8_t)`, `setTapQuietTime(uint8_t)` |
| Double Tap | `enableDoubleTapDetection(pin)` / `disableDoubleTapDetection()` | `setTapDurationTime(uint8_t)` |
| 6D Orientation | `enable6dOrientation(pin)` / `disable6dOrientation()` | `get6dOrientationXL/XH/YL/YH/ZL/ZH(uint8_t*)` |

#### Interrupts

| Method | Description |
|--------|-------------|
| `attachInt1Irq(func)` / `enableInt1Irq()` / `disableInt1Irq()` | INT1 pin interrupt |
| `attachInt2Irq(func)` / `enableInt2Irq()` / `disableInt2Irq()` | INT2 pin interrupt |
| `getEventStatus(LSM6DSL_Event_Status_t*)` | Read event flags |

#### Event Status

```cpp
typedef struct {
    uint8_t FreeFallStatus;
    uint8_t TapStatus;
    uint8_t DoubleTapStatus;
    uint8_t WakeUpStatus;
    uint8_t StepStatus;
    uint8_t TiltStatus;
    uint8_t D6DOrientationStatus;
} LSM6DSL_Event_Status_t;
```

---

### LIS2MDLSensor

3-axis magnetometer.

| Method | Description |
|--------|-------------|
| `init(void*)` | Initialize sensor |
| `getMAxes(int*)` | Read X/Y/Z magnetic field (mGauss) |
| `readId(unsigned char*)` | Read device ID |

---

### IRDASensor

Infrared transmitter.

| Method | Description |
|--------|-------------|
| `init()` | Initialize IrDA |
| `IRDATransmit(unsigned char* pData, int size, int timeout)` | Transmit data via IR |

---

### RGB_LED

Onboard RGB LED with PWM control.

| Method | Description |
|--------|-------------|
| `RGB_LED(PinName red, PinName green, PinName blue)` | Constructor (defaults: PB_4, PB_3, PC_7) |
| `setColor(int red, int green, int blue)` | Set RGB values (0–255 each) |
| `turnOff()` | Turn off LED |
| `setRed()`, `setGreen()`, `setBlue()` | Preset primary colors |
| `setYellow()`, `setCyan()`, `setMagenta()` | Preset secondary colors |
| `setWhite()`, `setOrange()` | Preset colors |

**Color Macros:** `RGB_LED_RED`, `RGB_LED_GREEN`, `RGB_LED_BLUE`, `RGB_LED_YELLOW`, `RGB_LED_CYAN`, `RGB_LED_MAGENTA`, `RGB_LED_WHITE`, `RGB_LED_ORANGE`

---

## Include Shortcuts

```cpp
#include <Sensor.h>         // Includes all individual sensor headers
#include <SensorManager.h>  // High-level SensorManager API
```

---

## Dependencies

- mbed OS (`I2C`, `PwmOut`, `InterruptIn`)
- [Wire](Wire.md) / DevI2C (I2C bus)
