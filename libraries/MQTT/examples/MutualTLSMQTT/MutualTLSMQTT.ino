/**
 * @file MutualTLSMQTT.ino
 * @brief Example: MQTT with mutual TLS (client certificate authentication)
 * 
 * This example demonstrates connecting to an MQTT broker using mutual TLS,
 * where both the server and client present certificates for authentication.
 * This is the preferred method for production IoT deployments.
 * 
 * Mutual TLS provides:
 * - Server authentication (client verifies server certificate)
 * - Client authentication (server verifies client certificate)
 * - No need for username/password
 * - Strong cryptographic identity
 * 
 * Setup:
 * 1. Generate a client certificate and private key for your device
 * 2. Register the certificate with your MQTT broker/cloud service
 * 3. Update the certificates below with your actual PEM data
 * 
 * Compatible services:
 * - Azure IoT Hub (X.509 authentication)
 * - AWS IoT Core (X.509 certificates)
 * - Mosquitto with require_certificate=true
 */

#include <AZ3166WiFi.h>
#include "AZ3166MQTTClient.h"

// WiFi credentials
static char* WIFI_SSID = (char*)"your-wifi-ssid";
static char* WIFI_PASSWORD = (char*)"your-wifi-password";

// MQTT broker settings
static const char* MQTT_HOST = "your-iot-hub.azure-devices.net";
static const int MQTT_PORT = 8883;
static const char* MQTT_CLIENT_ID = "az3166-device-001";

// CA Certificate - Root CA that signed the broker's certificate
// For Azure IoT Hub: DigiCert Global Root G2
// For AWS IoT Core: Amazon Root CA 1
static const char* CA_CERT = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
    "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
    "// ... truncated - use full DigiCert Global Root G2 certificate ...\n"
    "-----END CERTIFICATE-----\n";

// Client Certificate - Your device's X.509 certificate
// Generate using OpenSSL or your cloud provider's certificate service
static const char* CLIENT_CERT = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIICpDCCAYwCCQD... // Your device certificate\n"
    "// ... certificate data ...\n"
    "-----END CERTIFICATE-----\n";

// Client Private Key - Keep this secret!
// WARNING: For production, store in secure EEPROM instead of code
static const char* CLIENT_KEY = 
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEowIBAAKCAQEA... // Your private key\n"
    "// ... key data ...\n"
    "-----END RSA PRIVATE KEY-----\n";

// Azure IoT Hub MQTT topics
// Replace {device-id} with your actual device ID
static const char* TELEMETRY_TOPIC = "devices/az3166-device-001/messages/events/";
static const char* COMMAND_TOPIC = "devices/az3166-device-001/messages/devicebound/#";

// Standard MQTT topics for generic brokers
// static const char* PUBLISH_TOPIC = "sensors/az3166/data";
// static const char* SUBSCRIBE_TOPIC = "commands/az3166/#";

AZ3166MQTTClient mqttClient;
int messageCount = 0;

void messageHandler(MQTT::MessageData& data)
{
    Serial.println("Received message:");
    
    // Extract and print topic
    int topicLen = data.topicName.lenstring.len;
    char* topicData = data.topicName.lenstring.data;
    Serial.print("  Topic: ");
    for (int i = 0; i < topicLen && i < 128; i++)
    {
        Serial.print(topicData[i]);
    }
    Serial.println();
    
    // Extract and print payload
    int payloadLen = data.message.payloadlen;
    char* payload = (char*)data.message.payload;
    Serial.print("  Payload (");
    Serial.print(payloadLen);
    Serial.print(" bytes): ");
    for (int i = 0; i < payloadLen && i < 256; i++)
    {
        Serial.print(payload[i]);
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  Mutual TLS MQTT Example");
    Serial.println("  Client Certificate Authentication");
    Serial.println("========================================\n");
    
    // Connect to WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int wifiAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 30)
    {
        delay(500);
        Serial.print(".");
        wifiAttempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nFailed to connect to WiFi!");
        while (1) { delay(1000); }
    }
    
    Serial.println(" Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Connect with mutual TLS
    Serial.println("\nEstablishing mutual TLS connection...");
    Serial.print("  Broker: ");
    Serial.println(MQTT_HOST);
    Serial.print("  Port: ");
    Serial.println(MQTT_PORT);
    Serial.println("  Auth: Client Certificate (X.509)");
    
    int result = mqttClient.connectMutualTLS(
        MQTT_HOST,
        MQTT_PORT,
        CA_CERT,
        CLIENT_CERT,
        CLIENT_KEY,
        MQTT_CLIENT_ID,
        MQTT_CLIENT_ID  // Also pass client ID as username for Event Grid
    );
    
    if (result != 0)
    {
        Serial.print("\nConnection FAILED! Error code: ");
        Serial.println(result);
        Serial.println("\nTroubleshooting:");
        Serial.println("  1. Verify CA certificate is correct");
        Serial.println("  2. Check client certificate is registered with broker");
        Serial.println("  3. Ensure private key matches certificate");
        Serial.println("  4. Verify broker hostname and port");
        while (1) { delay(1000); }
    }
    
    Serial.println("\n*** Connected with Mutual TLS! ***\n");
    
    // Subscribe to commands
    Serial.print("Subscribing to: ");
    Serial.println(COMMAND_TOPIC);
    
    result = mqttClient.subscribe(COMMAND_TOPIC, MQTT::QOS1, messageHandler);
    if (result < 0)
    {
        Serial.print("Subscribe failed: ");
        Serial.println(result);
    }
    else
    {
        Serial.println("Subscribed successfully!");
    }
    
    Serial.println("\nReady. Sending telemetry every 30 seconds...\n");
}

void loop()
{
    static unsigned long lastPublish = 0;
    unsigned long now = millis();
    
    // Publish telemetry every 30 seconds
    if (now - lastPublish >= 30000 || lastPublish == 0)
    {
        lastPublish = now;
        
        // Simulate sensor readings
        float temperature = 20.0 + (random(200) / 10.0);  // 20.0 - 40.0
        float humidity = 40.0 + (random(400) / 10.0);     // 40.0 - 80.0
        
        // Create telemetry JSON
        char payload[256];
        snprintf(payload, sizeof(payload),
            "{"
            "\"messageId\":%d,"
            "\"temperature\":%.2f,"
            "\"humidity\":%.2f,"
            "\"timestamp\":%lu"
            "}",
            messageCount++,
            temperature,
            humidity,
            now / 1000
        );
        
        Serial.print("[");
        Serial.print(messageCount);
        Serial.print("] Publishing telemetry: ");
        Serial.println(payload);
        
        int result = mqttClient.publish(TELEMETRY_TOPIC, payload, 0, MQTT::QOS1);
        if (result != 0)
        {
            Serial.print("    Publish failed: ");
            Serial.println(result);
        }
        else
        {
            Serial.println("    Sent successfully");
        }
    }
    
    // Process incoming messages and keepalive
    if (!mqttClient.isConnected())
    {
        Serial.println("Connection lost! Reconnecting...");
        int result = mqttClient.connectMutualTLS(
            MQTT_HOST, MQTT_PORT, CA_CERT, CLIENT_CERT, CLIENT_KEY, 
            MQTT_CLIENT_ID, MQTT_CLIENT_ID
        );
        if (result == 0)
        {
            Serial.println("Reconnected!");
            mqttClient.subscribe(COMMAND_TOPIC, MQTT::QOS1, messageHandler);
        }
    }
    
    mqttClient.loop(100);
}
