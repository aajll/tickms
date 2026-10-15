# Design

`tickms` is a C11 library for a free-running tick counter. A periodic interrupt advances one 32-bit counter, and inline helpers turn the difference between two counts into elapsed time, timeout decisions, and unit conversions. The library owns no timer hardware and no scheduler.

## Goals and non-goals

Goals:

- Elapsed-time and timeout arithmetic that stays correct across the counter wrap.
- A compile-time tick period with exact integer conversions.
- Concurrent readers alongside one writer, without locks and without libatomic.
- Small, inline, allocation-free helpers usable from interrupt context.

Non-goals:

- Timer hardware setup, interrupt registration, and tickless scheduling.
- Software timer lists, callbacks, and alarms.
- Wall-clock time, calendars, and time zones.
- Several independent counters in one program.
- Sub-tick resolution.

## Counter model

`tickms.c` holds one static `tickms_tick_t` (`uint32_t`) counter. It is private to the module and reached only through the functions in `tickms.h`. The five non-inline functions read or write it; every other function is `static inline` in the header and calls `tickms_get_ticks()`.

The tick source calls `tickms_tick_increment()` once per `TICKMS_MS_PER_TICK` milliseconds, or `tickms_tick_advance(n)` after `n` periods. The counter value has no fixed meaning beyond "ticks since the value passed to `tickms_init`". It wraps from `UINT32_MAX` to zero, which takes 49.7 days at 1 ms per tick and 497 days at the 10 ms default.

### Tick period

`TICKMS_MS_PER_TICK` is a compile-time constant from 1 to 1000 that divides 1000. Three static assertions in the header reject any other value, so `TICKMS_TICKS_PER_SEC` is exact and tick-to-second conversion needs no rounding. The default is 10 ms. Every translation unit and the library build must agree on the period: the counter does not record it, so a mismatch silently scales every conversion.

## Wrap and saturation

Elapsed time is `current - start` in `tickms_tick_t`. Unsigned subtraction is defined modulo 2^32, so the result is the number of ticks between the two counts whether or not the counter wrapped in between. The result is correct for any interval shorter than 2^32 ticks; a longer interval aliases to its remainder. Timeouts compare elapsed ticks with `>=`, never the raw counts, which keeps them correct across the wrap.

Conversions widen to `uint64_t` before multiplying, then clamp:

- Ticks to milliseconds (`tickms_ticks_to_ms`, `tickms_elapsed_ms`, `tickms_current_ms`, `TICKMS_TICKS_TO_MS`) saturates at `UINT32_MAX`, which a counter above 2^32 / `TICKMS_MS_PER_TICK` would otherwise exceed.
- Seconds to ticks (`tickms_sec_to_ticks_saturated`, `TICKMS_SEC_TO_TICKS`) saturates at `UINT32_MAX`.
- Milliseconds to ticks rounds in 64 bits, so `UINT32_MAX` milliseconds converts without wrapping.
- Ticks to seconds and to microseconds cannot overflow their result type and need no clamp.
- `tickms_seconds_to_ticks_nearest` rejects zero, negative, and NaN input before converting, and returns `UINT32_MAX` for any `float` at or above 2^32 ticks. The guard is `>=` because `(float)UINT32_MAX` rounds up to 2^32.

The saturating macros remain constant expressions so that they work in static initialisers. That is why they evaluate their argument more than once.

### Expiry granularity

The counter only changes at tick edges, so a start tick recorded at any moment within a period carries the value of that period's opening edge. `tickms_expired(start, n)` becomes true at the `n`th edge after it, which is more than `n - 1` and at most `n` tick periods of real time later. `tickms_expired_ms` rounds the timeout up to whole ticks, so it never waits fewer ticks than requested, but the first of those ticks can still be partial. A caller that needs a guaranteed minimum adds one tick.

## Concurrency

The contract is one writer and any number of readers:

- Writers are `tickms_init`, `tickms_set_ticks`, `tickms_tick_increment`, and `tickms_tick_advance`. The application calls them from one context, normally the timer interrupt, or serialises them itself. The library has no lock.
- Readers are every other function. A reader may run concurrently with the writer and with other readers.
- The counter is accessed with relaxed ordering. A reader sees a value the counter really held, but the read orders no other memory. Data published alongside a tick needs its own synchronisation.

`tickms_init` is a plain store. Call it before the tick source and readers start, or serialise it against them.

### Atomic backends

All counter accesses go through four macros in `tickms_conf.h`: a qualifier, a load, a store, and a fetch-and-add. One of three backends defines them.

| Backend | Selector | Counter | Operations |
| --- | --- | --- | --- |
| GCC/Clang builtins | `TICKMS_USE_GNU_ATOMICS` | plain `uint32_t` | `__atomic_*` with `__ATOMIC_RELAXED` |
| C11 | `TICKMS_USE_C11_ATOMICS` | `_Atomic uint32_t` | `atomic_*_explicit` with `memory_order_relaxed` |
| Single core | `TICKMS_USE_NO_ATOMICS` | `volatile uint32_t` | plain loads, stores, and `+=` |

Without a selector the header picks the GCC/Clang builtins on GCC or Clang, then C11 atomics when the compiler supports C11 and does not define `__STDC_NO_ATOMICS__`, then the single-core backend. Defining more than one selector is a compile error. The library build and every consumer must see the same selector, because the counter's qualifier is part of how `tickms.c` accesses it.

The two atomic backends require the counter to be lock-free. The builtins backend asserts `__atomic_always_lock_free(sizeof(tickms_tick_t), 0)`. C11 has no size-based query, so the C11 backend matches the counter width to `int`, `long`, or `long long` and requires the matching `ATOMIC_*_LOCK_FREE` macro to be 2. A target without native 32-bit atomics, such as an 8-bit or 16-bit core or ARMv6-M, fails these assertions and must select `TICKMS_USE_NO_ATOMICS`. Without the assertions, every counter access would call into libatomic, which may use a lock, and a lock shared with the timer interrupt can deadlock.

### Single-core backend

`TICKMS_USE_NO_ATOMICS` uses only standard C, for compilers that have neither GCC builtins nor C11 atomics. `volatile` stops the compiler caching or reordering counter accesses but adds no hardware ordering and no atomicity, so the backend is correct only when the readers and the writer run on one core.

A core narrower than 32 bits reads the counter in several accesses. A tick that lands between them produces a value that is half old and half new. `tickms_get_ticks` therefore reads until two consecutive reads agree. A read that overlaps an update differs from the read after it, so two equal reads hold a real counter value unless the writer ran twice within them, which a tick period far longer than two reads rules out. In the common case the loop costs one extra read.

The writer's read-modify-write is not atomic either. If a reader can itself interrupt the writer, for example a higher-priority interrupt that reads ticks while the timer interrupt is mid-update, the reader can still observe a partial store; the application masks the tick update against such readers. Writers must never preempt one another on this backend.

## Portability

- C11 with `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`, and `<limits.h>`. No other library dependencies.
- No `uint8_t`, so the library builds where `CHAR_BIT` is 16.
- Arithmetic is in `tickms_tick_t` or `uint64_t`, so it does not depend on the width of `int`. The exception is `TICKMS_SEC_TO_MS`, which multiplies in the promoted type of its argument; the [API reference](api.md#conversion-macros) gives the rule.
- The `float` helpers need single-precision floating point. A `float` represents tick counts exactly only up to 2^24.

## Verification

The unit suite covers every public function and macro, including wrap across `UINT32_MAX`, saturation boundaries, NaN and negative input to the `float` conversion, and `NULL` start pointers. Separate executables force the C11 and single-core backends, and a configuration test is built at 1 ms, 20 ms, and 250 ms per tick. Configuration asserts that 1, 20, and 250 compile and that 0, 3, and 1001 fail. `src/` and `include/` reach 100 percent line and branch coverage.

A torn-read test replays the values a 16-bit core would read across an update and checks that `tickms_get_ticks` never returns a mixed value. A threaded test runs one producer and several readers on the builtins and C11 backends. Readers must see a non-decreasing count, the final count must equal the sum of all advances, and ThreadSanitizer must report no race. When Clang is available, configuration compiles a probe for the 16-bit `msp430` target and requires both atomic backends to fail with the lock-free diagnostic and the single-core backend to succeed. An installed-consumer test builds a separate project against the `pkg-config` package. A documentation test compiles every C example in the README and `docs/` against the header.

CI compiles the library source with warnings as errors under TI C2000 Code Generation Tools 22.6.2.LTS and 25.11.1.LTS in C11 C28x mode, under both the COFF and EABI ABIs, where the header selects the single-core backend. This validates the source dialect and type assumptions only. Linking, target execution, interrupt latency, and final map-file footprint remain consumer-owned checks.

MISRA C:2023 analysis with `misch` runs on each backend against a recorded baseline in `analysis/baseline/`, and CI fails on any finding outside it. The version macros in `tickms.h` carry an inline deviation of Rule 2.5 (unused macro), because they exist for consumers and the library does not use them.
