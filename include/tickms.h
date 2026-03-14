/**
 * @file
 *    timer.h
 *
 * @brief
 *    Hardware-agnostic timer module with configurable tick period.
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
 *    - Timer start/reset/expiry checking
 *    - Producer-side tick update helpers
 *
 *    This module is designed with MISRA C:2012 and IEC-61508 in mind:
 *    - Consistent use of fixed-width types
 *    - No implicit type conversions
 *    - Explicit overflow handling
 *    - Clear invariants and preconditions
 *
 *   Example (set tick period to 1 ms):
 *   ----------------------------------
 *   #define TIMER_MS_PER_TICK 1u
 *   #include "timer.h"
 *
 * @note
 *   Define **TIMER_MS_PER_TICK** *before* including this file to tailor the
 *   tick period to your application; otherwise it defaults to 10 ms.
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

/* ================ DEFINES ================================================= */

/* ---------------- Configuration ------------------------------------------- */

#ifndef TIMER_MS_PER_TICK
/**
 * @def TIMER_MS_PER_TICK
 * @brief Duration of one timer tick (in milliseconds).
 *
 * @details
 *    Override this macro *before* including @ref timer.h if your project
 *    needs a different tick period. Typical values are 1, 5, 10, 20 ms, etc.
 */
#define TIMER_MS_PER_TICK (10u)
#endif

#if TIMER_MS_PER_TICK == 0u
#error "TIMER_MS_PER_TICK must be > 0"
#endif

/**
 * @def TIMER_MS_PER_SEC
 * @brief The number of ms in a second.
 */
#define TIMER_MS_PER_SEC    (1000u)

/**
 * @def TIMER_TICK_RATE_MIN
 * @brief The minimum allowable timer tick rate.
 */
#define TIMER_TICK_RATE_MIN (1u)

/**
 * @def TIMER_TICK_RATE_MAX
 * @brief The maximum allowable timer tick rate.
 */
#define TIMER_TICK_RATE_MAX (1000u)

/**
 * @def TIMER_TICKS_PER_SEC
 * @brief Number of timer ticks per second (integer, truncated).
 */
#define TIMER_TICKS_PER_SEC (TIMER_MS_PER_SEC / TIMER_MS_PER_TICK)

/* ---------------- Compile-time assertions --------------------------------- */

/* Tick period must be between 1 and 1000 ms */
_Static_assert(TIMER_MS_PER_TICK >= TIMER_TICK_RATE_MIN
                   && TIMER_MS_PER_TICK <= TIMER_TICK_RATE_MAX,
               "TIMER_MS_PER_TICK must be in [1,1000]");

/* Tick period must divide 1000 cleanly for integer conversions */
_Static_assert(
    (TIMER_TICK_RATE_MAX % TIMER_MS_PER_TICK) == 0u,
    "TIMER_MS_PER_TICK must divide 1000 (integer conversions rely on it)");

_Static_assert(TIMER_TICKS_PER_SEC != 0u,
               "TIMER_TICKS_PER_SEC computed to 0; fix TIMER_MS_PER_TICK");

/* ================ STRUCTURES ============================================== */

/* ================ TYPEDEFS ================================================ */

/**
 * @brief Timer tick count type.
 *
 * @details
 *    This is an alias for uint32_t, representing the number of ticks
 *    elapsed since an arbitrary epoch. It is used throughout the timer
 *    module for all time-related calculations.
 */
typedef uint32_t timer_tick_t;

/* ================ MACROS ================================================== */

/**
 * @def TIMER_MS_TO_TICKS(ms)
 * @brief Convert a time expressed in milliseconds to the corresponding number
 *        of ticks, rounding up.
 *
 * @param ms        Time in milliseconds.
 * @return          Minimum number of ticks required to cover `ms`.
 *
 * @note Prefer the corresponding inline helper for runtime values when
 *       overflow handling must be explicit.
 */
#define TIMER_MS_TO_TICKS(ms)                                                  \
        ((timer_tick_t)(((ms) + (TIMER_MS_PER_TICK - 1u)) / TIMER_MS_PER_TICK))

/**
 * @def TIMER_SEC_TO_TICKS(sec)
 * @brief Convert seconds to ticks (integer, truncating fractional part).
 *
 * @param sec       Time in seconds.
 * @return          Number of ticks.
 *
 * @note Prefer the corresponding inline helper for runtime values when
 *       overflow handling must be explicit.
 */
#define TIMER_SEC_TO_TICKS(sec)  ((timer_tick_t)((sec) * TIMER_TICKS_PER_SEC))

/**
 * @def TIMER_SEC_TO_MS(sec)
 * @brief Convert seconds to milliseconds (integer, truncating fractional part).
 *
 * @param sec       Time in seconds.
 * @return          Milliseconds.
 */
#define TIMER_SEC_TO_MS(sec)     ((uint32_t)((sec) * TIMER_MS_PER_SEC))

/**
 * @def TIMER_SEC_TO_US(sec)
 * @brief Convert seconds to microseconds (64-bit integer).
 *
 * @param sec       Time in seconds.
 * @return          Microseconds.
 */
#define TIMER_SEC_TO_US(sec)     ((uint64_t)((sec) * 1000000ull))

/**
 * @def TIMER_TICKS_TO_MS(ticks)
 * @brief Convert ticks to milliseconds.
 *
 * @param ticks     Number of ticks.
 * @return          Milliseconds.
 */
#define TIMER_TICKS_TO_MS(ticks) ((uint32_t)((ticks) * (TIMER_MS_PER_TICK)))

/**
 * @def TIMER_TICKS_TO_US(ticks)
 * @brief Convert ticks to microseconds.
 *
 * @param ticks     Number of ticks.
 * @return          Microseconds.
 */
#define TIMER_TICKS_TO_US(ticks)                                               \
        ((uint64_t)((ticks) * (uint64_t)(TIMER_MS_PER_TICK) * 1000ull))

/* ================ GLOBAL VARIABLES ======================================== */

/* ================ GLOBAL PROTOTYPES ======================================= */

/**
 * @brief Initialize the timer module.
 *
 * @details
 *    Sets the initial value of the tick counter. This should be called
 *    once during system initialization.
 *
 * @param initial_ticks  Initial tick count value.
 */
void timer_init(timer_tick_t initial_ticks);

/**
 * @brief Set the current tick count.
 *
 * @details
 *    Updates the global tick counter to a specified value. This is useful
 *    for synchronizing the timer with an external time source or for
 *    testing purposes.
 *
 * @param ticks  New tick count value.
 */
void timer_set_ticks(timer_tick_t ticks);

/**
 * @brief Get the current tick count.
 *
 * @return Current tick count.
 */
timer_tick_t timer_get_ticks(void);

/**
 * @brief Advance the global tick count by one tick.
 *
 * @return The updated tick count after the increment.
 */
timer_tick_t timer_tick_increment(void);

/**
 * @brief Advance the global tick count by an arbitrary delta.
 *
 * @param delta  Number of ticks to add.
 *
 * @return The updated tick count after the increment.
 */
timer_tick_t timer_tick_advance(timer_tick_t delta);

/**
 * @brief Get elapsed time in milliseconds since a start tick.
 *
 * @details
 *    This function calculates the elapsed time in milliseconds between
 *    a start tick and the current tick count. It handles timer wrap-around
 *    correctly.
 *
 * @param start_ticks  The recorded start tick.
 *
 * @return             Elapsed time in milliseconds, saturated at UINT32_MAX.
 */
static inline uint32_t
timer_elapsed_ms(timer_tick_t start_ticks)
{
        const timer_tick_t current = timer_get_ticks();
        const timer_tick_t elapsed_ticks = current - start_ticks;
        const uint64_t ms =
            (uint64_t)elapsed_ticks * (uint64_t)TIMER_MS_PER_TICK;
        return (ms > (uint64_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)ms;
}

/**
 * @brief Get elapsed time in microseconds since a start tick.
 *
 * @details
 *    This function calculates the elapsed time in microseconds between
 *    a start tick and the current tick count. It handles timer wrap-around
 *    correctly.
 *
 * @param start_ticks  The recorded start tick.
 *
 * @return             Elapsed time in microseconds.
 */
static inline uint64_t
timer_elapsed_us(timer_tick_t start_ticks)
{
        const timer_tick_t current = timer_get_ticks();
        const timer_tick_t elapsed_ticks = current - start_ticks;
        return ((uint64_t)elapsed_ticks * (uint64_t)TIMER_MS_PER_TICK
                * 1000ull);
}

/**
 * @brief Get elapsed time in seconds since a start tick.
 *
 * @details
 *    This function calculates the elapsed time in seconds between
 *    a start tick and the current tick count. It handles timer wrap-around
 *    correctly.
 *
 * @param start_ticks  The recorded start tick.
 *
 * @return             Elapsed time in seconds.
 */
static inline uint32_t
timer_elapsed_sec(timer_tick_t start_ticks)
{
        const timer_tick_t current = timer_get_ticks();
        const timer_tick_t elapsed_ticks = current - start_ticks;
        const uint64_t elapsed_ms =
            (uint64_t)elapsed_ticks * (uint64_t)TIMER_MS_PER_TICK;
        return (uint32_t)(elapsed_ms / (uint64_t)TIMER_MS_PER_SEC);
}

/**
 * @brief Start a timer by recording the current tick count.
 *
 * @param[out] t    Pointer to a timer_tick_t to store the start tick.
 */
static inline void
timer_start(timer_tick_t *t)
{
        if (t != NULL) {
                *t = timer_get_ticks();
        }
}

/**
 * @brief Restart a timer by resetting the tick to now.
 *
 * @param[out] t    Pointer to a timer variable to restart.
 */
static inline void
timer_restart(timer_tick_t *t)
{
        if (t != NULL) {
                *t = timer_get_ticks();
        }
}

/**
 * @brief Check if a timer has expired based on elapsed ticks.
 *
 * @details
 *    This function checks whether a specified duration has elapsed since
 *    a start tick. It handles timer wrap-around correctly.
 *
 * @param start     The recorded start tick.
 * @param timeout   The number of ticks to wait.
 *
 * @return          true if the timeout has elapsed, false otherwise.
 */
static inline bool
timer_expired(timer_tick_t start, timer_tick_t timeout)
{
        const timer_tick_t current = timer_get_ticks();
        return (current - start) >= timeout;
}

/**
 * @brief Get number of ticks elapsed since a start tick.
 *
 * @details
 *    This function calculates the number of ticks elapsed since a start
 *    tick. It handles timer wrap-around correctly.
 *
 * @param start_ticks  The recorded start tick.
 *
 * @return             Number of elapsed ticks.
 */
static inline timer_tick_t
timer_elapsed_ticks(timer_tick_t start_ticks)
{
        const timer_tick_t current = timer_get_ticks();
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
static inline timer_tick_t
timer_remaining(timer_tick_t start, timer_tick_t timeout)
{
        const timer_tick_t current = timer_get_ticks();
        const timer_tick_t elapsed = current - start;
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
timer_ticks_to_ms(timer_tick_t ticks)
{
        const uint64_t ms = (uint64_t)ticks * (uint64_t)TIMER_MS_PER_TICK;
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
timer_ticks_to_seconds_f(timer_tick_t ticks)
{
        return ((float)ticks * (float)TIMER_MS_PER_TICK)
               / (float)TIMER_MS_PER_SEC;
}

/**
 * @brief Get the elapsed time in seconds (floating-point) since a start tick.
 *
 * @param start_ticks  Tick value obtained from timer_start or similar.
 *
 * @return            Elapsed seconds as float.
 */
static inline float
timer_elapsed_seconds_since(timer_tick_t start_ticks)
{
        return timer_ticks_to_seconds_f(timer_elapsed_ticks(start_ticks));
}

/**
 * @brief Convert a millisecond count to the corresponding tick count,
 *        rounding down (floor).
 *
 * @param ms        Milliseconds.
 *
 * @return          Number of ticks that fit completely within `ms`.
 */
static inline timer_tick_t
timer_ms_to_ticks_floor(timer_tick_t ms)
{
        return (timer_tick_t)(ms / TIMER_MS_PER_TICK);
}

/**
 * @brief Convert a millisecond count to the corresponding tick count,
 *        rounding up (ceil).
 *
 * @param ms        Milliseconds.
 *
 * @return          Minimum number of ticks required to cover `ms`.
 */
static inline timer_tick_t
timer_ms_to_ticks_ceil(timer_tick_t ms)
{
        const uint64_t adjusted =
            (uint64_t)ms + (uint64_t)(TIMER_MS_PER_TICK - 1u);
        return (timer_tick_t)(adjusted / (uint64_t)TIMER_MS_PER_TICK);
}

/**
 * @brief Convert a whole-second duration to ticks with saturation.
 *
 * @param seconds   Whole seconds.
 *
 * @return          Tick count corresponding to `seconds`, saturated at
 *                  `UINT32_MAX` on overflow.
 */
static inline timer_tick_t
timer_sec_to_ticks_saturated(uint32_t seconds)
{
        const uint64_t ticks =
            (uint64_t)seconds * (uint64_t)TIMER_TICKS_PER_SEC;
        return (ticks > (uint64_t)UINT32_MAX) ? UINT32_MAX
                                              : (timer_tick_t)ticks;
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
static inline timer_tick_t
timer_seconds_to_ticks_nearest(float seconds)
{
        if (seconds <= 0.0f) {
                return 0u;
        }
        const float ticks_f =
            (seconds * (float)TIMER_MS_PER_SEC) / (float)TIMER_MS_PER_TICK;
        if (ticks_f > (float)UINT32_MAX) {
                return UINT32_MAX;
        }
        return (timer_tick_t)(ticks_f + 0.5f);
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
timer_has_elapsed_since(timer_tick_t start_ticks, timer_tick_t duration_ticks)
{
        return timer_elapsed_ticks(start_ticks) >= duration_ticks;
}

/**
 * @brief Check if a timer has expired based on elapsed time in milliseconds.
 *
 * @details
 *    This function checks whether a specified duration in milliseconds has
 *    elapsed since a start tick. It is a convenience wrapper around
 *    timer_expired that converts milliseconds to ticks.
 *
 * @param start     The recorded start tick.
 * @param timeout_ms The timeout duration in milliseconds.
 *
 * @return          true if the timeout has elapsed, false otherwise.
 */
static inline bool
timer_expired_ms(timer_tick_t start, uint32_t timeout_ms)
{
        const timer_tick_t timeout_ticks = timer_ms_to_ticks_ceil(timeout_ms);
        return timer_expired(start, timeout_ticks);
}

/**
 * @brief Check if a timer has expired based on elapsed time in seconds.
 *
 * @details
 *    This function checks whether a specified duration in seconds has
 *    elapsed since a start tick. It is a convenience wrapper around
 *    timer_expired that converts seconds to ticks.
 *
 * @param start     The recorded start tick.
 * @param timeout_sec The timeout duration in seconds.
 *
 * @return          true if the timeout has elapsed, false otherwise.
 */
static inline bool
timer_expired_sec(timer_tick_t start, uint32_t timeout_sec)
{
        const timer_tick_t timeout_ticks =
            timer_sec_to_ticks_saturated(timeout_sec);
        return timer_expired(start, timeout_ticks);
}

/**
 * @brief Get the current time as milliseconds since epoch.
 *
 * @return Current time in milliseconds, saturated at UINT32_MAX.
 */
static inline uint32_t
timer_current_ms(void)
{
        const timer_tick_t current = timer_get_ticks();
        return timer_ticks_to_ms(current);
}

/**
 * @brief Get the current time as microseconds since epoch.
 *
 * @return Current time in microseconds.
 */
static inline uint64_t
timer_current_us(void)
{
        const timer_tick_t current = timer_get_ticks();
        return TIMER_TICKS_TO_US(current);
}

/**
 * @brief Get the current time as seconds since epoch.
 *
 * @return Current time in seconds.
 */
static inline uint32_t
timer_current_sec(void)
{
        const timer_tick_t current = timer_get_ticks();
        return (uint32_t)(timer_ticks_to_ms(current) / TIMER_MS_PER_SEC);
}

#ifdef __cplusplus
}
#endif

#endif /* TICKMS_H_ */
