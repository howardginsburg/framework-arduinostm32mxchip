/**
 * @file SensorManager.cpp
 * @brief Implementation of the SensorManager abstraction layer.
 */

#include "SensorManager.h"
#include <Arduino.h>

// Global instance — initialized by the framework in _main_sys.cpp
SensorManager Sensors;

// Strong override of the weak stub in _main_sys.cpp.
// Automatically called by the framework at startup when this library is linked.
extern "C" void sensor_framework_init()
{
    Sensors.init();
}

SensorManager::SensorManager()
    : _i2c(nullptr)
    , _hts221(nullptr)
    , _lps22hb(nullptr)
    , _lsm6dsl(nullptr)
    , _lis2mdl(nullptr)
    , _initialized(false)
{
}

SensorManager::~SensorManager()
{
    delete _lis2mdl;
    delete _lsm6dsl;
    delete _lps22hb;
    delete _hts221;
    delete _i2c;
}

bool SensorManager::init()
{
    if (_initialized) return true;

    Serial.println("[SensorManager] Initializing sensors...");

    // Create shared I2C bus (D14=SDA, D15=SCL on MXChip AZ3166)
    _i2c = new DevI2C(D14, D15);
    if (!_i2c)
    {
        Serial.println("[SensorManager] ERROR: Failed to create I2C bus");
        return false;
    }

    bool allOk = true;

    // --- HTS221: Temperature & Humidity ---
    _hts221 = new HTS221Sensor(*_i2c);
    if (_hts221->init(NULL) != 0)
    {
        Serial.println("[SensorManager] WARNING: HTS221 init failed");
        allOk = false;
    }
    else
    {
        _hts221->enable();
        Serial.println("[SensorManager] HTS221 (Temp/Humidity) OK");
    }

    // --- LPS22HB: Pressure ---
    _lps22hb = new LPS22HBSensor(*_i2c);
    if (_lps22hb->init(NULL) != 0)
    {
        Serial.println("[SensorManager] WARNING: LPS22HB init failed");
        allOk = false;
    }
    else
    {
        Serial.println("[SensorManager] LPS22HB (Pressure) OK");
    }

    // --- LSM6DSL: Accelerometer & Gyroscope ---
    _lsm6dsl = new LSM6DSLSensor(*_i2c, D4, D5);
    if (_lsm6dsl->init(NULL) != 0)
    {
        Serial.println("[SensorManager] WARNING: LSM6DSL init failed");
        allOk = false;
    }
    else
    {
        _lsm6dsl->enableAccelerator();
        _lsm6dsl->enableGyroscope();
        Serial.println("[SensorManager] LSM6DSL (Accel/Gyro) OK");
    }

    // --- LIS2MDL: Magnetometer ---
    _lis2mdl = new LIS2MDLSensor(*_i2c);
    if (_lis2mdl->init(NULL) != 0)
    {
        Serial.println("[SensorManager] WARNING: LIS2MDL init failed");
        allOk = false;
    }
    else
    {
        Serial.println("[SensorManager] LIS2MDL (Magnetometer) OK");
    }

    _initialized = true;
    Serial.printf("[SensorManager] Initialization %s\n", allOk ? "complete" : "complete with warnings");
    return allOk;
}

// ---------------------------------------------------------------------------
// Bulk read
// ---------------------------------------------------------------------------

SensorData SensorManager::readAll()
{
    SensorData data = {};

    data.temperature = getTemperature();
    data.humidity    = getHumidity();
    data.pressure    = getPressure();
    getAccelerometer(data.accelX, data.accelY, data.accelZ);
    getGyroscope(data.gyroX, data.gyroY, data.gyroZ);
    getMagnetometer(data.magX, data.magY, data.magZ);
    data.buttonA     = isButtonAPressed();
    data.buttonB     = isButtonBPressed();

    return data;
}

// ---------------------------------------------------------------------------
// Individual accessors
// ---------------------------------------------------------------------------

float SensorManager::getTemperature()
{
    float value = 0;
    if (_hts221) _hts221->getTemperature(&value);
    return value;
}

float SensorManager::getHumidity()
{
    float value = 0;
    if (_hts221) _hts221->getHumidity(&value);
    return value;
}

float SensorManager::getPressure()
{
    float value = 0;
    if (_lps22hb) _lps22hb->getPressure(&value);
    return value;
}

void SensorManager::getAccelerometer(int32_t &x, int32_t &y, int32_t &z)
{
    int axes[3] = {0};
    if (_lsm6dsl) _lsm6dsl->getXAxes(axes);
    x = axes[0];
    y = axes[1];
    z = axes[2];
}

void SensorManager::getGyroscope(int32_t &x, int32_t &y, int32_t &z)
{
    int axes[3] = {0};
    if (_lsm6dsl) _lsm6dsl->getGAxes(axes);
    x = axes[0];
    y = axes[1];
    z = axes[2];
}

void SensorManager::getMagnetometer(int32_t &x, int32_t &y, int32_t &z)
{
    int axes[3] = {0};
    if (_lis2mdl) _lis2mdl->getMAxes(axes);
    x = axes[0];
    y = axes[1];
    z = axes[2];
}

bool SensorManager::isButtonAPressed()
{
    return digitalRead(USER_BUTTON_A) == LOW;
}

bool SensorManager::isButtonBPressed()
{
    return digitalRead(USER_BUTTON_B) == LOW;
}

int SensorManager::toJson(char *buf, size_t bufLen)
{
    SensorData d = readAll();

    int n = snprintf(buf, bufLen,
        "{\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":%.2f,"
        "\"accelerometer\":{\"x\":%ld,\"y\":%ld,\"z\":%ld},"
        "\"gyroscope\":{\"x\":%ld,\"y\":%ld,\"z\":%ld},"
        "\"magnetometer\":{\"x\":%ld,\"y\":%ld,\"z\":%ld},"
        "\"buttons\":{\"a\":%s,\"b\":%s}}",
        d.temperature, d.humidity, d.pressure,
        d.accelX, d.accelY, d.accelZ,
        d.gyroX,  d.gyroY,  d.gyroZ,
        d.magX,   d.magY,   d.magZ,
        d.buttonA ? "true" : "false",
        d.buttonB ? "true" : "false");

    return (n > 0 && (size_t)n < bufLen) ? n : 0;
}
