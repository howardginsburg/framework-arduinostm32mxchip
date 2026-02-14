/*
 * AzureIoTHub.cpp - Azure IoT Hub MQTT client implementation
 *
 * Part of the MXChip AZ3166 framework Azure IoT library.
 */

#include "AzureIoTHub.h"
#include "AzureIoTConfig.h"
#include "AzureIoTCrypto.h"
#include "AzureIoTDPS.h"
#include "DeviceConfig.h"
#include "SystemTime.h"

#include <PubSubClient.h>
#include "AZ3166WiFi.h"

// ===== PROFILE-SPECIFIC BUFFERS =====

#if CONNECTION_PROFILE == PROFILE_IOTHUB_SAS || CONNECTION_PROFILE == PROFILE_IOTHUB_CERT
static char connectionString[600];
#endif

#if CONNECTION_PROFILE == PROFILE_DPS_SAS || CONNECTION_PROFILE == PROFILE_DPS_SAS_GROUP
static char symmetricKey[128];
#endif

#if CONNECTION_PROFILE == PROFILE_DPS_SAS || CONNECTION_PROFILE == PROFILE_DPS_SAS_GROUP || CONNECTION_PROFILE == PROFILE_DPS_CERT
static char dpsEndpoint[128];
static char scopeId[64];
static char registrationId[128];
#endif

#if CONNECTION_PROFILE == PROFILE_DPS_CERT || CONNECTION_PROFILE == PROFILE_IOTHUB_CERT
static char deviceCertPem[2700];
static char privateKeyPem[1300];
#endif

#if CONNECTION_PROFILE == PROFILE_IOTHUB_SAS || CONNECTION_PROFILE == PROFILE_DPS_SAS || CONNECTION_PROFILE == PROFILE_DPS_SAS_GROUP
static char deviceKey[128];
static char sasToken[512];
#endif

// ===== INTERNAL STATE =====
static WiFiClientSecure wifiClient;
static PubSubClient mqttClient(wifiClient);

static bool isInitialized = false;
static bool isConnected = false;

static char iotHubHostname[128];
static char deviceId[64];

static char telemetryTopic[128];
static char c2dTopic[128];
static char mqttUsername[256];

static int twinRequestId = 0;
static bool twinGetPending = false;

static C2DMessageCallback c2dCallback = NULL;
static DesiredPropertiesCallback desiredPropsCallback = NULL;
static TwinReceivedCallback twinReceivedCallback = NULL;

// ===== HELPER FUNCTIONS =====

#if CONNECTION_PROFILE == PROFILE_IOTHUB_SAS || CONNECTION_PROFILE == PROFILE_IOTHUB_CERT
// Load and parse connection string from EEPROM
static bool loadConnectionString()
{
    if (!DeviceConfig_IsSettingAvailable(SETTING_CONNECTION_STRING))
    {
        Serial.println("[AzureIoT] Error: Connection string setting not available!");
        return false;
    }

    int bytesRead = DeviceConfig_Read(SETTING_CONNECTION_STRING, connectionString, sizeof(connectionString));
    if (bytesRead < 0 || connectionString[0] == '\0')
    {
        Serial.println("[AzureIoT] Error: Failed to read connection string from EEPROM!");
        Serial.println("[AzureIoT] Use: set_az_iothub <connection_string>");
        return false;
    }
    Serial.print("[AzureIoT] Connection string loaded (");
    Serial.print(strlen(connectionString));
    Serial.println(" bytes)");

    // Parse HostName
    const char* hostStart = strstr(connectionString, "HostName=");
    if (hostStart == NULL)
    {
        Serial.println("[AzureIoT] Error: HostName not found!");
        return false;
    }
    hostStart += 9;
    const char* hostEnd = strchr(hostStart, ';');
    if (hostEnd == NULL) hostEnd = connectionString + strlen(connectionString);
    size_t hostLen = hostEnd - hostStart;
    if (hostLen >= sizeof(iotHubHostname))
    {
        Serial.println("[AzureIoT] Error: HostName too long!");
        return false;
    }
    strncpy(iotHubHostname, hostStart, hostLen);
    iotHubHostname[hostLen] = '\0';

    // Parse DeviceId
    const char* deviceStart = strstr(connectionString, "DeviceId=");
    if (deviceStart == NULL)
    {
        Serial.println("[AzureIoT] Error: DeviceId not found!");
        return false;
    }
    deviceStart += 9;
    const char* deviceEnd = strchr(deviceStart, ';');
    if (deviceEnd == NULL) deviceEnd = connectionString + strlen(connectionString);
    size_t deviceLen = deviceEnd - deviceStart;
    if (deviceLen >= sizeof(deviceId))
    {
        Serial.println("[AzureIoT] Error: DeviceId too long!");
        return false;
    }
    strncpy(deviceId, deviceStart, deviceLen);
    deviceId[deviceLen] = '\0';

#if CONNECTION_PROFILE == PROFILE_IOTHUB_SAS
    // Parse SharedAccessKey
    const char* keyStart = strstr(connectionString, "SharedAccessKey=");
    if (keyStart == NULL)
    {
        Serial.println("[AzureIoT] Error: SharedAccessKey not found!");
        return false;
    }
    keyStart += 16;
    const char* keyEnd = strchr(keyStart, ';');
    if (keyEnd == NULL) keyEnd = connectionString + strlen(connectionString);
    size_t keyLen = keyEnd - keyStart;
    if (keyLen >= sizeof(deviceKey))
    {
        Serial.println("[AzureIoT] Error: SharedAccessKey too long!");
        return false;
    }
    strncpy(deviceKey, keyStart, keyLen);
    deviceKey[keyLen] = '\0';
#endif

    Serial.print("  HostName: ");
    Serial.println(iotHubHostname);
    Serial.print("  DeviceId: ");
    Serial.println(deviceId);

    return true;
}
#endif // PROFILE_IOTHUB_SAS || PROFILE_IOTHUB_CERT

#if CONNECTION_PROFILE == PROFILE_DPS_SAS || CONNECTION_PROFILE == PROFILE_DPS_SAS_GROUP || CONNECTION_PROFILE == PROFILE_DPS_CERT
// Load DPS settings from EEPROM
static bool loadDPSSettings()
{
    if (!DeviceConfig_IsSettingAvailable(SETTING_DPS_ENDPOINT))
    {
        Serial.println("[DPS] Error: DPS endpoint not available!");
        return false;
    }
    DeviceConfig_Read(SETTING_DPS_ENDPOINT, dpsEndpoint, sizeof(dpsEndpoint));
    if (dpsEndpoint[0] == '\0')
    {
        Serial.println("[DPS] Error: DPS endpoint not configured!");
        Serial.println("[DPS] Use: set_dps_endpoint global.azure-devices-provisioning.net");
        return false;
    }

    if (!DeviceConfig_IsSettingAvailable(SETTING_SCOPE_ID))
    {
        Serial.println("[DPS] Error: Scope ID not available!");
        return false;
    }
    DeviceConfig_Read(SETTING_SCOPE_ID, scopeId, sizeof(scopeId));
    if (scopeId[0] == '\0')
    {
        Serial.println("[DPS] Error: Scope ID not configured!");
        Serial.println("[DPS] Use: set_scopeid <scope_id>");
        return false;
    }

    if (!DeviceConfig_IsSettingAvailable(SETTING_REGISTRATION_ID))
    {
        Serial.println("[DPS] Error: Registration ID not available!");
        return false;
    }
    DeviceConfig_Read(SETTING_REGISTRATION_ID, registrationId, sizeof(registrationId));
    if (registrationId[0] == '\0')
    {
        Serial.println("[DPS] Error: Registration ID not configured!");
        Serial.println("[DPS] Use: set_regid <registration_id>");
        return false;
    }

    Serial.print("[DPS] Endpoint: ");
    Serial.println(dpsEndpoint);
    Serial.print("[DPS] Scope ID: ");
    Serial.println(scopeId);
    Serial.print("[DPS] Registration ID: ");
    Serial.println(registrationId);

    return true;
}
#endif // DPS profiles

#if CONNECTION_PROFILE == PROFILE_DPS_CERT || CONNECTION_PROFILE == PROFILE_IOTHUB_CERT
// Load and parse device certificate and private key from EEPROM
static bool loadAndParseCert()
{
    if (!DeviceConfig_IsSettingAvailable(SETTING_DEVICE_CERT))
    {
        Serial.println("[AzureIoT] Error: Device certificate not available!");
        return false;
    }
    DeviceConfig_Read(SETTING_DEVICE_CERT, deviceCertPem, sizeof(deviceCertPem));
    if (deviceCertPem[0] == '\0')
    {
        Serial.println("[AzureIoT] Error: Device certificate not configured!");
        Serial.println("[AzureIoT] Use: set_devicecert <pem_cert_and_key>");
        return false;
    }

    // Find end of certificate
    const char* endMarker = "-----END CERTIFICATE-----";
    char* endPos = strstr(deviceCertPem, endMarker);
    if (endPos == NULL)
    {
        Serial.println("[AzureIoT] Error: Certificate end marker not found!");
        return false;
    }
    endPos += strlen(endMarker);
    if (*endPos == '\r') endPos++;
    if (*endPos == '\n') endPos++;

    // Find private key start
    const char* keyStart = strstr(endPos, "-----BEGIN");
    if (keyStart == NULL)
    {
        Serial.println("[AzureIoT] Error: Private key not found in certificate data!");
        return false;
    }

    // Copy private key to separate buffer
    size_t keyLen = strlen(keyStart);
    if (keyLen >= sizeof(privateKeyPem))
    {
        Serial.println("[AzureIoT] Error: Private key too long!");
        return false;
    }
    strncpy(privateKeyPem, keyStart, sizeof(privateKeyPem) - 1);
    privateKeyPem[sizeof(privateKeyPem) - 1] = '\0';

    // Null-terminate cert portion in-place (key already copied)
    char* certEnd = strstr(deviceCertPem, endMarker);
    certEnd += strlen(endMarker);
    if (*certEnd == '\r') certEnd++;
    if (*certEnd == '\n') certEnd++;
    *certEnd = '\0';

    Serial.println("[AzureIoT] Certificate and key parsed successfully");
    return true;
}
#endif // PROFILE_DPS_CERT || PROFILE_IOTHUB_CERT

#if CONNECTION_PROFILE == PROFILE_IOTHUB_SAS || CONNECTION_PROFILE == PROFILE_DPS_SAS || CONNECTION_PROFILE == PROFILE_DPS_SAS_GROUP
// Sync time via NTP and get expiry timestamp
static uint32_t syncTimeAndGetExpiry()
{
    Serial.println("[AzureIoT] Syncing time via NTP...");
    SyncTime();

    uint32_t expiryTime;
    if (IsTimeSynced() == 0)
    {
        time_t epochTime = time(NULL);
        Serial.print("[AzureIoT] Time synced, epoch: ");
        Serial.println((unsigned long)epochTime);
        expiryTime = (uint32_t)epochTime + SAS_TOKEN_DURATION;
    }
    else
    {
        Serial.println("[AzureIoT] NTP failed, using fallback expiry");
        expiryTime = 1770076800;
    }
    return expiryTime;
}

// Generate SAS token for IoT Hub connection
static bool generateIoTHubSasToken(uint32_t expiryTime)
{
    char resourceUri[256];
    snprintf(resourceUri, sizeof(resourceUri), "%s/devices/%s", iotHubHostname, deviceId);
    return AzureIoT_GenerateSasToken(resourceUri, deviceKey, expiryTime, sasToken, sizeof(sasToken));
}
#endif // SAS profiles

// Internal MQTT callback - routes messages to application callbacks
static void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    char messageContent[1024];
    unsigned int copyLength = (length < sizeof(messageContent) - 1) ? length : sizeof(messageContent) - 1;
    memcpy(messageContent, payload, copyLength);
    messageContent[copyLength] = '\0';

    Serial.println();
    Serial.println("[AzureIoT] ======================================");
    Serial.print("[AzureIoT] Message on: ");
    Serial.println(topic);
    Serial.print("[AzureIoT] Payload (");
    Serial.print(length);
    Serial.println(" bytes)");
    Serial.println("[AzureIoT] ======================================");

    // Route: C2D messages
    if (strstr(topic, "/messages/devicebound/") != NULL)
    {
        Serial.println("[AzureIoT] -> C2D Message");
        if (c2dCallback != NULL)
        {
            c2dCallback(topic, messageContent, length);
        }
    }
    // Route: Device Twin Response
    else if (strncmp(topic, "$iothub/twin/res/", 17) == 0)
    {
        int status = atoi(topic + 17);
        Serial.print("[AzureIoT] -> Twin Response, status: ");
        Serial.println(status);

        if (status == 200 && twinGetPending)
        {
            twinGetPending = false;
            Serial.println("[AzureIoT] Full Device Twin received");
            if (twinReceivedCallback != NULL)
            {
                twinReceivedCallback(messageContent);
            }
        }
        else if (status == 204)
        {
            Serial.println("[AzureIoT] Reported properties accepted");
        }
        else if (status != 200)
        {
            Serial.print("[AzureIoT] Twin operation failed: ");
            Serial.println(status);
        }
    }
    // Route: Desired Property Update
    else if (strncmp(topic, "$iothub/twin/PATCH/properties/desired/", 38) == 0)
    {
        int version = 0;
        const char* versionStart = strstr(topic, "$version=");
        if (versionStart)
        {
            version = atoi(versionStart + 9);
        }

        Serial.print("[AzureIoT] -> Desired Properties, version: ");
        Serial.println(version);

        if (desiredPropsCallback != NULL)
        {
            desiredPropsCallback(messageContent, version);
        }
    }
    else
    {
        Serial.println("[AzureIoT] -> Unknown message type");
    }
}

// ===== PUBLIC API =====

bool azureIoTInit()
{
    Serial.println("[AzureIoT] Initializing...");
    Serial.print("[AzureIoT] Profile: ");
    Serial.println(DeviceConfig_GetProfileName());

#if CONNECTION_PROFILE == PROFILE_IOTHUB_SAS
    // ===== IoT Hub SAS: Load connection string, generate SAS token =====
    if (!loadConnectionString()) return false;

    uint32_t expiryTime = syncTimeAndGetExpiry();
    if (!generateIoTHubSasToken(expiryTime)) return false;

#elif CONNECTION_PROFILE == PROFILE_IOTHUB_CERT
    // ===== IoT Hub X.509: Load connection string + certificate =====
    if (!loadConnectionString()) return false;
    if (!loadAndParseCert()) return false;

    wifiClient.setCertificate(deviceCertPem);
    wifiClient.setPrivateKey(privateKeyPem);

#elif CONNECTION_PROFILE == PROFILE_DPS_SAS || CONNECTION_PROFILE == PROFILE_DPS_SAS_GROUP
    // ===== DPS SAS: Provision with symmetric key, then connect =====
    if (!loadDPSSettings()) return false;

    // Read symmetric key
    if (!DeviceConfig_IsSettingAvailable(SETTING_SYMMETRIC_KEY))
    {
        Serial.println("[DPS] Error: Symmetric key not available!");
        return false;
    }
    DeviceConfig_Read(SETTING_SYMMETRIC_KEY, symmetricKey, sizeof(symmetricKey));
    if (symmetricKey[0] == '\0')
    {
        Serial.println("[DPS] Error: Symmetric key not configured!");
        Serial.println("[DPS] Use: set_symkey <key>");
        return false;
    }

    // Derive device key from group master key if needed
#if CONNECTION_PROFILE == PROFILE_DPS_SAS_GROUP
    if (!AzureIoT_DeriveGroupKey(symmetricKey, registrationId, symmetricKey, sizeof(symmetricKey)))
        return false;
#elif defined(DPS_GROUP_ENROLLMENT)
    if (!AzureIoT_DeriveGroupKey(symmetricKey, registrationId, symmetricKey, sizeof(symmetricKey)))
        return false;
#endif

    // Generate DPS SAS token
    uint32_t expiryTime = syncTimeAndGetExpiry();
    char dpsSasToken[512];
    {
        char dpsResourceUri[256];
        snprintf(dpsResourceUri, sizeof(dpsResourceUri), "%s/registrations/%s", scopeId, registrationId);
        if (!AzureIoT_GenerateSasToken(dpsResourceUri, symmetricKey, expiryTime, dpsSasToken, sizeof(dpsSasToken)))
        {
            Serial.println("[DPS] Failed to generate SAS token!");
            return false;
        }
    }

    // Register with DPS
    if (!AzureIoT_DPSRegister(wifiClient, dpsEndpoint, scopeId, registrationId, dpsSasToken,
                                iotHubHostname, sizeof(iotHubHostname), deviceId, sizeof(deviceId)))
        return false;

    // Generate IoT Hub SAS token using the (possibly derived) key
    strncpy(deviceKey, symmetricKey, sizeof(deviceKey) - 1);
    deviceKey[sizeof(deviceKey) - 1] = '\0';
    if (!generateIoTHubSasToken(expiryTime)) return false;

#elif CONNECTION_PROFILE == PROFILE_DPS_CERT
    // ===== DPS X.509: Provision with certificate =====
    if (!loadDPSSettings()) return false;
    if (!loadAndParseCert()) return false;

    wifiClient.setCertificate(deviceCertPem);
    wifiClient.setPrivateKey(privateKeyPem);

    if (!AzureIoT_DPSRegister(wifiClient, dpsEndpoint, scopeId, registrationId, NULL,
                                iotHubHostname, sizeof(iotHubHostname), deviceId, sizeof(deviceId)))
        return false;

    // Client certificate stays configured on wifiClient from DPS registration

#else
    #error "Unsupported CONNECTION_PROFILE. Use PROFILE_IOTHUB_SAS (4), PROFILE_IOTHUB_CERT (5), PROFILE_DPS_SAS (6), PROFILE_DPS_CERT (7), or PROFILE_DPS_SAS_GROUP (8)"
#endif

    // ===== Common setup for all profiles =====
    snprintf(mqttUsername, sizeof(mqttUsername),
        "%s/%s/?api-version=%s",
        iotHubHostname, deviceId, IOT_HUB_API_VERSION);

    snprintf(telemetryTopic, sizeof(telemetryTopic),
        "devices/%s/messages/events/", deviceId);
    snprintf(c2dTopic, sizeof(c2dTopic),
        "devices/%s/messages/devicebound/#", deviceId);

    Serial.println("[AzureIoT] Configuration:");
    Serial.print("  Hub: ");
    Serial.println(iotHubHostname);
    Serial.print("  Device: ");
    Serial.println(deviceId);
    Serial.print("  Username: ");
    Serial.println(mqttUsername);
    Serial.print("  D2C Topic: ");
    Serial.println(telemetryTopic);

    // Configure TLS for IoT Hub
    Serial.println("[AzureIoT] Configuring TLS...");
    wifiClient.setCACert(AZURE_IOT_ROOT_CA);

    isInitialized = true;
    Serial.println("[AzureIoT] Initialization complete");
    return true;
}

bool azureIoTConnect()
{
    if (!isInitialized)
    {
        Serial.println("[AzureIoT] Not initialized!");
        return false;
    }

    Serial.println("[AzureIoT] Connecting to IoT Hub...");

    mqttClient.setServer(iotHubHostname, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(1024);
    mqttClient.setKeepAlive(60);
    mqttClient.setSocketTimeout(30);

    int retries = 0;
    while (!mqttClient.connected() && retries < 5)
    {
        Serial.print("[AzureIoT] Attempt ");
        Serial.println(retries + 1);

#if CONNECTION_PROFILE == PROFILE_DPS_CERT || CONNECTION_PROFILE == PROFILE_IOTHUB_CERT
        if (mqttClient.connect(deviceId, mqttUsername, ""))
#else
        if (mqttClient.connect(deviceId, mqttUsername, sasToken))
#endif
        {
            isConnected = true;
            Serial.println("[AzureIoT] Connected!");

            bool subOk = true;
            subOk &= mqttClient.subscribe(c2dTopic);
            subOk &= mqttClient.subscribe("$iothub/twin/res/#");
            subOk &= mqttClient.subscribe("$iothub/twin/PATCH/properties/desired/#");

            if (subOk)
                Serial.println("[AzureIoT] Subscribed to all topics");
            else
                Serial.println("[AzureIoT] Warning: Some subscriptions failed");

            return true;
        }
        else
        {
            int state = mqttClient.state();
            Serial.print("[AzureIoT] Failed, state: ");
            Serial.println(state);
            retries++;
            delay(3000);
        }
    }

    isConnected = false;
    Serial.println("[AzureIoT] Connection failed after retries");
    return false;
}

bool azureIoTIsConnected()
{
    return isConnected && mqttClient.connected();
}

void azureIoTLoop()
{
    if (!isInitialized) return;

    if (!mqttClient.connected())
    {
        isConnected = false;
        Serial.println("[AzureIoT] Disconnected, attempting reconnect...");
        azureIoTConnect();
    }

    mqttClient.loop();
}

void azureIoTSetC2DCallback(C2DMessageCallback callback)
{
    c2dCallback = callback;
}

void azureIoTSetDesiredPropertiesCallback(DesiredPropertiesCallback callback)
{
    desiredPropsCallback = callback;
}

void azureIoTSetTwinReceivedCallback(TwinReceivedCallback callback)
{
    twinReceivedCallback = callback;
}

bool azureIoTSendTelemetry(const char* payload, const char* properties)
{
    if (!azureIoTIsConnected())
    {
        Serial.println("[AzureIoT] Cannot send: not connected");
        return false;
    }

    char topic[256];
    if (properties != NULL && strlen(properties) > 0)
    {
        snprintf(topic, sizeof(topic), "devices/%s/messages/events/%s", deviceId, properties);
    }
    else
    {
        snprintf(topic, sizeof(topic), "devices/%s/messages/events/", deviceId);
    }

    bool success = mqttClient.publish(topic, payload);
    if (success)
        Serial.println("[AzureIoT] Telemetry sent");
    else
        Serial.println("[AzureIoT] Telemetry send failed");
    return success;
}

void azureIoTRequestTwin()
{
    if (!azureIoTIsConnected())
    {
        Serial.println("[AzureIoT] Cannot request twin: not connected");
        return;
    }

    char topic[64];
    snprintf(topic, sizeof(topic), "$iothub/twin/GET/?$rid=%d", ++twinRequestId);

    twinGetPending = true;

    if (mqttClient.publish(topic, ""))
        Serial.println("[AzureIoT] Twin GET request sent");
    else
    {
        Serial.println("[AzureIoT] Twin GET request failed");
        twinGetPending = false;
    }
}

void azureIoTUpdateReportedProperties(const char* jsonPayload)
{
    if (!azureIoTIsConnected())
    {
        Serial.println("[AzureIoT] Cannot update reported: not connected");
        return;
    }

    char topic[64];
    snprintf(topic, sizeof(topic),
        "$iothub/twin/PATCH/properties/reported/?$rid=%d", ++twinRequestId);

    if (mqttClient.publish(topic, jsonPayload))
        Serial.println("[AzureIoT] Reported properties sent");
    else
        Serial.println("[AzureIoT] Reported properties send failed");
}

const char* azureIoTGetDeviceId()
{
    return deviceId;
}

const char* azureIoTGetHostname()
{
    return iotHubHostname;
}
