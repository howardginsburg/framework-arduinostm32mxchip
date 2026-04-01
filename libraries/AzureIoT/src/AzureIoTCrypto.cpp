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
#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/wolfcrypt/hmac.h"
#include "wolfssl/wolfcrypt/coding.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

/**
 * Simple raw-base64 decoder for Azure IoT keys.
 *
 * wolfSSL's Base64_Decode is PEM-oriented and assumes newlines at 64-char
 * boundaries.  Azure symmetric keys are raw base64 with no embedded newlines,
 * which can trip the PEM estimator.  This tiny decoder handles standard
 * base64 (RFC 4648) without any line-length assumptions.
 */
static int base64DecodeChar(byte c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static int rawBase64Decode(const byte* in, word32 inLen, byte* out, word32* outLen)
{
    /* Strip trailing padding and whitespace */
    while (inLen > 0 && (in[inLen - 1] == '=' || in[inLen - 1] == '\n' ||
           in[inLen - 1] == '\r' || in[inLen - 1] == ' '))
        inLen--;

    word32 maxOut = *outLen;
    word32 i = 0, o = 0;

    while (i < inLen) {
        /* Skip whitespace */
        if (in[i] == ' ' || in[i] == '\r' || in[i] == '\n') { i++; continue; }

        int a = base64DecodeChar(in[i++]);
        int b = (i < inLen) ? base64DecodeChar(in[i++]) : -1;
        if (a < 0 || b < 0) return -1;

        if (o >= maxOut) return -1;
        out[o++] = (byte)((a << 2) | (b >> 4));

        if (i >= inLen) break;
        if (in[i] == '=') break;
        int c = base64DecodeChar(in[i++]);
        if (c < 0) return -1;
        if (o >= maxOut) return -1;
        out[o++] = (byte)(((b & 0x0F) << 4) | (c >> 2));

        if (i >= inLen) break;
        if (in[i] == '=') break;
        int d = base64DecodeChar(in[i++]);
        if (d < 0) return -1;
        if (o >= maxOut) return -1;
        out[o++] = (byte)(((c & 0x03) << 6) | d);
    }

    *outLen = o;
    return 0;
}

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
    int ret = rawBase64Decode((const byte *)signingKey, (word32)strlen(signingKey),
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
    int ret = rawBase64Decode((const byte *)groupKey, (word32)strlen(groupKey),
                              decodedGroupKey, &decodedKeyLen);
    if (ret != 0)
    {
        Serial.print("[DPS] Failed to decode group key! keyLen=");
        Serial.println((int)strlen(groupKey));
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
