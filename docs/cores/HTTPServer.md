# HTTP Server

Embedded web server used for device configuration UI. Supports WSGI-style handler registration, chunked transfer, and multipart form data.

> **Source:** [cores/arduino/httpserver/](../../cores/arduino/httpserver/)

---

## Quick Start — Configuration Server

```cpp
#include "httpserver/app_httpd.h"

httpd_server_start();  // Start the web configuration UI
// ...
app_httpd_stop();      // Stop the server
```

---

## Core Server API

```cpp
#include "httpserver/httpd.h"
```

| Function | Description |
|----------|-------------|
| `int httpd_init(void)` | Initialize httpd (call first) |
| `int httpd_shutdown(void)` | Shutdown and free resources |
| `int httpd_start(void)` | Start accepting connections |
| `int httpd_stop(void)` | Stop accepting connections |
| `int httpd_is_running(void)` | Non-zero if running |
| `bool httpd_is_https_active(void)` | True if HTTPS is active |

---

## WSGI Handler Registration

```cpp
#include "httpserver/httpd_wsgi.h"
```

### Handler Structure

```c
struct httpd_wsgi_call {
    const char *uri;
    int hdr_fields;
    int http_flags;
    int (*get_handler)(httpd_request_t *req);
    int (*set_handler)(httpd_request_t *req);   // POST
    int (*put_handler)(httpd_request_t *req);
    int (*delete_handler)(httpd_request_t *req);
};
```

### Registration Functions

| Function | Description |
|----------|-------------|
| `int httpd_register_wsgi_handler(struct httpd_wsgi_call *wsgi)` | Register one handler |
| `int httpd_register_wsgi_handlers(struct httpd_wsgi_call *list, int count)` | Register multiple handlers |
| `int httpd_unregister_wsgi_handler(struct httpd_wsgi_call *wsgi)` | Unregister one handler |
| `int httpd_unregister_wsgi_handlers(struct httpd_wsgi_call *list, int count)` | Unregister multiple |

---

## Request Structure

```c
typedef struct {
    int type;                               // HTTPD_REQ_TYPE_GET, POST, PUT, DELETE, HEAD
    char filename[HTTPD_MAX_URI_LENGTH + 1]; // Request URI
    int sock;                               // Socket descriptor
    int remaining_bytes;
    int body_nbytes;
    const struct httpd_wsgi_call *wsgi;
    char content_type[HTTPD_MAX_CONTENT_TYPE_LENGTH];
    bool if_none_match;
    unsigned etag_val;
} httpd_request_t;
```

### Request Types

| Constant | Value |
|----------|-------|
| `HTTPD_REQ_TYPE_GET` | 2 |
| `HTTPD_REQ_TYPE_POST` | 1 |
| `HTTPD_REQ_TYPE_PUT` | 4 |
| `HTTPD_REQ_TYPE_DELETE` | 8 |
| `HTTPD_REQ_TYPE_HEAD` | 16 |

---

## Response Helpers

```cpp
#include "httpserver/httpd_handle.h"
```

| Function | Description |
|----------|-------------|
| `int httpd_send_response(httpd_request_t *req, const char *first_line, char *content, int length, const char *content_type)` | Send complete response |
| `int httpd_send_response_301(httpd_request_t *req, char *location, const char *content_type, char *content, int len)` | Send 301 redirect |
| `int httpd_send(int sock, const char *buf, int len)` | Send raw data |
| `int httpd_send_chunk(int sock, const char *buf, int len)` | Send chunked data (len=0 for last chunk) |
| `int httpd_send_hdr_from_code(int sock, int status_code, enum http_content_type type)` | Send status headers |
| `int httpd_send_header(int sock, const char *name, const char *value)` | Send single header |
| `int httpd_send_all_header(httpd_request_t *req, const char *first_line, int body_length, const char *content_type)` | Send all headers |
| `int httpd_send_body(int sock, const unsigned char *body, uint32_t size)` | Send body data |
| `int httpd_recv(int sock, void *buf, size_t n, int flags)` | Receive data |
| `void httpd_set_error(const char *fmt, ...)` | Set error message |

---

## Data Parsing

```cpp
#include "httpserver/http_parse.h"
#include "httpserver/helper.h"
```

| Function | Description |
|----------|-------------|
| `int httpd_parse_hdr_tags(httpd_request_t *req, int sock, char *scratch, int len)` | Parse HTTP headers |
| `int httpd_get_tag_from_post_data(char *inbuf, const char *tag, char *val, unsigned val_len)` | Extract tag from POST body |
| `int httpd_get_tag_from_url(httpd_request_t *req, const char *tag, char *val, unsigned val_len)` | Extract tag from URL query |
| `int httpd_get_data(httpd_request_t *req, char *content, int length)` | Get POST data body |
| `int httpd_get_tag_from_multipart_form(char *inbuf, char *boundary, const char *tag, char *val, unsigned val_len)` | Extract from multipart form |

---

## Content Types

| Enum | Type |
|------|------|
| `HTTP_CONTENT_PLAIN_TEXT` | text/plain |
| `HTTP_CONTENT_JSON` | application/json |
| `HTTP_CONTENT_XML` | text/xml |
| `HTTP_CONTENT_HTML` | text/html |
| `HTTP_CONTENT_JPEG` | image/jpeg |

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `HTTP_PORT` | 80 | HTTP port |
| `HTTPS_PORT` | 443 | HTTPS port |
| `HTTPD_MAX_MESSAGE` | 1024 | Max request/POST body |
| `HTTPD_MAX_URI_LENGTH` | 64 | Max URI length |
| `HTTPD_MAX_VAL_LENGTH` | 64 | Max tag value length |
| `HTTPD_MAX_CONTENT_TYPE_LENGTH` | 128 | Max content type string |

---

## See Also

- [HTTP Client](HTTPClient.md) — Outbound HTTP requests
- [DeviceConfig](DeviceConfig.md) — Configuration data served by the web UI
- [System Services](SystemServices.md) — `EnableSystemWeb()` / `StartupSystemWeb()`
