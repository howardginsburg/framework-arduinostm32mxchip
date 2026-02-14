/*
 * AzureIoTHub.h - Azure IoT Hub MQTT client for MXChip AZ3166
 *
 * Pure MQTT implementation supporting multiple connection profiles:
 * - PROFILE_IOTHUB_SAS:       Direct IoT Hub with SAS token
 * - PROFILE_IOTHUB_CERT:      Direct IoT Hub with X.509 certificate
 * - PROFILE_DPS_SAS:          DPS provisioning with individual symmetric key
 * - PROFILE_DPS_SAS_GROUP:    DPS provisioning with group symmetric key
 * - PROFILE_DPS_CERT:         DPS provisioning with X.509 certificate
 *
 * Handles credential loading from EEPROM (via DeviceConfig), DPS provisioning,
 * SAS token generation, MQTT connection management, telemetry, C2D messages,
 * and Device Twin operations.
 *
 * Part of the MXChip AZ3166 framework Azure IoT library.
 */

#ifndef AZURE_IOT_HUB_H
#define AZURE_IOT_HUB_H

#include <Arduino.h>

// ===== CALLBACK TYPES =====

// Called when a C2D message is received
typedef void (*C2DMessageCallback)(const char* topic, const char* payload, unsigned int length);

// Called when desired properties are updated
typedef void (*DesiredPropertiesCallback)(const char* payload, int version);

// Called when full twin is received (response to GET)
typedef void (*TwinReceivedCallback)(const char* payload);

// ===== INITIALIZATION =====

// Initialize the Azure IoT MQTT library. Must be called after WiFi is connected.
bool azureIoTInit();

// Connect to Azure IoT Hub via MQTT
bool azureIoTConnect();

// Check if connected to IoT Hub
bool azureIoTIsConnected();

// Must be called in loop() to process MQTT messages
void azureIoTLoop();

// ===== CALLBACKS =====

void azureIoTSetC2DCallback(C2DMessageCallback callback);
void azureIoTSetDesiredPropertiesCallback(DesiredPropertiesCallback callback);
void azureIoTSetTwinReceivedCallback(TwinReceivedCallback callback);

// ===== TELEMETRY (D2C) =====

// Send telemetry message with optional URL-encoded properties
bool azureIoTSendTelemetry(const char* payload, const char* properties = NULL);

// ===== DEVICE TWIN =====

// Request full device twin (response via TwinReceivedCallback)
void azureIoTRequestTwin();

// Update reported properties with a JSON object
void azureIoTUpdateReportedProperties(const char* jsonPayload);

// ===== ACCESSORS =====

const char* azureIoTGetDeviceId();
const char* azureIoTGetHostname();

#endif // AZURE_IOT_HUB_H
