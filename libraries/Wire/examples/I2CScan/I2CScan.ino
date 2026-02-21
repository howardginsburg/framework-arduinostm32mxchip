/*
  I2CScan

  Scans the I2C bus for connected devices by probing addresses 1-127.
  Reports found devices on the Serial monitor and OLED screen.

  The MXChip AZ3166 has several onboard I2C peripherals that should
  be detected (e.g., HTS221, LPS22HB, LSM6DSL, LIS2MDL).
*/

#include "Arduino.h"
#include "Wire.h"
#include "OledDisplay.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("AZ3166 I2C Scanner");
  Serial.println("Scanning...");

  Screen.clean();
  Screen.print(0, "I2C Scanner");
  Screen.print(1, "Scanning...", true);

  Wire.begin();

  int deviceCount = 0;
  char oledBuf[64] = {0};
  int oledPos = 0;

  for (unsigned char address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    unsigned char error = Wire.endTransmission();

    if (error == I2C_OK) {
      Serial.printf("  Found device at 0x%02X\r\n", address);
      if (oledPos < (int)sizeof(oledBuf) - 6) {
        oledPos += snprintf(oledBuf + oledPos, sizeof(oledBuf) - oledPos,
                            "0x%02X ", address);
      }
      deviceCount++;
    }
  }

  Serial.printf("Scan complete: %d device(s) found.\r\n", deviceCount);

  Screen.clean();
  Screen.print(0, "I2C Scanner");
  if (deviceCount > 0) {
    char summary[32];
    snprintf(summary, sizeof(summary), "Found %d device(s)", deviceCount);
    Screen.print(1, summary, true);
    Screen.print(2, oledBuf, true);
  } else {
    Screen.print(1, "No devices found", true);
  }
}

void loop()
{
  delay(5000);
}
