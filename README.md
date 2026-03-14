# tickms

Configurable tick-based timer system for time measurement, timeout handling, and duration calculation.

## Features

- **Configurable tick period**: Set tick duration from 1-1000 ms via `TICKMS_MS_PER_TICK` macro
- **Wrap-safe elapsed time**: All elapsed time calculations handle 32-bit wrap-around correctly
- **Multiple time units**: Convert between ticks, milliseconds, microseconds, and seconds
- **Timer utilities**: Start, restart, check expiry, and calculate remaining time
- **Concurrency-safe reads**: Atomic operations protect the tick counter; concurrent readers are safe alongside a single writer
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

To keep configuration separate from application code, you can optionally use
the public configuration header before including `tickms.h`:

```c
#define TICKMS_MS_PER_TICK 1u
#include "tickms_conf.h"
#include "tickms.h"
```

`tickms.h` includes `tickms_conf.h` automatically, so defining
`TICKMS_MS_PER_TICK` before `tickms.h` also works when you prefer a single
include.

### As an installed dependency

If the library is installed system-wide, include the namespaced header path:

```c
#include <tickms/tickms.h>
```

The public configuration header is also installed as:

```c
#include <tickms/tickms_conf.h>
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

The test suite includes core unit tests, configuration-matrix tests for
multiple `TICKMS_MS_PER_TICK` values, and an installed-consumer smoke test
that validates the exported pkg-config package and public headers.

## Quick Start

```c
/* Configure tick period before including tickms.h */
#define TICKMS_MS_PER_TICK 1u
#include "tickms_conf.h"
#include "tickms.h"

int main(void)
{
        tickms_tick_t start;

        /* Initialize tickms */
        tickms_init(0u);

        /* Start a tick counter */
        tickms_start(&start);

        /* ... do work ... */

        /* Check if 500 ms has elapsed */
        if (tickms_expired_ms(start, 500u)) {
                /* Timeout occurred */
        }

        /* Get elapsed time in ms */
        uint32_t elapsed = tickms_elapsed_ms(start);

        return 0;
}
```

## API Reference

### Configuration Macros

| Macro | Description |
|-------|-------------|
| `TICKMS_MS_PER_TICK` | Tick period in milliseconds (configured via `tickms_conf.h`, default: 10) |
| `TICKMS_MS_PER_SEC` | Milliseconds per second (1000) |
| `TICKMS_TICKS_PER_SEC` | Ticks per second (computed) |

### Public Configuration Header

```c
#define TICKMS_MS_PER_TICK 1u
#include "tickms_conf.h"
#include "tickms.h"
```

### Type Definitions

| Type | Description |
|------|-------------|
| `tickms_tick_t` | 32-bit unsigned tick count |

### Conversion Macros

| Macro | Description |
|-------|-------------|
| `TICKMS_MS_TO_TICKS(ms)` | Convert ms to ticks (round up) |
| `TICKMS_SEC_TO_TICKS(sec)` | Convert seconds to ticks |
| `TICKMS_SEC_TO_MS(sec)` | Convert seconds to ms |
| `TICKMS_SEC_TO_US(sec)` | Convert seconds to microseconds |
| `TICKMS_TICKS_TO_MS(ticks)` | Convert ticks to ms |
| `TICKMS_TICKS_TO_US(ticks)` | Convert ticks to microseconds |

### Core Tick Functions

```c
void tickms_init(tickms_tick_t initial_ticks);
void tickms_set_ticks(tickms_tick_t ticks);
tickms_tick_t tickms_get_ticks(void);
tickms_tick_t tickms_tick_increment(void);
tickms_tick_t tickms_tick_advance(tickms_tick_t delta);
```

### Elapsed Time Functions

```c
static inline uint32_t tickms_elapsed_ms(tickms_tick_t start_ticks);
static inline uint64_t tickms_elapsed_us(tickms_tick_t start_ticks);
static inline uint32_t tickms_elapsed_sec(tickms_tick_t start_ticks);
static inline tickms_tick_t tickms_elapsed_ticks(tickms_tick_t start_ticks);
```

### Tick Control Functions

```c
static inline void tickms_start(tickms_tick_t *t);
static inline void tickms_restart(tickms_tick_t *t);
static inline bool tickms_expired(tickms_tick_t start, tickms_tick_t timeout);
static inline bool tickms_expired_ms(tickms_tick_t start, uint32_t timeout_ms);
static inline bool tickms_expired_sec(tickms_tick_t start, uint32_t timeout_sec);
static inline tickms_tick_t tickms_remaining(tickms_tick_t start, tickms_tick_t timeout);
static inline bool tickms_has_elapsed_since(tickms_tick_t start_ticks, tickms_tick_t duration_ticks);
```

### Conversion Functions

```c
static inline uint32_t tickms_ticks_to_ms(tickms_tick_t ticks);
static inline float tickms_ticks_to_seconds_f(tickms_tick_t ticks);
static inline float tickms_elapsed_seconds_since(tickms_tick_t start_ticks);
static inline tickms_tick_t tickms_ms_to_ticks_floor(uint32_t ms);
static inline tickms_tick_t tickms_ms_to_ticks_ceil(uint32_t ms);
static inline tickms_tick_t tickms_sec_to_ticks_saturated(uint32_t seconds);
static inline tickms_tick_t tickms_seconds_to_ticks_nearest(float seconds);
```

### Current Time Functions

```c
static inline uint32_t tickms_current_ms(void);
static inline uint64_t tickms_current_us(void);
static inline uint32_t tickms_current_sec(void);
```

## Use Cases

- **Operation timeout**: Monitor if an operation exceeds a time limit
  ```c
  tickms_tick_t start;
  tickms_start(&start);
  while (!tickms_expired_ms(start, 1000u)) {
          /* wait for operation */
  }
  ```

- **Periodic task execution**: Run code at regular intervals
  ```c
  tickms_tick_t last = tickms_get_ticks();
  while (1) {
          if (tickms_elapsed_ms(last) >= 100u) {
                  periodic_task();
                  last = tickms_get_ticks();
          }
  }
  ```

- **Duration measurement**: Measure how long an operation takes
  ```c
  tickms_tick_t start;
  tickms_start(&start);
  expensive_operation();
  uint32_t duration = tickms_elapsed_ms(start);
  ```

- **Debouncing**: Ignore rapid successive events
  ```c
  static tickms_tick_t last_event = 0;
  if (tickms_elapsed_ms(last_event) > DEBOUNCE_MS) {
          handle_event();
          last_event = tickms_get_ticks();
  }
  ```

## Notes

| Topic | Note |
|-------|------|
| **Tick period** | Must divide 1000 evenly (1, 2, 4, 5, 10, 20, 25, 50, 100, 200, 250, 500, 1000 ms) |
| **Wrap-around** | All elapsed time functions handle 32-bit overflow correctly |
| **Thread safety** | Atomic operations protect the tick counter; safe for concurrent reads with a single writer — multiple concurrent writers must be externally serialized |
| **Saturation** | Conversion functions saturate at UINT32_MAX instead of wrapping |
| **Initialization** | Call `tickms_init()` once before any other tick functions |
| **Tick updates** | Increment ticks from a periodic interrupt or tick callback |
