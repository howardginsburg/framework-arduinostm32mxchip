/**
 * @file SecureMQTT.ino
 * @brief Example: MQTT over TLS with username/password authentication
 * 
 * This example demonstrates connecting to an MQTT broker using TLS
 * with server certificate verification and username/password authentication.
 * 
 * Setup:
 * 1. Update WIFI_SSID and WIFI_PASSWORD with your network credentials
 * 2. Update MQTT_HOST with your broker hostname
 * 3. Update MQTT_USERNAME and MQTT_PASSWORD
 * 4. Replace CA_CERT with your broker's CA certificate (PEM format)
 * 
 * Compatible brokers:
 * - Azure IoT Hub
 * - AWS IoT Core (with custom authentication)
 * - Mosquitto with TLS enabled
 * - HiveMQ Cloud
 * - Any MQTT 3.1.1 broker with TLS support
 */

#include <AZ3166WiFi.h>
#include "AZ3166MQTTClient.h"

// WiFi credentials
static char* WIFI_SSID = (char*)"your-wifi-ssid";
static char* WIFI_PASSWORD = (char*)"your-wifi-password";

// MQTT broker settings
static const char* MQTT_HOST = "broker.example.com";
static const int MQTT_PORT = 8883;
static const char* MQTT_CLIENT_ID = "az3166-device-001";
static const char* MQTT_USERNAME = "your-username";
static const char* MQTT_PASSWORD = "your-password";

// CA Certificate (PEM format)
// Replace with your broker's CA certificate
static const char* CA_CERT = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
    "// ... truncated for example - use full certificate ...\n"
    "-----END CERTIFICATE-----\n";

// Topics
static const char* PUBLISH_TOPIC = "devices/az3166/telemetry";
static const char* SUBSCRIBE_TOPIC = "devices/az3166/commands";

// MQTT client
AZ3166MQTTClient mqttClient;

// Message counter
int messageCount = 0;

/**
 * @brief Callback for incoming MQTT messages
 */
void messageHandler(MQTT::MessageData& data)
{
    Serial.print("Message received on topic: ");
    
    // Print topic
    char topic[64];
    int topicLen = data.topicName.lenstring.len;
    if (topicLen > 63) topicLen = 63;
    memcpy(topic, data.topicName.lenstring.data, topicLen);
    topic[topicLen] = '\0';
    Serial.println(topic);
    
    // Print payload
    Serial.print("Payload: ");
    char* payload = (char*)data.message.payload;
    int payloadLen = data.message.payloadlen;
    for (int i = 0; i < payloadLen; i++)
    {
        Serial.print(payload[i]);
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\n=== Secure MQTT Example ===\n");
    
    // Connect to WiFi
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Connect to MQTT broker with TLS
    Serial.print("Connecting to MQTT broker: ");
    Serial.print(MQTT_HOST);
    Serial.print(":");
    Serial.println(MQTT_PORT);
    
    int result = mqttClient.connectSecure(
        MQTT_HOST, 
        MQTT_PORT, 
        CA_CERT,
        MQTT_CLIENT_ID,
        MQTT_USERNAME,
        MQTT_PASSWORD
    );
    
    if (result != 0)
    {
        Serial.print("MQTT connection failed with error: ");
        Serial.println(result);
        while (1) { delay(1000); }
    }
    
    Serial.println("MQTT connected with TLS!");
    Serial.print("Connection mode: ");
    Serial.println(mqttClient.getConnectionMode() == AZ3166MQTTClient::MODE_TLS_SERVER_ONLY 
                   ? "TLS (Server Auth)" : "Unknown");
    
    // Subscribe to topic
    Serial.print("Subscribing to: ");
    Serial.println(SUBSCRIBE_TOPIC);
    
    result = mqttClient.subscribe(SUBSCRIBE_TOPIC, MQTT::QOS0, messageHandler);
    if (result != 0)
    {
        Serial.print("Subscribe failed: ");
        Serial.println(result);
    }
    else
    {
        Serial.println("Subscribed successfully!");
    }
}

void loop()
{
    // Send a message every 10 seconds
    static unsigned long lastPublish = 0;
    unsigned long now = millis();
    
    if (now - lastPublish >= 10000)
    {
        lastPublish = now;
        
        // Create JSON payload
        char payload[128];
        snprintf(payload, sizeof(payload), 
                 "{\"deviceId\":\"%s\",\"messageId\":%d,\"temperature\":%.1f}",
                 MQTT_CLIENT_ID, messageCount++, 25.0 + (random(100) / 10.0));
        
        Serial.print("Publishing: ");
        Serial.println(payload);
        
        int result = mqttClient.publish(PUBLISH_TOPIC, payload);
        if (result != 0)
        {
            Serial.print("Publish failed: ");
            Serial.println(result);
            
            // Attempt to reconnect
            if (!mqttClient.isConnected())
            {
                Serial.println("Reconnecting...");
                mqttClient.connectSecure(MQTT_HOST, MQTT_PORT, CA_CERT,
                                         MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
            }
        }
    }
    
    // Process incoming messages
    mqttClient.loop(100);
}
