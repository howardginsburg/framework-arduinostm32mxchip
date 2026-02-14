// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file device_config_default.h
 * @brief Default connection profile configuration
 * 
 * This file is used when the user sketch does not provide a device_config.h.
 * It selects PROFILE_IOTHUB_SAS for backward compatibility with existing projects.
 */

#ifndef __DEVICE_CONFIG_DEFAULT_H__
#define __DEVICE_CONFIG_DEFAULT_H__

// Default to no EEPROM usage - sketch must provide its own configuration
// To use EEPROM-based configuration, create a device_config.h in your sketch
// with CONNECTION_PROFILE set to one of:
//   PROFILE_MQTT_USERPASS, PROFILE_MQTT_USERPASS_TLS, PROFILE_MQTT_MTLS,
//   PROFILE_IOTHUB_SAS, PROFILE_IOTHUB_CERT, PROFILE_DPS_SAS, PROFILE_DPS_CERT
#define CONNECTION_PROFILE PROFILE_NONE

#endif /* __DEVICE_CONFIG_DEFAULT_H__ */
