# WiFi Library

**Version:** 1.2.6 | **Author:** Arduino | **Category:** Communication | **Architecture:** stm32f4

Full networking library for the MXChip AZ3166. Provides WiFi station and AP modes, TCP client/server, TLS client (`WiFiClientSecure`), and UDP support.

---

## Quick Start

### Station Mode

```cpp
#include <AZ3166WiFi.h>

void setup() {
    Serial.begin(115200);
    WiFi.begin("MyNetwork", "MyPassword");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}
```

### TLS Client

```cpp
#include <AZ3166WiFi.h>
#include <AZ3166WiFiClientSecure.h>

WiFiClientSecure client;

void setup() {
    WiFi.begin("ssid", "password");
    client.setCACert(rootCA);
    client.setCertificate(clientCert);
    client.setPrivateKey(clientKey);
    client.connect("secure.example.com", 443);
}
```

---

## API Reference

### WiFiClass (Global Instance: `WiFi`)

#### Connection Management

| Method | Signature | Description |
|--------|-----------|-------------|
| `begin` | `int begin()` | Connect using EEPROM credentials |
| `begin` | `int begin(char* ssid)` | Connect to open network |
| `begin` | `int begin(char* ssid, const char* passphrase)` | Connect with password |
| `beginAP` | `int beginAP(char* ssid, const char* passphrase)` | Start access point |
| `disconnect` | `int disconnect()` | Disconnect station |
| `disconnectAP` | `int disconnectAP()` | Stop access point |

#### Network Information

| Method | Signature | Description |
|--------|-----------|-------------|
| `localIP` | `IPAddress localIP()` | Get assigned IP address |
| `subnetMask` | `IPAddress subnetMask()` | Get subnet mask |
| `gatewayIP` | `IPAddress gatewayIP()` | Get gateway IP |
| `macAddress` | `unsigned char* macAddress(unsigned char* mac)` | Get MAC address |
| `SSID` | `const char* SSID()` | Get connected network SSID |
| `BSSID` | `unsigned char* BSSID(unsigned char* bssid)` | Get AP MAC address |
| `RSSI` | `int RSSI()` | Signal strength (dBm) |
| `encryptionType` | `int encryptionType()` | Current encryption type |
| `status` | `unsigned char status()` | Connection status |
| `firmwareVersion` | `const char* firmwareVersion()` | Get firmware version string |

#### Network Scanning

| Method | Signature | Description |
|--------|-----------|-------------|
| `scanNetworks` | `int scanNetworks()` | Scan for available networks |
| `SSID` | `const char* SSID(unsigned char networkItem)` | Get scanned network SSID |
| `encryptionType` | `int encryptionType(unsigned char networkItem)` | Get scanned network encryption |
| `RSSI` | `int RSSI(unsigned char networkItem)` | Get scanned network signal strength |

---

### WiFiClient

TCP client implementing the Arduino `Client` interface.

| Method | Signature | Description |
|--------|-----------|-------------|
| `connect` | `int connect(const char* host, uint16_t port)` | TCP connect by hostname |
| `connect` | `int connect(IPAddress ip, uint16_t port)` | TCP connect by IP |
| `write` | `size_t write(uint8_t byte)` | Send one byte |
| `write` | `size_t write(const uint8_t* buf, size_t size)` | Send buffer |
| `available` | `int available()` | Bytes available to read |
| `read` | `int read()` | Read one byte |
| `read` | `int read(uint8_t* buf, size_t size)` | Read into buffer |
| `peek` | `int peek()` | Peek at next byte |
| `flush` | `void flush()` | Flush send buffer |
| `stop` | `void stop()` | Close connection |
| `connected` | `uint8_t connected()` | Check if connected |

---

### WiFiClientSecure

TLS client for secure connections. Extends the `Client` interface with certificate configuration.

| Method | Signature | Description |
|--------|-----------|-------------|
| `setCACert` | `void setCACert(const char* rootCA)` | Set root CA for server verification |
| `setCertificate` | `void setCertificate(const char* clientCert)` | Set client certificate (mTLS) |
| `setPrivateKey` | `void setPrivateKey(const char* privateKey)` | Set client private key (mTLS) |
| `setInsecure` | `void setInsecure()` | Skip certificate verification (not recommended) |
| `setTimeout` | `void setTimeout(unsigned int timeout)` | Set socket timeout (ms) |
| All `Client` methods | — | `connect`, `write`, `read`, `available`, `stop`, etc. |

---

### WiFiServer

TCP server extending `Print`.

| Method | Signature | Description |
|--------|-----------|-------------|
| Constructor | `WiFiServer(unsigned short port)` | Create server on port |
| `begin` | `void begin()` | Start listening |
| `available` | `WiFiClient available()` | Accept incoming connection |
| `setTimeout` | `void setTimeout(int timeout)` | Set accept timeout (0=non-blocking, -1=blocking) |
| `close` | `void close()` | Stop server |
| `send` | `void send(int code, char* content_type, const String& content)` | HTTP-style response |
| `write` | `size_t write(uint8_t)` / `size_t write(const uint8_t*, size_t)` | Write data |

---

### WiFiUDP

UDP socket.

| Method | Signature | Description |
|--------|-----------|-------------|
| `begin` | `unsigned int begin(unsigned short port)` | Start listening |
| `stop` | `void stop()` | Stop UDP |
| `beginPacket` | `int beginPacket(const char* host, uint16_t port)` | Start outgoing packet |
| `endPacket` | `int endPacket()` | Send packet |
| `write` | `size_t write(uint8_t)` / `size_t write(const uint8_t*, size_t)` | Write to packet |
| `read` | `int read()` / `int read(uint8_t*, size_t)` | Read from packet |
| `flush` | `void flush()` | Finish reading current packet |
| `remoteIP` | `IPAddress remoteIP()` | Sender IP |
| `remotePort` | `uint16_t remotePort()` | Sender port |

---

## Connection Status Codes

| Constant | Value | Description |
|----------|-------|-------------|
| `WL_NO_SHIELD` | 255 | No WiFi hardware |
| `WL_IDLE_STATUS` | 0 | Idle |
| `WL_NO_SSID_AVAIL` | 1 | Requested SSID not found |
| `WL_SCAN_COMPLETED` | 2 | Scan complete |
| `WL_CONNECTED` | 3 | Connected |
| `WL_CONNECT_FAILED` | 4 | Connection failed |
| `WL_CONNECTION_LOST` | 5 | Connection lost |
| `WL_DISCONNECTED` | 6 | Disconnected |

---

## Encryption Types

| Constant | Value | Description |
|----------|-------|-------------|
| `ENC_TYPE_WEP` | 5 | WEP |
| `ENC_TYPE_TKIP` | 2 | WPA (TKIP) |
| `ENC_TYPE_CCMP` | 4 | WPA2 (AES) |
| `ENC_TYPE_NONE` | 7 | Open (no encryption) |
| `ENC_TYPE_AUTO` | 8 | Auto-detect |

---

## Examples

- **APmodeWiFiConfig** — WiFi access point configuration
- **ConnectWithWPA** — Connect to WPA network
- **ScanNetworks** — Scan and list available networks
- **SimpleWebServerWiFi** — Simple web server
- **WiFiUdpNtpClient** — NTP time sync over UDP
- **WiFiUdpSendReceiveString** — UDP send/receive
- **WiFiWebClient** — HTTP client
- **WiFiWebServer** — HTTP server

---

## Dependencies

- Arduino core
- mbed OS (`TCPSocket`, `TCPServer`, `UDPSocket`)
- TLSSocket (for `WiFiClientSecure`)
