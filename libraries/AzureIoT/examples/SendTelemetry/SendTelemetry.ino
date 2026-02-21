/*
  SendTelemetry

  Connects to Azure IoT Hub and sends telemetry messages containing
  sensor data every 10 seconds.

  Prerequisites:
  - Define CONNECTION_PROFILE at build time (e.g., PROFILE_IOTHUB_SAS).
    In PlatformIO: build_flags = -DCONNECTION_PROFILE=PROFILE_IOTHUB_SAS
  - Configure your IoT Hub connection via the DeviceConfig system
    (store credentials in EEPROM using the configuration sketch or
    serial commands before running this example).
  - The device must be connected to WiFi before azureIoTInit() is called.
*/

#include "Arduino.h"
#include "AZ3166WiFi.h"
#include "OledDisplay.h"
#include "AzureIoTHub.h"
#include "SensorManager.h"

// WiFi credentials
char ssid[] = "yourNetwork";
char pass[] = "yourPassword";

int msgCount = 0;

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
  Serial.println("AZ3166 Azure IoT SendTelemetry Example");

  initWiFi();

  // Initialize and connect to Azure IoT Hub
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
  Serial.printf("Device ID: %s\r\n", azureIoTGetDeviceId());
  Screen.clean();
  Screen.print(0, "IoT Hub Connected");
  Screen.print(1, azureIoTGetDeviceId(), true);
}

void loop()
{
  if (!azureIoTIsConnected()) {
    Serial.println("Disconnected. Reconnecting...");
    azureIoTConnect();
    delay(5000);
    return;
  }

  // Process incoming messages
  azureIoTLoop();

  // Read sensors and build telemetry payload
  float temperature = Sensors.getTemperature();
  float humidity = Sensors.getHumidity();
  float pressure = Sensors.getPressure();

  char payload[256];
  snprintf(payload, sizeof(payload),
           "{\"messageId\":%d,\"temperature\":%.1f,"
           "\"humidity\":%.1f,\"pressure\":%.1f}",
           msgCount, temperature, humidity, pressure);

  if (azureIoTSendTelemetry(payload)) {
    Serial.printf("Sent: %s\r\n", payload);
    Screen.clean();
    Screen.print(0, "Telemetry Sent");
    char line[32];
    snprintf(line, sizeof(line), "#%d T:%.1f H:%.0f%%", msgCount, temperature, humidity);
    Screen.print(1, line, true);
    msgCount++;
  } else {
    Serial.println("Send telemetry failed.");
  }

  delay(10000);
}
