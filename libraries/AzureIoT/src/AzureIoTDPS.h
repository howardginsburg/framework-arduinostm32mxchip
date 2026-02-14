/*
 * AzureIoTDPS.h - Azure Device Provisioning Service (DPS) registration
 *
 * Performs device registration over MQTT using either SAS token or
 * X.509 certificate authentication. Returns the assigned IoT Hub
 * hostname and device ID.
 *
 * Part of the MXChip AZ3166 framework Azure IoT library.
 */

#ifndef AZURE_IOT_DPS_H
#define AZURE_IOT_DPS_H

#include <stddef.h>

// Forward declaration to avoid pulling in WiFi headers
class WiFiClientSecure;

// Register a device with Azure DPS via MQTT.
//
// For SAS auth: pass the SAS token in `password`.
// For X.509 auth: set client cert/key on wifiClient before calling, pass NULL for `password`.
//
// On success, assignedHub and assignedDeviceId are populated with null-terminated strings.
// Returns true if registration was successful.
bool AzureIoT_DPSRegister(WiFiClientSecure& wifiClient,
                            const char* endpoint, const char* scopeId,
                            const char* registrationId, const char* password,
                            char* assignedHub, size_t hubSize,
                            char* assignedDeviceId, size_t deviceIdSize);

#endif // AZURE_IOT_DPS_H
