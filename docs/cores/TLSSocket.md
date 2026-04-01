# TLS Socket

Secure TLS socket backed by **wolfSSL 5.7.6** (replaces mbedTLS). Supports
server-only (one-way) and mutual TLS (mTLS) authentication with TLS 1.2 and
TLS 1.3.

> **Source:** [cores/arduino/TLSSocket.h](../../cores/arduino/TLSSocket.h)

> **TLS library:** wolfSSL 5.7.6 — see [system/wolfssl/README.md](../../system/wolfssl/README.md)
> for details.

---

## Constructors

```cpp
// Server-only TLS (verify server certificate)
TLSSocket(const char *ssl_ca_pem, NetworkInterface *net_iface);

// Mutual TLS (pass NULL for client cert/key to skip mTLS)
TLSSocket(const char *ssl_ca_pem, const char *ssl_client_cert,
          const char *ssl_client_key, NetworkInterface *net_iface);
```

---

## Methods

| Method | Description |
|--------|-------------|
| `nsapi_error_t connect(const char *host, uint16_t port)` | Connect and perform TLS handshake |
| `nsapi_error_t close()` | Close the connection |
| `nsapi_size_or_error_t send(const void *data, nsapi_size_t size)` | Send data over TLS |
| `nsapi_size_or_error_t recv(void *data, nsapi_size_t size)` | Receive data over TLS |
| `bool isMutualTLS() const` | Returns true if client certificate is configured |

---

## Supported Cipher Suites

The wolfSSL `user_settings.h` enables the following cipher suite families:

| Cipher Suite | Key Exchange | Encryption | MAC |
|-------------|-------------|-----------|-----|
| TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 | ECDHE | AES-128-GCM | AEAD |
| TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 | ECDHE | AES-128-GCM | AEAD |
| TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256 | ECDHE | ChaCha20 | Poly1305 |
| TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256 | ECDHE | ChaCha20 | Poly1305 |
| TLS_AES_128_GCM_SHA256 (TLS 1.3) | ECDHE | AES-128-GCM | AEAD |
| TLS_CHACHA20_POLY1305_SHA256 (TLS 1.3) | ECDHE | ChaCha20 | Poly1305 |

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `TLSIO_RECV_BUFFER_SIZE` | 256 | Internal receive buffer size |
| `HANDSHAKE_TIMEOUT_MS` | 5000 | TLS handshake timeout |
| `HANDSHAKE_WAIT_INTERVAL_MS` | 10 | Handshake retry interval |

---

## Example

```cpp
#include <TLSSocket.h>

const char *ca_cert = "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----\n";

TLSSocket *tlsSocket = new TLSSocket(ca_cert, WiFiInterface());
if (tlsSocket->connect("example.com", 443) == 0) {
    tlsSocket->send("GET / HTTP/1.1\r\n\r\n", 18);
    char buf[256];
    int len = tlsSocket->recv(buf, sizeof(buf));
    tlsSocket->close();
}
delete tlsSocket;
```

---

## Migration from mbedTLS

No changes are required in user sketches.  The `TLSSocket` public API is
identical to the previous mbedTLS-backed version.

The underlying TLS library changed from mbedTLS to wolfSSL 5.7.6 (GPLv2+).
The pre-compiled system binaries (`libdevkit-sdk-core-lib.a`, `libstsafe.a`)
continue to use mbedTLS internally, which is satisfied by the mbedTLS headers
still present in `system/mbed-os/features/mbedtls/`. Your sketch code calls
only wolfSSL for all TLS and crypto operations.

---

## See Also

- [HTTP Client](HTTPClient.md) — Higher-level HTTPS client
- [EEPROM](EEPROM.md) — Storing CA certificates and client keys
- [wolfSSL integration](../../system/wolfssl/README.md) — wolfSSL configuration and update guide
