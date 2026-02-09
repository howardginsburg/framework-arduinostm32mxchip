// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "Arduino.h"
#include "app_httpd.h"
#include "DeviceConfig.h"

static bool (*_startup_web_server)(void) = NULL;
static ConnectionProfile s_webProfile = PROFILE_NONE;

static bool startupWebServer(void)
{
    int ret = httpd_server_start(s_webProfile);
    return (ret == 0);
}

void EnableSystemWeb(ConnectionProfile profile)
{
    _startup_web_server = startupWebServer;
    s_webProfile = profile;
}

void StartupSystemWeb(void)
{
    if (_startup_web_server == NULL)
    {
        return;
    }
    _startup_web_server();

    while (true)
    {
        wait_ms(1000);
    }
}