# TLS Socket Modifications

This document describes the modifications made to the TLS socket layer to improve connection stability for MQTT and other TLS-based protocols.

> **See also**: [Main README](../README.md) for general usage and limitations.

---

## Background

The TLS socket layer has been migrated from **mbedTLS** to **wolfSSL 5.7.6**, adding TLS 1.3 support and using an open-source, source-compiled TLS stack. The underlying `TCPSocket` (mbed NSAPI) still returns `NSAPI_ERROR_WOULD_BLOCK` frequently on embedded platforms, so the custom I/O callbacks retain IoT Hub SDK-style polling loops.

---

## Root Cause Analysis

| Aspect | Original IoT Hub SDK | Native TLSSocket (Before) |
|--------|---------------------|---------------------------|
| **Retry Logic** | Polling loop with 50+ retries | Single retry, 5ms wait |
| **Buffering** | Internal buffer with realloc | None |
| **Timeout Handling** | `ThreadAPI_Sleep()` in loop | Returns `WOULD_BLOCK` immediately |
| **Error Recovery** | Returns `WANT_READ` for retry | Returns error as-is |

---

## Solution

Modified `cores/arduino/TLSSocket.cpp` to use wolfSSL with custom I/O callbacks that implement IoT Hub SDK-style polling loops with internal buffering, while the wolfSSL library handles TLS 1.2/1.3 handshake and record processing.

---

## Implementation Details

### wolfSSL I/O Callbacks

The `wolfssl_recv` and `wolfssl_send` callbacks replace the previous mbedTLS callbacks:

```cpp
static int wolfssl_recv(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    TLSSocket *tls   = static_cast<TLSSocket *>(ctx);
    TCPSocket *socket = tls->_tcp_socket;
    int pending = 0;

    // Fill the internal buffer if it is empty.
    while (tls->_recv_buffer_count == 0)
    {
        unsigned char temp_buf[128];
        int recv_result = socket->recv(temp_buf, sizeof(temp_buf));

        if (recv_result > 0)
        {
            // Buffer received data with realloc
            size_t new_size = tls->_recv_buffer_count + recv_result;
            unsigned char *new_buffer =
                (unsigned char *)realloc(tls->_recv_buffer, new_size);
            if (new_buffer != NULL)
            {
                tls->_recv_buffer = new_buffer;
                memcpy(tls->_recv_buffer + tls->_recv_buffer_count,
                       temp_buf, recv_result);
                tls->_recv_buffer_count = new_size;
            }
            break;
        }
        else if (recv_result == NSAPI_ERROR_WOULD_BLOCK || recv_result == 0)
        {
            if (tls->_handshake_complete)
                break;  // Post-handshake: non-blocking
            else
            {
                // Bounded poll during handshake
                if (pending++ >= HANDSHAKE_TIMEOUT_MS / HANDSHAKE_WAIT_INTERVAL_MS)
                    return WOLFSSL_CBIO_ERR_TIMEOUT;
                wait_ms(HANDSHAKE_WAIT_INTERVAL_MS);
            }
        }
        else
            return WOLFSSL_CBIO_ERR_GENERAL;
    }

    // Serve data from the internal buffer
    // ... returns WOLFSSL_CBIO_ERR_WANT_READ if buffer is empty
}
```

The send callback retries up to 10 times on `NSAPI_ERROR_WOULD_BLOCK`:

```cpp
static int wolfssl_send(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    TLSSocket *tls   = static_cast<TLSSocket *>(ctx);
    TCPSocket *socket = tls->_tcp_socket;

    for (int i = 0; i < 10; ++i)
    {
        int result = socket->send((const unsigned char *)buf, sz);
        if (result > 0)
            return result;
        else if (result == NSAPI_ERROR_WOULD_BLOCK || result == 0)
            wait_ms(100);
        else
            return WOLFSSL_CBIO_ERR_GENERAL;
    }
    return sz;
}
```

### TLS-Level Methods

The `TLSSocket::send()` and `TLSSocket::recv()` methods use wolfSSL APIs:

```cpp
nsapi_size_or_error_t TLSSocket::send(const void *data, nsapi_size_t size)
{
    int out_left = (int)size;
    do
    {
        int ret = wolfSSL_write(_ssl, ptr + (size - out_left), out_left);
        if (ret > 0)
            out_left -= ret;
        else
        {
            int err = wolfSSL_get_error(_ssl, ret);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
                wait_ms(10);
            else
                return WOLFSSL_FATAL_ERROR;
        }
    } while (out_left > 0);
    return (nsapi_size_or_error_t)size;
}

nsapi_size_or_error_t TLSSocket::recv(void *data, nsapi_size_t size)
{
    int ret = wolfSSL_read(_ssl, data, (int)size);
    if (ret > 0)
        return (nsapi_size_or_error_t)ret;
    if (ret == 0)
        return 0;  // graceful close
    int err = wolfSSL_get_error(_ssl, ret);
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        return 0;  // no data yet — caller should retry
    return WOLFSSL_FATAL_ERROR;
}
```

---

## Effective Timeouts

| Layer | Operation | Retries | Delay | Max Wait |
|-------|-----------|---------|-------|----------|
| I/O callback | recv (handshake) | HANDSHAKE_TIMEOUT_MS / HANDSHAKE_WAIT_INTERVAL_MS | 10ms | 5000ms |
| I/O callback | recv (post-handshake) | 1 (non-blocking) | — | — |
| I/O callback | send | 10 | 100ms | 1000ms |
| TLS | `send()` method | until complete | 10ms | unbounded |
| TLS | `recv()` method | 1 (non-blocking) | — | — |

---

## wolfSSL Configuration

The wolfSSL build is configured via `system/wolfssl/user_settings.h`:

| Feature | Setting |
|---------|---------|
| TLS 1.3 | `WOLFSSL_TLS13` |
| TLS 1.2 | Enabled (default) |
| TLS 1.0/1.1 | Disabled (`NO_OLD_TLS`) |
| Key exchange | ECDHE (P-256, P-384), FFDHE-2048 |
| Ciphers | AES-GCM, ChaCha20-Poly1305, AES-CBC |
| Hashes | SHA-256, SHA-384, SHA-512 |
| Client-only | `NO_WOLFSSL_SERVER` |
| Alt cert chains | `WOLFSSL_ALT_CERT_CHAINS` |
| Math | SP assembly for ARM Cortex-M |
| Custom I/O | `WOLFSSL_USER_IO` callbacks |

---

## Migration from mbedTLS

The TLS layer was fully migrated from mbedTLS to wolfSSL 5.7.6. Key differences:

| Aspect | mbedTLS (before) | wolfSSL (now) |
|--------|-----------------|---------------|
| TLS versions | TLS 1.1, 1.2 | TLS 1.2, 1.3 |
| API style | `mbedtls_ssl_*` context structs | wolfSSL context/session (`WOLFSSL_CTX`, `WOLFSSL`) |
| I/O callbacks | `mbedtls_ssl_set_bio()` | `wolfSSL_CTX_SetIORecv/Send()` |
| Error codes | `MBEDTLS_ERR_SSL_*` | `WOLFSSL_CBIO_ERR_*`, `SSL_ERROR_*` |
| Cert loading | `mbedtls_x509_crt_parse()` | `wolfSSL_CTX_load_verify_buffer()` |
| Crypto (SAS tokens) | `mbedtls_md_hmac()`, `mbedtls_base64_encode()` | `wc_HmacSetKey()/wc_HmacUpdate()/wc_HmacFinal()`, `Base64_Encode()` |
| X.509 CN extraction | `mbedtls_x509_crt_parse()` + string parsing | `wc_CertPemToDer()` + `wc_ParseCert()` + `DecodedCert.subjectCN` |
| License | Apache 2.0 | GPLv2+ |

No changes are required in user sketches. The `TLSSocket` and `WiFiClientSecure` public APIs are identical to the previous mbedTLS-backed versions.

> **Note:** The pre-compiled system binaries (`libdevkit-sdk-core-lib.a`, `libstsafe.a`) still link mbedTLS internally. The mbedTLS headers in `system/mbed-os/features/mbedtls/` are retained for those dependencies.

---

## Known Limitations

The polling-based I/O callbacks work well for MQTT publish and subscribe patterns. For very high-throughput bidirectional traffic, the bounded retry loops may need tuning. Adjust `HANDSHAKE_TIMEOUT_MS` and `HANDSHAKE_WAIT_INTERVAL_MS` in `TLSSocket.h` if needed for your environment.

---

## See Also

- [TLS Socket API](cores/TLSSocket.md) — wolfSSL-based TLS socket reference
- [wolfSSL integration](../system/wolfssl/README.md) — wolfSSL configuration and update guide
- [README](../README.md) — General usage
