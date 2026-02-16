# AudioV2 Library

**Version:** 1.2.0 | **Author:** Microsoft | **Category:** Communication | **Architecture:** stm32f4

Enhanced version of the Audio library with callback-based streaming, volume control, automatic level control (ALC), and direct NAU88C10 register access.

---

## Quick Start

### Callback-Based Streaming

```cpp
#include <AudioClassV2.h>

AudioClass& audio = AudioClass::getInstance();
bool dataReady = false;

void recordCallback() {
    dataReady = true;
}

void setup() {
    audio.format(8000, 16);
    audio.startRecord(recordCallback);
}

void loop() {
    if (dataReady) {
        char buffer[AUDIO_CHUNK_SIZE];
        int len = audio.readFromRecordBuffer(buffer, AUDIO_CHUNK_SIZE);
        // Process audio data...
        dataReady = false;
    }
}
```

### Direct WAV Recording

```cpp
#include <AudioClassV2.h>

AudioClass& audio = AudioClass::getInstance();
char wavBuffer[32000];

void setup() {
    audio.format(8000, 16);
    audio.startRecord(wavBuffer, sizeof(wavBuffer));
}
```

---

## API Reference

### Class: `AudioClass`

Access via singleton: `AudioClass& audio = AudioClass::getInstance();`

#### Configuration

| Method | Signature | Description |
|--------|-----------|-------------|
| `format` | `void format(unsigned int sampleRate = 8000, unsigned short sampleBitLength = 16)` | Configure sample rate and bit depth |
| `stop` | `void stop()` | Stop any audio operation |
| `getAudioState` | `int getAudioState()` | Get current audio state |

#### Callback-Based API

| Method | Signature | Description |
|--------|-----------|-------------|
| `startRecord` | `int startRecord(callbackFunc func = NULL)` | Start recording; callback fires on each chunk |
| `startPlay` | `int startPlay(callbackFunc func = NULL)` | Start playback; callback fires on each chunk |
| `readFromRecordBuffer` | `int readFromRecordBuffer(char* buffer, int length)` | Copy recorded data to application buffer |
| `writeToPlayBuffer` | `int writeToPlayBuffer(char* buffer, int length)` | Supply playback data to driver buffer |

#### Direct WAV API

| Method | Signature | Description |
|--------|-----------|-------------|
| `startRecord` | `int startRecord(char* audioBuffer, int size)` | Record WAV directly to buffer |
| `startPlay` | `int startPlay(char* audioBuffer, int size)` | Play WAV from buffer |
| `getCurrentSize` | `int getCurrentSize()` | Current WAV data size |
| `convertToMono` | `int convertToMono(char* audioBuffer, int size, int sampleBitLength)` | Convert stereo to mono |

#### Codec Control

| Method | Signature | Description |
|--------|-----------|-------------|
| `setVolume` | `bool setVolume(uint8_t volume)` | Set volume (0–100%) |
| `readRegister` | `uint16_t readRegister(uint16_t registerAddress)` | Read NAU88C10 register |
| `writeRegister` | `void writeRegister(uint16_t registerAddress, uint16_t value)` | Write NAU88C10 register |
| `enableLevelControl` | `void enableLevelControl(uint8_t maxGain, uint8_t minGain)` | Enable ALC (gain range 0–7) |
| `disableLevelControl` | `void disableLevelControl()` | Disable ALC |
| `setPGAGain` | `void setPGAGain(uint8_t gain)` | Set PGA gain directly (0–0x3F) |

---

## Callback Type

```cpp
typedef void (*callbackFunc)();
```

The callback is invoked each time `AUDIO_CHUNK_SIZE` (512) bytes of audio data are available (recording) or consumed (playback).

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `AUDIO_CHUNK_SIZE` | 512 | Callback chunk size in bytes |

---

## Examples

- **RecordVoiceToWAV** — Record audio to WAV buffer
- **VoiceRecord** — Record and play back audio

---

## Dependencies

- mbed OS
- NAU88C10 codec driver (included)
- STM32 HAL (I2S, DMA, DFSDM)

---

## See Also

- [Audio](Audio.md) — Simpler API without streaming callbacks
