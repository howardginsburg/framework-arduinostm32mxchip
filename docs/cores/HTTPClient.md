# HTTP Client

HTTP and HTTPS request client with URL parsing and response handling.

> **Source:** [cores/arduino/httpclient/](../../cores/arduino/httpclient/)

---

## HTTPClient (High-Level)

```cpp
#include "httpclient/http_client.h"
```

### Constructors

```cpp
// Plain HTTP
HTTPClient(http_method method, const char *url,
           Callback<void(const char *at, size_t length)> body_callback = 0);

// HTTPS with CA certificate
HTTPClient(const char *ssl_ca_pem, http_method method, const char *url,
           Callback<void(const char *at, size_t length)> body_callback = 0);
```

### Methods

| Method | Description |
|--------|-------------|
| `const Http_Response* send(const void *body = NULL, int body_size = 0)` | Execute request and return response |
| `void set_header(const char *key, const char *value)` | Set a request header |
| `nsapi_error_t get_error()` | Get error code after failure |

### Http_Response Structure

```c
typedef struct {
    int status_code;
    int body_length;
    const char *status_message;
    const char *body;
    const KEYVALUE *headers;  // Linked list of key-value pairs
} Http_Response;
```

---

## HttpsRequest (Low-Level)

```cpp
#include "httpclient/https_request.h"
```

### Constructor

```cpp
HttpsRequest(NetworkInterface *net_iface, const char *ssl_ca_pem,
             http_method method, const char *url,
             Callback<void(const char *at, size_t length)> body_callback = 0);
```

### Methods

| Method | Description |
|--------|-------------|
| `HttpResponse* send(const void *body = NULL, nsapi_size_t body_size = 0)` | Execute HTTPS request |
| `void set_header(const char *key, const char *value)` | Set request header |
| `nsapi_error_t get_error()` | Get error code |

---

## HttpResponse

| Method | Description |
|--------|-------------|
| `int get_status_code()` | HTTP status code |
| `const char* get_status_message()` | Status message string |
| `const KEYVALUE* get_headers()` | Linked list of headers |
| `const char* get_body()` | Response body |
| `int get_body_length()` | Body length |
| `bool is_message_complete()` | Whether full response was received |

---

## URL Parsing

```cpp
#include "httpclient/http_parsed_url.h"
```

```cpp
ParsedUrl url("https://example.com:8443/api/data?key=val");
url.schema();   // "https"
url.host();     // "example.com"
url.port();     // 8443
url.path();     // "/api/data"
url.query();    // "key=val"
url.userinfo(); // NULL
```

---

## Constants

| Constant | Value |
|----------|-------|
| `HTTP_RECEIVE_BUFFER_SIZE` | 2048 |

---

## Example

```cpp
#include "httpclient/http_client.h"

HTTPClient *client = new HTTPClient(HTTP_GET, "http://httpbin.org/get");
const Http_Response *response = client->send();
if (response) {
    Serial.printf("Status: %d\n", response->status_code);
    Serial.println(response->body);
}
delete client;
```

---

## See Also

- [TLS Socket](TLSSocket.md) — Lower-level TLS connections
- [HTTP Server](HTTPServer.md) — Embedded web server
