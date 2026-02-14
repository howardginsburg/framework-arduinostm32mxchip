// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_WEB_H__
#define __SYSTEM_WEB_H__

#include "mbed.h"

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

/**
 * @brief Enable the web configuration UI
 * 
 * The web UI will dynamically show configuration fields based on
 * what settings are available in the active profile (set by DeviceConfig_Init).
 */
void EnableSystemWeb(void);

/**
 * @brief Start the web configuration server
 */
void StartupSystemWeb(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTEM_WEB_H__
