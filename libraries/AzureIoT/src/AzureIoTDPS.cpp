/*
 * AzureIoTDPS.cpp - Azure Device Provisioning Service (DPS) registration
 *
 * Part of the MXChip AZ3166 framework Azure IoT library.
 */

#include "AzureIoTDPS.h"
#include "AzureIoTConfig.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include "AZ3166WiFi.h"

// ===== DPS registration state (file-scope, single-threaded safe) =====
static char s_operationId[128];
static char s_assignedHub[128];
static char s_assignedDeviceId[64];
static bool s_assigned = false;
static int  s_responseStatus = 0;

// Minimal JSON string extractor - finds "key":"value" and copies value to output
static bool jsonExtractString(const char* json, const char* key, char* output, size_t outputSize)
{
    char searchKey[64];
    snprintf(searchKey, sizeof(searchKey), "\"%s\"", key);
    const char* keyPos = strstr(json, searchKey);
    if (keyPos == NULL) return false;

    keyPos += strlen(searchKey);
    while (*keyPos == ':' || *keyPos == ' ' || *keyPos == '\t') keyPos++;
    if (*keyPos != '"') return false;
    keyPos++; // skip opening quote

    const char* valueEnd = strchr(keyPos, '"');
    if (valueEnd == NULL) return false;

    size_t valueLen = valueEnd - keyPos;
    if (valueLen >= outputSize) return false;

    strncpy(output, keyPos, valueLen);
    output[valueLen] = '\0';
    return true;
}

// DPS MQTT callback - handles registration responses
static void dpsCallback(char* topic, byte* payload, unsigned int length)
{
    char message[1024];
    unsigned int copyLen = (length < sizeof(message) - 1) ? length : sizeof(message) - 1;
    memcpy(message, payload, copyLen);
    message[copyLen] = '\0';

    Serial.println("[DPS] Response received");
    Serial.print("[DPS] Topic: ");
    Serial.println(topic);
    Serial.print("[DPS] Payload: ");
    Serial.println(message);

    // Parse status from topic: $dps/registrations/res/{status}/?$rid={rid}
    const char* statusStr = topic + strlen("$dps/registrations/res/");
    int status = atoi(statusStr);
    s_responseStatus = status;

    Serial.print("[DPS] Status: ");
    Serial.println(status);

    if (status == 202)
    {
        // Registration in progress - extract operationId
        if (jsonExtractString(message, "operationId", s_operationId, sizeof(s_operationId)))
        {
            Serial.print("[DPS] Operation ID: ");
            Serial.println(s_operationId);
        }
        else
        {
            Serial.println("[DPS] Error: Could not extract operationId!");
        }
    }
    else if (status == 200)
    {
        // Registration complete - extract assigned hub and device ID
        if (jsonExtractString(message, "assignedHub", s_assignedHub, sizeof(s_assignedHub)))
        {
            Serial.print("[DPS] Assigned Hub: ");
            Serial.println(s_assignedHub);
        }
        else
        {
            Serial.println("[DPS] Error: Could not extract assignedHub!");
            return;
        }

        if (!jsonExtractString(message, "deviceId", s_assignedDeviceId, sizeof(s_assignedDeviceId)))
        {
            // deviceId not returned is non-fatal; caller can use registrationId
            s_assignedDeviceId[0] = '\0';
        }
        else
        {
            Serial.print("[DPS] Assigned Device ID: ");
            Serial.println(s_assignedDeviceId);
        }

        s_assigned = true;
    }
    else
    {
        Serial.print("[DPS] Registration failed with status: ");
        Serial.println(status);
    }
}

bool AzureIoT_DPSRegister(WiFiClientSecure& wifiClient,
                            const char* endpoint, const char* scopeId,
                            const char* registrationId, const char* password,
                            char* assignedHub, size_t hubSize,
                            char* assignedDeviceId, size_t deviceIdSize)
{
    Serial.println("[DPS] Starting device provisioning...");
    Serial.print("[DPS] Endpoint: ");
    Serial.println(endpoint);
    Serial.print("[DPS] Scope ID: ");
    Serial.println(scopeId);
    Serial.print("[DPS] Registration ID: ");
    Serial.println(registrationId);

    // Configure TLS CA cert for DPS
    wifiClient.setCACert(AZURE_IOT_ROOT_CA);

    // Build DPS MQTT username
    char dpsUsername[256];
    snprintf(dpsUsername, sizeof(dpsUsername),
        "%s/registrations/%s/api-version=%s&ClientVersion=1.0",
        scopeId, registrationId, DPS_API_VERSION);

    // Create a PubSubClient for DPS registration
    PubSubClient dpsMqtt(wifiClient);
    dpsMqtt.setServer(endpoint, MQTT_PORT);
    dpsMqtt.setCallback(dpsCallback);
    dpsMqtt.setBufferSize(1024);
    dpsMqtt.setKeepAlive(60);
    dpsMqtt.setSocketTimeout(30);

    // Reset state
    s_assigned = false;
    s_responseStatus = 0;
    s_operationId[0] = '\0';

    // Connect to DPS endpoint
    Serial.print("[DPS] Connecting to ");
    Serial.println(endpoint);

    bool connected;
    if (password != NULL)
    {
        connected = dpsMqtt.connect(registrationId, dpsUsername, password);
    }
    else
    {
        connected = dpsMqtt.connect(registrationId, dpsUsername, "");
    }

    if (!connected)
    {
        Serial.println("[DPS] Failed to connect to DPS!");
        Serial.print("[DPS] MQTT state: ");
        Serial.println(dpsMqtt.state());
        return false;
    }
    Serial.println("[DPS] Connected to DPS");

    // Subscribe to DPS response topic
    if (!dpsMqtt.subscribe("$dps/registrations/res/#"))
    {
        Serial.println("[DPS] Failed to subscribe to response topic!");
        dpsMqtt.disconnect();
        return false;
    }

    // Publish registration request
    static int dpsRid = 0;
    char registerTopic[64];
    snprintf(registerTopic, sizeof(registerTopic),
        "$dps/registrations/PUT/iotdps-register/?$rid=%d", ++dpsRid);

    char registerPayload[256];
    snprintf(registerPayload, sizeof(registerPayload),
        "{\"registrationId\":\"%s\"}", registrationId);

    Serial.println("[DPS] Sending registration request...");
    if (!dpsMqtt.publish(registerTopic, registerPayload))
    {
        Serial.println("[DPS] Failed to send registration request!");
        dpsMqtt.disconnect();
        return false;
    }

    // Poll for registration result
    int retries = 0;

    while (!s_assigned && retries < DPS_MAX_RETRIES)
    {
        // Wait for a DPS response via the callback
        unsigned long waitStart = millis();
        while (s_responseStatus == 0 && (millis() - waitStart) < 10000)
        {
            dpsMqtt.loop();
            delay(100);
        }

        if (s_responseStatus == 0)
        {
            Serial.println("[DPS] Timeout waiting for response");
            retries++;
            continue;
        }

        if (s_assigned)
        {
            break;
        }

        if (s_responseStatus == 202 && s_operationId[0] != '\0')
        {
            Serial.print("[DPS] Polling status (attempt ");
            Serial.print(retries + 1);
            Serial.println(")...");

            delay(DPS_POLL_INTERVAL);

            char statusTopic[256];
            snprintf(statusTopic, sizeof(statusTopic),
                "$dps/registrations/GET/iotdps-get-operationstatus/?$rid=%d&operationId=%s",
                ++dpsRid, s_operationId);

            s_responseStatus = 0;
            dpsMqtt.publish(statusTopic, "");
            retries++;
        }
        else if (s_responseStatus != 200)
        {
            Serial.print("[DPS] Registration failed with status: ");
            Serial.println(s_responseStatus);
            dpsMqtt.disconnect();
            return false;
        }
    }

    // Disconnect from DPS
    dpsMqtt.disconnect();
    wifiClient.stop();
    delay(500);

    if (!s_assigned)
    {
        Serial.println("[DPS] Registration timed out!");
        return false;
    }

    // Copy results to caller's buffers
    strncpy(assignedHub, s_assignedHub, hubSize - 1);
    assignedHub[hubSize - 1] = '\0';

    if (s_assignedDeviceId[0] != '\0')
    {
        strncpy(assignedDeviceId, s_assignedDeviceId, deviceIdSize - 1);
        assignedDeviceId[deviceIdSize - 1] = '\0';
    }
    else
    {
        // Use registration ID as device ID if DPS didn't return one
        strncpy(assignedDeviceId, registrationId, deviceIdSize - 1);
        assignedDeviceId[deviceIdSize - 1] = '\0';
        Serial.print("[DPS] Using registration ID as Device ID: ");
        Serial.println(assignedDeviceId);
    }

    Serial.println("[DPS] Device provisioned successfully!");
    Serial.print("[DPS] Assigned to: ");
    Serial.println(assignedHub);
    Serial.print("[DPS] Device ID: ");
    Serial.println(assignedDeviceId);

    return true;
}
