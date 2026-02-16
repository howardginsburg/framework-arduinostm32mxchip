# PubSubClient Library

**Author:** Nick O'Leary | **License:** MIT

Lightweight MQTT 3.1.1 client for Arduino. Supports publish/subscribe, QoS 0/1, Last Will and Testament (LWT), keep-alive, and streaming publish for large payloads. Used internally by the [AzureIoT](AzureIoT.md) library.

---

## Quick Start

```cpp
#include <AZ3166WiFi.h>
#include <AZ3166WiFiClient.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void messageCallback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("Message on %s: %.*s\n", topic, length, payload);
}

void setup() {
    WiFi.begin("ssid", "password");
    mqtt.setServer("broker.example.com", 1883);
    mqtt.setCallback(messageCallback);
}

void loop() {
    if (!mqtt.connected()) {
        mqtt.connect("myDevice");
        mqtt.subscribe("commands/#");
    }
    mqtt.publish("telemetry", "{\"temp\":22.5}");
    mqtt.loop();
    delay(5000);
}
```

---

## API Reference

### Class: `PubSubClient`

Extends `Print` for streaming publish support.

#### Connection

| Method | Signature | Description |
|--------|-----------|-------------|
| `setServer` | `PubSubClient& setServer(const char* domain, uint16_t port)` | Set broker by hostname |
| `setServer` | `PubSubClient& setServer(IPAddress ip, uint16_t port)` | Set broker by IP |
| `connect` | `boolean connect(const char* id)` | Connect with client ID |
| `connect` | `boolean connect(const char* id, const char* user, const char* pass)` | Connect with credentials |
| `connect` | `boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage)` | Connect with LWT |
| `connect` | `boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage)` | Full connect |
| `connect` | `boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession)` | Full connect with clean session flag |
| `disconnect` | `void disconnect()` | Disconnect from broker |
| `connected` | `boolean connected()` | Check if connected |
| `state` | `int state()` | Get connection state code |

#### Publish / Subscribe

| Method | Signature | Description |
|--------|-----------|-------------|
| `publish` | `boolean publish(const char* topic, const char* payload)` | Publish string |
| `publish` | `boolean publish(const char* topic, const char* payload, boolean retained)` | Publish with retain flag |
| `publish` | `boolean publish(const char* topic, const uint8_t* payload, unsigned int length)` | Publish binary |
| `publish` | `boolean publish(const char* topic, const uint8_t* payload, unsigned int length, boolean retained)` | Publish binary with retain |
| `publish_P` | `boolean publish_P(const char* topic, const uint8_t* payload, unsigned int length, boolean retained)` | Publish from PROGMEM |
| `beginPublish` | `boolean beginPublish(const char* topic, unsigned int plength, boolean retained)` | Start streaming publish |
| `endPublish` | `int endPublish()` | End streaming publish |
| `write` | `size_t write(uint8_t)` | Write single byte (streaming) |
| `write` | `size_t write(const uint8_t* buffer, size_t size)` | Write bytes (streaming) |
| `subscribe` | `boolean subscribe(const char* topic, uint8_t qos = 0)` | Subscribe to topic |
| `unsubscribe` | `boolean unsubscribe(const char* topic)` | Unsubscribe from topic |

#### Configuration

| Method | Signature | Description |
|--------|-----------|-------------|
| `setCallback` | `PubSubClient& setCallback(callback)` | Set message received callback |
| `setClient` | `PubSubClient& setClient(Client& client)` | Set network client |
| `setStream` | `PubSubClient& setStream(Stream& stream)` | Set stream for large payloads |
| `setKeepAlive` | `PubSubClient& setKeepAlive(uint16_t keepAlive)` | Set keep-alive interval (seconds) |
| `setSocketTimeout` | `PubSubClient& setSocketTimeout(uint16_t timeout)` | Set socket timeout (seconds) |
| `setBufferSize` | `boolean setBufferSize(uint16_t size)` | Resize internal buffer |
| `getBufferSize` | `uint16_t getBufferSize()` | Get current buffer size |
| `loop` | `boolean loop()` | Process incoming messages — call frequently |

---

## Callback Signature

```cpp
void callback(char* topic, byte* payload, unsigned int length);
```

---

## Connection State Codes

| Constant | Value | Description |
|----------|-------|-------------|
| `MQTT_CONNECTION_TIMEOUT` | -4 | Connection timed out |
| `MQTT_CONNECTION_LOST` | -3 | Connection lost |
| `MQTT_CONNECT_FAILED` | -2 | Connection failed |
| `MQTT_DISCONNECTED` | -1 | Disconnected |
| `MQTT_CONNECTED` | 0 | Connected |
| `MQTT_CONNECT_BAD_PROTOCOL` | 1 | Bad protocol version |
| `MQTT_CONNECT_BAD_CLIENT_ID` | 2 | Bad client ID |
| `MQTT_CONNECT_UNAVAILABLE` | 3 | Server unavailable |
| `MQTT_CONNECT_BAD_CREDENTIALS` | 4 | Bad credentials |
| `MQTT_CONNECT_UNAUTHORIZED` | 5 | Not authorized |

---

## Compile-Time Configuration

Override these defaults by defining them before including the header:

| Define | Default | Description |
|--------|---------|-------------|
| `MQTT_MAX_PACKET_SIZE` | 256 | Maximum MQTT packet size (bytes) |
| `MQTT_KEEPALIVE` | 15 | Keep-alive interval (seconds) |
| `MQTT_SOCKET_TIMEOUT` | 15 | Socket timeout (seconds) |
| `MQTT_VERSION` | 4 (MQTT 3.1.1) | Protocol version |

---

## Dependencies

- Arduino core (`Client`, `IPAddress`, `Stream`, `Print`)
