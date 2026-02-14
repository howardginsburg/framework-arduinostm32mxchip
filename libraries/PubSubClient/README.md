# PubSubClient

An MQTT client library for Arduino, used in this framework to connect the MXChip AZ3166 to Azure IoT Hub over MQTT.

## Original Source

This library is based on the **PubSubClient** library by **Nick O'Leary**.

- **Author:** Nick O'Leary ([knolleary.net](http://knolleary.net))
- **Repository:** [https://github.com/knolleary/pubsubclient](https://github.com/knolleary/pubsubclient)
- **License:** MIT

## Overview

PubSubClient provides a lightweight MQTT 3.1.1 client implementation for Arduino. It supports:

- MQTT connect, disconnect, publish, subscribe, and unsubscribe
- QoS 0 and QoS 1 message delivery
- Keep-alive and automatic ping
- Last Will and Testament (LWT) messages
- Configurable buffer size, keep-alive interval, and socket timeout
- Streaming publish for arbitrarily large payloads via `beginPublish` / `write` / `endPublish`

## Usage in This Framework

The `AzureIoT` library uses PubSubClient as its underlying MQTT transport to communicate with Azure IoT Hub and the Azure Device Provisioning Service. Application code does not typically interact with PubSubClient directly — the `AzureIoTHub` API wraps it.

## Files

| File | Description |
|---|---|
| `src/PubSubClient.h` | Class declaration, MQTT constants, and compile-time configuration defines |
| `src/PubSubClient.cpp` | Full MQTT client implementation |

## Compile-Time Configuration

These defaults can be overridden by defining them before including the header:

| Define | Default | Description |
|---|---|---|
| `MQTT_MAX_PACKET_SIZE` | 256 | Maximum MQTT packet size in bytes |
| `MQTT_KEEPALIVE` | 15 | Keep-alive interval in seconds |
| `MQTT_SOCKET_TIMEOUT` | 15 | Socket read timeout in seconds |
| `MQTT_VERSION` | `MQTT_VERSION_3_1_1` | MQTT protocol version |
