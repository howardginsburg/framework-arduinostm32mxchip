# WebSocket Library

**Version:** 0.0.1 | **Author:** Microsoft | **Category:** Communication | **Architecture:** stm32f4

WebSocket client (RFC 6455) for non-SSL connections. Supports text and binary messages, ping/pong, connection close, and fragmented messages.

> **Note:** This library does **not** support SSL/TLS. For secure WebSocket connections, use a TLS transport layer separately.

---

## Quick Start

```cpp
#include <AZ3166WiFi.h>
#include <WebSocketClient.h>

WebSocketClient* ws;

void setup() {
    WiFi.begin("ssid", "password");
    ws = new WebSocketClient("ws://echo.example.com:8080/ws");
    ws->connect();
}

void loop() {
    if (ws->connected()) {
        // Send a message
        const char* msg = "Hello WebSocket!";
        ws->send(msg, strlen(msg));

        // Receive response
        char buffer[256];
        WebSocketReceiveResult* result = ws->receive(buffer, sizeof(buffer));
        if (result != NULL && result->length > 0) {
            buffer[result->length] = '\0';
            Serial.println(buffer);
        }
    }
    delay(1000);
}
```

---

## API Reference

### Class: `WebSocketClient`

| Method | Signature | Description |
|--------|-----------|-------------|
| Constructor | `WebSocketClient(char* url)` | URL format: `ws://host[:port]/path` |
| Destructor | `~WebSocketClient()` | Cleanup resources |
| `connect` | `bool connect(int timeout = 10000)` | Connect to WebSocket server |
| `connected` | `bool connected()` | Check if connection is active |
| `send` | `int send(const char* data, long size, WS_Message_Type msgType = WS_Message_Text, bool isFinal = true)` | Send a message |
| `sendPing` | `int sendPing(char* str, int size)` | Send a ping frame |
| `receive` | `WebSocketReceiveResult* receive(char* msgBuffer, int size, int timeout = 10000)` | Receive a message |
| `close` | `bool close()` | Close the connection |
| `getPath` | `const char* getPath()` | Get the URL path |

---

## Message Types

| Enum Value | Description |
|------------|-------------|
| `WS_Message_Text` | Text message (UTF-8) |
| `WS_Message_Binary` | Binary message |
| `WS_Message_Ping` | Ping frame |
| `WS_Message_Pong` | Pong frame |
| `WS_Message_Close` | Close frame |
| `WS_Message_Timeout` | Receive timed out |
| `WS_Message_BufferOverrun` | Buffer too small |

---

## Receive Result

```cpp
struct WebSocketReceiveResult {
    int length;                    // Bytes received
    bool isEndOfMessage;           // True if this is the final fragment
    WS_Message_Type messageType;   // Type of message received
};
```

---

## Close Status Codes

| Code | Constant | Description |
|------|----------|-------------|
| 1000 | `WS_NORMAL` | Normal closure |
| 1001 | `WS_GOING_AWAY` | Endpoint going away |
| 1002 | `WS_PROTOCOL_ERROR` | Protocol error |
| 1003 | `WS_DATA_NOT_ACCEPTED` | Unsupported data type |
| 1005 | `WS_STATUS_NOT_RECEIVED` | No status code |
| 1006 | `WS_ABNORMAL_CLOSE` | Abnormal closure |
| 1007 | `WS_INVALID_PAYLOAD` | Invalid message data |
| 1008 | `WS_POLICY_VIOLATION` | Policy violation |
| 1009 | `WS_MESSAGE_TOO_BIG` | Message too large |

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `TIMEOUT_IN_MS` | 10000 | Default timeout (ms) |

---

## Examples

- **WebSocketEcho** — Connect to a WebSocket echo server

---

## Dependencies

- mbed OS (`TCPSocket`)
- [WiFi](WiFi.md) (`SystemWiFi.h`)
