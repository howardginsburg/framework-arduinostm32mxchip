/**
 * @file EEPROMCredentials.ino
 * @brief Example: Loading MQTT credentials from secure EEPROM
 * 
 * This example demonstrates loading certificates and credentials from
 * the STSAFE-A100 secure element on the AZ3166, rather than embedding
 * them in the firmware.
 * 
 * Benefits of EEPROM storage:
 * - Credentials can be provisioned per-device without rebuilding firmware
 * - Private keys can be encrypted at rest (with secure channel enabled)
 * - Easier credential rotation without firmware updates
 * - Supports manufacturing provisioning workflows
 * 
 * Provisioning workflow:
 * 1. Use a separate provisioning sketch to store credentials once
 * 2. This sketch reads the stored credentials at runtime
 * 
 * See: ProvisionCredentials.ino for credential storage example
 */

#include <AZ3166WiFi.h>
#include "AZ3166MQTTClient.h"
#include "EEPROMInterface.h"

// WiFi credentials (could also be in EEPROM)
static char* WIFI_SSID = (char*)"your-wifi-ssid";
static char* WIFI_PASSWORD = (char*)"your-wifi-password";

// MQTT settings - host can also be loaded from EEPROM
static const char* MQTT_CLIENT_ID = "az3166-device-001";
static const int MQTT_PORT = 8883;

AZ3166MQTTClient mqttClient;
EEPROMInterface eeprom;

// Buffers for EEPROM data
char mqttHost[MQTT_MAX_LEN + 1];
char caCert[AZ_IOT_X509_MAX_LEN + 1];
char deviceUsername[DEVICE_ID_MAX_LEN + 1];
char devicePassword[DEVICE_PASSWORD_MAX_LEN + 1];

void messageHandler(MQTT::MessageData& data)
{
    Serial.print("Message on ");
    Serial.write((uint8_t*)data.topicName.lenstring.data, data.topicName.lenstring.len);
    Serial.print(": ");
    Serial.write((uint8_t*)data.message.payload, data.message.payloadlen);
    Serial.println();
}

bool loadCredentials()
{
    Serial.println("Loading credentials from EEPROM...");
    
    // Load MQTT broker address
    if (eeprom.readMQTTAddress(mqttHost, sizeof(mqttHost)) != 0 || mqttHost[0] == 0)
    {
        Serial.println("  ERROR: No MQTT address stored");
        return false;
    }
    Serial.print("  MQTT Host: ");
    Serial.println(mqttHost);
    
    // Load CA certificate
    if (eeprom.readX509Cert(caCert, sizeof(caCert)) != 0 || caCert[0] == 0)
    {
        Serial.println("  ERROR: No CA certificate stored");
        return false;
    }
    Serial.println("  CA Cert: Loaded");
    
    // Load username (device ID)
    if (eeprom.readDeviceID(deviceUsername, sizeof(deviceUsername)) != 0)
    {
        deviceUsername[0] = '\0';  // Optional, can be empty
    }
    if (deviceUsername[0])
    {
        Serial.print("  Username: ");
        Serial.println(deviceUsername);
    }
    
    // Load password
    if (eeprom.readDevicePassword(devicePassword, sizeof(devicePassword)) != 0)
    {
        devicePassword[0] = '\0';  // Optional, can be empty
    }
    if (devicePassword[0])
    {
        Serial.println("  Password: ****");
    }
    
    Serial.println("Credentials loaded successfully!");
    return true;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  EEPROM Credentials Example");
    Serial.println("========================================\n");
    
    // Load credentials first
    if (!loadCredentials())
    {
        Serial.println("\nFailed to load credentials!");
        Serial.println("Run ProvisionCredentials sketch first.");
        while (1) { delay(1000); }
    }
    
    // Connect to WiFi
    Serial.print("\nConnecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Connected!");
    
    // Connect to MQTT using loaded credentials
    Serial.println("\nConnecting to MQTT broker...");
    
    int result = mqttClient.connectSecure(
        mqttHost,
        MQTT_PORT,
        caCert,
        MQTT_CLIENT_ID,
        deviceUsername[0] ? deviceUsername : NULL,
        devicePassword[0] ? devicePassword : NULL
    );
    
    if (result != 0)
    {
        Serial.print("Connection failed: ");
        Serial.println(result);
        while (1) { delay(1000); }
    }
    
    Serial.println("Connected successfully!");
    
    // Subscribe to a topic
    mqttClient.subscribe("test/topic", MQTT::QOS0, messageHandler);
}

void loop()
{
    static unsigned long lastMsg = 0;
    
    if (millis() - lastMsg > 10000)
    {
        lastMsg = millis();
        mqttClient.publish("test/topic", "Hello from EEPROM credentials!");
        Serial.println("Published message");
    }
    
    mqttClient.loop(100);
}
