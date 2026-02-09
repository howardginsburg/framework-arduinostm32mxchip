// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_WEB_H__
#define __SYSTEM_WEB_H__

#include "mbed.h"
#include "DeviceConfig.h"

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

/**
 * @brief Enable the web configuration UI with a connection profile
 * 
 * The web UI will dynamically show configuration fields based on
 * what settings are available in the specified profile.
 * 
 * @param profile ConnectionProfile to configure for (from DeviceConfig.h)
 */
void EnableSystemWeb(ConnectionProfile profile);

/**
 * @brief Start the web configuration server
 */
void StartupSystemWeb(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTEM_WEB_H__
