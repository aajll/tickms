/**
 * @file
 *    tickms.h
 *
 * @brief
 *    Hardware-agnostic tick-based timer module with configurable tick period.
 *
 * @details
 *    This module provides a configurable tick-based timer system for time
 *    measurement, timeout handling, and duration calculations. It is designed
 *    to be independent of any state machine framework and can be used as a
 *    standalone timer abstraction layer.
 *
 *    Key features:
 *    - Configurable tick period (1-1000 ms)
 *    - Wrap-safe elapsed time calculations
 *    - Time conversion utilities (ms, us, seconds)
 *    - Tick-based timer start/reset/expiry checking
 *    - Producer-side tick update helpers
 *
 *    Concurrency contract:
 *    - One application-defined producer context should advance or set ticks,
 *      or all writer calls must be externally serialized.
 *    - Readers may call query helpers concurrently with the producer.
 *    - Atomic operations protect the tick counter value itself; they do not
 *      impose ordering on unrelated shared application state.
 *
 *    This module is designed with MISRA C:2012 and IEC-61508 in mind:
 *    - Consistent use of fixed-width types
 *    - No implicit type conversions
 *    - Explicit overflow handling
 *    - Clear invariants and preconditions
 *
 *   Example (set tick period to 1 ms):
 *   ----------------------------------
 *   #define TICKMS_MS_PER_TICK 1u
 *   #include "tickms.h"
 *
 * @note
 *   Configure **TICKMS_MS_PER_TICK** before including this file, either
 *   directly or via **tickms_conf.h**; otherwise it defaults to 10 ms.
 *
 */
#ifndef TICKMS_H_
#define TICKMS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ================ INCLUDES ================================================ */

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Include configuration header first */
#include "tickms_conf.h"

/* ================ DEFINES ================================================= */

/* ---------------- Configuration ------------------------------------------- */

/* TICKMS_MS_PER_TICK is configured via tickms_conf.h. */

#if TICKMS_MS_PER_TICK == 0u
#error "TICKMS_MS_PER_TICK must be > 0"
#endif

/**
 * @def TICKMS_MS_PER_SEC
 * @brief The number of ms in a second.
 */
#define TICKMS_MS_PER_SEC    (1000u)

/**
 * @def TICKMS_TICK_RATE_MIN
 * @brief The minimum allowable tick rate.
 */
#define TICKMS_TICK_RATE_MIN (1u)

/**
 * @def TICKMS_TICK_RATE_MAX
 * @brief The maximum allowable tick rate.
 */
#define TICKMS_TICK_RATE_MAX (1000u)

/**
 * @def TICKMS_TICKS_PER_SEC
 * @brief Number of ticks per second (integer, truncated).
 */
#define TICKMS_TICKS_PER_SEC (TICKMS_MS_PER_SEC / TICKMS_MS_PER_TICK)

/* ---------------- Compile-time assertions --------------------------------- */

/* Tick period must be between 1 and 1000 ms */
_Static_assert(TICKMS_MS_PER_TICK >= TICKMS_TICK_RATE_MIN
                   && TICKMS_MS_PER_TICK <= TICKMS_TICK_RATE_MAX,
               "TICKMS_MS_PER_TICK must be in [1,1000]");

/* Tick period must divide 1000 cleanly for integer conversions */
_Static_assert(
    (TICKMS_TICK_RATE_MAX % TICKMS_MS_PER_TICK) == 0u,
    "TICKMS_MS_PER_TICK must divide 1000 (integer conversions rely on it)");

_Static_assert(TICKMS_TICKS_PER_SEC != 0u,
               "TICKMS_TICKS_PER_SEC computed to 0; fix TICKMS_MS_PER_TICK");

/* ================ STRUCTURES ============================================== */

/* ================ TYPEDEFS ================================================ */

/**
 * @brief Tick count type.
 *
 * @details
 *    This is an alias for uint32_t, representing the number of ticks
 *    elapsed since an arbitrary epoch. It is used throughout the tickms
 *    module for all time-related calculations.
 */
typedef uint32_t tickms_tick_t;

/* ================ MACROS ================================================== */

/**
 * @def TICKMS_MS_TO_TICKS(ms)
 * @brief Convert a time expressed in milliseconds to the corresponding number
 *        of ticks, rounding up.
 *
 * @param ms        Time in milliseconds.
 * @return          Minimum number of ticks required to cover `ms`.
 *
 * @note Prefer the corresponding inline helper for runtime values when
 *       overflow handling must be explicit.
 */
#define TICKMS_MS_TO_TICKS(ms)                                                 \
        ((tickms_tick_t)(((ms) + (TICKMS_MS_PER_TICK - 1u))                    \
                         / TICKMS_MS_PER_TICK))

/**
 * @def TICKMS_SEC_TO_TICKS(sec)
 * @brief Convert seconds to ticks (integer, truncating fractional part).
 *
 * @param sec       Time in seconds.
 * @return          Number of ticks.
 *
 * @note Prefer the corresponding inline helper for runtime values when
 *       overflow handling must be explicit.
 */
#define TICKMS_SEC_TO_TICKS(sec)  ((tickms_tick_t)((sec) * TICKMS_TICKS_PER_SEC))

/**
 * @def TICKMS_SEC_TO_MS(sec)
 * @brief Convert seconds to milliseconds (integer, truncating fractional part).
 *
 * @param sec       Time in seconds.
 * @return          Milliseconds.
 */
#define TICKMS_SEC_TO_MS(sec)     ((uint32_t)((sec) * TICKMS_MS_PER_SEC))

/**
 * @def TICKMS_SEC_TO_US(sec)
 * @brief Convert seconds to microseconds (64-bit integer).
 *
 * @param sec       Time in seconds.
 * @return          Microseconds.
 */
#define TICKMS_SEC_TO_US(sec)     ((uint64_t)((sec) * 1000000ull))

/**
 * @def TICKMS_TICKS_TO_MS(ticks)
 * @brief Convert ticks to milliseconds.
 *
 * @param ticks     Number of ticks.
 * @return          Milliseconds.
 */
#define TICKMS_TICKS_TO_MS(ticks) ((uint32_t)((ticks) * (TICKMS_MS_PER_TICK)))

/**
 * @def TICKMS_TICKS_TO_US(ticks)
 * @brief Convert ticks to microseconds.
 *
 * @param ticks     Number of ticks.
 * @return          Microseconds.
 */
#define TICKMS_TICKS_TO_US(ticks)                                              \
        ((uint64_t)((ticks) * (uint64_t)(TICKMS_MS_PER_TICK) * 1000ull))

/* ================ GLOBAL VARIABLES ======================================== */

/* ================ GLOBAL PROTOTYPES ======================================= */

/**
 * @brief Initialize the tickms module.
 *
 * @details
 *    Sets the initial value of the tick counter. This should be called
 *    once during system initialization.
 *
 * @pre
 *    Call before concurrent readers or writers begin using the module, or
 *    externally serialize the call against all other tickms operations.
 *
 * @param initial_ticks  Initial tick count value.
 */
void tickms_init(tickms_tick_t initial_ticks);

/**
 * @brief Set the current tick count.
 *
 * @details
 *    Updates the global tick counter to a specified value. This is useful
 *    for synchronizing the tickms with an external time source or for
 *    testing purposes.
 *
 * @warning
 *    Treat this as a writer operation. Do not mix calls to tickms_set_ticks,
 *    tickms_tick_increment, and tickms_tick_advance from multiple contexts
 *    unless your application serializes them externally.
 *
 * @param ticks  New tick count value.
 */
void tickms_set_ticks(tickms_tick_t ticks);

/**
 * @brief Get the current tick count.
 *
 * @note
 *    This is an atomic snapshot of the counter value only. It does not imply
 *    ordering for unrelated shared application data.
 *
 * @return Current tick count.
 */
tickms_tick_t tickms_get_ticks(void);

/**
 * @brief Advance the global tick count by one tick.
 *
 * @pre
 *    The application provides a single producer for tick advancement, or it
 *    externally serializes all writer operations.
 *
 * @return The updated tick count after the increment.
 */
tickms_tick_t tickms_tick_increment(void);

/**
 * @brief Advance the global tick count by an arbitrary delta.
 *
 * @pre
 *    The application provides a single producer for tick advancement, or it
 *    externally serializes all writer operations.
 *
 * @param delta  Number of ticks to add.
 *
 * @return The updated tick count after the increment.
 */
tickms_tick_t tickms_tick_advance(tickms_tick_t delta);

/**
 * @brief Get elapsed time in milliseconds since a start tick.
 *
 * @details
 *    This function calculates the elapsed time in milliseconds between
 *    a start tick and the current tick count. It handles wrap-around
 *    correctly.
 *
 * @param start_ticks  The recorded start tick.
 *
 * @return             Elapsed time in milliseconds, saturated at UINT32_MAX.
 */
static inline uint32_t
tickms_elapsed_ms(tickms_tick_t start_ticks)
{
        const tickms_tick_t current = tickms_get_ticks();
        const tickms_tick_t elapsed_ticks = current - start_ticks;
        const uint64_t ms =
            (uint64_t)elapsed_ticks * (uint64_t)TICKMS_MS_PER_TICK;
        return (ms > (uint64_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)ms;
}

/**
 * @brief Get elapsed time in microseconds since a start tick.
 *
 * @details
 *    This function calculates the elapsed time in microseconds between
 *    a start tick and the current tick count. It handles wrap-around
 *    correctly.
 *
 * @param start_ticks  The recorded start tick.
 *
 * @return             Elapsed time in microseconds.
 */
static inline uint64_t
tickms_elapsed_us(tickms_tick_t start_ticks)
{
        const tickms_tick_t current = tickms_get_ticks();
        const tickms_tick_t elapsed_ticks = current - start_ticks;
        return ((uint64_t)elapsed_ticks * (uint64_t)TICKMS_MS_PER_TICK
                * 1000ull);
}

/**
 * @brief Get elapsed time in seconds since a start tick.
 *
 * @details
 *    This function calculates the elapsed time in seconds between
 *    a start tick and the current tick count. It handles wrap-around
 *    correctly.
 *
 * @param start_ticks  The recorded start tick.
 *
 * @return             Elapsed time in seconds.
 */
static inline uint32_t
tickms_elapsed_sec(tickms_tick_t start_ticks)
{
        const tickms_tick_t current = tickms_get_ticks();
        const tickms_tick_t elapsed_ticks = current - start_ticks;
        const uint64_t elapsed_ms =
            (uint64_t)elapsed_ticks * (uint64_t)TICKMS_MS_PER_TICK;
        return (uint32_t)(elapsed_ms / (uint64_t)TICKMS_MS_PER_SEC);
}

/**
 * @brief Start a tick counter by recording the current tick count.
 *
 * @param[out] t    Pointer to a tickms_tick_t to store the start tick.
 */
static inline void
tickms_start(tickms_tick_t *t)
{
        if (t != NULL) {
                *t = tickms_get_ticks();
        }
}

/**
 * @brief Restart a tick counter by resetting the tick to now.
 *
 * @param[out] t    Pointer to a tick counter variable to restart.
 */
static inline void
tickms_restart(tickms_tick_t *t)
{
        if (t != NULL) {
                *t = tickms_get_ticks();
        }
}

/**
 * @brief Check if a tick counter has expired based on elapsed ticks.
 *
 * @details
 *    This function checks whether a specified duration has elapsed since
 *    a start tick. It handles wrap-around correctly.
 *
 * @param start     The recorded start tick.
 * @param timeout   The number of ticks to wait.
 *
 * @return          true if the timeout has elapsed, false otherwise.
 */
static inline bool
tickms_expired(tickms_tick_t start, tickms_tick_t timeout)
{
        const tickms_tick_t current = tickms_get_ticks();
        return (current - start) >= timeout;
}

/**
 * @brief Get number of ticks elapsed since a start tick.
 *
 * @details
 *    This function calculates the number of ticks elapsed since a start
 *    tick. It handles wrap-around correctly.
 *
 * @param start_ticks  The recorded start tick.
 *
 * @return             Number of elapsed ticks.
 */
static inline tickms_tick_t
tickms_elapsed_ticks(tickms_tick_t start_ticks)
{
        const tickms_tick_t current = tickms_get_ticks();
        return current - start_ticks;
}

/**
 * @brief Calculate remaining ticks until timeout.
 *
 * @details
 *    This function calculates the number of ticks remaining until a timeout
 *    expires. If the timeout has already expired, it returns 0.
 *
 * @param start     The recorded start tick.
 * @param timeout   Timeout duration in ticks.
 *
 * @return          Number of remaining ticks, or 0 if expired.
 */
static inline tickms_tick_t
tickms_remaining(tickms_tick_t start, tickms_tick_t timeout)
{
        const tickms_tick_t current = tickms_get_ticks();
        const tickms_tick_t elapsed = current - start;
        return (elapsed >= timeout) ? 0u : (timeout - elapsed);
}

/**
 * @brief Convert a tick count to whole milliseconds, saturating at
 *        UINT32_MAX on overflow.
 *
 * @param ticks     Number of ticks to convert.
 *
 * @return          Milliseconds corresponding to `ticks`.
 */
static inline uint32_t
tickms_ticks_to_ms(tickms_tick_t ticks)
{
        const uint64_t ms = (uint64_t)ticks * (uint64_t)TICKMS_MS_PER_TICK;
        return (ms > (uint64_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)ms;
}

/**
 * @brief Convert a tick count to fractional seconds expressed as float.
 *
 * @param ticks     Number of ticks.
 *
 * @return          Seconds as a floating-point value.
 */
static inline float
tickms_ticks_to_seconds_f(tickms_tick_t ticks)
{
        return ((float)ticks * (float)TICKMS_MS_PER_TICK)
               / (float)TICKMS_MS_PER_SEC;
}

/**
 * @brief Get the elapsed time in seconds (floating-point) since a start tick.
 *
 * @param start_ticks  Tick value obtained from tickms_start or similar.
 *
 * @return            Elapsed seconds as float.
 */
static inline float
tickms_elapsed_seconds_since(tickms_tick_t start_ticks)
{
        return tickms_ticks_to_seconds_f(tickms_elapsed_ticks(start_ticks));
}

/**
 * @brief Convert a millisecond count to the corresponding tick count,
 *        rounding down (floor).
 *
 * @param ms        Milliseconds.
 *
 * @return          Number of ticks that fit completely within `ms`.
 */
static inline tickms_tick_t
tickms_ms_to_ticks_floor(uint32_t ms)
{
        return (tickms_tick_t)(ms / TICKMS_MS_PER_TICK);
}

/**
 * @brief Convert a millisecond count to the corresponding tick count,
 *        rounding up (ceil).
 *
 * @param ms        Milliseconds.
 *
 * @return          Minimum number of ticks required to cover `ms`.
 */
static inline tickms_tick_t
tickms_ms_to_ticks_ceil(uint32_t ms)
{
        const uint64_t adjusted =
            (uint64_t)ms + (uint64_t)(TICKMS_MS_PER_TICK - 1u);
        return (tickms_tick_t)(adjusted / (uint64_t)TICKMS_MS_PER_TICK);
}

/**
 * @brief Convert a whole-second duration to ticks with saturation.
 *
 * @param seconds   Whole seconds.
 *
 * @return          Tick count corresponding to `seconds`, saturated at
 *                  `UINT32_MAX` on overflow.
 */
static inline tickms_tick_t
tickms_sec_to_ticks_saturated(uint32_t seconds)
{
        const uint64_t ticks =
            (uint64_t)seconds * (uint64_t)TICKMS_TICKS_PER_SEC;
        return (ticks > (uint64_t)UINT32_MAX) ? UINT32_MAX
                                              : (tickms_tick_t)ticks;
}

/**
 * @brief Convert a time expressed in seconds (float) to the nearest tick count.
 *
 * @details
 *    The result is saturated to UINT32_MAX on overflow and rounded to the
 *    nearest tick (0.5-tick bias).
 *
 * @param seconds   Time in seconds as float.
 *
 * @return          Nearest tick count, or 0 if seconds <= 0.
 */
static inline tickms_tick_t
tickms_seconds_to_ticks_nearest(float seconds)
{
        /* Use !(> 0) instead of (<= 0) so that NaN is caught here too,
         * since all comparisons involving NaN evaluate to false. */
        if (!(seconds > 0.0f)) {
                return 0u;
        }
        const float ticks_f =
            (seconds * (float)TICKMS_MS_PER_SEC) / (float)TICKMS_MS_PER_TICK;
        /* (float)UINT32_MAX rounds up to 2^32, so a guard of > would pass
         * ticks_f == 2^32, which then wraps to 0 on cast.  Use >= instead. */
        if (ticks_f >= (float)UINT32_MAX) {
                return UINT32_MAX;
        }
        return (tickms_tick_t)(ticks_f + 0.5f);
}

/**
 * @brief Test whether a given duration (in ticks) has elapsed since a start
 *        tick.
 *
 * @param start_ticks     Start tick value.
 * @param duration_ticks  Duration to compare, in ticks.
 *
 * @return                true if the elapsed time is greater than or equal to
 *                        `duration_ticks`, false otherwise.
 */
static inline bool
tickms_has_elapsed_since(tickms_tick_t start_ticks,
                         tickms_tick_t duration_ticks)
{
        return tickms_elapsed_ticks(start_ticks) >= duration_ticks;
}

/**
 * @brief Check if a tick counter has expired based on elapsed time in
 * milliseconds.
 *
 * @details
 *    This function checks whether a specified duration in milliseconds has
 *    elapsed since a start tick. It is a convenience wrapper around
 *    tickms_expired that converts milliseconds to ticks.
 *
 * @param start     The recorded start tick.
 * @param timeout_ms The timeout duration in milliseconds.
 *
 * @return          true if the timeout has elapsed, false otherwise.
 */
static inline bool
tickms_expired_ms(tickms_tick_t start, uint32_t timeout_ms)
{
        const tickms_tick_t timeout_ticks = tickms_ms_to_ticks_ceil(timeout_ms);
        return tickms_expired(start, timeout_ticks);
}

/**
 * @brief Check if a tick counter has expired based on elapsed time in seconds.
 *
 * @details
 *    This function checks whether a specified duration in seconds has
 *    elapsed since a start tick. It is a convenience wrapper around
 *    tickms_expired that converts seconds to ticks.
 *
 * @param start     The recorded start tick.
 * @param timeout_sec The timeout duration in seconds.
 *
 * @return          true if the timeout has elapsed, false otherwise.
 */
static inline bool
tickms_expired_sec(tickms_tick_t start, uint32_t timeout_sec)
{
        const tickms_tick_t timeout_ticks =
            tickms_sec_to_ticks_saturated(timeout_sec);
        return tickms_expired(start, timeout_ticks);
}

/**
 * @brief Get the current time as milliseconds since epoch.
 *
 * @return Current time in milliseconds, saturated at UINT32_MAX.
 */
static inline uint32_t
tickms_current_ms(void)
{
        const tickms_tick_t current = tickms_get_ticks();
        return tickms_ticks_to_ms(current);
}

/**
 * @brief Get the current time as microseconds since epoch.
 *
 * @return Current time in microseconds.
 */
static inline uint64_t
tickms_current_us(void)
{
        const tickms_tick_t current = tickms_get_ticks();
        return TICKMS_TICKS_TO_US(current);
}

/**
 * @brief Get the current time as seconds since epoch.
 *
 * @return Current time in seconds.
 */
static inline uint32_t
tickms_current_sec(void)
{
        const tickms_tick_t current = tickms_get_ticks();
        const uint64_t ms = (uint64_t)current * (uint64_t)TICKMS_MS_PER_TICK;
        return (uint32_t)(ms / (uint64_t)TICKMS_MS_PER_SEC);
}

#ifdef __cplusplus
}
#endif

#endif /* TICKMS_H_ */
