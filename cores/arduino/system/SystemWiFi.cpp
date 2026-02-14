// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

 
#include "Arduino.h"
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"
#include "NTPClient.h"
#include "SystemWiFi.h"
#include "SystemTime.h"
#include "DeviceConfig.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// WiFi related functions
NetworkInterface *_defaultSystemNetwork = NULL;
static char ssid[WIFI_SSID_MAX_LEN + 1] = { 0 };

bool InitSystemWiFi(void)
{
    if (_defaultSystemNetwork == NULL)
    {
        _defaultSystemNetwork = (NetworkInterface*)new EMW10xxInterface();
    }
    
    return (_defaultSystemNetwork != NULL);
}

bool SystemWiFiConnect(void)
{
    const char* wifiSsid = DeviceConfig_GetWifiSsid();
    const char* wifiPwd = DeviceConfig_GetWifiPassword();
    
    if (wifiSsid == NULL || wifiSsid[0] == '\0')
    {
        Serial.print("INFO: the Wi-Fi SSID is empty, please set the value in configuration mode.\r\n");
        return false;
    }
    
    // Copy to local ssid for SystemWiFiSSID()
    strncpy(ssid, wifiSsid, WIFI_SSID_MAX_LEN);
    ssid[WIFI_SSID_MAX_LEN] = '\0';

    ((EMW10xxInterface*)_defaultSystemNetwork)->set_interface(Station);
    int ret = ((EMW10xxInterface*)_defaultSystemNetwork)->connect((char*)wifiSsid, wifiPwd, NSAPI_SECURITY_WPA_WPA2, 0);
    if(ret != 0)
    {
      	Serial.printf("ERROR: Failed to connect Wi-Fi %s.\r\n", ssid);
        return false;
    }
    else
    {
        Serial.printf("Wi-Fi %s connected.\r\n", ssid);
        
        // Sync system from NTP time server
        SyncTime();
        if (IsTimeSynced() == 0)
        {
            time_t t = time(NULL);
            Serial.printf("Now is (UTC): %s\r\n", ctime(&t));
        }
        else
        {
            Serial.println("Time sync failed");
        }

        return true;
    }
}

const char* SystemWiFiSSID(void)
{
    return ssid;
}

NetworkInterface* WiFiInterface(void)
{
    return _defaultSystemNetwork;
}

int SystemWiFiRSSI(void)
{
    return (int)((EMW10xxInterface*)_defaultSystemNetwork)->get_rssi();
}

int WiFiScan(WiFiAccessPoint *res, unsigned count)
{
    if (_defaultSystemNetwork != NULL)
    {
        return ((EMW10xxInterface*)_defaultSystemNetwork)->scan(res, count);
    }
    return 0;
}

bool SystemWiFiAPStart(const char *ssid, const char *passphrase)
{
    if (_defaultSystemNetwork != NULL)
    {
        ((EMW10xxInterface*)_defaultSystemNetwork)->set_interface(Soft_AP);
        int ret = ((EMW10xxInterface*)_defaultSystemNetwork)->connect( (char*)ssid, (char*)passphrase, NSAPI_SECURITY_WPA_WPA2, 0 );
        if(ret != 0)
        {
            Serial.printf("ERROR: Failed to start AP for Wi-Fi %s.\r\n", ssid);
            return false;
        }
        else
        {
            Serial.printf("AP mode Wi-Fi %s started .\r\n", ssid);
            return true;
        }
    }

    return false;
}

NetworkInterface* WiFiAPInterface(void)
{
    return _defaultSystemNetwork;
}