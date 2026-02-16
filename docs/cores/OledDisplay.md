# OLED Display

128×64 pixel OLED display (SSD1306) accessible via the `Screen` global object.

> **Source:** [cores/arduino/OledDisplay.h](../../cores/arduino/OledDisplay.h)

---

## Global Object

```cpp
#include <Arduino.h>

extern OLEDDisplay Screen;
```

---

## Methods

| Method | Description |
|--------|-------------|
| `void init()` | Initialize the OLED display |
| `void clean()` | Clear the display |
| `int print(const char *s, bool wrap = false)` | Print text at current cursor position |
| `int print(unsigned int line, const char *s, bool wrap = false)` | Print text on a specific line (0–3) |
| `void draw(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])` | Draw a bitmap image |

---

## Display Layout

- **Resolution:** 128 × 64 pixels
- **Text lines:** 4 lines (0–3), ~21 characters per line
- **Coordinate system:** Origin at top-left

### `draw()` Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| `x0` | 0–127 | Left edge (column start) |
| `y0` | 0–7 | Top edge (page start, each page = 8 pixels) |
| `x1` | 1–128 | Width in columns |
| `y1` | 1–8 | Height in pages |
| `BMP[]` | — | Pixel byte array: each byte encodes 8 vertical pixels in one column |

---

## Example

```cpp
#include <Arduino.h>

void setup() {
    Screen.init();
    Screen.clean();
    Screen.print(0, "Hello, World!");
    Screen.print(1, "Line 2 text");
}

void loop() {
}
```

---

## See Also

- [Arduino API](ArduinoAPI.md) — Core functions reference
