/**
 * @file SensorManager.h
 * @brief Sensor abstraction layer for MXChip AZ3166
 * 
 * Manages initialization and reading of all onboard sensors:
 *   - HTS221:   Temperature & Humidity
 *   - LPS22HB:  Barometric Pressure
 *   - LSM6DSL:  Accelerometer & Gyroscope (6-axis IMU)
 *   - LIS2MDL:  Magnetometer
 *
 * The framework creates and initializes a global SensorManager instance
 * during system startup. User code accesses it via Sensors (a global reference).
 *
 * Usage in Arduino sketch:
 *   float temp = Sensors.getTemperature();
 *   SensorData data = Sensors.readAll();
 *   char json[512];
 *   Sensors.toJson(json, sizeof(json));
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "HTS221Sensor.h"
#include "LPS22HBSensor.h"
#include "LSM6DSLSensor.h"
#include "LIS2MDLSensor.h"

/**
 * Holds a snapshot of all sensor readings.
 */
struct SensorData
{
    // HTS221
    float temperature;    // °C
    float humidity;       // %RH

    // LPS22HB
    float pressure;       // hPa

    // LSM6DSL – Accelerometer (mg) & Gyroscope (mdps)
    int32_t accelX, accelY, accelZ;
    int32_t gyroX, gyroY, gyroZ;

    // LIS2MDL – Magnetometer (mGauss)
    int32_t magX, magY, magZ;

    // User buttons (true = pressed)
    bool buttonA;
    bool buttonB;
};

/**
 * Manages all onboard sensors on the MXChip AZ3166.
 */
class SensorManager
{
public:
    SensorManager();
    ~SensorManager();

    /**
     * Initialize the I2C bus and all sensors.
     * Called automatically by the framework during system startup.
     * @return true if all sensors initialized successfully.
     */
    bool init();

    /**
     * Read all sensors and return a snapshot.
     */
    SensorData readAll();

    // --- Individual sensor accessors ---

    /** @return Temperature in °C */
    float getTemperature();

    /** @return Relative humidity in %RH */
    float getHumidity();

    /** @return Barometric pressure in hPa */
    float getPressure();

    /**
     * Get accelerometer axes (mg).
     * @param x, y, z  Output parameters.
     */
    void getAccelerometer(int32_t &x, int32_t &y, int32_t &z);

    /**
     * Get gyroscope axes (mdps).
     * @param x, y, z  Output parameters.
     */
    void getGyroscope(int32_t &x, int32_t &y, int32_t &z);

    /**
     * Get magnetometer axes (mGauss).
     * @param x, y, z  Output parameters.
     */
    void getMagnetometer(int32_t &x, int32_t &y, int32_t &z);

    /** @return true if Button A is currently pressed */
    bool isButtonAPressed();

    /** @return true if Button B is currently pressed */
    bool isButtonBPressed();

    /**
     * Read all sensors and return a JSON string.
     * Writes into the provided buffer.
     * @param buf    Output buffer.
     * @param bufLen Size of the output buffer.
     * @return Number of characters written (excluding null terminator), or 0 on error.
     */
    int toJson(char *buf, size_t bufLen);

private:
    DevI2C         *_i2c;
    HTS221Sensor   *_hts221;
    LPS22HBSensor  *_lps22hb;
    LSM6DSLSensor  *_lsm6dsl;
    LIS2MDLSensor  *_lis2mdl;
    bool            _initialized;
};

/**
 * Global SensorManager instance, initialized by the framework at startup.
 * Use this in your Arduino sketch to access sensor data.
 */
extern SensorManager Sensors;

#endif // SENSOR_MANAGER_H
