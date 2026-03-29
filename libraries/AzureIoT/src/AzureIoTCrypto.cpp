/*
 * AzureIoTCrypto.cpp - Cryptographic utilities for Azure IoT
 *
 * All HMAC-SHA256 and base64 operations now use wolfCrypt (part of the
 * wolfSSL library) instead of mbedTLS.  The pre-compiled system binaries
 * (libdevkit-sdk-core-lib.a, libstsafe.a) continue to use mbedTLS
 * internally; this file no longer depends on mbedTLS at all.
 *
 * Part of the MXChip AZ3166 framework Azure IoT library.
 */

#include "AzureIoTCrypto.h"

#ifndef WOLFSSL_USER_SETTINGS
#  define WOLFSSL_USER_SETTINGS
#endif
#include "wolfssl/wolfcrypt/hmac.h"
#include "wolfssl/wolfcrypt/coding.h"

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
            snprintf(&output[j], 4, "%%%02x", (unsigned char)c);
            j += 3;
        }
    }
    output[j] = '\0';
}

bool AzureIoT_HmacSHA256(const unsigned char* key, size_t keyLen,
                          const unsigned char* data, size_t dataLen,
                          unsigned char* output, size_t outputSize)
{
    if (outputSize < WC_SHA256_DIGEST_SIZE) return false;

    Hmac hmac;
    int ret;

    ret = wc_HmacInit(&hmac, NULL, INVALID_DEVID);
    if (ret != 0) return false;

    ret = wc_HmacSetKey(&hmac, WC_SHA256,
                        (const byte *)key, (word32)keyLen);
    if (ret != 0)
    {
        wc_HmacFree(&hmac);
        return false;
    }

    ret = wc_HmacUpdate(&hmac, (const byte *)data, (word32)dataLen);
    if (ret != 0)
    {
        wc_HmacFree(&hmac);
        return false;
    }

    ret = wc_HmacFinal(&hmac, (byte *)output);
    wc_HmacFree(&hmac);
    return (ret == 0);
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
    snprintf(signatureString, sizeof(signatureString),
             "%s\n%lu", encodedUri, (unsigned long)expiryTimeSeconds);

    // Base64-decode the signing key
    unsigned char decodedKey[64];
    word32 decodedKeyLen = sizeof(decodedKey);
    int ret = Base64_Decode((const byte *)signingKey, (word32)strlen(signingKey),
                            decodedKey, &decodedKeyLen);
    if (ret != 0)
    {
        Serial.print("[AzureIoT] Failed to decode key! Error: ");
        Serial.println(ret);
        return false;
    }

    // Compute HMAC-SHA256
    unsigned char hmacResult[WC_SHA256_DIGEST_SIZE];
    if (!AzureIoT_HmacSHA256(decodedKey, decodedKeyLen,
                              (const unsigned char *)signatureString,
                              strlen(signatureString),
                              hmacResult, sizeof(hmacResult)))
    {
        Serial.println("[AzureIoT] Failed to compute HMAC!");
        return false;
    }

    // Base64-encode the HMAC result (no embedded newlines for URL use)
    unsigned char base64Signature[64];
    word32 base64Len = sizeof(base64Signature);
    ret = Base64_Encode_NoNl(hmacResult, (word32)sizeof(hmacResult),
                              base64Signature, &base64Len);
    if (ret != 0)
    {
        Serial.println("[AzureIoT] Failed to base64 encode signature!");
        return false;
    }
    base64Signature[base64Len] = '\0';

    // URL-encode the base64 signature
    char encodedSignature[128];
    AzureIoT_UrlEncode((const char *)base64Signature,
                        encodedSignature, sizeof(encodedSignature));

    // Assemble the final SAS token
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

    // Base64-decode the group key
    unsigned char decodedGroupKey[64];
    word32 decodedKeyLen = sizeof(decodedGroupKey);
    int ret = Base64_Decode((const byte *)groupKey, (word32)strlen(groupKey),
                            decodedGroupKey, &decodedKeyLen);
    if (ret != 0)
    {
        Serial.println("[DPS] Failed to decode group key!");
        return false;
    }

    // HMAC-SHA256(groupKey, registrationId)
    unsigned char hmacResult[WC_SHA256_DIGEST_SIZE];
    if (!AzureIoT_HmacSHA256(decodedGroupKey, decodedKeyLen,
                              (const unsigned char *)registrationId,
                              strlen(registrationId),
                              hmacResult, sizeof(hmacResult)))
    {
        Serial.println("[DPS] Failed to derive device key!");
        return false;
    }

    // Base64-encode the derived key (no embedded newlines)
    unsigned char base64Key[64];
    word32 base64Len = sizeof(base64Key);
    ret = Base64_Encode_NoNl(hmacResult, (word32)sizeof(hmacResult),
                              base64Key, &base64Len);
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

    strncpy(derivedKeyBuffer, (const char *)base64Key, derivedKeyBufferSize - 1);
    derivedKeyBuffer[derivedKeyBufferSize - 1] = '\0';

    Serial.println("[DPS] Device key derived successfully");
    return true;
}
