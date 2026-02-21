/*
  MQTTClient

  Connects to an MQTT broker over WiFi using PubSubClient, publishes
  messages to a topic, and subscribes to receive messages.

  Before running:
  1. Set your WiFi SSID and password below.
  2. Set the MQTT broker address (e.g., a public broker like
     "broker.hivemq.com" or your own).
*/

#include "Arduino.h"
#include "AZ3166WiFi.h"
#include "PubSubClient.h"
#include "OledDisplay.h"

// WiFi credentials
char ssid[] = "yourNetwork";
char pass[] = "yourPassword";

// MQTT broker
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* publishTopic = "az3166/outgoing";
const char* subscribeTopic = "az3166/incoming";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int msgCount = 0;

void mqttCallback(char* topic, uint8_t* payload, unsigned int length)
{
  Serial.printf("Message on [%s]: ", topic);
  char msg[256];
  unsigned int copyLen = (length < sizeof(msg) - 1) ? length : sizeof(msg) - 1;
  memcpy(msg, payload, copyLen);
  msg[copyLen] = '\0';
  Serial.println(msg);

  Screen.clean();
  Screen.print(0, "MQTT Received:");
  Screen.print(1, topic, true);
  Screen.print(2, msg, true);
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

bool connectMQTT()
{
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);

  Serial.printf("Connecting to MQTT broker %s...\r\n", mqttServer);
  Screen.clean();
  Screen.print(0, "MQTT connecting...");

  // Generate a unique client ID using MAC address
  char clientId[32];
  byte mac[6];
  WiFi.macAddress(mac);
  snprintf(clientId, sizeof(clientId), "az3166_%02x%02x%02x",
           mac[3], mac[4], mac[5]);

  if (mqttClient.connect(clientId)) {
    Serial.println("MQTT connected.");
    mqttClient.subscribe(subscribeTopic);
    Serial.printf("Subscribed to: %s\r\n", subscribeTopic);
    Screen.clean();
    Screen.print(0, "MQTT Connected");
    Screen.print(1, mqttServer, true);
    return true;
  } else {
    Serial.printf("MQTT connect failed, state=%d\r\n", mqttClient.state());
    Screen.print(1, "Connect failed", true);
    return false;
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("AZ3166 MQTT Client Example");

  initWiFi();
  connectMQTT();
}

void loop()
{
  if (!mqttClient.connected()) {
    Serial.println("MQTT disconnected, reconnecting...");
    connectMQTT();
  }

  mqttClient.loop();

  // Publish a message every 10 seconds
  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"deviceId\":\"AZ3166\",\"messageId\":%d}", msgCount);

  if (mqttClient.publish(publishTopic, payload)) {
    Serial.printf("Published [%s]: %s\r\n", publishTopic, payload);
    Screen.clean();
    Screen.print(0, "MQTT Published:");
    Screen.print(1, payload, true);
    msgCount++;
  } else {
    Serial.println("Publish failed.");
  }

  delay(10000);
}
