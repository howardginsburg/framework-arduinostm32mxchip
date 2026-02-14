// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __SETTING_UI_H__
#define __SETTING_UI_H__

#include "DeviceConfig.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Field type for UI rendering
 */
typedef enum {
    UI_FIELD_TEXT,        // Single-line text input
    UI_FIELD_TEXTAREA     // Multi-line text (for certificates/keys)
} UIFieldType;

/**
 * @brief UI metadata for a setting
 * 
 * This structure provides all the information needed to render
 * a setting in both CLI and Web interfaces.
 */
typedef struct {
    SettingID id;              // Setting ID from DeviceConfig
    const char* label;         // Human-readable label
    const char* cliCommand;    // CLI command name (e.g., "set_wifissid")
    const char* webFormName;   // HTML form field name (e.g., "SSID")
    const char* webPlaceholder;// Web form placeholder text
    const char* defaultValue;  // Default value for web forms (NULL if none)
    UIFieldType fieldType;     // Type of input field
} SettingUIMetadata;

/**
 * @brief UI metadata for all settings
 * 
 * The order determines display order in both CLI help and web forms.
 * WiFi is handled specially in web UI (dropdown + manual entry).
 */
static const SettingUIMetadata SETTING_UI[] = {
    // WiFi settings
    {SETTING_WIFI_SSID,        "WiFi SSID",              "set_wifissid",    "SSID",             "WiFi Network Name",                          NULL, UI_FIELD_TEXT},
    {SETTING_WIFI_PASSWORD,    "WiFi Password",          "set_wifipwd",     "PASS",             "WiFi Password",                              NULL, UI_FIELD_TEXT},
    
    // MQTT broker settings
    {SETTING_BROKER_URL,       "Broker URL",             "set_broker",      "BrokerURL",        "Broker URL (e.g., mqtts://broker:8883)",     NULL, UI_FIELD_TEXT},
    {SETTING_DEVICE_ID,        "Device ID",              "set_deviceid",    "DeviceID",         "Device/Client ID",                           NULL, UI_FIELD_TEXT},
    {SETTING_DEVICE_PASSWORD,  "Device Password",        "set_devicepwd",   "DevicePassword",   "Password",                                   NULL, UI_FIELD_TEXT},
    
    // Azure IoT Hub settings
    {SETTING_CONNECTION_STRING,"Connection String",      "set_connstring",  "ConnectionString", "IoT Hub Connection String",                  NULL, UI_FIELD_TEXT},
    
    // Azure DPS settings
    {SETTING_DPS_ENDPOINT,     "DPS Endpoint",           "set_dps_endpoint","DPSEndpoint",      "DPS Endpoint",                               "global.azure-devices-provisioning.net", UI_FIELD_TEXT},
    {SETTING_SCOPE_ID,         "Scope ID",               "set_scopeid",     "ScopeId",          "DPS ID Scope",                               NULL, UI_FIELD_TEXT},
    {SETTING_REGISTRATION_ID,  "Registration ID",        "set_regid",       "RegistrationId",   "Registration ID",                            NULL, UI_FIELD_TEXT},
    {SETTING_SYMMETRIC_KEY,    "Symmetric Key",          "set_symkey",      "SymmetricKey",     "Symmetric Key",                              NULL, UI_FIELD_TEXT},
    
    // Certificate/key settings (textarea in web)
    {SETTING_CA_CERT,          "CA Certificate",         "set_cacert",      "CACert",           "CA Certificate (PEM)",                       NULL, UI_FIELD_TEXTAREA},
    {SETTING_CLIENT_CERT,      "Client Certificate",     "set_clientcert",  "ClientCert",       "Client Certificate (PEM)",                   NULL, UI_FIELD_TEXTAREA},
    {SETTING_CLIENT_KEY,       "Client Private Key",     "set_clientkey",   "ClientKey",        "Client Private Key (PEM)",                   NULL, UI_FIELD_TEXTAREA},
    {SETTING_DEVICE_CERT,      "Device Certificate",     "set_devicecert",  "DeviceCert",       "Device X.509 Certificate (PEM)",             NULL, UI_FIELD_TEXTAREA},
};

#define SETTING_UI_COUNT (sizeof(SETTING_UI) / sizeof(SettingUIMetadata))

/**
 * @brief Find UI metadata by setting ID
 */
static inline const SettingUIMetadata* SettingUI_FindById(SettingID id)
{
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        if (SETTING_UI[i].id == id)
        {
            return &SETTING_UI[i];
        }
    }
    return NULL;
}

/**
 * @brief Find UI metadata by CLI command name
 */
static inline const SettingUIMetadata* SettingUI_FindByCliCommand(const char* cmd)
{
    if (cmd == NULL) return NULL;
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        if (strcmp(SETTING_UI[i].cliCommand, cmd) == 0)
        {
            return &SETTING_UI[i];
        }
    }
    return NULL;
}

/**
 * @brief Find UI metadata by web form field name
 */
static inline const SettingUIMetadata* SettingUI_FindByFormName(const char* name)
{
    if (name == NULL) return NULL;
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        if (strcmp(SETTING_UI[i].webFormName, name) == 0)
        {
            return &SETTING_UI[i];
        }
    }
    return NULL;
}

/**
 * @brief Check if a setting is a certificate/key (multi-line)
 */
static inline bool SettingUI_IsMultiLine(const SettingUIMetadata* meta)
{
    return meta != NULL && meta->fieldType == UI_FIELD_TEXTAREA;
}

#ifdef __cplusplus
}
#endif

#endif /* __SETTING_UI_H__ */
