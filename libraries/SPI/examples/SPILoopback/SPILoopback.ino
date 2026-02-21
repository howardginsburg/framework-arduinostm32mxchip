/*
  SPILoopback

  Demonstrates the SPI interface on the MXChip AZ3166.

  Sends a sequence of bytes via SPI transfer() and prints the
  returned values to Serial. In a loopback configuration (MOSI
  connected to MISO), the received data should match the sent data.

  Without a physical loopback wire, the received bytes will typically
  be 0x00 or 0xFF, which is expected.
*/

#include "Arduino.h"
#include "AZ3166SPI.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("AZ3166 SPI Loopback Example");

  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV16, MSBFIRST, SPI_MODE0));

  Serial.println("Sending bytes 0x00..0x0F via SPI:");
  Serial.println("Sent -> Received");

  for (uint8_t i = 0; i < 16; i++) {
    uint8_t received = SPI.transfer(i);
    Serial.printf("0x%02X -> 0x%02X %s\r\n", i, received,
                  (received == i) ? "(match)" : "");
  }

  SPI.endTransaction();
  SPI.end();

  Serial.println("Done. Connect MOSI to MISO for a true loopback test.");
}

void loop()
{
  delay(1000);
}
