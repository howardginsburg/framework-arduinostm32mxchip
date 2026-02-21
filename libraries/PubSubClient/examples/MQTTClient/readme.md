# MQTTClient

Connects to an MQTT broker, publishes JSON messages every 10 seconds, and subscribes to a topic to receive messages.

## Configuration

Edit the sketch to set:
- `ssid` / `pass` — your WiFi credentials
- `mqttServer` — the MQTT broker hostname (default: `broker.hivemq.com`)
- `publishTopic` / `subscribeTopic` — MQTT topics

## Running

1. Open the Serial Monitor at 115200 baud.
2. The device connects to WiFi, then to the MQTT broker.
3. Published messages appear on Serial and OLED. Incoming messages on the subscribe topic are also displayed.
