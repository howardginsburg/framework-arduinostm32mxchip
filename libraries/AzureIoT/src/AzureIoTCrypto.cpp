/*
 * AzureIoTCrypto.cpp - Cryptographic utilities for Azure IoT
 *
 * Part of the MXChip AZ3166 framework Azure IoT library.
 */

#include "AzureIoTCrypto.h"
#include "mbedtls/md.h"
#include "mbedtls/base64.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

void AzureIoT_UrlEncode(const char* input, char* output, size_t outputSize)
{
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < outputSize - 4; i++)
    {
        char c = input[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
        {
            output[j++] = c;
        }
        else
        {
            snprintf(&output[j], 4, "%%%02X", (unsigned char)c);
            j += 3;
        }
    }
    output[j] = '\0';
}

bool AzureIoT_HmacSHA256(const unsigned char* key, size_t keyLen,
                          const unsigned char* data, size_t dataLen,
                          unsigned char* output, size_t outputSize)
{
    if (outputSize < 32) return false;

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    int ret = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    if (ret != 0)
    {
        mbedtls_md_free(&ctx);
        return false;
    }

    mbedtls_md_hmac_starts(&ctx, key, keyLen);
    mbedtls_md_hmac_update(&ctx, data, dataLen);
    mbedtls_md_hmac_finish(&ctx, output);
    mbedtls_md_free(&ctx);
    return true;
}

bool AzureIoT_GenerateSasToken(const char* resourceUri, const char* signingKey,
                                uint32_t expiryTimeSeconds,
                                char* tokenBuffer, size_t tokenBufferSize)
{
    Serial.println("[AzureIoT] Generating SAS token...");

    // URL-encode the resource URI
    char encodedUri[256];
    AzureIoT_UrlEncode(resourceUri, encodedUri, sizeof(encodedUri));

    // Build signature string: "<encodedUri>\n<expiry>"
    char signatureString[512];
    snprintf(signatureString, sizeof(signatureString), "%s\n%lu", encodedUri, (unsigned long)expiryTimeSeconds);

    // Decode the base64-encoded signing key
    unsigned char decodedKey[64];
    size_t decodedKeyLen = 0;
    int ret = mbedtls_base64_decode(decodedKey, sizeof(decodedKey), &decodedKeyLen,
                                     (const unsigned char*)signingKey, strlen(signingKey));
    if (ret != 0)
    {
        Serial.print("[AzureIoT] Failed to decode key! Error: ");
        Serial.println(ret);
        return false;
    }

    // Compute HMAC-SHA256
    unsigned char hmacResult[32];
    if (!AzureIoT_HmacSHA256(decodedKey, decodedKeyLen,
                              (const unsigned char*)signatureString, strlen(signatureString),
                              hmacResult, sizeof(hmacResult)))
    {
        Serial.println("[AzureIoT] Failed to compute HMAC!");
        return false;
    }

    // Base64-encode the HMAC result
    unsigned char base64Signature[64];
    size_t base64Len = 0;
    ret = mbedtls_base64_encode(base64Signature, sizeof(base64Signature), &base64Len,
                                 hmacResult, sizeof(hmacResult));
    if (ret != 0)
    {
        Serial.println("[AzureIoT] Failed to base64 encode signature!");
        return false;
    }
    base64Signature[base64Len] = '\0';

    // URL-encode the signature
    char encodedSignature[128];
    AzureIoT_UrlEncode((const char*)base64Signature, encodedSignature, sizeof(encodedSignature));

    // Build the final SAS token
    snprintf(tokenBuffer, tokenBufferSize,
        "SharedAccessSignature sr=%s&sig=%s&se=%lu",
        encodedUri, encodedSignature, (unsigned long)expiryTimeSeconds);

    Serial.println("[AzureIoT] SAS token generated successfully");
    return true;
}

bool AzureIoT_DeriveGroupKey(const char* groupKey, const char* registrationId,
                              char* derivedKeyBuffer, size_t derivedKeyBufferSize)
{
    Serial.println("[DPS] Deriving device key from group key...");

    // Decode the base64-encoded group key
    unsigned char decodedGroupKey[64];
    size_t decodedKeyLen = 0;
    int ret = mbedtls_base64_decode(decodedGroupKey, sizeof(decodedGroupKey), &decodedKeyLen,
                                     (const unsigned char*)groupKey, strlen(groupKey));
    if (ret != 0)
    {
        Serial.println("[DPS] Failed to decode group key!");
        return false;
    }

    // HMAC-SHA256(groupKey, registrationId)
    unsigned char hmacResult[32];
    if (!AzureIoT_HmacSHA256(decodedGroupKey, decodedKeyLen,
                              (const unsigned char*)registrationId, strlen(registrationId),
                              hmacResult, sizeof(hmacResult)))
    {
        Serial.println("[DPS] Failed to derive device key!");
        return false;
    }

    // Base64-encode the derived key
    unsigned char base64Key[64];
    size_t base64Len = 0;
    ret = mbedtls_base64_encode(base64Key, sizeof(base64Key), &base64Len,
                                 hmacResult, sizeof(hmacResult));
    if (ret != 0)
    {
        Serial.println("[DPS] Failed to encode derived key!");
        return false;
    }
    base64Key[base64Len] = '\0';

    if (base64Len >= derivedKeyBufferSize)
    {
        Serial.println("[DPS] Derived key buffer too small!");
        return false;
    }

    strncpy(derivedKeyBuffer, (const char*)base64Key, derivedKeyBufferSize - 1);
    derivedKeyBuffer[derivedKeyBufferSize - 1] = '\0';

    Serial.println("[DPS] Device key derived successfully");
    return true;
}
