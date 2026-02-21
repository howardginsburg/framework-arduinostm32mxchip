/*
  DeviceTwin

  Demonstrates Azure IoT Hub Device Twin operations:
    - Requests the full device twin on startup
    - Listens for desired property updates from the cloud
    - Reports properties back to IoT Hub

  Prerequisites:
  - Define CONNECTION_PROFILE at build time (e.g., PROFILE_IOTHUB_SAS).
    In PlatformIO: build_flags = -DCONNECTION_PROFILE=PROFILE_IOTHUB_SAS
  - Configure your IoT Hub connection via the DeviceConfig system.
  - The device must be connected to WiFi before azureIoTInit() is called.
*/

#include "Arduino.h"
#include "AZ3166WiFi.h"
#include "OledDisplay.h"
#include "AzureIoTHub.h"

// WiFi credentials
char ssid[] = "yourNetwork";
char pass[] = "yourPassword";

void onTwinReceived(const char* payload)
{
  Serial.println("Full twin received:");
  Serial.println(payload);
  Screen.clean();
  Screen.print(0, "Twin Received");
}

void onDesiredProperties(const char* payload, int version)
{
  Serial.printf("Desired properties (v%d):\r\n", version);
  Serial.println(payload);

  Screen.clean();
  Screen.print(0, "Desired Props");
  char line[32];
  snprintf(line, sizeof(line), "version: %d", version);
  Screen.print(1, line, true);

  // Acknowledge by reporting back
  char reported[128];
  snprintf(reported, sizeof(reported),
           "{\"lastDesiredVersion\":%d}", version);
  azureIoTUpdateReportedProperties(reported);
  Serial.printf("Reported: %s\r\n", reported);
}

bool initWiFi()
{
  Screen.print("WiFi\r\nConnecting...\r\n \r\n \r\n");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.printf("Connecting to %s...\r\n", ssid);
    status = WiFi.begin(ssid, pass);
    if (status != WL_CONNECTED) delay(5000);
  }
  Serial.printf("WiFi connected. IP: %s\r\n", WiFi.localIP().get_address());
  return true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("AZ3166 Azure IoT DeviceTwin Example");

  initWiFi();

  // Register callbacks before connecting
  azureIoTSetTwinReceivedCallback(onTwinReceived);
  azureIoTSetDesiredPropertiesCallback(onDesiredProperties);

  if (!azureIoTInit()) {
    Serial.println("Azure IoT init failed. Check EEPROM config.");
    Screen.print(0, "IoT Init Failed");
    return;
  }

  if (!azureIoTConnect()) {
    Serial.println("Azure IoT connect failed.");
    Screen.print(0, "IoT Connect Failed");
    return;
  }

  Serial.printf("Connected to IoT Hub: %s\r\n", azureIoTGetHostname());
  Screen.clean();
  Screen.print(0, "IoT Hub Connected");
  Screen.print(1, azureIoTGetDeviceId(), true);

  // Request the full device twin
  azureIoTRequestTwin();
  Serial.println("Requested device twin.");

  // Report initial properties
  char reported[128];
  snprintf(reported, sizeof(reported),
           "{\"firmwareVersion\":\"1.0.0\",\"deviceModel\":\"AZ3166\"}");
  azureIoTUpdateReportedProperties(reported);
  Serial.printf("Reported: %s\r\n", reported);
}

void loop()
{
  if (!azureIoTIsConnected()) {
    Serial.println("Disconnected. Reconnecting...");
    azureIoTConnect();
    delay(5000);
    return;
  }

  // Process MQTT messages (twin updates arrive here)
  azureIoTLoop();
  delay(100);
}
