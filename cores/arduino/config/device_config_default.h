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

// Default to Azure IoT Hub with SAS authentication for backward compatibility
#define CONNECTION_PROFILE PROFILE_IOTHUB_SAS

#endif /* __DEVICE_CONFIG_DEFAULT_H__ */
