# MQTT Client Library for AZ3166

A full-featured MQTT client library with TLS/SSL support for the MXChip AZ3166 IoT DevKit.

## Features

- **Plain MQTT** - Standard MQTT 3.1.1 protocol over TCP (port 1883)
- **TLS with Server Authentication** - Encrypted connection with server certificate verification (port 8883)
- **Mutual TLS (mTLS)** - Client certificate authentication for zero-trust security
- **Flexible Credential Storage**:
  - Compile-time embedded certificates
  - Secure EEPROM storage via STSAFE-A100 secure element
- **Compatible with major cloud platforms**:
  - Azure IoT Hub
  - AWS IoT Core
  - Mosquitto
  - HiveMQ
  - Any MQTT 3.1.1 broker

## Quick Start

### Basic Usage (High-Level API)

```cpp
#include "AZ3166MQTTClient.h"

AZ3166MQTTClient mqtt;

// Unsecured connection
mqtt.connect("broker.example.com", 1883, "device-id", "user", "pass");

// TLS with username/password
mqtt.connectSecure("broker.example.com", 8883, caCert, "device-id", "user", "pass");

// Mutual TLS (client certificate)
mqtt.connectMutualTLS("broker.example.com", 8883, caCert, clientCert, clientKey, "device-id");

// Publish and subscribe
mqtt.subscribe("topic/commands", MQTT::QOS0, messageHandler);
mqtt.publish("topic/telemetry", "{\"temp\":25.5}");

// Main loop
while (true) {
    mqtt.loop();
}
```

### Low-Level API (Paho MQTT)

```cpp
#include "MQTTClient.h"
#include "MQTTNetworkTLS.h"

// Create TLS network transport
MQTTNetworkTLS network(caCert, clientCert, clientKey);
network.connect("broker.example.com", 8883);

// Create MQTT client
MQTT::Client<MQTTNetworkTLS, Countdown> client(network);

// Connect with options
MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
options.clientID.cstring = "device-id";
client.connect(options);
```

## Connection Modes

### 1. Unsecured (TCP)
```cpp
mqtt.connect(host, 1883, clientId, username, password);
```
- Port: 1883
- Use case: Development, trusted networks
- Security: None (not recommended for production)

### 2. Server-Only TLS
```cpp
mqtt.connectSecure(host, 8883, caCert, clientId, username, password);
```
- Port: 8883
- Use case: Production with password authentication
- Security: Encrypted channel, server verification

### 3. Mutual TLS (Certificate Authentication)
```cpp
mqtt.connectMutualTLS(host, 8883, caCert, clientCert, clientKey, clientId);
```
- Port: 8883
- Use case: Zero-trust IoT deployments
- Security: Encrypted channel, bi-directional authentication

## Certificate Management

### Compile-Time Embedding

Override `MQTTCerts.cpp` in your sketch folder:

```cpp
#include "MQTTCerts.h"

const char* MQTT_CA_CERT = "-----BEGIN CERTIFICATE-----\n...";
const char* MQTT_CLIENT_CERT = "-----BEGIN CERTIFICATE-----\n...";
const char* MQTT_CLIENT_KEY = "-----BEGIN RSA PRIVATE KEY-----\n...";
```

### Secure EEPROM Storage

```cpp
#include "EEPROMInterface.h"

EEPROMInterface eeprom;

// Store credentials (provisioning)
eeprom.saveX509Cert(caCertPEM);
eeprom.saveClientCert(clientCertPEM);
eeprom.saveClientKey(clientKeyPEM);
eeprom.saveMQTTAddress("broker.example.com");

// Load at runtime
mqtt.connectFromEEPROM(host, port, clientId, true);  // true = mutual TLS
```

## API Reference

### AZ3166MQTTClient

| Method | Description |
|--------|-------------|
| `connect(host, port, clientId, user, pass)` | Plain MQTT connection |
| `connectSecure(host, port, caCert, clientId, user, pass)` | TLS with password auth |
| `connectMutualTLS(host, port, caCert, clientCert, clientKey, clientId)` | Mutual TLS |
| `connectFromEEPROM(host, port, clientId, useMutualTLS)` | Load certs from EEPROM |
| `subscribe(topic, qos, callback)` | Subscribe to topic |
| `unsubscribe(topic)` | Unsubscribe from topic |
| `publish(topic, payload, len, qos, retained)` | Publish message |
| `loop(timeoutMs)` | Process messages and keepalive |
| `disconnect()` | Disconnect from broker |
| `isConnected()` | Check connection status |

### MQTTNetworkTLS

| Method | Description |
|--------|-------------|
| `MQTTNetworkTLS(caCert)` | Create for server-only TLS |
| `MQTTNetworkTLS(caCert, clientCert, clientKey)` | Create for mutual TLS |
| `connect(hostname, port)` | Connect with TLS handshake |
| `disconnect()` | Close connection |
| `read(buffer, len, timeout)` | Read data |
| `write(buffer, len, timeout)` | Write data |

### EEPROMInterface (New Methods)

| Method | Description |
|--------|-------------|
| `saveClientCert(certPEM)` | Store client certificate |
| `readClientCert(buffer, size)` | Retrieve client certificate |
| `saveClientKey(keyPEM)` | Store client private key |
| `readClientKey(buffer, size)` | Retrieve client private key |

## Examples

- **SecureMQTT** - TLS connection with username/password
- **MutualTLSMQTT** - Client certificate authentication
- **EEPROMCredentials** - Loading credentials from secure storage

## Certificate Size Limits

The STSAFE-A100 secure element has limited storage:

| Storage Zone | Max Size | Usage |
|--------------|----------|-------|
| CA Certificate | 2639 bytes | Zones 0+7+8 combined |
| Client Certificate | 192 bytes | Zone 2 |
| Client Private Key | 880 bytes | Zone 8 |

For larger certificates, consider:
- Using EC (elliptic curve) instead of RSA keys
- Compile-time embedding for CA certificates
- Certificate compression techniques

## Troubleshooting

### Connection Fails
1. Verify WiFi is connected before MQTT
2. Check CA certificate matches broker's certificate chain
3. Ensure port 8883 is not blocked by firewall
4. Verify hostname matches certificate CN/SAN

### Certificate Errors
1. Certificates must be PEM format (text with BEGIN/END markers)
2. Include the full certificate chain if needed
3. Private key must match the client certificate's public key
4. Check certificate expiration dates

### Memory Issues
- Reduce `AZ3166_MQTT_PACKET_SIZE` if needed
- Use QoS0 to avoid message buffering
- Limit concurrent subscriptions

## License

MIT License - Copyright (c) Microsoft Corporation
