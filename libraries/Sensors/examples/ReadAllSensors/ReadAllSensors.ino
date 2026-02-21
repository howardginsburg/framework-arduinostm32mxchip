/*
  ReadAllSensors

  Reads all onboard sensors on the MXChip AZ3166 DevKit:
    - HTS221:  Temperature & Humidity
    - LPS22HB: Barometric Pressure
    - LSM6DSL: Accelerometer & Gyroscope
    - LIS2MDL: Magnetometer

  Displays readings on the OLED screen and prints to Serial.

  This example uses the SensorManager high-level API.
*/

#include "Arduino.h"
#include "OledDisplay.h"
#include "SensorManager.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("AZ3166 ReadAllSensors Example");

  // SensorManager is initialized by the framework at startup.
  // If you need to re-init after a deep sleep, call Sensors.init().
}

void loop()
{
  // Read all sensors in one call
  SensorData data = Sensors.readAll();

  // Print to Serial
  Serial.println("---- Sensor Readings ----");
  Serial.printf("Temperature: %.1f C\r\n", data.temperature);
  Serial.printf("Humidity:    %.1f %%\r\n", data.humidity);
  Serial.printf("Pressure:    %.1f hPa\r\n", data.pressure);
  Serial.printf("Accel:  X=%d  Y=%d  Z=%d mg\r\n",
                (int)data.accelX, (int)data.accelY, (int)data.accelZ);
  Serial.printf("Gyro:   X=%d  Y=%d  Z=%d mdps\r\n",
                (int)data.gyroX, (int)data.gyroY, (int)data.gyroZ);
  Serial.printf("Mag:    X=%d  Y=%d  Z=%d mGauss\r\n",
                (int)data.magX, (int)data.magY, (int)data.magZ);
  Serial.println();

  // Display summary on OLED
  char line[64];
  Screen.clean();
  snprintf(line, sizeof(line), "T:%.1fC H:%.0f%%", data.temperature, data.humidity);
  Screen.print(0, line);
  snprintf(line, sizeof(line), "P:%.1f hPa", data.pressure);
  Screen.print(1, line, true);
  snprintf(line, sizeof(line), "A:%d %d %d",
           (int)data.accelX, (int)data.accelY, (int)data.accelZ);
  Screen.print(2, line, true);
  snprintf(line, sizeof(line), "M:%d %d %d",
           (int)data.magX, (int)data.magY, (int)data.magZ);
  Screen.print(3, line, true);

  delay(2000);
}
