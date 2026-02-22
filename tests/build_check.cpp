/**
 * Build verification sketch.
 *
 * Including a library's public header is enough to make PlatformIO's LDF
 * discover and compile its sources.  Any error anywhere in a library tree
 * will fail the build.
 *
 * No runtime calls are made — the goal is compile + link coverage only.
 */

#include <Arduino.h>
#include <IPAddress.h>
#include <EEPROMInterface.h>
#include <TLSSocket.h>

#include <SensorManager.h>
#include <RGB_LED.h>

#include <AZ3166WiFi.h>
#include <AZ3166WiFiClient.h>
#include <AZ3166WiFiClientSecure.h>
#include <AZ3166WiFiServer.h>
#include <AZ3166WiFiUdp.h>

#include <AZ3166SPI.h>
#include <Wire.h>

#include <SFlashBlockDevice.h>

#include <PubSubClient.h>
#include <WebSocketClient.h>

#if __has_include(<AzureIoTHub.h>)
#include <AzureIoTHub.h>
#include <AzureIoTCrypto.h>
#endif

void setup() {
    Serial.begin(115200);
}

void loop() {
    delay(1000);
}
