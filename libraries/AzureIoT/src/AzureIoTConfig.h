/*
 * AzureIoTConfig.h - Protocol constants and root certificate for Azure IoT
 *
 * Part of the MXChip AZ3166 framework Azure IoT library.
 */

#ifndef AZURE_IOT_CONFIG_H
#define AZURE_IOT_CONFIG_H

// ===== Azure IoT Hub Protocol Settings =====
#define IOT_HUB_API_VERSION "2021-04-12"
#define MQTT_PORT           8883
#define SAS_TOKEN_DURATION  86400   // 24 hours in seconds

// ===== Azure DPS Protocol Settings =====
#define DPS_API_VERSION     "2021-06-01"
#define DPS_POLL_INTERVAL   3000    // ms between status polls
#define DPS_MAX_RETRIES     10

// ===== Azure IoT Hub Root Certificate =====
// DigiCert Global Root G2 - Valid until January 15, 2038
static const char AZURE_IOT_ROOT_CA[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
"1Yl9PMCcit652T4Vs5rHh5zhQVrBdPZBp9NOZGerGm5HaDgcqQ3L2jTPNsONq6vL\n"
"HOgszJEzY5d2LO7D+VQ8qf9w1fUfx4ztcdL0Y5Bx7ey/ZL/OB0d9m0K5SH5Rp4gf\n"
"qyeHeSnYLJwHJG/NPawNl/WPtjplVp2B8l4hy2aVpv8XNNP/9KlIjN8C4yKp9hsj\n"
"p+mD9LKuGCBiIIXBu7K2UVT/yWJmM6g9jZJDLf3uXMiPcOq6BNFuPaH7t7bP3MxW\n"
"3WF5+VGPYtM8k+8W3dKhpGnlB8KdvO7ItGp4PysVIxbGNfyXFCy4h6PTY7NxJVma\n"
"lJM=\n"
"-----END CERTIFICATE-----\n";

#endif // AZURE_IOT_CONFIG_H
