# Watchdog Timer

Hardware Independent Watchdog Timer (IWDG) for STM32F412. Resets the system if not periodically kicked.

> **Source:** [cores/arduino/Watchdog.h](../../cores/arduino/Watchdog.h)

---

## Usage

```cpp
#include <Watchdog.h>

Watchdog watchdog;
```

---

## Methods

| Method | Description |
|--------|-------------|
| `bool configure(float timeoutInMs)` | Enable the watchdog with a timeout in milliseconds. Returns true on success. |
| `void resetTimer()` | Kick the watchdog to prevent a system reset |
| `bool resetTriggered()` | Returns true if the previous system reset was caused by the watchdog |

---

## Example

```cpp
#include <Watchdog.h>

Watchdog watchdog;

void setup() {
    if (watchdog.resetTriggered()) {
        Serial.println("Watchdog reset detected!");
    }
    watchdog.configure(10000); // 10s timeout
}

void loop() {
    // Normal work...
    watchdog.resetTimer(); // Kick the watchdog
}
```

---

## Notes

- Once enabled, the IWDG cannot be disabled — it can only be reset.
- If `resetTimer()` is not called before the timeout expires, the system resets.
- Use `resetTriggered()` after reboot to detect watchdog-initiated resets.
