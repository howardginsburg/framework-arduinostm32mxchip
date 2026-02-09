/**
 ******************************************************************************
 * @file    app_httpd.cpp
 * @author  QQ DING (original), Microsoft (DeviceConfig integration)
 * @version V2.0.0
 * @date    8-February-2026
 * @brief   Web-based device configuration UI using DeviceConfig profiles.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2016 MXCHIP Inc.
 *  Copyright (c) Microsoft Corporation. All rights reserved.
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
#include "mbed.h"
#include "mico.h"
#include "app_httpd.h"
#include "DeviceConfig.h"
#include "EMW10xxInterface.h"
#include "httpd.h"
#include "OledDisplay.h"
#include "SystemVariables.h"
#include "SystemWeb.h"

#define HTTPD_HDR_DEFORT (HTTPD_HDR_ADD_SERVER|HTTPD_HDR_ADD_CONN_CLOSE|HTTPD_HDR_ADD_PRAGMA_NO_CACHE)

#define DEFAULT_PAGE_SIZE (12*1024)
#define MAX_FIELD_VALUE_SIZE 3000  // For certificates/keys

// Maximum lengths (these should match DeviceConfig zone sizes)
#define WIFI_SSID_MAX_LEN   120
#define WIFI_PWD_MAX_LEN    88

// ============================================================================
// HTML Page Templates
// ============================================================================

static const char* page_head = 
    "<!DOCTYPE html><html lang=\"en\"><head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">"
    "<title>IoT DevKit Configuration</title>"
    "<style>"
    "@charset \"UTF-8\";"
    "html{font-size:16px;}"
    "html,*{font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",\"Roboto\",\"Droid Sans\",\"Helvetica Neue\",Helvetica,Arial,sans-serif;line-height:1.5;-webkit-text-size-adjust:100%;}"
    "*{font-size:1rem;}"
    "body{margin:0;color:#212121;background:#f8f8f8;}"
    "section{display:block;}"
    "input{overflow:visible;}"
    "[type=\"radio\"]{position:absolute;left:-2rem;}"
    "h1,h2{line-height:1.2em;margin:0.75rem 0.5rem;font-weight:500;}"
    "h2 small{color:#424242;display:block;margin-top:-0.25rem;}"
    "h1{font-size:2rem;}"
    "h2{font-size:1.6875rem;}"
    "p{margin:0.5rem;}"
    "small{font-size:0.75em;}"
    "a{color:#0277bd;text-decoration:underline;opacity:1;transition:opacity 0.3s;}"
    "a:visited{color:#01579b;}"
    "a:hover,a:focus{opacity:0.75;}"
    ".container{margin:0 auto;padding:0 0.75rem;}"
    ".row{box-sizing:border-box;display:flex;flex:0 1 auto;flex-flow:row wrap;}"
    "[class^='col-sm-']{box-sizing:border-box;flex:0 0 auto;padding:0 0.25rem;}"
    ".col-sm-10{max-width:83.33333%;flex-basis:83.33333%;}"
    ".col-sm-offset-1{margin-left:8.33333%;}"
    "@media screen and (min-width:768px){.col-md-6{max-width:50%;flex-basis:50%;}.col-md-offset-3{margin-left:25%;}}"
    "header{display:block;height:2.75rem;background:#1e6bb8;color:#f5f5f5;padding:0.125rem 0.5rem;white-space:nowrap;overflow-x:auto;overflow-y:hidden;}"
    "header .logo{color:#f5f5f5;font-size:1.35rem;line-height:1.8125em;margin:0.0625rem 0.375rem 0.0625rem 0.0625rem;text-decoration:none;}"
    "form{background:#eeeeee;border:1px solid #c9c9c9;margin:0.5rem;padding:0.75rem 0.5rem 1.125rem;}"
    ".input-group{display:inline-block;margin-left:2rem;position:relative;}"
    ".input-group.fluid{display:flex;align-items:center;justify-content:center;}"
    ".input-group.fluid>input:not([type=\"radio\"]),.input-group.fluid>textarea,.input-group.fluid>select{width:100%;flex-grow:1;flex-basis:0;}"
    "@media screen and (max-width:767px){.input-group.fluid{align-items:stretch;flex-direction:column;}}"
    "[type=\"password\"],[type=\"text\"],select,textarea{width:100%;box-sizing:border-box;background:#fafafa;color:#212121;border:1px solid #c9c9c9;border-radius:2px;margin:0.25rem 0;padding:0.5rem 0.75rem;}"
    "input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):hover,input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):focus,select:hover,select:focus{border-color:#0288d1;box-shadow:none;}"
    "input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):disabled,select:disabled{cursor:not-allowed;opacity:0.75;}"
    "::placeholder{opacity:1;color:#616161;}"
    "button::-moz-focus-inner,[type=\"submit\"]::-moz-focus-inner{border-style:none;padding:0;}"
    "button,[type=\"submit\"]{-webkit-appearance:button;}"
    "button{overflow:visible;text-transform:none;}"
    "button,[type=\"submit\"],a.button,.button{display:inline-block;background:rgba(208,208,208,0.75);color:#212121;border:0;border-radius:2px;padding:0.5rem 0.75rem;margin:0.5rem;text-decoration:none;transition:background 0.3s;cursor:pointer;}"
    "button:hover,button:focus,[type=\"submit\"]:hover,[type=\"submit\"]:focus{background:#d0d0d0;opacity:1;}"
    "button:disabled,[type=\"submit\"]:disabled{cursor:not-allowed;opacity:0.75;}"
    "button.primary,[type=\"submit\"].primary,.button.primary{background:rgba(30,107,184,0.9);color:#fafafa;}"
    "button.primary:hover,button.primary:focus,[type=\"submit\"].primary:hover,[type=\"submit\"].primary:focus{background:#0277bd;}"
    "#content{margin-top:2em;}"
    "table,th,td{border:1px solid #c9c9c9;border-collapse:collapse;}"
    "th,td{padding:5px;padding-left:10px;}"
    "td{text-align:left;}"
    "th{background-color:#EEEEEE;color:#616161;}"
    "tr{background-color:#EEEEEE;color:#616161;}"
    "fieldset{border:1px solid #c9c9c9;margin:0.5rem 0;padding:0.5rem;}"
    "legend{padding:0 0.5rem;color:#1e6bb8;font-weight:500;}"
    ".profile-badge{background:#1e6bb8;color:#fff;padding:0.2rem 0.5rem;border-radius:3px;font-size:0.8rem;margin-left:0.5rem;}"
    "</style></head>";

static const char* page_body_start = 
    "<body><header><h1 class=\"logo\">IoT DevKit Configuration</h1></header>"
    "<section class=\"container\"><div id=\"content\" class=\"row\">"
    "<div class=\"col-sm-10 col-sm-offset-1 col-md-6 col-md-offset-3\" style=\"text-align:center;\">"
    "<form action=\"result\" method=\"post\" enctype=\"multipart/form-data\">";

static const char* wifi_fieldset_start = 
    "<fieldset><legend>Wi-Fi Settings</legend>"
    "<div class=\"input-group fluid\">"
    "<input type=\"radio\" name=\"input_ssid_method\" value=\"select\" onclick=\"changeSSIDInput()\" checked>"
    "<select name=\"SSID\" id=\"SSID-select\">";

static const char* wifi_fieldset_mid = 
    "</select></div>"
    "<div class=\"input-group fluid\">"
    "<input type=\"radio\" name=\"input_ssid_method\" value=\"text\" onclick=\"changeSSIDInput()\">"
    "<input type=\"text\" id=\"SSID-text\" placeholder=\"Enter SSID manually\" disabled>"
    "</div>"
    "<div class=\"input-group fluid\">"
    "<input type=\"password\" value=\"\" name=\"PASS\" id=\"password\" placeholder=\"Wi-Fi Password\">"
    "</div></fieldset>";

// MQTT Settings fieldset templates
static const char* mqtt_fieldset_start = "<fieldset><legend>MQTT Broker Settings</legend>";
static const char* mqtt_broker_url = "<div class=\"input-group fluid\"><input type=\"text\" name=\"BrokerURL\" placeholder=\"Broker URL (e.g., mqtts://broker.example.com:8883)\"></div>";
static const char* mqtt_device_id = "<div class=\"input-group fluid\"><input type=\"text\" name=\"DeviceID\" placeholder=\"Device/Client ID\"></div>";
static const char* mqtt_password = "<div class=\"input-group fluid\"><input type=\"password\" name=\"DevicePassword\" placeholder=\"Password\"></div>";
static const char* mqtt_ca_cert = "<div class=\"input-group fluid\"><textarea name=\"CACert\" rows=\"4\" placeholder=\"Server CA Certificate (PEM format)\"></textarea></div>";
static const char* mqtt_client_cert = "<div class=\"input-group fluid\"><textarea name=\"ClientCert\" rows=\"4\" placeholder=\"Client Certificate (PEM format)\"></textarea></div>";
static const char* mqtt_client_key = "<div class=\"input-group fluid\"><textarea name=\"ClientKey\" rows=\"4\" placeholder=\"Client Private Key (PEM format)\"></textarea></div>";
static const char* fieldset_end = "</fieldset>";

// Azure IoT Hub Settings templates
static const char* iothub_fieldset_start = "<fieldset><legend>Azure IoT Hub Settings</legend>";
static const char* iothub_conn_string = "<div class=\"input-group fluid\"><input type=\"text\" name=\"ConnectionString\" placeholder=\"IoT Hub Device Connection String\"></div>";
static const char* iothub_device_cert = "<div class=\"input-group fluid\"><textarea name=\"DeviceCert\" rows=\"4\" placeholder=\"Device X.509 Certificate (PEM format)\"></textarea></div>";

// Azure DPS Settings templates  
static const char* dps_fieldset_start = "<fieldset><legend>Azure DPS Settings</legend>";
static const char* dps_endpoint = "<div class=\"input-group fluid\"><input type=\"text\" name=\"DPSEndpoint\" placeholder=\"DPS Endpoint\" value=\"global.azure-devices-provisioning.net\"></div>";
static const char* dps_scope_id = "<div class=\"input-group fluid\"><input type=\"text\" name=\"ScopeId\" placeholder=\"DPS ID Scope\"></div>";
static const char* dps_registration_id = "<div class=\"input-group fluid\"><input type=\"text\" name=\"RegistrationId\" placeholder=\"Registration ID\"></div>";
static const char* dps_symmetric_key = "<div class=\"input-group fluid\"><input type=\"text\" name=\"SymmetricKey\" placeholder=\"Symmetric Key\"></div>";

static const char* page_body_end = 
    "<div class=\"input-group fluid\"><button type=\"submit\" class=\"primary\">Save Configuration</button></div>"
    "</form>"
    "<h5 style=\"color:#616161;\">Refresh this page to update the Wi-Fi network list</h5>"
    "</div></div></section>"
    "<script>"
    "function changeSSIDInput(){"
    "var inputFromSelect=document.getElementsByName('input_ssid_method')[0].checked;"
    "var select=document.getElementById('SSID-select');"
    "var text=document.getElementById('SSID-text');"
    "if(inputFromSelect){"
    "select.name='SSID';select.removeAttribute('disabled');"
    "text.name='';text.setAttribute('disabled','');"
    "}else{"
    "select.name='';select.setAttribute('disabled','');"
    "text.name='SSID';text.removeAttribute('disabled');"
    "}}"
    "</script></body></html>";

// Result page templates
static const char* result_body_start = 
    "<body><header><h1 class=\"logo\">IoT DevKit Configuration</h1></header>"
    "<section class=\"container\"><div id=\"content\" class=\"row\">"
    "<div class=\"col-sm-10 col-sm-offset-1 col-md-6 col-md-offset-3\" style=\"text-align:center;\">";
static const char* result_table_start = "<table align=\"center\" style=\"width:90%\"><tr><th>Setting</th><th>Status</th></tr>";
static const char* result_table_end = "</table>";
static const char* result_row = "<tr><td>%s</td><td>%s</td></tr>";
static const char* result_success = "<h5 style=\"color:DodgerBlue;\">Configuration saved! The device will reboot...</h5>";
static const char* result_failed = "<h5 style=\"color:Tomato;\">Configuration failed (error code: %d)</h5>";
static const char* result_body_end = "<button onclick=\"window.location.href='/'\">Back</button></div></div></section></body></html>";

// ============================================================================
// Global State
// ============================================================================

extern OLEDDisplay Screen;
extern NetworkInterface *_defaultSystemNetwork;

static ConnectionProfile s_activeProfile = PROFILE_NONE;
static bool s_isHttpInit = false;
static bool s_isHandlersRegistered = false;

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Save a configuration value using DeviceConfig
 */
static bool saveConfigValue(SettingID setting, const char* value)
{
    if (value == NULL || value[0] == '\0')
    {
        return true;  // Empty value = no change, consider success
    }
    
    if (!DeviceConfig_IsSettingAvailable(setting))
    {
        return false;
    }
    
    return DeviceConfig_Save(setting, value) == 0;
}

/**
 * @brief Connect to WiFi with given credentials
 */
static bool connectWiFi(const char* ssid, const char* password)
{
    Screen.clean();
    Screen.print("WiFi \r\n \r\nConnecting...\r\n \r\n");

    if (_defaultSystemNetwork == NULL)
    {
        _defaultSystemNetwork = new EMW10xxInterface();
    }

    ((EMW10xxInterface*)_defaultSystemNetwork)->set_interface(Station);
    int err = ((EMW10xxInterface*)_defaultSystemNetwork)->connect(
        (char*)ssid, (char*)password, NSAPI_SECURITY_WPA_WPA2, 0);
    
    if (err != 0)
    {
        Screen.print("WiFi \r\n \r\nNo connection \r\n \r\n");
        return false;
    }
    else
    {
        char wifiBuff[128];
        snprintf(wifiBuff, sizeof(wifiBuff), "WiFi \r\n %s\r\n %s \r\n \r\n", 
                 ssid, _defaultSystemNetwork->get_ip_address());
        Screen.print(wifiBuff);
    }
    return true;
}

/**
 * @brief Append profile-specific form fields to the page buffer
 */
static int appendProfileFields(char* buffer, int bufferSize, int currentLen)
{
    int len = currentLen;
    int ret;
    
    switch (s_activeProfile)
    {
        case PROFILE_MQTT_USERPASS:
            // MQTT with username/password (no TLS)
            strcpy(&buffer[len], mqtt_fieldset_start);
            len += strlen(mqtt_fieldset_start);
            strcpy(&buffer[len], mqtt_broker_url);
            len += strlen(mqtt_broker_url);
            strcpy(&buffer[len], mqtt_device_id);
            len += strlen(mqtt_device_id);
            strcpy(&buffer[len], mqtt_password);
            len += strlen(mqtt_password);
            strcpy(&buffer[len], fieldset_end);
            len += strlen(fieldset_end);
            break;
            
        case PROFILE_MQTT_USERPASS_TLS:
            // MQTT with username/password over TLS
            strcpy(&buffer[len], mqtt_fieldset_start);
            len += strlen(mqtt_fieldset_start);
            strcpy(&buffer[len], mqtt_broker_url);
            len += strlen(mqtt_broker_url);
            strcpy(&buffer[len], mqtt_device_id);
            len += strlen(mqtt_device_id);
            strcpy(&buffer[len], mqtt_password);
            len += strlen(mqtt_password);
            strcpy(&buffer[len], mqtt_ca_cert);
            len += strlen(mqtt_ca_cert);
            strcpy(&buffer[len], fieldset_end);
            len += strlen(fieldset_end);
            break;
            
        case PROFILE_MQTT_MTLS:
            // MQTT with mutual TLS
            strcpy(&buffer[len], mqtt_fieldset_start);
            len += strlen(mqtt_fieldset_start);
            strcpy(&buffer[len], mqtt_broker_url);
            len += strlen(mqtt_broker_url);
            strcpy(&buffer[len], mqtt_ca_cert);
            len += strlen(mqtt_ca_cert);
            strcpy(&buffer[len], mqtt_client_cert);
            len += strlen(mqtt_client_cert);
            strcpy(&buffer[len], mqtt_client_key);
            len += strlen(mqtt_client_key);
            strcpy(&buffer[len], fieldset_end);
            len += strlen(fieldset_end);
            break;
            
        case PROFILE_IOTHUB_SAS:
            // Azure IoT Hub with SAS
            strcpy(&buffer[len], iothub_fieldset_start);
            len += strlen(iothub_fieldset_start);
            strcpy(&buffer[len], iothub_conn_string);
            len += strlen(iothub_conn_string);
            strcpy(&buffer[len], fieldset_end);
            len += strlen(fieldset_end);
            break;
            
        case PROFILE_IOTHUB_CERT:
            // Azure IoT Hub with X.509
            strcpy(&buffer[len], iothub_fieldset_start);
            len += strlen(iothub_fieldset_start);
            strcpy(&buffer[len], iothub_conn_string);
            len += strlen(iothub_conn_string);
            strcpy(&buffer[len], iothub_device_cert);
            len += strlen(iothub_device_cert);
            strcpy(&buffer[len], fieldset_end);
            len += strlen(fieldset_end);
            break;
            
        case PROFILE_DPS_SAS:
            // Azure DPS with symmetric key
            strcpy(&buffer[len], dps_fieldset_start);
            len += strlen(dps_fieldset_start);
            strcpy(&buffer[len], dps_endpoint);
            len += strlen(dps_endpoint);
            strcpy(&buffer[len], dps_scope_id);
            len += strlen(dps_scope_id);
            strcpy(&buffer[len], dps_registration_id);
            len += strlen(dps_registration_id);
            strcpy(&buffer[len], dps_symmetric_key);
            len += strlen(dps_symmetric_key);
            strcpy(&buffer[len], fieldset_end);
            len += strlen(fieldset_end);
            break;
            
        case PROFILE_DPS_CERT:
            // Azure DPS with X.509
            strcpy(&buffer[len], dps_fieldset_start);
            len += strlen(dps_fieldset_start);
            strcpy(&buffer[len], dps_endpoint);
            len += strlen(dps_endpoint);
            strcpy(&buffer[len], dps_scope_id);
            len += strlen(dps_scope_id);
            strcpy(&buffer[len], dps_registration_id);
            len += strlen(dps_registration_id);
            strcpy(&buffer[len], iothub_device_cert);  // Reuse device cert field
            len += strlen(iothub_device_cert);
            strcpy(&buffer[len], fieldset_end);
            len += strlen(fieldset_end);
            break;
            
        case PROFILE_NONE:
        default:
            // No additional fields for PROFILE_NONE
            // Add a note explaining the profile
            ret = snprintf(&buffer[len], bufferSize - len,
                "<p style=\"color:#616161;\">Profile: %s<br>Only Wi-Fi settings are available.</p>",
                DeviceConfig_GetProfileName());
            len += (ret > 0 ? ret : 0);
            break;
    }
    
    return len;
}

// ============================================================================
// HTTP Request Handlers
// ============================================================================

/**
 * @brief Generate and send the configuration page
 */
static int webSettingsPage(httpd_request_t *req)
{
    char *settingPage = NULL;
    int len = 0;
    int err = kNoErr;
    const char *ssid = "";
    int ssidLen = 0;

    // Scan for WiFi networks
    WiFiAccessPoint wifiScanResult[50];
    int validWifiIndex[15];
    int wifiCount = ((EMW10xxInterface*)_defaultSystemNetwork)->scan(wifiScanResult, 50);
    int validWifiCount = 0;
    
    for (int i = 0; i < wifiCount; ++i)
    {
        // Skip weak signals
        if (wifiScanResult[i].get_rssi() < -100)
        {
            continue;
        }

        bool shouldSkip = false;
        for (int j = 0; j < i; ++j)
        {
            if (wifiScanResult[j].get_rssi() < -100)
            {
                continue;
            }
            else if (strcmp(wifiScanResult[i].get_ssid(), wifiScanResult[j].get_ssid()) == 0)
            {
                shouldSkip = true;
                break;
            }
        }

        if (shouldSkip)
        {
            continue;
        }

        ssid = (char*)wifiScanResult[i].get_ssid();
        ssidLen = strlen(ssid);
        
        // Skip our own AP
        if (ssidLen == BOARD_AP_LENGTH && strncmp(ssid, boardAPHeader, strlen(boardAPHeader)) == 0)
        {
            shouldSkip = true;
            for (int j = strlen(boardAPHeader); j < BOARD_AP_LENGTH; ++j)
            {
                if (!isxdigit(ssid[j]))
                {
                    shouldSkip = false;
                }
            }
            if (shouldSkip)
            {
                continue;
            }
        }

        if (ssidLen > 0 && ssidLen <= WIFI_SSID_MAX_LEN)
        {
            validWifiIndex[validWifiCount++] = i;
        }

        if (validWifiCount >= 15)
        {
            break;
        }
    }

    // Allocate page buffer
    settingPage = (char*)calloc(DEFAULT_PAGE_SIZE, 1);
    if (settingPage == NULL)
    {
        err = kGeneralErr;
        goto exit;
    }

    // Build the page
    strcpy(settingPage, page_head);
    len = strlen(page_head);
    
    // Add profile badge to body start
    {
        int ret = snprintf(&settingPage[len], DEFAULT_PAGE_SIZE - len,
            "<body><header><h1 class=\"logo\">IoT DevKit Configuration"
            "<span class=\"profile-badge\">%s</span></h1></header>"
            "<section class=\"container\"><div id=\"content\" class=\"row\">"
            "<div class=\"col-sm-10 col-sm-offset-1 col-md-6 col-md-offset-3\" style=\"text-align:center;\">"
            "<form action=\"result\" method=\"post\" enctype=\"multipart/form-data\">",
            DeviceConfig_GetProfileName());
        len += (ret > 0 ? ret : 0);
    }
    
    // WiFi fieldset (always shown if available)
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_SSID))
    {
        strcpy(&settingPage[len], wifi_fieldset_start);
        len += strlen(wifi_fieldset_start);
        
        // Add WiFi options
        for (int i = 0; i < validWifiCount; ++i)
        {
            ssid = (char*)wifiScanResult[validWifiIndex[i]].get_ssid();
            ssidLen = strlen(ssid);
            if (ssidLen > 0 && ssidLen <= WIFI_SSID_MAX_LEN)
            {
                int ret = snprintf(&settingPage[len], DEFAULT_PAGE_SIZE - len,
                    "<option value=\"%s\">%s</option>", ssid, ssid);
                len += (ret > 0 ? ret : 0);
            }
        }
        
        strcpy(&settingPage[len], wifi_fieldset_mid);
        len += strlen(wifi_fieldset_mid);
    }
    
    // Add profile-specific fields
    len = appendProfileFields(settingPage, DEFAULT_PAGE_SIZE, len);
    
    // Page end
    strcpy(&settingPage[len], page_body_end);
    len += strlen(page_body_end) + 1;
    
    err = httpd_send_all_header(req, HTTP_RES_200, len, HTTP_CONTENT_HTML_STR);
    require_noerr(err, exit);

    err = httpd_send_body(req->sock, (const unsigned char*)settingPage, len);
    require_noerr(err, exit);

exit:
    if (settingPage)
    {
        free(settingPage);
    }
    return err;
}

/**
 * @brief Structure to hold parsed form values
 */
typedef struct {
    char ssid[WIFI_SSID_MAX_LEN + 1];
    char password[WIFI_PWD_MAX_LEN + 1];
    char* brokerUrl;
    char* deviceId;
    char* devicePassword;
    char* caCert;
    char* clientCert;
    char* clientKey;
    char* connectionString;
    char* dpsEndpoint;
    char* scopeId;
    char* registrationId;
    char* symmetricKey;
    char* deviceCert;
} FormValues;

/**
 * @brief Free dynamically allocated form values
 */
static void freeFormValues(FormValues* values)
{
    if (values->brokerUrl) free(values->brokerUrl);
    if (values->deviceId) free(values->deviceId);
    if (values->devicePassword) free(values->devicePassword);
    if (values->caCert) free(values->caCert);
    if (values->clientCert) free(values->clientCert);
    if (values->clientKey) free(values->clientKey);
    if (values->connectionString) free(values->connectionString);
    if (values->dpsEndpoint) free(values->dpsEndpoint);
    if (values->scopeId) free(values->scopeId);
    if (values->registrationId) free(values->registrationId);
    if (values->symmetricKey) free(values->symmetricKey);
    if (values->deviceCert) free(values->deviceCert);
}

/**
 * @brief Allocate buffer for a setting if it's available in the profile
 */
static char* allocateIfAvailable(SettingID setting)
{
    if (DeviceConfig_IsSettingAvailable(setting))
    {
        int maxLen = DeviceConfig_GetMaxLen(setting);
        if (maxLen > 0)
        {
            return (char*)calloc(maxLen + 1, 1);
        }
    }
    return NULL;
}

/**
 * @brief Parse form data based on active profile
 */
static int parseFormData(httpd_request_t *req, char *buf, FormValues *values)
{
    OSStatus err = kNoErr;
    char *boundary = NULL;
    bool isMultipart = strstr(req->content_type, "multipart/form-data") != NULL;
    
    if (isMultipart)
    {
        boundary = strstr(req->content_type, "boundary=");
        boundary += 9;
    }
    
    // Parse WiFi settings (always)
    if (isMultipart)
    {
        err = httpd_get_tag_from_multipart_form(buf, boundary, "SSID", values->ssid, WIFI_SSID_MAX_LEN);
        httpd_get_tag_from_multipart_form(buf, boundary, "PASS", values->password, WIFI_PWD_MAX_LEN);
    }
    else
    {
        err = httpd_get_tag_from_post_data(buf, "SSID", values->ssid, WIFI_SSID_MAX_LEN);
        httpd_get_tag_from_post_data(buf, "PASS", values->password, WIFI_PWD_MAX_LEN);
    }
    
    if (values->ssid[0] == 0)
    {
        err = kParamErr;
    }
    require_noerr(err, exit);
    
    // Parse profile-specific fields
    switch (s_activeProfile)
    {
        case PROFILE_MQTT_USERPASS:
        case PROFILE_MQTT_USERPASS_TLS:
            values->brokerUrl = allocateIfAvailable(SETTING_BROKER_URL);
            values->deviceId = allocateIfAvailable(SETTING_DEVICE_ID);
            values->devicePassword = allocateIfAvailable(SETTING_DEVICE_PASSWORD);
            
            if (values->brokerUrl)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "BrokerURL", values->brokerUrl, DeviceConfig_GetMaxLen(SETTING_BROKER_URL));
                else
                    httpd_get_tag_from_post_data(buf, "BrokerURL", values->brokerUrl, DeviceConfig_GetMaxLen(SETTING_BROKER_URL));
            }
            if (values->deviceId)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "DeviceID", values->deviceId, DeviceConfig_GetMaxLen(SETTING_DEVICE_ID));
                else
                    httpd_get_tag_from_post_data(buf, "DeviceID", values->deviceId, DeviceConfig_GetMaxLen(SETTING_DEVICE_ID));
            }
            if (values->devicePassword)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "DevicePassword", values->devicePassword, DeviceConfig_GetMaxLen(SETTING_DEVICE_PASSWORD));
                else
                    httpd_get_tag_from_post_data(buf, "DevicePassword", values->devicePassword, DeviceConfig_GetMaxLen(SETTING_DEVICE_PASSWORD));
            }
            
            // CA cert for TLS variant
            if (s_activeProfile == PROFILE_MQTT_USERPASS_TLS)
            {
                values->caCert = allocateIfAvailable(SETTING_CA_CERT);
                if (values->caCert)
                {
                    if (isMultipart)
                        httpd_get_tag_from_multipart_form(buf, boundary, "CACert", values->caCert, DeviceConfig_GetMaxLen(SETTING_CA_CERT));
                    else
                        httpd_get_tag_from_post_data(buf, "CACert", values->caCert, DeviceConfig_GetMaxLen(SETTING_CA_CERT));
                }
            }
            break;
            
        case PROFILE_MQTT_MTLS:
            values->brokerUrl = allocateIfAvailable(SETTING_BROKER_URL);
            values->caCert = allocateIfAvailable(SETTING_CA_CERT);
            values->clientCert = allocateIfAvailable(SETTING_CLIENT_CERT);
            values->clientKey = allocateIfAvailable(SETTING_CLIENT_KEY);
            
            if (values->brokerUrl)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "BrokerURL", values->brokerUrl, DeviceConfig_GetMaxLen(SETTING_BROKER_URL));
                else
                    httpd_get_tag_from_post_data(buf, "BrokerURL", values->brokerUrl, DeviceConfig_GetMaxLen(SETTING_BROKER_URL));
            }
            if (values->caCert)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "CACert", values->caCert, DeviceConfig_GetMaxLen(SETTING_CA_CERT));
                else
                    httpd_get_tag_from_post_data(buf, "CACert", values->caCert, DeviceConfig_GetMaxLen(SETTING_CA_CERT));
            }
            if (values->clientCert)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "ClientCert", values->clientCert, DeviceConfig_GetMaxLen(SETTING_CLIENT_CERT));
                else
                    httpd_get_tag_from_post_data(buf, "ClientCert", values->clientCert, DeviceConfig_GetMaxLen(SETTING_CLIENT_CERT));
            }
            if (values->clientKey)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "ClientKey", values->clientKey, DeviceConfig_GetMaxLen(SETTING_CLIENT_KEY));
                else
                    httpd_get_tag_from_post_data(buf, "ClientKey", values->clientKey, DeviceConfig_GetMaxLen(SETTING_CLIENT_KEY));
            }
            break;
            
        case PROFILE_IOTHUB_SAS:
            values->connectionString = allocateIfAvailable(SETTING_CONNECTION_STRING);
            if (values->connectionString)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "ConnectionString", values->connectionString, DeviceConfig_GetMaxLen(SETTING_CONNECTION_STRING));
                else
                    httpd_get_tag_from_post_data(buf, "ConnectionString", values->connectionString, DeviceConfig_GetMaxLen(SETTING_CONNECTION_STRING));
            }
            break;
            
        case PROFILE_IOTHUB_CERT:
            values->connectionString = allocateIfAvailable(SETTING_CONNECTION_STRING);
            values->deviceCert = allocateIfAvailable(SETTING_DEVICE_CERT);
            
            if (values->connectionString)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "ConnectionString", values->connectionString, DeviceConfig_GetMaxLen(SETTING_CONNECTION_STRING));
                else
                    httpd_get_tag_from_post_data(buf, "ConnectionString", values->connectionString, DeviceConfig_GetMaxLen(SETTING_CONNECTION_STRING));
            }
            if (values->deviceCert)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "DeviceCert", values->deviceCert, DeviceConfig_GetMaxLen(SETTING_DEVICE_CERT));
                else
                    httpd_get_tag_from_post_data(buf, "DeviceCert", values->deviceCert, DeviceConfig_GetMaxLen(SETTING_DEVICE_CERT));
            }
            break;
            
        case PROFILE_DPS_SAS:
            values->dpsEndpoint = allocateIfAvailable(SETTING_DPS_ENDPOINT);
            values->scopeId = allocateIfAvailable(SETTING_SCOPE_ID);
            values->registrationId = allocateIfAvailable(SETTING_REGISTRATION_ID);
            values->symmetricKey = allocateIfAvailable(SETTING_SYMMETRIC_KEY);
            
            if (values->dpsEndpoint)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "DPSEndpoint", values->dpsEndpoint, DeviceConfig_GetMaxLen(SETTING_DPS_ENDPOINT));
                else
                    httpd_get_tag_from_post_data(buf, "DPSEndpoint", values->dpsEndpoint, DeviceConfig_GetMaxLen(SETTING_DPS_ENDPOINT));
            }
            if (values->scopeId)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "ScopeId", values->scopeId, DeviceConfig_GetMaxLen(SETTING_SCOPE_ID));
                else
                    httpd_get_tag_from_post_data(buf, "ScopeId", values->scopeId, DeviceConfig_GetMaxLen(SETTING_SCOPE_ID));
            }
            if (values->registrationId)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "RegistrationId", values->registrationId, DeviceConfig_GetMaxLen(SETTING_REGISTRATION_ID));
                else
                    httpd_get_tag_from_post_data(buf, "RegistrationId", values->registrationId, DeviceConfig_GetMaxLen(SETTING_REGISTRATION_ID));
            }
            if (values->symmetricKey)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "SymmetricKey", values->symmetricKey, DeviceConfig_GetMaxLen(SETTING_SYMMETRIC_KEY));
                else
                    httpd_get_tag_from_post_data(buf, "SymmetricKey", values->symmetricKey, DeviceConfig_GetMaxLen(SETTING_SYMMETRIC_KEY));
            }
            break;
            
        case PROFILE_DPS_CERT:
            values->dpsEndpoint = allocateIfAvailable(SETTING_DPS_ENDPOINT);
            values->scopeId = allocateIfAvailable(SETTING_SCOPE_ID);
            values->registrationId = allocateIfAvailable(SETTING_REGISTRATION_ID);
            values->deviceCert = allocateIfAvailable(SETTING_DEVICE_CERT);
            
            if (values->dpsEndpoint)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "DPSEndpoint", values->dpsEndpoint, DeviceConfig_GetMaxLen(SETTING_DPS_ENDPOINT));
                else
                    httpd_get_tag_from_post_data(buf, "DPSEndpoint", values->dpsEndpoint, DeviceConfig_GetMaxLen(SETTING_DPS_ENDPOINT));
            }
            if (values->scopeId)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "ScopeId", values->scopeId, DeviceConfig_GetMaxLen(SETTING_SCOPE_ID));
                else
                    httpd_get_tag_from_post_data(buf, "ScopeId", values->scopeId, DeviceConfig_GetMaxLen(SETTING_SCOPE_ID));
            }
            if (values->registrationId)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "RegistrationId", values->registrationId, DeviceConfig_GetMaxLen(SETTING_REGISTRATION_ID));
                else
                    httpd_get_tag_from_post_data(buf, "RegistrationId", values->registrationId, DeviceConfig_GetMaxLen(SETTING_REGISTRATION_ID));
            }
            if (values->deviceCert)
            {
                if (isMultipart)
                    httpd_get_tag_from_multipart_form(buf, boundary, "DeviceCert", values->deviceCert, DeviceConfig_GetMaxLen(SETTING_DEVICE_CERT));
                else
                    httpd_get_tag_from_post_data(buf, "DeviceCert", values->deviceCert, DeviceConfig_GetMaxLen(SETTING_DEVICE_CERT));
            }
            break;
            
        case PROFILE_NONE:
        default:
            // Only WiFi settings for PROFILE_NONE
            break;
    }
    
exit:
    return err;
}

/**
 * @brief Save all form values to storage
 */
static bool saveFormValues(FormValues *values, char *resultBuffer, int bufferSize, int *len)
{
    bool success = true;
    int ret;
    
    // Save WiFi settings
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_SSID) && values->ssid[0] != '\0')
    {
        bool wifiSaved = saveConfigValue(SETTING_WIFI_SSID, values->ssid) &&
                         saveConfigValue(SETTING_WIFI_PASSWORD, values->password);
        ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                      "Wi-Fi", wifiSaved ? "Saved" : "Failed");
        *len += (ret > 0 ? ret : 0);
        if (!wifiSaved) success = false;
    }
    
    // Save profile-specific settings
    switch (s_activeProfile)
    {
        case PROFILE_MQTT_USERPASS:
        case PROFILE_MQTT_USERPASS_TLS:
            if (values->brokerUrl && values->brokerUrl[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_BROKER_URL, values->brokerUrl);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Broker URL", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->deviceId && values->deviceId[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_DEVICE_ID, values->deviceId);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Device ID", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->devicePassword && values->devicePassword[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_DEVICE_PASSWORD, values->devicePassword);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Password", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (s_activeProfile == PROFILE_MQTT_USERPASS_TLS && values->caCert && values->caCert[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_CA_CERT, values->caCert);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "CA Certificate", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            break;
            
        case PROFILE_MQTT_MTLS:
            if (values->brokerUrl && values->brokerUrl[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_BROKER_URL, values->brokerUrl);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Broker URL", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->caCert && values->caCert[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_CA_CERT, values->caCert);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "CA Certificate", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->clientCert && values->clientCert[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_CLIENT_CERT, values->clientCert);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Client Certificate", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->clientKey && values->clientKey[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_CLIENT_KEY, values->clientKey);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Client Key", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            break;
            
        case PROFILE_IOTHUB_SAS:
            if (values->connectionString && values->connectionString[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_CONNECTION_STRING, values->connectionString);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Connection String", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            break;
            
        case PROFILE_IOTHUB_CERT:
            if (values->connectionString && values->connectionString[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_CONNECTION_STRING, values->connectionString);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Connection String", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->deviceCert && values->deviceCert[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_DEVICE_CERT, values->deviceCert);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Device Certificate", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            break;
            
        case PROFILE_DPS_SAS:
            if (values->dpsEndpoint && values->dpsEndpoint[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_DPS_ENDPOINT, values->dpsEndpoint);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "DPS Endpoint", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->scopeId && values->scopeId[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_SCOPE_ID, values->scopeId);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Scope ID", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->registrationId && values->registrationId[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_REGISTRATION_ID, values->registrationId);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Registration ID", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->symmetricKey && values->symmetricKey[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_SYMMETRIC_KEY, values->symmetricKey);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Symmetric Key", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            break;
            
        case PROFILE_DPS_CERT:
            if (values->dpsEndpoint && values->dpsEndpoint[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_DPS_ENDPOINT, values->dpsEndpoint);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "DPS Endpoint", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->scopeId && values->scopeId[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_SCOPE_ID, values->scopeId);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Scope ID", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->registrationId && values->registrationId[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_REGISTRATION_ID, values->registrationId);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Registration ID", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            if (values->deviceCert && values->deviceCert[0] != '\0')
            {
                bool saved = saveConfigValue(SETTING_DEVICE_CERT, values->deviceCert);
                ret = snprintf(&resultBuffer[*len], bufferSize - *len, result_row,
                              "Device Certificate", saved ? "Saved" : "Failed");
                *len += (ret > 0 ? ret : 0);
                if (!saved) success = false;
            }
            break;
            
        case PROFILE_NONE:
        default:
            break;
    }
    
    return success;
}

/**
 * @brief Handle form submission and show result page
 */
static int webSettingsResultPage(httpd_request_t *req)
{
    OSStatus err = kNoErr;
    int bufSize = 8192;  // Base size for certificates
    char *buf = NULL;
    char *resultPage = NULL;
    FormValues values = {0};
    int len = 0;
    int ret;
    bool saveSuccess = false;

    // Calculate buffer size needed based on profile
    if (DeviceConfig_IsSettingAvailable(SETTING_CA_CERT))
    {
        bufSize += DeviceConfig_GetMaxLen(SETTING_CA_CERT);
    }
    if (DeviceConfig_IsSettingAvailable(SETTING_CLIENT_CERT))
    {
        bufSize += DeviceConfig_GetMaxLen(SETTING_CLIENT_CERT);
    }
    if (DeviceConfig_IsSettingAvailable(SETTING_CLIENT_KEY))
    {
        bufSize += DeviceConfig_GetMaxLen(SETTING_CLIENT_KEY);
    }
    if (DeviceConfig_IsSettingAvailable(SETTING_DEVICE_CERT))
    {
        bufSize += DeviceConfig_GetMaxLen(SETTING_DEVICE_CERT);
    }

    buf = (char*)calloc(bufSize, 1);
    resultPage = (char*)calloc(DEFAULT_PAGE_SIZE, 1);
    
    if (buf == NULL || resultPage == NULL)
    {
        err = kGeneralErr;
        goto exit;
    }

    // Get POST data
    err = httpd_get_data(req, buf, bufSize - 1);
    require_noerr(err, exit);

    // Parse form data
    err = parseFormData(req, buf, &values);
    require_noerr(err, exit);

    // Build result page header
    strcpy(resultPage, page_head);
    len = strlen(page_head);
    strcpy(&resultPage[len], result_body_start);
    len += strlen(result_body_start);
    strcpy(&resultPage[len], result_table_start);
    len += strlen(result_table_start);

    // Save values and build result rows
    saveSuccess = saveFormValues(&values, resultPage, DEFAULT_PAGE_SIZE, &len);

    // Close table
    strcpy(&resultPage[len], result_table_end);
    len += strlen(result_table_end);

exit:
    // Add success/failure message
    if (err == kNoErr && saveSuccess)
    {
        strcpy(&resultPage[len], result_success);
        len += strlen(result_success);
    }
    else
    {
        ret = snprintf(&resultPage[len], DEFAULT_PAGE_SIZE - len, result_failed, err);
        len += (ret > 0 ? ret : 0);
    }

    // Page end
    strcpy(&resultPage[len], result_body_end);
    len += strlen(result_body_end) + 1;

    // Send response
    httpd_send_all_header(req, HTTP_RES_200, len, HTTP_CONTENT_HTML_STR);
    httpd_send_body(req->sock, (const unsigned char*)resultPage, len);

    // Cleanup
    freeFormValues(&values);
    if (buf) free(buf);
    if (resultPage) free(resultPage);

    // Reboot if save was successful
    if (err == kNoErr && saveSuccess)
    {
        wait_ms(3000);
        mico_system_reboot();
    }

    return err;
}

// ============================================================================
// HTTP Server Management
// ============================================================================

static struct httpd_wsgi_call g_appHandlers[] = {
    {"/", HTTPD_HDR_DEFORT, 0, webSettingsPage, NULL, NULL, NULL},
    {"/result", HTTPD_HDR_DEFORT, 0, NULL, webSettingsResultPage, NULL, NULL}
};

static int g_appHandlersCount = sizeof(g_appHandlers) / sizeof(struct httpd_wsgi_call);

static void registerHttpHandlers(void)
{
    httpd_register_wsgi_handlers(g_appHandlers, g_appHandlersCount);
}

static int startHttpServer(void)
{
    OSStatus err = kNoErr;

    if (!s_isHttpInit)
    {
        err = httpd_init();
        require_noerr(err, exit);
        s_isHttpInit = true;
    }

    err = httpd_start();
    if (err != kNoErr)
    {
        httpd_shutdown();
    }

exit:
    return err;
}

// ============================================================================
// Public API
// ============================================================================

int httpd_server_start(ConnectionProfile profile)
{
    int err = kNoErr;
    
    s_activeProfile = profile;
    DeviceConfig_Init(profile);
    
    err = startHttpServer();
    require_noerr(err, exit);

    if (!s_isHandlersRegistered)
    {
        registerHttpHandlers();
        s_isHandlersRegistered = true;
    }

exit:
    return err;
}

int app_httpd_stop(void)
{
    OSStatus err = kNoErr;
    err = httpd_stop();
    require_noerr(err, exit);
exit:
    return err;
}
