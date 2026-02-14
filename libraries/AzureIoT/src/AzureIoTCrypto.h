/*
 * AzureIoTCrypto.h - Cryptographic utilities for Azure IoT
 *
 * SAS token generation, HMAC-SHA256, URL encoding, and group key derivation.
 * Part of the MXChip AZ3166 framework Azure IoT library.
 */

#ifndef AZURE_IOT_CRYPTO_H
#define AZURE_IOT_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

// URL-encode a string (RFC 3986 unreserved characters pass through)
void AzureIoT_UrlEncode(const char* input, char* output, size_t outputSize);

// Compute HMAC-SHA256. Output must be at least 32 bytes.
bool AzureIoT_HmacSHA256(const unsigned char* key, size_t keyLen,
                          const unsigned char* data, size_t dataLen,
                          unsigned char* output, size_t outputSize);

// Generate a SharedAccessSignature SAS token.
// tokenBuffer receives the full "SharedAccessSignature sr=...&sig=...&se=..." string.
bool AzureIoT_GenerateSasToken(const char* resourceUri, const char* signingKey,
                                uint32_t expiryTimeSeconds,
                                char* tokenBuffer, size_t tokenBufferSize);

// Derive a device key from a group enrollment master key.
// Computes HMAC-SHA256(base64decode(groupKey), registrationId) and writes the
// base64-encoded result into derivedKeyBuffer.
bool AzureIoT_DeriveGroupKey(const char* groupKey, const char* registrationId,
                              char* derivedKeyBuffer, size_t derivedKeyBufferSize);

#endif // AZURE_IOT_CRYPTO_H
