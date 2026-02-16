# Arduino API Reference

The standard Arduino API as implemented on the MXChip AZ3166. These functions are included via `Arduino.h`.

> **Source:** [cores/arduino/](../../cores/arduino/)

---

## Digital I/O

| Function | Description |
|----------|-------------|
| `void pinMode(uint32_t pin, uint32_t mode)` | Configure pin as `INPUT`, `OUTPUT`, `INPUT_PULLUP`, or `INPUT_PULLDOWN` |
| `void digitalWrite(uint32_t pin, uint32_t val)` | Write `HIGH` or `LOW` to a digital pin |
| `int digitalRead(uint32_t pin)` | Read `HIGH` or `LOW` from a digital pin |

### Pin Mode Constants

| Constant | Value |
|----------|-------|
| `INPUT` | 0x1 |
| `OUTPUT` | 0x2 |
| `INPUT_PULLUP` | 0x3 |
| `INPUT_PULLDOWN` | 0x5 |

---

## Analog I/O

| Function | Description |
|----------|-------------|
| `uint32_t analogRead(uint32_t pin)` | Read analog value (0–1023 default) |
| `void analogWrite(uint32_t pin, uint32_t value)` | Write PWM value (0–255 default) |
| `void analogReference(eAnalogReference mode)` | Set reference voltage (only `AR_DEFAULT`) |
| `void analogReadResolution(int res)` | Set ADC resolution (default 10 bits) |
| `void analogWriteResolution(int res)` | Set PWM resolution (default 8 bits) |
| `void analogOutputInit(void)` | Initialize analog output subsystem |

### Analog Constants

| Constant | Value |
|----------|-------|
| `ADC_RESOLUTION` | 10 |
| `PWM_RESOLUTION` | 8 |
| `PWM_FREQUENCY` | 1000 |
| `PWM_MAX_DUTY_CYCLE` | 255 |

---

## Timing

| Function | Description |
|----------|-------------|
| `unsigned long millis(void)` | Milliseconds since boot (overflows ~50 days) |
| `uint32_t micros(void)` | Microseconds since boot (overflows ~70 min) |
| `void delay(uint32_t ms)` | Pause for milliseconds |
| `void delayMicroseconds(uint32_t us)` | Pause for microseconds |

### Clock Macros

| Macro | Description |
|-------|-------------|
| `F_CPU` | System core clock frequency |
| `clockCyclesPerMicrosecond()` | Clock cycles per µs |
| `clockCyclesToMicroseconds(a)` | Convert cycles to µs |
| `microsecondsToClockCycles(a)` | Convert µs to cycles |

---

## Interrupts

| Function | Description |
|----------|-------------|
| `int attachInterrupt(PinName pin, Callback<void()> ISR, int mode)` | Attach interrupt callback. Returns 0 on success, -1 on fail. |
| `int detachInterrupt(PinName pin)` | Detach interrupt. Returns 0 on success, -1 on fail. |
| `interrupts()` | Enable interrupts (macro for `__enable_irq()`) |
| `noInterrupts()` | Disable interrupts (macro for `__disable_irq()`) |

### Interrupt Modes

| Constant | Value |
|----------|-------|
| `CHANGE` | 2 |
| `FALLING` | 3 |
| `RISING` | 4 |

### Valid Interrupt Pins

`PA_4`, `PA_5`, `PA_10`, `PB_0`, `PB_2`, `PB_3`, `PB_6`, `PB_7`, `PB_8`, `PB_9`, `PB_13`, `PB_14`, `PB_15`

---

## Serial (UART)

The global `Serial` object is a `UARTClass` instance (inherits `Stream`).

| Method | Description |
|--------|-------------|
| `Serial.begin(uint32_t baudRate)` | Initialize UART at given baud rate |
| `Serial.end()` | Close UART |
| `Serial.available()` | Bytes available in receive buffer |
| `Serial.read()` | Read one byte (-1 if none) |
| `Serial.peek()` | Peek at next byte |
| `Serial.write(uint8_t c)` | Write a single byte |
| `Serial.write(const uint8_t *buf, size_t size)` | Write a buffer |
| `Serial.flush()` | Flush transmit buffer |

Receive buffer size: `UART_RCV_SIZE` = 256

---

## Print

Base class for text output. `Serial`, `Screen`, and other output classes inherit from `Print`.

| Method | Description |
|--------|-------------|
| `print(val)` | Print a value (String, int, float, char, etc.) |
| `print(val, base)` | Print with base (`DEC`, `HEX`, `OCT`, `BIN`) |
| `println(val)` | Print value followed by `\r\n` |
| `println()` | Print blank line |
| `printf(format, ...)` | Formatted print (printf-style) |
| `write(uint8_t)` | Write a single byte |
| `write(buf, size)` | Write a buffer |

### Print Base Constants

| Constant | Value |
|----------|-------|
| `BASE_DEC` | 10 |
| `BASE_HEX` | 16 |
| `BASE_OCT` | 8 |
| `BASE_BIN` | 2 |

---

## Stream

Extends `Print` with input parsing. Base class for `Serial` and network clients.

### Core Methods (pure virtual)

| Method | Description |
|--------|-------------|
| `int available()` | Bytes available to read |
| `int read()` | Read single byte (-1 if none) |
| `int peek()` | Peek at next byte |
| `void flush()` | Flush output |

### Timeout

| Method | Description |
|--------|-------------|
| `void setTimeout(unsigned long timeout)` | Set read timeout in ms (default 1000) |

### Parsing

| Method | Description |
|--------|-------------|
| `bool find(const char *target)` | Read until target found |
| `bool findUntil(const char *target, const char *terminator)` | Find target, stop at terminator |
| `long parseInt()` | Parse next integer |
| `float parseFloat()` | Parse next float |

### Reading

| Method | Description |
|--------|-------------|
| `size_t readBytes(char *buf, size_t len)` | Read bytes into buffer (with timeout) |
| `size_t readBytesUntil(char term, char *buf, size_t len)` | Read until terminator |
| `String readString()` | Read all available data as String |
| `String readStringUntil(char term)` | Read as String until terminator |

---

## Math

| Function | Description |
|----------|-------------|
| `long random(long max)` | Random number in [0, max) |
| `long random(long min, long max)` | Random number in [min, max) |
| `void randomSeed(uint32_t seed)` | Seed the RNG |
| `long map(long value, long fromLow, long fromHigh, long toLow, long toHigh)` | Re-map a value between ranges |
| `uint16_t makeWord(uint8_t h, uint8_t l)` | Combine bytes into 16-bit word |

### Utility Macros

| Macro | Description |
|-------|-------------|
| `min(a, b)` | Minimum of two values |
| `max(a, b)` | Maximum of two values |
| `abs(x)` | Absolute value |
| `constrain(amt, low, high)` | Clamp value to range |
| `sq(x)` | Square of x |
| `radians(deg)` / `degrees(rad)` | Angle conversion |
| `lowByte(w)` / `highByte(w)` | Extract bytes |
| `bitRead(val, bit)` / `bitSet(val, bit)` / `bitClear(val, bit)` / `bitWrite(val, bit, bitval)` | Bit manipulation |
| `bit(b)` | Compute `1 << b` |

### Math Constants

| Constant | Value |
|----------|-------|
| `PI` | 3.14159265… |
| `HALF_PI` | 1.57079632… |
| `TWO_PI` | 6.28318530… |
| `DEG_TO_RAD` | 0.01745329… |
| `RAD_TO_DEG` | 57.2957795… |
| `EULER` | 2.71828182… |

---

## String Class

Arduino `String` class with dynamic memory management.

### Constructors

```cpp
String(const char *cstr = "");
String(const String &str);
String(char c);
String(int val, unsigned char base = 10);
String(unsigned int val, unsigned char base = 10);
String(long val, unsigned char base = 10);
String(unsigned long val, unsigned char base = 10);
String(float val, unsigned char decimalPlaces = 2);
String(double val, unsigned char decimalPlaces = 2);
```

### Memory

| Method | Description |
|--------|-------------|
| `unsigned char reserve(unsigned int size)` | Pre-allocate buffer |
| `unsigned int length()` | String length |

### Modification

| Method | Description |
|--------|-------------|
| `concat(val)` / `operator+=` | Append value |
| `replace(find, replace)` | Replace all occurrences |
| `remove(index)` / `remove(index, count)` | Remove characters |
| `toLowerCase()` / `toUpperCase()` | Case conversion |
| `trim()` | Remove leading/trailing whitespace |

### Search

| Method | Description |
|--------|-------------|
| `indexOf(ch)` / `indexOf(str)` | First occurrence |
| `lastIndexOf(ch)` / `lastIndexOf(str)` | Last occurrence |
| `substring(begin)` / `substring(begin, end)` | Extract substring |
| `startsWith(prefix)` / `endsWith(suffix)` | Prefix/suffix test |

### Comparison

`compareTo()`, `equals()`, `equalsIgnoreCase()`, and operators `==`, `!=`, `<`, `>`, `<=`, `>=`

### Access

| Method | Description |
|--------|-------------|
| `charAt(index)` / `operator[]` | Character access |
| `c_str()` | C-string pointer |
| `toCharArray(buf, size)` | Copy to char buffer |
| `toInt()` / `toFloat()` | Parse as number |

### Flash String Macros

| Macro | Description |
|-------|-------------|
| `F(string_literal)` | Store string in flash |
| `FPSTR(pstr_pointer)` | Cast to `__FlashStringHelper*` |

---

## Character Functions

All take `int c` and return `boolean`:

| Function | Description |
|----------|-------------|
| `isAlphaNumeric(c)` | Letter or digit |
| `isAlpha(c)` | Letter |
| `isDigit(c)` | Decimal digit |
| `isHexadecimalDigit(c)` | Hex digit |
| `isUpperCase(c)` / `isLowerCase(c)` | Case check |
| `isPrintable(c)` | Printable character |
| `isSpace(c)` / `isWhitespace(c)` | Whitespace |
| `isControl(c)` / `isPunct(c)` / `isGraph(c)` | Character class |
| `isAscii(c)` | ASCII range (0–127) |
| `toAscii(c)` | Mask to 7-bit ASCII |
| `toLowerCase(c)` / `toUpperCase(c)` | Case conversion |

---

## Type Aliases

```cpp
typedef bool boolean;
typedef uint8_t byte;
typedef unsigned int word;
```

---

## See Also

- [OLED Display](OledDisplay.md) — `Screen` global object
- [EEPROM](EEPROM.md) — Secure element storage
- [Watchdog](Watchdog.md) — Hardware watchdog timer
- [System Services](SystemServices.md) — WiFi, OTA, boot functions
