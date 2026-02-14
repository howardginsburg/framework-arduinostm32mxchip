// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file DeviceConfigRuntime.cpp
 * @brief Runtime configuration loading and parsing
 * 
 * This module handles loading configuration from EEPROM into runtime buffers
 * and provides parsed access to configuration values (e.g., extracting host/port
 * from a URL, extracting device ID from certificates or connection strings).
 */

#include "DeviceConfig.h"
#include "DeviceConfigZones.h"
#include "mbedtls/x509_crt.h"
#include <string.h>
#include <stdlib.h>

// =============================================================================
// Static Buffers for Loaded Configuration
// =============================================================================

static char s_wifiSsid[ZONE_3_SIZE];
static char s_wifiPassword[ZONE_10_SIZE];
static char s_brokerUrl[ZONE_5_SIZE];
static char s_brokerHost[ZONE_5_SIZE];
static int  s_brokerPort = 8883;
static char s_caCert[MAX_CA_CERT_SIZE];
static char s_clientCert[MAX_CLIENT_CERT_SIZE];
static char s_clientKey[MAX_CLIENT_KEY_SIZE];
static char s_connectionString[ZONE_5_SIZE];
static char s_deviceId[256];

// =============================================================================
// URL/String Parsing Functions
// =============================================================================

/**
 * @brief Parse broker URL into host and port
 * 
 * Supports formats:
 *   hostname
 *   hostname:port
 *   mqtts://hostname
 *   mqtts://hostname:port
 *   mqtt://hostname:port
 *   ssl://hostname:port
 */
static void parseBrokerUrl(const char* url)
{
    if (url == NULL || url[0] == '\0')
    {
        s_brokerHost[0] = '\0';
        s_brokerPort = 8883;
        return;
    }
    
    const char* hostStart = url;
    s_brokerPort = 8883;  // Default TLS port
    
    // Skip protocol prefix if present
    if (strncmp(url, "mqtts://", 8) == 0)
    {
        hostStart = url + 8;
    }
    else if (strncmp(url, "ssl://", 6) == 0)
    {
        hostStart = url + 6;
    }
    else if (strncmp(url, "mqtt://", 7) == 0)
    {
        hostStart = url + 7;
        s_brokerPort = 1883;  // Default non-TLS port
    }
    
    // Find port separator
    const char* portSep = strchr(hostStart, ':');
    
    if (portSep != NULL)
    {
        // Copy host portion
        int hostLen = portSep - hostStart;
        if (hostLen >= (int)sizeof(s_brokerHost))
        {
            hostLen = sizeof(s_brokerHost) - 1;
        }
        strncpy(s_brokerHost, hostStart, hostLen);
        s_brokerHost[hostLen] = '\0';
        
        // Parse port
        s_brokerPort = atoi(portSep + 1);
        if (s_brokerPort <= 0 || s_brokerPort > 65535)
        {
            s_brokerPort = 8883;
        }
    }
    else
    {
        // No port specified, use default
        strncpy(s_brokerHost, hostStart, sizeof(s_brokerHost) - 1);
        s_brokerHost[sizeof(s_brokerHost) - 1] = '\0';
    }
}

/**
 * @brief Extract CN (Common Name) from a PEM certificate
 * 
 * Uses mbedtls to parse the X.509 certificate and extract the CN from
 * the subject distinguished name.
 * 
 * @param cert PEM certificate string
 * @param cnBuffer Buffer to store extracted CN
 * @param bufferSize Size of cnBuffer
 */
static void extractCNFromCert(const char* cert, char* cnBuffer, int bufferSize)
{
    cnBuffer[0] = '\0';
    
    if (cert == NULL || cert[0] == '\0')
    {
        return;
    }
    
    mbedtls_x509_crt crt;
    mbedtls_x509_crt_init(&crt);
    
    // Parse the PEM certificate (length must include null terminator)
    int ret = mbedtls_x509_crt_parse(&crt, (const unsigned char*)cert, strlen(cert) + 1);
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&crt);
        return;
    }
    
    // Get the subject as a formatted string (e.g., "CN=Device1, O=Contoso, C=US")
    char subject[256];
    ret = mbedtls_x509_dn_gets(subject, sizeof(subject), &crt.subject);
    mbedtls_x509_crt_free(&crt);
    
    if (ret < 0)
    {
        return;
    }
    
    // Extract CN value from the formatted subject string
    const char* cnStart = strstr(subject, "CN=");
    if (cnStart == NULL)
    {
        return;
    }
    
    cnStart += 3;  // Skip "CN="
    int i = 0;
    while (cnStart[i] != '\0' && 
           cnStart[i] != ',' && 
           cnStart[i] != ' ' &&
           i < bufferSize - 1)
    {
        cnBuffer[i] = cnStart[i];
        i++;
    }
    cnBuffer[i] = '\0';
}

/**
 * @brief Extract DeviceId from an IoT Hub connection string
 * 
 * Connection string format:
 * HostName=<hub>.azure-devices.net;DeviceId=<device>;SharedAccessKey=<key>
 * 
 * @param connStr Connection string
 * @param deviceId Buffer to store extracted device ID
 * @param bufferSize Size of deviceId buffer
 */
static void extractDeviceIdFromConnectionString(const char* connStr, char* deviceId, int bufferSize)
{
    deviceId[0] = '\0';
    
    if (connStr == NULL || connStr[0] == '\0')
    {
        return;
    }
    
    // Look for "DeviceId="
    const char* start = strstr(connStr, "DeviceId=");
    if (start == NULL)
    {
        start = strstr(connStr, "deviceId=");
    }
    
    if (start != NULL)
    {
        start += 9;  // Skip "DeviceId="
        
        // Find end (terminated by ';' or end of string)
        int i = 0;
        while (start[i] != '\0' && start[i] != ';' && i < bufferSize - 1)
        {
            deviceId[i] = start[i];
            i++;
        }
        deviceId[i] = '\0';
    }
}

// =============================================================================
// Configuration Loading
// =============================================================================

bool DeviceConfig_LoadAll(void)
{
    ConnectionProfile activeProfile = DeviceConfig_GetActiveProfile();
    
    if (activeProfile == PROFILE_NONE)
    {
        return false;
    }
    
    bool success = true;
    
    // Load WiFi SSID
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_SSID))
    {
        if (DeviceConfig_Read(SETTING_WIFI_SSID, s_wifiSsid, sizeof(s_wifiSsid)) < 0)
        {
            s_wifiSsid[0] = '\0';
            success = false;
        }
    }
    
    // Load WiFi Password
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_PASSWORD))
    {
        if (DeviceConfig_Read(SETTING_WIFI_PASSWORD, s_wifiPassword, sizeof(s_wifiPassword)) < 0)
        {
            s_wifiPassword[0] = '\0';
            success = false;
        }
    }
    
    // Load Broker URL (for MQTT profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_BROKER_URL))
    {
        if (DeviceConfig_Read(SETTING_BROKER_URL, s_brokerUrl, sizeof(s_brokerUrl)) >= 0)
        {
            parseBrokerUrl(s_brokerUrl);
        }
        else
        {
            s_brokerUrl[0] = '\0';
            s_brokerHost[0] = '\0';
            success = false;
        }
    }
    
    // Load CA Certificate (for TLS profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_CA_CERT))
    {
        if (DeviceConfig_Read(SETTING_CA_CERT, s_caCert, sizeof(s_caCert)) < 0)
        {
            s_caCert[0] = '\0';
            success = false;
        }
    }
    
    // Load Client Certificate (for mTLS profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_CLIENT_CERT))
    {
        if (DeviceConfig_Read(SETTING_CLIENT_CERT, s_clientCert, sizeof(s_clientCert)) < 0)
        {
            s_clientCert[0] = '\0';
            success = false;
        }
    }
    
    // Load Client Private Key (for mTLS profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_CLIENT_KEY))
    {
        if (DeviceConfig_Read(SETTING_CLIENT_KEY, s_clientKey, sizeof(s_clientKey)) < 0)
        {
            s_clientKey[0] = '\0';
            success = false;
        }
    }
    
    // Load Connection String (for IoT Hub profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_CONNECTION_STRING))
    {
        if (DeviceConfig_Read(SETTING_CONNECTION_STRING, s_connectionString, sizeof(s_connectionString)) < 0)
        {
            s_connectionString[0] = '\0';
            success = false;
        }
    }
    
    // Load/derive Device ID based on profile type
    s_deviceId[0] = '\0';
    
    if (activeProfile == PROFILE_MQTT_MTLS || 
        activeProfile == PROFILE_IOTHUB_CERT ||
        activeProfile == PROFILE_DPS_CERT)
    {
        // Extract CN from client certificate
        if (s_clientCert[0] != '\0')
        {
            extractCNFromCert(s_clientCert, s_deviceId, sizeof(s_deviceId));
        }
    }
    else if (activeProfile == PROFILE_IOTHUB_SAS)
    {
        // Extract DeviceId from connection string
        if (s_connectionString[0] != '\0')
        {
            extractDeviceIdFromConnectionString(s_connectionString, s_deviceId, sizeof(s_deviceId));
        }
    }
    else if (DeviceConfig_IsSettingAvailable(SETTING_DEVICE_ID))
    {
        // Read directly from EEPROM for username/password profiles
        DeviceConfig_Read(SETTING_DEVICE_ID, s_deviceId, sizeof(s_deviceId));
    }
    
    return success;
}

// =============================================================================
// Getter Functions
// =============================================================================

const char* DeviceConfig_GetWifiSsid(void)
{
    return s_wifiSsid;
}

const char* DeviceConfig_GetWifiPassword(void)
{
    return s_wifiPassword;
}

const char* DeviceConfig_GetBrokerHost(void)
{
    return s_brokerHost;
}

int DeviceConfig_GetBrokerPort(void)
{
    return s_brokerPort;
}

const char* DeviceConfig_GetCACert(void)
{
    return s_caCert;
}

const char* DeviceConfig_GetClientCert(void)
{
    return s_clientCert;
}

const char* DeviceConfig_GetClientKey(void)
{
    return s_clientKey;
}

const char* DeviceConfig_GetDeviceId(void)
{
    return s_deviceId;
}
