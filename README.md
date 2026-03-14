# tickms

Configurable tick-based timer system for time measurement, timeout handling, and, duration calculation.

## Features

- **Configurable tick period**: Set tick duration from 1-1000 ms via `TIMER_MS_PER_TICK` macro
- **Wrap-safe elapsed time**: All elapsed time calculations handle 32-bit wrap-around correctly
- **Multiple time units**: Convert between ticks, milliseconds, microseconds, and seconds
- **Timer utilities**: Start, restart, check expiry, and calculate remaining time
- **Thread-safe operations**: Uses atomic operations for tick counter access
- **Compile-time validation**: Static assertions catch invalid configurations at compile time
- **Compliance-aware design goals**: Small auditable codebase with static allocation, explicit contracts, and unit-test coverage.

## Using the Library

### As a Meson subproject

```meson
tickms_dep = dependency('tickms', fallback: ['tickms', 'tickms_dep'])
```

The project also exports `meson.override_dependency('tickms', ...)`
so downstream Meson builds can resolve the subproject dependency by name.

For subproject builds, include the public header directly:

```c
#include "tickms.h"
```

### As an installed dependency

If the library is installed system-wide, include the namespaced header path:

```c
#include <tickms/tickms.h>
```

If `pkg-config` files are installed in your environment, downstream builds can
also discover the package as `tickms`.

The generated version header is available as `tickms_version.h` in the
build tree and as `<tickms/tickms_version.h>` after install.

## Building

```sh
# Library only (release)
meson setup build --buildtype=release -Dbuild_tests=false
meson compile -C build

# With unit tests
meson setup build --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

## Quick Start

```c
/* Set tick period to 1 ms BEFORE including the header */
#define TIMER_MS_PER_TICK 1u
#include "tickms.h"

int main(void)
{
        timer_tick_t start;

        /* Initialize timer */
        timer_init(0u);

        /* Start a timer */
        timer_start(&start);

        /* ... do work ... */

        /* Check if 500 ms has elapsed */
        if (timer_expired_ms(start, 500u)) {
                /* Timeout occurred */
        }

        /* Get elapsed time in ms */
        uint32_t elapsed = timer_elapsed_ms(start);

        return 0;
}
```

## API Reference

### Configuration Macros

| Macro | Description |
|-------|-------------|
| `TIMER_MS_PER_TICK` | Tick period in milliseconds (define before include, default: 10) |
| `TIMER_MS_PER_SEC` | Milliseconds per second (1000) |
| `TIMER_TICKS_PER_SEC` | Ticks per second (computed) |

### Type Definitions

| Type | Description |
|------|-------------|
| `timer_tick_t` | 32-bit unsigned tick count |

### Conversion Macros

| Macro | Description |
|-------|-------------|
| `TIMER_MS_TO_TICKS(ms)` | Convert ms to ticks (round up) |
| `TIMER_SEC_TO_TICKS(sec)` | Convert seconds to ticks |
| `TIMER_SEC_TO_MS(sec)` | Convert seconds to ms |
| `TIMER_SEC_TO_US(sec)` | Convert seconds to microseconds |
| `TIMER_TICKS_TO_MS(ticks)` | Convert ticks to ms |
| `TIMER_TICKS_TO_US(ticks)` | Convert ticks to microseconds |

### Core Timer Functions

```c
void timer_init(timer_tick_t initial_ticks);
void timer_set_ticks(timer_tick_t ticks);
timer_tick_t timer_get_ticks(void);
timer_tick_t timer_tick_increment(void);
timer_tick_t timer_tick_advance(timer_tick_t delta);
```

### Elapsed Time Functions

```c
static inline uint32_t timer_elapsed_ms(timer_tick_t start_ticks);
static inline uint64_t timer_elapsed_us(timer_tick_t start_ticks);
static inline uint32_t timer_elapsed_sec(timer_tick_t start_ticks);
static inline timer_tick_t timer_elapsed_ticks(timer_tick_t start_ticks);
```

### Timer Control Functions

```c
static inline void timer_start(timer_tick_t *t);
static inline void timer_restart(timer_tick_t *t);
static inline bool timer_expired(timer_tick_t start, timer_tick_t timeout);
static inline bool timer_expired_ms(timer_tick_t start, uint32_t timeout_ms);
static inline bool timer_expired_sec(timer_tick_t start, uint32_t timeout_sec);
static inline timer_tick_t timer_remaining(timer_tick_t start, timer_tick_t timeout);
static inline bool timer_has_elapsed_since(timer_tick_t start_ticks, timer_tick_t duration_ticks);
```

### Conversion Functions

```c
static inline uint32_t timer_ticks_to_ms(timer_tick_t ticks);
static inline float timer_ticks_to_seconds_f(timer_tick_t ticks);
static inline float timer_elapsed_seconds_since(timer_tick_t start_ticks);
static inline timer_tick_t timer_ms_to_ticks_floor(timer_tick_t ms);
static inline timer_tick_t timer_ms_to_ticks_ceil(timer_tick_t ms);
static inline timer_tick_t timer_sec_to_ticks_saturated(uint32_t seconds);
static inline timer_tick_t timer_seconds_to_ticks_nearest(float seconds);
```

### Current Time Functions

```c
static inline uint32_t timer_current_ms(void);
static inline uint64_t timer_current_us(void);
static inline uint32_t timer_current_sec(void);
```

## Use Cases

- **Operation timeout**: Monitor if an operation exceeds a time limit
  ```c
  timer_tick_t start;
  timer_start(&start);
  while (!timer_expired_ms(start, 1000u)) {
          /* wait for operation */
  }
  ```

- **Periodic task execution**: Run code at regular intervals
  ```c
  timer_tick_t last = timer_get_ticks();
  while (1) {
          if (timer_elapsed_ms(last) >= 100u) {
                  periodic_task();
                  last = timer_get_ticks();
          }
  }
  ```

- **Duration measurement**: Measure how long an operation takes
  ```c
  timer_tick_t start;
  timer_start(&start);
  expensive_operation();
  uint32_t duration = timer_elapsed_ms(start);
  ```

- **Debouncing**: Ignore rapid successive events
  ```c
  static timer_tick_t last_event = 0;
  if (timer_elapsed_ms(last_event) > DEBOUNCE_MS) {
          handle_event();
          last_event = timer_get_ticks();
  }
  ```

## Notes

| Topic | Note |
|-------|------|
| **Tick period** | Must divide 1000 evenly (1, 2, 4, 5, 10, 20, 25, 50, 100, 200, 250, 500, 1000 ms) |
| **Wrap-around** | All elapsed time functions handle 32-bit overflow correctly |
| **Thread safety** | Uses atomic operations; safe for multi-threaded environments |
| **Saturation** | Conversion functions saturate at UINT32_MAX instead of wrapping |
| **Initialization** | Call `timer_init()` once before any other timer functions |
| **Tick updates** | Increment ticks from a periodic interrupt or timer callback |
