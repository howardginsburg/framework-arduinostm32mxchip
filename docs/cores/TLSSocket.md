# TLS Socket

Secure TLS socket using mbedTLS. Supports server-only (one-way) and mutual TLS (mTLS) authentication.

> **Source:** [cores/arduino/TLSSocket.h](../../cores/arduino/TLSSocket.h)

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

## See Also

- [HTTP Client](HTTPClient.md) — Higher-level HTTPS client
- [EEPROM](EEPROM.md) — Storing CA certificates and client keys
