# TLS Socket Modifications

This document describes the modifications made to the TLS socket layer to improve connection stability for MQTT and other TLS-based protocols.

> **See also**: [Main README](README.md) for general usage and limitations.

---

## Background

The native `TLSSocket` and `TCPSocket` classes return `NSAPI_ERROR_WOULD_BLOCK` frequently on embedded platforms, expecting callers to implement their own retry logic. The original Azure IoT Hub SDK worked reliably because it implemented polling loops with retries internally.

When we removed the Azure IoT Hub SDK and added `WiFiClientSecure`, we needed to replicate this retry behavior in the TLS layer itself.

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

Modified `cores/arduino/TLSSocket.cpp` to use IoT Hub SDK-style polling loops with multiple retries at both the socket callback and TLS method levels.

---

## Implementation Details

### Socket-Level Callbacks

The `ssl_recv` and `ssl_send` callbacks now implement polling loops:

```cpp
// Retry configuration (matches Azure IoT SDK tlsio_mbedtls behavior)
#define SSL_RECV_MAX_RETRIES 50
#define SSL_RECV_RETRY_DELAY_MS 10
#define SSL_SEND_MAX_RETRIES 30
#define SSL_SEND_RETRY_DELAY_MS 10

static int ssl_recv(void *ctx, unsigned char *buf, size_t len) 
{
    TCPSocket *socket = static_cast<TCPSocket *>(ctx);
    int retries = 0;
    
    while (retries < SSL_RECV_MAX_RETRIES)
    {
        int recv_result = socket->recv(buf, len);
        
        if (recv_result > 0) 
            return recv_result;
        else if (recv_result == NSAPI_ERROR_WOULD_BLOCK || recv_result == 0)
        {
            wait_ms(SSL_RECV_RETRY_DELAY_MS);
            retries++;
        }
        else 
            return -1;  // Real error
    }
    
    return MBEDTLS_ERR_SSL_WANT_READ;
}
```

### TLS-Level Methods

The `TLSSocket::send()` and `TLSSocket::recv()` methods also implement retry loops:

```cpp
#define TLS_SEND_MAX_RETRIES 30
#define TLS_RECV_MAX_RETRIES 50
#define TLS_RETRY_DELAY_MS 40

nsapi_size_or_error_t TLSSocket::recv(void *data, nsapi_size_t size)
{
    int retries = 0;
    
    while (retries < TLS_RECV_MAX_RETRIES)
    {
        int ret = mbedtls_ssl_read(&_ssl, ptr + total_recv, size - total_recv);
        
        if (ret > 0) 
            return (nsapi_size_or_error_t)total_recv;
        else if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            wait_ms(TLS_RETRY_DELAY_MS);
            retries++;
        }
        else if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || ret == 0)
            return (nsapi_size_or_error_t)total_recv;  // EOF
        else
            return ret;  // Real error
    }
    
    return (total_recv > 0) ? (nsapi_size_or_error_t)total_recv : NSAPI_ERROR_WOULD_BLOCK;
}
```

---

## Effective Timeouts

| Layer | Operation | Retries | Delay | Max Wait |
|-------|-----------|---------|-------|----------|
| Socket | recv callback | 50 | 10ms | 500ms |
| Socket | send callback | 30 | 10ms | 300ms |
| TLS | `recv()` method | 50 | 40ms | 2000ms |
| TLS | `send()` method | 30 | 40ms | 1200ms |

These values were tuned to balance responsiveness with stability, based on observed behavior of the original Azure IoT Hub SDK on MXChip devices.

---

## Known Limitations

Even with these improvements, **bidirectional MQTT** (subscriptions) may experience instability. The mbedTLS 2.x implementation's handling of idle polls can still trigger false disconnections with some MQTT libraries.

For production use, we recommend **publish-only** patterns when using `WiFiClientSecure`. See the [README](README.md#tlsssl-limitations) for workarounds.
