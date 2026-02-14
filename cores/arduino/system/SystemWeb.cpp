// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "Arduino.h"
#include "app_httpd.h"

static bool (*_startup_web_server)(void) = NULL;

static bool startupWebServer(void)
{
    int ret = httpd_server_start();
    return (ret == 0);
}

void EnableSystemWeb(void)
{
    _startup_web_server = startupWebServer;
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