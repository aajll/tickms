# API reference

Every declaration lives in [`include/tickms.h`](../include/tickms.h), with configuration in [`include/tickms_conf.h`](../include/tickms_conf.h). Both carry the full Doxygen contract. The reasons behind these rules are in [design.md](design.md).

## Conventions

- There is one tick counter per program. Functions take tick values, not a handle.
- Elapsed time is `current - start` in unsigned 32-bit arithmetic, so it stays correct across the counter wrap for intervals shorter than 2^32 ticks.
- Functions that return `uint32_t` milliseconds or ticks saturate at `UINT32_MAX` where the true value would not fit. Microsecond results are `uint64_t` and cannot overflow.
- Readers can run in any context while the tick source runs, within the backend limits in [design.md](design.md#concurrency). Writers must not run concurrently with each other.
- There is no `errno`, no callback, and no logging. Pointer arguments are checked for `NULL`, and a `NULL` makes the call do nothing.

## Types and constants

<!-- snippet: skip -->
```c
typedef uint32_t tickms_tick_t;

#define TICKMS_MS_PER_TICK    (10u)    /* configurable, see tickms_conf.h */
#define TICKMS_MS_PER_SEC     (1000u)
#define TICKMS_TICKS_PER_SEC  (TICKMS_MS_PER_SEC / TICKMS_MS_PER_TICK)
#define TICKMS_TICK_RATE_MIN  (1u)
#define TICKMS_TICK_RATE_MAX  (1000u)
```

`tickms_tick_t` counts ticks since an arbitrary origin and wraps from `UINT32_MAX` to zero. `TICKMS_TICKS_PER_SEC` is exact because the tick period must divide 1000. The configuration macros are described in the [integration guide](integration.md#configuration).

## Counter

<!-- snippet: skip -->
```c
void tickms_init(tickms_tick_t initial_ticks);
void tickms_set_ticks(tickms_tick_t ticks);
tickms_tick_t tickms_get_ticks(void);
tickms_tick_t tickms_tick_increment(void);
tickms_tick_t tickms_tick_advance(tickms_tick_t delta);
```

`tickms_init` sets the counter before readers or writers start; call it once during start-up, or serialise it against every other call. `tickms_set_ticks` sets the counter later, for example to align with an external time source. Setting the counter moves every outstanding start tick relative to it, so running timeouts expire early or late.

`tickms_get_ticks` returns a snapshot of the counter. It orders nothing else: a value read alongside it is not guaranteed to be from the same instant. Under `TICKMS_USE_NO_ATOMICS` it reads the counter until two consecutive reads agree.

`tickms_tick_increment` adds one tick and `tickms_tick_advance` adds `delta` ticks, which suits a tickless idle that sleeps for several periods. Both return the new count and wrap at `UINT32_MAX`.

`tickms_init`, `tickms_set_ticks`, `tickms_tick_increment`, and `tickms_tick_advance` are writers. Call them from one context, normally the timer interrupt, or serialise them.

<!-- snippet: body -->
```c
tickms_init(0u);
(void)tickms_tick_increment();     /* from the timer interrupt */
(void)tickms_tick_advance(5u);     /* after five sleeping periods */
```

## Starting and checking a timeout

<!-- snippet: skip -->
```c
void tickms_start(tickms_tick_t *t);
void tickms_restart(tickms_tick_t *t);

bool tickms_expired(tickms_tick_t start, tickms_tick_t timeout);
bool tickms_expired_ms(tickms_tick_t start, uint32_t timeout_ms);
bool tickms_expired_sec(tickms_tick_t start, uint32_t timeout_sec);
bool tickms_has_elapsed_since(tickms_tick_t start_ticks, tickms_tick_t duration_ticks);
tickms_tick_t tickms_remaining(tickms_tick_t start, tickms_tick_t timeout);
```

`tickms_start` and `tickms_restart` store the current count in `*t`. They behave identically; use the name that reads better at the call site.

`tickms_expired` returns `true` once at least `timeout` ticks have elapsed since `start`, so a timeout of zero has always expired. `tickms_has_elapsed_since` is the same test under another name. `tickms_expired_ms` converts the timeout with `tickms_ms_to_ticks_ceil`, so it never expires before the requested number of whole ticks. `tickms_expired_sec` converts with `tickms_sec_to_ticks_saturated`.

A start tick can be recorded just before the counter advances, so the real time at expiry exceeds `timeout - 1` tick periods but can fall short of `timeout` periods. Add one tick when the timeout is a minimum guarantee.

`tickms_remaining` returns the ticks left before `timeout` elapses, or zero once it has.

<!-- snippet: body -->
```c
tickms_tick_t last;

tickms_start(&last);
for (int i = 0; i < 3; i++) {
        if (tickms_expired_ms(last, 100u)) {
                tickms_restart(&last);
        }
}
```

## Elapsed and current time

<!-- snippet: skip -->
```c
tickms_tick_t tickms_elapsed_ticks(tickms_tick_t start_ticks);
uint32_t tickms_elapsed_ms(tickms_tick_t start_ticks);
uint64_t tickms_elapsed_us(tickms_tick_t start_ticks);
uint32_t tickms_elapsed_sec(tickms_tick_t start_ticks);
float tickms_elapsed_seconds_since(tickms_tick_t start_ticks);

uint32_t tickms_current_ms(void);
uint64_t tickms_current_us(void);
uint32_t tickms_current_sec(void);
```

The `elapsed` functions measure from `start_ticks` to the current count. `tickms_elapsed_ms` saturates at `UINT32_MAX`; the second and microsecond forms cannot overflow. Seconds are truncated.

The `current` functions convert the counter itself, measured from its origin (the value passed to `tickms_init`, normally zero). They wrap with the counter, so use them for timestamps and logs, not for intervals.

`tickms_elapsed_seconds_since` returns `float` seconds. A `float` holds integers exactly only up to 2^24, so the result loses precision past 2^24 ticks (about 4.7 hours at 1 ms).

## Conversion functions

<!-- snippet: skip -->
```c
uint32_t tickms_ticks_to_ms(tickms_tick_t ticks);
float tickms_ticks_to_seconds_f(tickms_tick_t ticks);
tickms_tick_t tickms_ms_to_ticks_floor(uint32_t ms);
tickms_tick_t tickms_ms_to_ticks_ceil(uint32_t ms);
tickms_tick_t tickms_sec_to_ticks_saturated(uint32_t seconds);
tickms_tick_t tickms_seconds_to_ticks_nearest(float seconds);
```

`tickms_ticks_to_ms` and `tickms_sec_to_ticks_saturated` saturate at `UINT32_MAX`. `tickms_ms_to_ticks_floor` returns the whole ticks that fit within `ms`, and `tickms_ms_to_ticks_ceil` the fewest ticks that cover it; neither can overflow.

`tickms_seconds_to_ticks_nearest` rounds to the nearest tick, with halves rounded up. It returns zero for zero, negative, and NaN input, and `UINT32_MAX` for anything that would not fit. `tickms_ticks_to_seconds_f` has the same 2^24 precision limit as `tickms_elapsed_seconds_since`.

## Conversion macros

<!-- snippet: skip -->
```c
TICKMS_MS_TO_TICKS(ms)        /* round up, as tickms_ms_to_ticks_ceil     */
TICKMS_SEC_TO_TICKS(sec)      /* saturating, as tickms_sec_to_ticks_saturated */
TICKMS_TICKS_TO_MS(ticks)     /* saturating, as tickms_ticks_to_ms        */
TICKMS_TICKS_TO_US(ticks)     /* uint64_t microseconds                     */
TICKMS_SEC_TO_MS(sec)         /* uint32_t milliseconds                     */
TICKMS_SEC_TO_US(sec)         /* uint64_t microseconds                     */
```

The macros are constant expressions, for static initialisers and array sizes. `TICKMS_SEC_TO_TICKS` and `TICKMS_TICKS_TO_MS` evaluate their argument more than once, so pass arguments without side effects or call the matching function. `TICKMS_SEC_TO_MS` multiplies in the promoted type of `sec` and narrows to `uint32_t`; pass a `uint32_t`, and keep `sec` below 4 294 968 so the result fits.

<!-- snippet: body -->
```c
static const tickms_tick_t debounce_ticks = TICKMS_MS_TO_TICKS(50u);
(void)debounce_ticks;
```

## Version

`tickms.h` defines the release it belongs to, so a copied-in header still identifies itself:

<!-- snippet: skip -->
```c
#define TICKMS_VERSION_MAJOR 1
#define TICKMS_VERSION_MINOR 3
#define TICKMS_VERSION_PATCH 0
#define TICKMS_VERSION "1.3.0"
```

`TICKMS_VERSION_NUMBER` is `MAJOR * 10000 + MINOR * 100 + PATCH` as a `long`, so it holds any version where `int` is 16 bits. Compare it in the preprocessor to require a minimum release:

```c
#if TICKMS_VERSION_NUMBER < 10300L
#error "tickms 1.3.0 or later is required"
#endif
```

`TICKMS_VERSION_STRING` is the same string as `TICKMS_VERSION`. `tickms_version.h` is a deprecated header that includes `tickms.h`, kept so existing includes compile until the next major release.
