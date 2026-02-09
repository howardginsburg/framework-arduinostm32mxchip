/**
 ******************************************************************************
 * @file    app_httpd.cpp
 * @author  QQ DING (original), Microsoft (DeviceConfig integration)
 * @version V2.1.0
 * @date    8-February-2026
 * @brief   Web-based device configuration UI using DeviceConfig profiles.
 *          Uses data-driven approach - fields automatically shown based on
 *          what settings are available in the active profile.
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
#include "SettingUI.h"
#include "SettingValidator.h"
#include "EMW10xxInterface.h"
#include "httpd.h"
#include "OledDisplay.h"
#include "SystemVariables.h"
#include "SystemWeb.h"

#define HTTPD_HDR_DEFORT (HTTPD_HDR_ADD_SERVER|HTTPD_HDR_ADD_CONN_CLOSE|HTTPD_HDR_ADD_PRAGMA_NO_CACHE)

#define DEFAULT_PAGE_SIZE (12*1024)

// Maximum lengths (these should match DeviceConfig zone sizes)
#define WIFI_SSID_MAX_LEN   120
#define WIFI_PWD_MAX_LEN    88

// ============================================================================
// HTML Templates
// ============================================================================

static const char* page_head = 
    "<!DOCTYPE html><html lang=\"en\"><head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>IoT DevKit Configuration</title>"
    "<style>"
    "html{font-size:16px;}"
    "html,*{font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",\"Roboto\",\"Helvetica Neue\",Helvetica,Arial,sans-serif;line-height:1.5;}"
    "*{font-size:1rem;}"
    "body{margin:0;color:#212121;background:#f8f8f8;}"
    "section{display:block;}"
    "input{overflow:visible;}"
    "[type=\"radio\"]{position:absolute;left:-2rem;}"
    "h1{font-size:1.5rem;line-height:1.2em;margin:0.5rem;font-weight:500;}"
    "p{margin:0.5rem;}"
    ".container{margin:0 auto;padding:0 0.75rem;max-width:600px;}"
    "header{display:block;background:#1e6bb8;color:#f5f5f5;padding:0.5rem;}"
    "header .logo{color:#f5f5f5;text-decoration:none;}"
    "form{background:#eee;border:1px solid #c9c9c9;margin:0.5rem;padding:0.75rem;}"
    ".input-group{margin:0.5rem 0;}"
    ".input-group.fluid{display:flex;align-items:center;}"
    ".input-group.fluid>input:not([type=\"radio\"]),.input-group.fluid>textarea,.input-group.fluid>select{width:100%;flex-grow:1;}"
    "[type=\"password\"],[type=\"text\"],select,textarea{width:100%;box-sizing:border-box;background:#fafafa;color:#212121;border:1px solid #c9c9c9;border-radius:2px;margin:0.25rem 0;padding:0.5rem;}"
    "input:focus,select:focus,textarea:focus{border-color:#0288d1;outline:none;}"
    "::placeholder{color:#616161;}"
    "button,[type=\"submit\"]{display:inline-block;background:rgba(30,107,184,0.9);color:#fafafa;border:0;border-radius:2px;padding:0.5rem 1rem;margin:0.5rem 0;cursor:pointer;}"
    "button:hover,[type=\"submit\"]:hover{background:#0277bd;}"
    "fieldset{border:1px solid #c9c9c9;margin:0.5rem 0;padding:0.5rem;}"
    "legend{padding:0 0.5rem;color:#1e6bb8;font-weight:500;}"
    ".profile-badge{background:#1e6bb8;color:#fff;padding:0.2rem 0.5rem;border-radius:3px;font-size:0.8rem;margin-left:0.5rem;}"
    "table{width:100%;border-collapse:collapse;margin:1rem 0;}"
    "th,td{border:1px solid #c9c9c9;padding:0.5rem;text-align:left;background:#eee;}"
    "th{background:#ddd;}"
    ".success{color:DodgerBlue;}"
    ".error{color:Tomato;}"
    "</style></head>";

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
    "<input type=\"password\" name=\"PASS\" placeholder=\"Wi-Fi Password\">"
    "</div></fieldset>";

static const char* page_body_end = 
    "<div class=\"input-group\"><button type=\"submit\">Save Configuration</button></div>"
    "</form>"
    "<p style=\"color:#616161;text-align:center;\">Refresh to update Wi-Fi list</p>"
    "</div></section>"
    "<script>"
    "function changeSSIDInput(){"
    "var sel=document.getElementsByName('input_ssid_method')[0].checked;"
    "var s=document.getElementById('SSID-select');"
    "var t=document.getElementById('SSID-text');"
    "s.name=sel?'SSID':'';s.disabled=!sel;"
    "t.name=sel?'':'SSID';t.disabled=sel;"
    "}"
    "</script></body></html>";

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
 * @brief Check if any non-WiFi settings are available
 */
static bool hasConnectionSettings(void)
{
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        // Skip WiFi settings
        if (SETTING_UI[i].id == SETTING_WIFI_SSID || 
            SETTING_UI[i].id == SETTING_WIFI_PASSWORD)
        {
            continue;
        }
        if (DeviceConfig_IsSettingAvailable(SETTING_UI[i].id))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Generate HTML for a single form field
 */
static int generateFieldHtml(char* buffer, int bufferSize, const SettingUIMetadata* field)
{
    const char* inputType;
    char defaultVal[128] = "";
    
    if (field->defaultValue && field->defaultValue[0] != '\0')
    {
        snprintf(defaultVal, sizeof(defaultVal), " value=\"%s\"", field->defaultValue);
    }
    
    if (field->fieldType == UI_FIELD_TEXTAREA)
    {
        return snprintf(buffer, bufferSize,
            "<div class=\"input-group fluid\"><textarea name=\"%s\" rows=\"3\" placeholder=\"%s\"></textarea></div>",
            field->webFormName, field->webPlaceholder);
    }
    else
    {
        inputType = (field->fieldType == UI_FIELD_PASSWORD) ? "password" : "text";
        return snprintf(buffer, bufferSize,
            "<div class=\"input-group fluid\"><input type=\"%s\" name=\"%s\"%s placeholder=\"%s\"></div>",
            inputType, field->webFormName, defaultVal, field->webPlaceholder);
    }
}

/**
 * @brief Generate all available profile fields HTML
 * Simply iterates SETTING_UI and shows any that are available in the profile
 */
static int generateProfileFieldsHtml(char* buffer, int bufferSize, int currentLen)
{
    int len = currentLen;
    int ret;
    bool hasFields = hasConnectionSettings();
    
    if (!hasFields)
    {
        // No connection settings - just WiFi
        if (s_activeProfile == PROFILE_NONE)
        {
            ret = snprintf(&buffer[len], bufferSize - len,
                "<p style=\"color:#616161;\">Only Wi-Fi settings available.</p>");
            len += (ret > 0 ? ret : 0);
        }
        return len;
    }
    
    // Open a single fieldset for all connection settings
    ret = snprintf(&buffer[len], bufferSize - len,
        "<fieldset><legend>Connection Settings</legend>");
    len += (ret > 0 ? ret : 0);
    
    // Add all available fields in order (skip WiFi - handled separately)
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        // Skip WiFi settings
        if (SETTING_UI[i].id == SETTING_WIFI_SSID || 
            SETTING_UI[i].id == SETTING_WIFI_PASSWORD)
        {
            continue;
        }
        if (DeviceConfig_IsSettingAvailable(SETTING_UI[i].id))
        {
            ret = generateFieldHtml(&buffer[len], bufferSize - len, &SETTING_UI[i]);
            len += (ret > 0 ? ret : 0);
        }
    }
    
    // Close fieldset
    ret = snprintf(&buffer[len], bufferSize - len, "</fieldset>");
    len += (ret > 0 ? ret : 0);
    
    return len;
}

// ============================================================================
// Form Values Storage - Array indexed by SettingID
// ============================================================================

typedef struct {
    char ssid[WIFI_SSID_MAX_LEN + 1];
    char password[WIFI_PWD_MAX_LEN + 1];
    char* values[SETTING_COUNT];  // Array of pointers, indexed by SettingID
} FormValues;

static void initFormValues(FormValues* fv)
{
    memset(fv, 0, sizeof(FormValues));
}

static void freeFormValues(FormValues* fv)
{
    for (int i = 0; i < SETTING_COUNT; i++)
    {
        if (fv->values[i])
        {
            free(fv->values[i]);
            fv->values[i] = NULL;
        }
    }
}

/**
 * @brief Allocate and parse a field value if available in profile
 */
static void parseField(char* buf, char* boundary, bool isMultipart,
                       SettingID setting, FormValues* fv)
{
    if (!DeviceConfig_IsSettingAvailable(setting))
    {
        return;
    }
    
    const SettingUIMetadata* field = SettingUI_FindById(setting);
    if (!field)
    {
        return;
    }
    
    int maxLen = DeviceConfig_GetMaxLen(setting);
    if (maxLen <= 0)
    {
        return;
    }
    
    fv->values[setting] = (char*)calloc(maxLen + 1, 1);
    if (!fv->values[setting])
    {
        return;
    }
    
    if (isMultipart)
    {
        httpd_get_tag_from_multipart_form(buf, boundary, field->webFormName, 
                                          fv->values[setting], maxLen);
    }
    else
    {
        httpd_get_tag_from_post_data(buf, field->webFormName, 
                                     fv->values[setting], maxLen);
    }
}

/**
 * @brief Parse all form data based on available settings
 */
static int parseFormData(httpd_request_t* req, char* buf, FormValues* fv)
{
    OSStatus err = kNoErr;
    char* boundary = NULL;
    bool isMultipart = strstr(req->content_type, "multipart/form-data") != NULL;
    
    if (isMultipart)
    {
        boundary = strstr(req->content_type, "boundary=");
        if (boundary) boundary += 9;
    }
    
    // Parse WiFi (always, if available)
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_SSID))
    {
        if (isMultipart)
        {
            httpd_get_tag_from_multipart_form(buf, boundary, "SSID", fv->ssid, WIFI_SSID_MAX_LEN);
            httpd_get_tag_from_multipart_form(buf, boundary, "PASS", fv->password, WIFI_PWD_MAX_LEN);
        }
        else
        {
            httpd_get_tag_from_post_data(buf, "SSID", fv->ssid, WIFI_SSID_MAX_LEN);
            httpd_get_tag_from_post_data(buf, "PASS", fv->password, WIFI_PWD_MAX_LEN);
        }
        
        if (fv->ssid[0] == '\0')
        {
            err = kParamErr;
            goto exit;
        }
    }
    
    // Parse all other fields (data-driven!)
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        // Skip WiFi settings
        if (SETTING_UI[i].id == SETTING_WIFI_SSID || 
            SETTING_UI[i].id == SETTING_WIFI_PASSWORD)
        {
            continue;
        }
        parseField(buf, boundary, isMultipart, SETTING_UI[i].id, fv);
    }
    
exit:
    return err;
}

/**
 * @brief Save all form values and generate result rows
 */
static bool saveFormValues(FormValues* fv, char* resultBuffer, int bufferSize, int* len)
{
    bool success = true;
    int ret;
    
    // Save WiFi
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_SSID) && fv->ssid[0] != '\0')
    {
        // Validate WiFi settings
        ValidationResult ssidResult = Validator_CheckLength(SETTING_WIFI_SSID, fv->ssid);
        ValidationResult pwdResult = (fv->password[0] != '\0') 
            ? Validator_CheckLength(SETTING_WIFI_PASSWORD, fv->password) 
            : VALIDATE_OK;
        
        if (ssidResult != VALIDATE_OK || pwdResult != VALIDATE_OK)
        {
            const char* errMsg = (ssidResult != VALIDATE_OK) 
                ? Validator_GetErrorMessage(ssidResult)
                : Validator_GetErrorMessage(pwdResult);
            ret = snprintf(&resultBuffer[*len], bufferSize - *len,
                "<tr><td>Wi-Fi</td><td>%s</td></tr>", errMsg);
            *len += (ret > 0 ? ret : 0);
            success = false;
        }
        else
        {
            bool wifiOk = (DeviceConfig_Save(SETTING_WIFI_SSID, fv->ssid) == 0);
            if (fv->password[0] != '\0')
            {
                wifiOk = wifiOk && (DeviceConfig_Save(SETTING_WIFI_PASSWORD, fv->password) == 0);
            }
            ret = snprintf(&resultBuffer[*len], bufferSize - *len,
                "<tr><td>Wi-Fi</td><td>%s</td></tr>", wifiOk ? "Saved" : "Save failed");
            *len += (ret > 0 ? ret : 0);
            if (!wifiOk) success = false;
        }
    }
    
    // Save all other fields (data-driven!)
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        SettingID setting = SETTING_UI[i].id;
        
        // Skip WiFi settings
        if (setting == SETTING_WIFI_SSID || setting == SETTING_WIFI_PASSWORD)
        {
            continue;
        }
        
        if (!DeviceConfig_IsSettingAvailable(setting))
        {
            continue;
        }
        
        char* value = fv->values[setting];
        if (!value || value[0] == '\0')
        {
            continue;
        }
        
        // Validate before saving
        ValidationResult validResult = Validator_ValidateSetting(setting, value);
        if (validResult != VALIDATE_OK)
        {
            ret = snprintf(&resultBuffer[*len], bufferSize - *len,
                "<tr><td>%s</td><td>%s</td></tr>",
                SETTING_UI[i].label, Validator_GetErrorMessage(validResult));
            *len += (ret > 0 ? ret : 0);
            success = false;
            continue;
        }
        
        bool saved = (DeviceConfig_Save(setting, value) == 0);
        ret = snprintf(&resultBuffer[*len], bufferSize - *len,
            "<tr><td>%s</td><td>%s</td></tr>",
            SETTING_UI[i].label, saved ? "Saved" : "Save failed");
        *len += (ret > 0 ? ret : 0);
        if (!saved) success = false;
    }
    
    return success;
}

// ============================================================================
// HTTP Request Handlers
// ============================================================================

static int webSettingsPage(httpd_request_t* req)
{
    char* page = NULL;
    int len = 0;
    int err = kNoErr;
    
    // Scan WiFi
    WiFiAccessPoint wifiScanResult[50];
    int validWifiIndex[15];
    int wifiCount = ((EMW10xxInterface*)_defaultSystemNetwork)->scan(wifiScanResult, 50);
    int validWifiCount = 0;
    
    for (int i = 0; i < wifiCount && validWifiCount < 15; ++i)
    {
        if (wifiScanResult[i].get_rssi() < -100) continue;
        
        const char* ssid = wifiScanResult[i].get_ssid();
        int ssidLen = strlen(ssid);
        if (ssidLen == 0 || ssidLen > WIFI_SSID_MAX_LEN) continue;
        
        // Skip duplicates
        bool dup = false;
        for (int j = 0; j < i; ++j)
        {
            if (wifiScanResult[j].get_rssi() >= -100 && 
                strcmp(ssid, wifiScanResult[j].get_ssid()) == 0)
            {
                dup = true;
                break;
            }
        }
        if (dup) continue;
        
        // Skip our own AP
        if (ssidLen == BOARD_AP_LENGTH && strncmp(ssid, boardAPHeader, strlen(boardAPHeader)) == 0)
        {
            bool isOurAp = true;
            for (int j = strlen(boardAPHeader); j < BOARD_AP_LENGTH; ++j)
            {
                if (!isxdigit(ssid[j])) { isOurAp = false; break; }
            }
            if (isOurAp) continue;
        }
        
        validWifiIndex[validWifiCount++] = i;
    }
    
    // Allocate page
    page = (char*)calloc(DEFAULT_PAGE_SIZE, 1);
    if (!page)
    {
        err = kGeneralErr;
        goto exit;
    }
    
    // Build page
    strcpy(page, page_head);
    len = strlen(page_head);
    
    // Header with profile badge
    {
        int ret = snprintf(&page[len], DEFAULT_PAGE_SIZE - len,
            "<body><header><h1 class=\"logo\">IoT DevKit"
            "<span class=\"profile-badge\">%s</span></h1></header>"
            "<section class=\"container\"><div>"
            "<form action=\"result\" method=\"post\" enctype=\"multipart/form-data\">",
            DeviceConfig_GetProfileName());
        len += (ret > 0 ? ret : 0);
    }
    
    // WiFi fieldset
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_SSID))
    {
        strcpy(&page[len], wifi_fieldset_start);
        len += strlen(wifi_fieldset_start);
        
        for (int i = 0; i < validWifiCount; ++i)
        {
            const char* ssid = wifiScanResult[validWifiIndex[i]].get_ssid();
            int ret = snprintf(&page[len], DEFAULT_PAGE_SIZE - len,
                "<option value=\"%s\">%s</option>", ssid, ssid);
            len += (ret > 0 ? ret : 0);
        }
        
        strcpy(&page[len], wifi_fieldset_mid);
        len += strlen(wifi_fieldset_mid);
    }
    
    // Profile fields (data-driven)
    len = generateProfileFieldsHtml(page, DEFAULT_PAGE_SIZE, len);
    
    // Page end
    strcpy(&page[len], page_body_end);
    len += strlen(page_body_end) + 1;
    
    err = httpd_send_all_header(req, HTTP_RES_200, len, HTTP_CONTENT_HTML_STR);
    if (err == kNoErr)
    {
        err = httpd_send_body(req->sock, (const unsigned char*)page, len);
    }

exit:
    if (page) free(page);
    return err;
}

static int webSettingsResultPage(httpd_request_t* req)
{
    OSStatus err = kNoErr;
    int bufSize = 4096;
    char* buf = NULL;
    char* page = NULL;
    FormValues fv;
    int len = 0;
    bool saveSuccess = false;
    
    initFormValues(&fv);
    
    // Calculate buffer size for certificates
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        if (SETTING_UI[i].fieldType == UI_FIELD_TEXTAREA &&
            DeviceConfig_IsSettingAvailable(SETTING_UI[i].id))
        {
            bufSize += DeviceConfig_GetMaxLen(SETTING_UI[i].id);
        }
    }
    
    buf = (char*)calloc(bufSize, 1);
    page = (char*)calloc(DEFAULT_PAGE_SIZE, 1);
    
    if (!buf || !page)
    {
        err = kGeneralErr;
        goto exit;
    }
    
    err = httpd_get_data(req, buf, bufSize - 1);
    if (err != kNoErr) goto exit;
    
    err = parseFormData(req, buf, &fv);
    if (err != kNoErr) goto exit;
    
    // Build result page
    strcpy(page, page_head);
    len = strlen(page_head);
    
    {
        int ret = snprintf(&page[len], DEFAULT_PAGE_SIZE - len,
            "<body><header><h1 class=\"logo\">Configuration Result</h1></header>"
            "<section class=\"container\"><div>"
            "<table><tr><th>Setting</th><th>Status</th></tr>");
        len += (ret > 0 ? ret : 0);
    }
    
    saveSuccess = saveFormValues(&fv, page, DEFAULT_PAGE_SIZE, &len);
    
    {
        int ret = snprintf(&page[len], DEFAULT_PAGE_SIZE - len, "</table>");
        len += (ret > 0 ? ret : 0);
    }

exit:
    if (err == kNoErr && saveSuccess)
    {
        int ret = snprintf(&page[len], DEFAULT_PAGE_SIZE - len,
            "<p class=\"success\">Configuration saved! Rebooting...</p>");
        len += (ret > 0 ? ret : 0);
    }
    else
    {
        int ret = snprintf(&page[len], DEFAULT_PAGE_SIZE - len,
            "<p class=\"error\">Configuration failed (error: %d)</p>", err);
        len += (ret > 0 ? ret : 0);
    }
    
    {
        int ret = snprintf(&page[len], DEFAULT_PAGE_SIZE - len,
            "<button onclick=\"location.href='/'\">Back</button>"
            "</div></section></body></html>");
        len += (ret > 0 ? ret : 0);
    }
    
    httpd_send_all_header(req, HTTP_RES_200, len, HTTP_CONTENT_HTML_STR);
    httpd_send_body(req->sock, (const unsigned char*)page, len);
    
    freeFormValues(&fv);
    if (buf) free(buf);
    if (page) free(page);
    
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

static void registerHttpHandlers(void)
{
    httpd_register_wsgi_handlers(g_appHandlers, 2);
}

static int startHttpServer(void)
{
    OSStatus err = kNoErr;

    if (!s_isHttpInit)
    {
        err = httpd_init();
        if (err != kNoErr) return err;
        s_isHttpInit = true;
    }

    err = httpd_start();
    if (err != kNoErr)
    {
        httpd_shutdown();
    }

    return err;
}

// ============================================================================
// Public API
// ============================================================================

int httpd_server_start(ConnectionProfile profile)
{
    int err;
    
    s_activeProfile = profile;
    DeviceConfig_Init(profile);
    
    err = startHttpServer();
    if (err != kNoErr) return err;

    if (!s_isHandlersRegistered)
    {
        registerHttpHandlers();
        s_isHandlersRegistered = true;
    }

    return kNoErr;
}

int app_httpd_stop(void)
{
    return httpd_stop();
}
