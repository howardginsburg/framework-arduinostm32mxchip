/**
 ******************************************************************************
 * @file    app_httpd.h
 * @author  QQ DING
 * @version V2.0.0
 * @date    8-February-2026
 * @brief   This header contains function prototypes for the web-based device
 *          configuration UI. Updated to use DeviceConfig profiles.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 */

#ifndef __APP_HTTPD_H__
#define __APP_HTTPD_H__

#include "WiFiAccessPoint.h"
#include "DeviceConfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Start the HTTP server
 * 
 * The web UI will dynamically show configuration fields based on
 * what settings are available in the active profile (set by DeviceConfig_Init).
 * 
 * @return 0 on success, error code on failure
 */
int httpd_server_start(void);

/**
 * @brief Stop the HTTP server
 * 
 * @return 0 on success, error code on failure
 */
int app_httpd_stop(void);

#ifdef __cplusplus
}
#endif

#endif // __APP_HTTPD_H__