# Utilities

Memory debug and diagnostic helpers for runtime analysis.

> **Source:** [system/utilities/mbed_memory_status.h](../../system/utilities/mbed_memory_status.h)

---

## Memory Debug Functions

| Function | Description |
|----------|-------------|
| `void print_current_thread_id(void)` | Print current RTOS thread ID to serial |
| `void print_all_thread_info(void)` | Print info for all active threads |
| `void print_heap_info(void)` | Print heap usage statistics |
| `void print_isr_stack_info(void)` | Print ISR stack usage |
| `void print_heap_and_isr_stack_info(void)` | Print both heap and ISR stack info |

---

## Prerequisites

These functions require the following build defines (both enabled by default in [mbed_config.h](../../system/mbed_config.h)):

| Define | Description |
|--------|-------------|
| `MBED_HEAP_STATS_ENABLED` | Enable heap usage tracking |
| `MBED_STACK_STATS_ENABLED` | Enable stack usage tracking |

---

## See Also

- [mbed OS](MbedOS.md) — Build configuration and RTOS
