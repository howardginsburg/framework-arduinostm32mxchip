// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __SETTING_VALIDATOR_H__
#define __SETTING_VALIDATOR_H__

#include "DeviceConfig.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validation result codes
 */
typedef enum {
    VALIDATE_OK = 0,
    VALIDATE_ERROR_NULL,
    VALIDATE_ERROR_EMPTY,
    VALIDATE_ERROR_TOO_LONG,
    VALIDATE_ERROR_INVALID_FORMAT,
    VALIDATE_ERROR_MISSING_REQUIRED,
    VALIDATE_ERROR_SETTING_UNAVAILABLE
} ValidationResult;

/**
 * @brief Get human-readable validation error message
 */
static inline const char* Validator_GetErrorMessage(ValidationResult result)
{
    switch (result)
    {
        case VALIDATE_OK:                     return "OK";
        case VALIDATE_ERROR_NULL:             return "Value is NULL";
        case VALIDATE_ERROR_EMPTY:            return "Value is empty";
        case VALIDATE_ERROR_TOO_LONG:         return "Value exceeds maximum length";
        case VALIDATE_ERROR_INVALID_FORMAT:   return "Invalid format";
        case VALIDATE_ERROR_MISSING_REQUIRED: return "Required field missing";
        case VALIDATE_ERROR_SETTING_UNAVAILABLE: return "Setting not available in this profile";
        default:                              return "Unknown error";
    }
}

/**
 * @brief Validate value length against setting's maximum
 * 
 * @param setting Setting ID
 * @param value Value to validate
 * @return ValidationResult
 */
static inline ValidationResult Validator_CheckLength(SettingID setting, const char* value)
{
    if (value == NULL)
    {
        return VALIDATE_ERROR_NULL;
    }
    
    if (!DeviceConfig_IsSettingAvailable(setting))
    {
        return VALIDATE_ERROR_SETTING_UNAVAILABLE;
    }
    
    int len = strlen(value);
    int maxLen = DeviceConfig_GetMaxLen(setting);
    
    if (len == 0)
    {
        return VALIDATE_ERROR_EMPTY;
    }
    
    if (len > maxLen)
    {
        return VALIDATE_ERROR_TOO_LONG;
    }
    
    return VALIDATE_OK;
}

/**
 * @brief Validate MQTT/broker URL format
 * 
 * Valid formats:
 *   hostname
 *   hostname:port
 *   mqtt://hostname
 *   mqtt://hostname:port
 *   mqtts://hostname
 *   mqtts://hostname:port
 *   ssl://hostname:port
 * 
 * @param url URL to validate
 * @return ValidationResult
 */
static inline ValidationResult Validator_BrokerUrl(const char* url)
{
    if (url == NULL)
    {
        return VALIDATE_ERROR_NULL;
    }
    
    int len = strlen(url);
    if (len == 0)
    {
        return VALIDATE_ERROR_EMPTY;
    }
    
    const char* hostStart = url;
    
    // Skip protocol prefix if present
    if (strncmp(url, "mqtts://", 8) == 0)
    {
        hostStart = url + 8;
    }
    else if (strncmp(url, "mqtt://", 7) == 0)
    {
        hostStart = url + 7;
    }
    else if (strncmp(url, "ssl://", 6) == 0)
    {
        hostStart = url + 6;
    }
    
    // Must have at least a hostname
    if (*hostStart == '\0' || *hostStart == ':')
    {
        return VALIDATE_ERROR_INVALID_FORMAT;
    }
    
    // If there's a port, validate it
    const char* portSep = strchr(hostStart, ':');
    if (portSep != NULL)
    {
        portSep++;
        if (*portSep == '\0')
        {
            return VALIDATE_ERROR_INVALID_FORMAT;  // Port separator but no port
        }
        
        // Check port is numeric
        while (*portSep != '\0')
        {
            if (!isdigit((unsigned char)*portSep))
            {
                return VALIDATE_ERROR_INVALID_FORMAT;
            }
            portSep++;
        }
    }
    
    return VALIDATE_OK;
}

/**
 * @brief Validate PEM certificate format
 * 
 * Checks for BEGIN/END markers. Does not validate crypto.
 * 
 * @param pem PEM string to validate
 * @return ValidationResult
 */
static inline ValidationResult Validator_PemCertificate(const char* pem)
{
    if (pem == NULL)
    {
        return VALIDATE_ERROR_NULL;
    }
    
    int len = strlen(pem);
    if (len == 0)
    {
        return VALIDATE_ERROR_EMPTY;
    }
    
    // Check for certificate markers
    if (strstr(pem, "-----BEGIN CERTIFICATE-----") == NULL)
    {
        return VALIDATE_ERROR_INVALID_FORMAT;
    }
    
    if (strstr(pem, "-----END CERTIFICATE-----") == NULL)
    {
        return VALIDATE_ERROR_INVALID_FORMAT;
    }
    
    return VALIDATE_OK;
}

/**
 * @brief Validate PEM private key format
 * 
 * Checks for BEGIN/END markers. Does not validate crypto.
 * 
 * @param pem PEM string to validate
 * @return ValidationResult
 */
static inline ValidationResult Validator_PemPrivateKey(const char* pem)
{
    if (pem == NULL)
    {
        return VALIDATE_ERROR_NULL;
    }
    
    int len = strlen(pem);
    if (len == 0)
    {
        return VALIDATE_ERROR_EMPTY;
    }
    
    // Check for various private key markers
    bool hasBegin = (strstr(pem, "-----BEGIN PRIVATE KEY-----") != NULL ||
                     strstr(pem, "-----BEGIN RSA PRIVATE KEY-----") != NULL ||
                     strstr(pem, "-----BEGIN EC PRIVATE KEY-----") != NULL);
    
    bool hasEnd = (strstr(pem, "-----END PRIVATE KEY-----") != NULL ||
                   strstr(pem, "-----END RSA PRIVATE KEY-----") != NULL ||
                   strstr(pem, "-----END EC PRIVATE KEY-----") != NULL);
    
    if (!hasBegin || !hasEnd)
    {
        return VALIDATE_ERROR_INVALID_FORMAT;
    }
    
    return VALIDATE_OK;
}

/**
 * @brief Validate Azure IoT Hub connection string format
 * 
 * Format: HostName=<hub>.azure-devices.net;DeviceId=<id>;SharedAccessKey=<key>
 * 
 * @param connStr Connection string to validate
 * @return ValidationResult
 */
static inline ValidationResult Validator_IotHubConnectionString(const char* connStr)
{
    if (connStr == NULL)
    {
        return VALIDATE_ERROR_NULL;
    }
    
    int len = strlen(connStr);
    if (len == 0)
    {
        return VALIDATE_ERROR_EMPTY;
    }
    
    // Check for required components
    if (strstr(connStr, "HostName=") == NULL)
    {
        return VALIDATE_ERROR_INVALID_FORMAT;
    }
    
    if (strstr(connStr, "DeviceId=") == NULL)
    {
        return VALIDATE_ERROR_INVALID_FORMAT;
    }
    
    // Need either SharedAccessKey or x509=true
    if (strstr(connStr, "SharedAccessKey=") == NULL &&
        strstr(connStr, "x509=true") == NULL)
    {
        return VALIDATE_ERROR_INVALID_FORMAT;
    }
    
    return VALIDATE_OK;
}

/**
 * @brief Validate DPS scope ID format
 * 
 * Format: 0neXXXXXXXX (starts with "0ne" followed by hex digits)
 * 
 * @param scopeId Scope ID to validate
 * @return ValidationResult
 */
static inline ValidationResult Validator_DpsScopeId(const char* scopeId)
{
    if (scopeId == NULL)
    {
        return VALIDATE_ERROR_NULL;
    }
    
    int len = strlen(scopeId);
    if (len == 0)
    {
        return VALIDATE_ERROR_EMPTY;
    }
    
    // Check for "0ne" prefix (Azure DPS scope IDs typically start with this)
    if (len >= 3 && strncmp(scopeId, "0ne", 3) == 0)
    {
        return VALIDATE_OK;
    }
    
    // Allow other formats too - just check it's not empty
    return VALIDATE_OK;
}

/**
 * @brief Validate a setting value based on its type
 * 
 * @param setting Setting ID
 * @param value Value to validate
 * @return ValidationResult
 */
static inline ValidationResult Validator_ValidateSetting(SettingID setting, const char* value)
{
    // First check basic length
    ValidationResult result = Validator_CheckLength(setting, value);
    if (result != VALIDATE_OK)
    {
        return result;
    }
    
    // Then apply type-specific validation
    switch (setting)
    {
        case SETTING_BROKER_URL:
            return Validator_BrokerUrl(value);
            
        case SETTING_CA_CERT:
        case SETTING_CLIENT_CERT:
        case SETTING_DEVICE_CERT:
            return Validator_PemCertificate(value);
            
        case SETTING_CLIENT_KEY:
            return Validator_PemPrivateKey(value);
            
        case SETTING_CONNECTION_STRING:
            return Validator_IotHubConnectionString(value);
            
        case SETTING_SCOPE_ID:
            return Validator_DpsScopeId(value);
            
        // These settings don't have special format requirements
        case SETTING_WIFI_SSID:
        case SETTING_WIFI_PASSWORD:
        case SETTING_DEVICE_ID:
        case SETTING_DEVICE_PASSWORD:
        case SETTING_DPS_ENDPOINT:
        case SETTING_REGISTRATION_ID:
        case SETTING_SYMMETRIC_KEY:
        default:
            return VALIDATE_OK;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* __SETTING_VALIDATOR_H__ */
