# Audio Library

**Version:** 0.8.1 | **Author:** Microsoft | **Category:** Communication | **Architecture:** stm32f4

Record and play audio using the onboard NAU88C10 codec. Audio is captured in WAV format into a user-provided memory buffer.

---

## Quick Start

```cpp
#include <AudioClassV2.h>

AudioClass& audio = AudioClass::getInstance();
char audioBuffer[32000];

void setup() {
    audio.format(8000, 16);  // 8 kHz, 16-bit
}

void loop() {
    // Record 2 seconds
    audio.startRecord(audioBuffer, sizeof(audioBuffer), 2);
    while (audio.getAudioState() == AUDIO_STATE_RECORDING) {
        delay(100);
    }

    // Play it back
    int size = audio.getCurrentSize();
    audio.startPlay(audioBuffer, size);
    while (audio.getAudioState() == AUDIO_STATE_PLAYING) {
        delay(100);
    }
}
```

---

## API Reference

### Class: `AudioClass`

Access via singleton: `AudioClass& audio = AudioClass::getInstance();`

| Method | Signature | Description |
|--------|-----------|-------------|
| `format` | `void format(unsigned int sampleRate = 8000, unsigned short sampleBitLength = 16)` | Configure sample rate and bit depth |
| `startRecord` | `int startRecord(char* audioFile, int fileSize, int durationInSeconds)` | Record audio into buffer as WAV |
| `startPlay` | `int startPlay(char* audioFile, int size)` | Play WAV data from buffer |
| `stop` | `void stop()` | Stop recording or playing |
| `getAudioState` | `int getAudioState()` | Returns current audio state |
| `getCurrentSize` | `int getCurrentSize()` | Bytes recorded/played so far |
| `getRecordedDuration` | `double getRecordedDuration()` | Duration of recorded audio in seconds |
| `getWav` | `char* getWav(int* fileSize)` | Get pointer to WAV buffer and file size |
| `convertToMono` | `int convertToMono(char* audioFile, int size, int sampleBitLength)` | Convert stereo WAV to mono |

---

## Audio States

| State | Value | Description |
|-------|-------|-------------|
| `AUDIO_STATE_IDLE` | 0 | No operation in progress |
| `AUDIO_STATE_INIT` | 1 | Initializing |
| `AUDIO_STATE_RECORDING` | 2 | Recording in progress |
| `AUDIO_STATE_PLAYING` | 3 | Playback in progress |
| `AUDIO_STATE_RECORDING_FINISH` | 4 | Recording complete |
| `AUDIO_STATE_PLAYING_FINISH` | 5 | Playback complete |

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `DURATION_IN_SECONDS` | 2 | Default recording duration |
| `DEFAULT_SAMPLE_RATE` | 8000 | Default sample rate (Hz) |
| `DEFAULT_BITS_PER_SAMPLE` | 16 | Default bit depth |
| `MONO` | 1 | Mono channel count |
| `STEREO` | 2 | Stereo channel count |
| `WAVE_HEADER_SIZE` | 44 | WAV header size in bytes |
| `BATCH_TRANSMIT_SIZE` | 1024 | Transmit chunk size |

---

## Examples

- **VoiceRecord** — Record and play back audio

---

## Dependencies

- mbed OS
- NAU88C10 codec driver (included)
- STM32 HAL (I2S, DMA, DFSDM)

---

## See Also

- [AudioV2](AudioV2.md) — Enhanced version with callback-based streaming and volume control
