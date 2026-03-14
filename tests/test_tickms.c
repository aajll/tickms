/*
 * @file test_tickms.c
 * @brief Unit tests for tickms.
 */

#define TICKMS_MS_PER_TICK 10u
#include "tickms.h"
#include "tickms_conf.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_ASSERT(expr)                                                      \
        do {                                                                   \
                if (!(expr)) {                                                 \
                        fprintf(stderr, "FAIL  %s:%d  %s\n", __FILE__,         \
                                __LINE__, #expr);                              \
                        exit(EXIT_FAILURE);                                    \
                }                                                              \
        } while (0)

#define TEST_PASS(name) fprintf(stdout, "PASS  %s\n", (name))

#define TEST_CASE(name)                                                        \
        static void name(void);                                                \
        static void name(void)

TEST_CASE(test_tickms_init)
{
        tickms_init(42u);
        TEST_ASSERT(tickms_get_ticks() == 42u);
}

TEST_CASE(test_tickms_set_ticks)
{
        tickms_init(0u);
        tickms_set_ticks(100u);
        TEST_ASSERT(tickms_get_ticks() == 100u);
}

TEST_CASE(test_tickms_tick_increment)
{
        tickms_init(50u);
        tickms_tick_t result = tickms_tick_increment();
        TEST_ASSERT(result == 51u);
        TEST_ASSERT(tickms_get_ticks() == 51u);
}

TEST_CASE(test_tickms_tick_advance)
{
        tickms_init(100u);
        tickms_tick_t result = tickms_tick_advance(25u);
        TEST_ASSERT(result == 125u);
        TEST_ASSERT(tickms_get_ticks() == 125u);
}

TEST_CASE(test_tickms_tick_advance_zero)
{
        tickms_init(100u);
        tickms_tick_t result = tickms_tick_advance(0u);
        TEST_ASSERT(result == 100u);
        TEST_ASSERT(tickms_get_ticks() == 100u);
}

TEST_CASE(test_tickms_elapsed_ms)
{
        tickms_init(100u);
        tickms_tick_t start = 80u;
        uint32_t elapsed = tickms_elapsed_ms(start);
        TEST_ASSERT(elapsed == 200u);
}

TEST_CASE(test_tickms_elapsed_us)
{
        tickms_init(100u);
        tickms_tick_t start = 90u;
        uint64_t elapsed = tickms_elapsed_us(start);
        TEST_ASSERT(elapsed == 100000ull);
}

TEST_CASE(test_tickms_elapsed_sec)
{
        tickms_init(200u);
        tickms_tick_t start = 100u;
        uint32_t elapsed = tickms_elapsed_sec(start);
        TEST_ASSERT(elapsed == 1u);
}

TEST_CASE(test_tickms_elapsed_sec_truncates)
{
        tickms_init(199u);
        tickms_tick_t start = 100u;
        uint32_t elapsed = tickms_elapsed_sec(start);
        TEST_ASSERT(elapsed == 0u);
}

TEST_CASE(test_tickms_elapsed_ms_saturated)
{
        tickms_init(UINT32_MAX);
        TEST_ASSERT(tickms_elapsed_ms(0u) == UINT32_MAX);
}

TEST_CASE(test_tickms_start)
{
        tickms_init(123u);
        tickms_tick_t t;
        tickms_start(&t);
        TEST_ASSERT(t == 123u);
}

TEST_CASE(test_tickms_restart)
{
        tickms_init(100u);
        tickms_tick_t t = 50u;
        tickms_restart(&t);
        TEST_ASSERT(t == 100u);
}

TEST_CASE(test_tickms_expired_not_expired)
{
        tickms_init(100u);
        tickms_tick_t start = 80u;
        tickms_tick_t timeout = 50u;
        TEST_ASSERT(!tickms_expired(start, timeout));
}

TEST_CASE(test_tickms_expired_expired)
{
        tickms_init(150u);
        tickms_tick_t start = 80u;
        tickms_tick_t timeout = 50u;
        TEST_ASSERT(tickms_expired(start, timeout));
}

TEST_CASE(test_tickms_expired_ms)
{
        tickms_init(150u);
        tickms_tick_t start = 100u;
        TEST_ASSERT(tickms_expired_ms(start, 500u));
        TEST_ASSERT(tickms_expired_ms(start, 490u));
}

TEST_CASE(test_tickms_expired_sec)
{
        tickms_init(200u);
        tickms_tick_t start = 100u;
        TEST_ASSERT(tickms_expired_sec(start, 1u));
        TEST_ASSERT(!tickms_expired_sec(start, 2u));
}

TEST_CASE(test_tickms_expired_wrap_around)
{
        tickms_init(5u);
        TEST_ASSERT(tickms_expired(UINT32_MAX - 3u, 9u));
        TEST_ASSERT(!tickms_expired(UINT32_MAX - 3u, 10u));
}

TEST_CASE(test_tickms_remaining)
{
        tickms_init(120u);
        tickms_tick_t start = 100u;
        tickms_tick_t timeout = 50u;
        tickms_tick_t remaining = tickms_remaining(start, timeout);
        TEST_ASSERT(remaining == 30u);
}

TEST_CASE(test_tickms_remaining_expired)
{
        tickms_init(200u);
        tickms_tick_t start = 100u;
        tickms_tick_t timeout = 50u;
        tickms_tick_t remaining = tickms_remaining(start, timeout);
        TEST_ASSERT(remaining == 0u);
}

TEST_CASE(test_tickms_remaining_wrap_around)
{
        tickms_init(5u);
        TEST_ASSERT(tickms_remaining(UINT32_MAX - 3u, 10u) == 1u);
}

TEST_CASE(test_tickms_elapsed_ticks)
{
        tickms_init(150u);
        tickms_tick_t start = 100u;
        TEST_ASSERT(tickms_elapsed_ticks(start) == 50u);
}

TEST_CASE(test_tickms_has_elapsed_since)
{
        tickms_init(150u);
        tickms_tick_t start = 100u;
        TEST_ASSERT(tickms_has_elapsed_since(start, 40u));
        TEST_ASSERT(!tickms_has_elapsed_since(start, 60u));
}

TEST_CASE(test_tickms_ticks_to_ms)
{
        TEST_ASSERT(tickms_ticks_to_ms(100u) == 1000u);
        TEST_ASSERT(tickms_ticks_to_ms(1u) == 10u);
}

TEST_CASE(test_tickms_ticks_to_ms_saturated)
{
        TEST_ASSERT(tickms_ticks_to_ms(UINT32_MAX) == UINT32_MAX);
}

TEST_CASE(test_tickms_ticks_to_seconds_f)
{
        float sec = tickms_ticks_to_seconds_f(100u);
        TEST_ASSERT(sec == 1.0f);
}

TEST_CASE(test_tickms_ms_to_ticks_floor)
{
        TEST_ASSERT(tickms_ms_to_ticks_floor(95u) == 9u);
        TEST_ASSERT(tickms_ms_to_ticks_floor(100u) == 10u);
}

TEST_CASE(test_tickms_ms_to_ticks_ceil)
{
        TEST_ASSERT(tickms_ms_to_ticks_ceil(91u) == 10u);
        TEST_ASSERT(tickms_ms_to_ticks_ceil(100u) == 10u);
}

TEST_CASE(test_tickms_sec_to_ticks_saturated)
{
        TEST_ASSERT(tickms_sec_to_ticks_saturated(1u) == 100u);
        TEST_ASSERT(tickms_sec_to_ticks_saturated(0u) == 0u);
}

TEST_CASE(test_tickms_sec_to_ticks_saturated_overflow)
{
        TEST_ASSERT(tickms_sec_to_ticks_saturated(UINT32_MAX) == UINT32_MAX);
}

TEST_CASE(test_tickms_seconds_to_ticks_nearest)
{
        TEST_ASSERT(tickms_seconds_to_ticks_nearest(1.0f) == 100u);
        TEST_ASSERT(tickms_seconds_to_ticks_nearest(0.0f) == 0u);
}

TEST_CASE(test_tickms_seconds_to_ticks_nearest_rounding)
{
        TEST_ASSERT(tickms_seconds_to_ticks_nearest(0.004f) == 0u);
        TEST_ASSERT(tickms_seconds_to_ticks_nearest(0.005f) == 1u);
        TEST_ASSERT(tickms_seconds_to_ticks_nearest(0.015f) == 2u);
}

TEST_CASE(test_tickms_seconds_to_ticks_nearest_saturated)
{
        TEST_ASSERT(tickms_seconds_to_ticks_nearest(1.0e20f) == UINT32_MAX);
}

TEST_CASE(test_tickms_current_ms)
{
        tickms_init(500u);
        TEST_ASSERT(tickms_current_ms() == 5000u);
}

TEST_CASE(test_tickms_current_ms_saturated)
{
        tickms_init(UINT32_MAX);
        TEST_ASSERT(tickms_current_ms() == UINT32_MAX);
}

TEST_CASE(test_tickms_current_us)
{
        tickms_init(100u);
        TEST_ASSERT(tickms_current_us() == 1000000ull);
}

TEST_CASE(test_tickms_current_us_large)
{
        tickms_init(UINT32_MAX);
        TEST_ASSERT(tickms_current_us() == ((uint64_t)UINT32_MAX * 10000ull));
}

TEST_CASE(test_tickms_current_sec)
{
        tickms_init(150u);
        TEST_ASSERT(tickms_current_sec() == 1u);
}

TEST_CASE(test_tickms_elapsed_seconds_since)
{
        tickms_init(150u);
        tickms_tick_t start = 100u;
        float sec = tickms_elapsed_seconds_since(start);
        TEST_ASSERT(sec == 0.5f);
}

TEST_CASE(test_tickms_wrap_around)
{
        tickms_init(UINT32_MAX);
        tickms_tick_t start = UINT32_MAX - 100u;
        uint32_t elapsed = tickms_elapsed_ms(start);
        TEST_ASSERT(elapsed == 1000u);
}

TEST_CASE(test_tickms_macro_ms_to_ticks)
{
        TEST_ASSERT(TICKMS_MS_TO_TICKS(95u) == 10u);
        TEST_ASSERT(TICKMS_MS_TO_TICKS(100u) == 10u);
}

TEST_CASE(test_tickms_macro_ticks_to_ms)
{
        TEST_ASSERT(TICKMS_TICKS_TO_MS(10u) == 100u);
        TEST_ASSERT(TICKMS_TICKS_TO_MS(100u) == 1000u);
}

TEST_CASE(test_tickms_macro_sec_to_ms)
{
        TEST_ASSERT(TICKMS_SEC_TO_MS(1u) == 1000u);
        TEST_ASSERT(TICKMS_SEC_TO_MS(2u) == 2000u);
}

TEST_CASE(test_tickms_init_null_start)
{
        tickms_init(100u);
        tickms_tick_t *null = NULL;
        tickms_start(null);
        TEST_ASSERT(tickms_get_ticks() == 100u);
}

TEST_CASE(test_tickms_init_null_restart)
{
        tickms_init(100u);
        tickms_tick_t *null = NULL;
        tickms_restart(null);
        TEST_ASSERT(tickms_get_ticks() == 100u);
}

/* ---- Regression: tickms_current_sec() large-tick overflow (bug #1) ------- */

/* At 10 ms/tick, ms overflow starts at tick 429496730.
 * The old code routed through tickms_ticks_to_ms() which saturates at
 * UINT32_MAX before dividing by 1000, producing a wrong ceiling of 4294967 s.
 * The fixed code uses 64-bit arithmetic throughout. */
TEST_CASE(test_tickms_current_sec_large)
{
        /* 529496730 ticks * 10 ms = 5294967300 ms > UINT32_MAX.
         * Old code:    UINT32_MAX / 1000 = 4294967  (wrong)
         * Fixed code:  5294967300 / 1000 = 5294967  (correct) */
        tickms_init(529496730u);
        TEST_ASSERT(tickms_current_sec() == 5294967u);
}

TEST_CASE(test_tickms_current_sec_uint32max)
{
        /* UINT32_MAX ticks * 10 ms / 1000 = 42949672 s */
        tickms_init(UINT32_MAX);
        TEST_ASSERT(tickms_current_sec()
                    == (uint32_t)(((uint64_t)UINT32_MAX * 10ull) / 1000ull));
}

/* ---- Regression: tickms_seconds_to_ticks_nearest() float boundary (bug #2) */

/* (float)UINT32_MAX rounds up to 2^32 = 4294967296.0f.
 * The old guard `ticks_f > (float)UINT32_MAX` was equivalent to
 * `ticks_f > 4294967296.0f`, so ticks_f == 4294967296.0f slipped through and
 * the subsequent cast to uint32_t wrapped to 0 (UB in practice).
 * The fixed guard uses >= so that value is caught and UINT32_MAX is returned.
 *
 * 42949672.0f * (1000.0f / 10.0f) == 4294967296.0f exactly in float. */
TEST_CASE(test_tickms_seconds_to_ticks_nearest_float_boundary)
{
        /* This value produces ticks_f == 2^32 with 10 ms/tick.
         * Must saturate to UINT32_MAX, not wrap to 0. */
        TEST_ASSERT(tickms_seconds_to_ticks_nearest(42949672.0f) == UINT32_MAX);
}

TEST_CASE(test_tickms_seconds_to_ticks_nearest_nan)
{
        /* NaN comparisons always return false, so the old `<= 0.0f` guard
         * did not catch NaN.  The fixed guard uses `!(> 0.0f)` which is
         * true for NaN, returning 0 safely without a UB cast. */
        float nan_val = (float)(0.0 / 0.0);
        TEST_ASSERT(tickms_seconds_to_ticks_nearest(nan_val) == 0u);
}

static void
run_test(void (*test_func)(void), const char *name)
{
        test_func();
        TEST_PASS(name);
}

int
main(void)
{
        fprintf(stdout, "\n=== Running tickms unit tests ===\n\n");

        run_test(test_tickms_init, "test_tickms_init");
        run_test(test_tickms_set_ticks, "test_tickms_set_ticks");
        run_test(test_tickms_tick_increment, "test_tickms_tick_increment");
        run_test(test_tickms_tick_advance, "test_tickms_tick_advance");
        run_test(test_tickms_tick_advance_zero,
                 "test_tickms_tick_advance_zero");
        run_test(test_tickms_elapsed_ms, "test_tickms_elapsed_ms");
        run_test(test_tickms_elapsed_us, "test_tickms_elapsed_us");
        run_test(test_tickms_elapsed_sec, "test_tickms_elapsed_sec");
        run_test(test_tickms_elapsed_sec_truncates,
                 "test_tickms_elapsed_sec_truncates");
        run_test(test_tickms_elapsed_ms_saturated,
                 "test_tickms_elapsed_ms_saturated");
        run_test(test_tickms_start, "test_tickms_start");
        run_test(test_tickms_restart, "test_tickms_restart");
        run_test(test_tickms_expired_not_expired,
                 "test_tickms_expired_not_expired");
        run_test(test_tickms_expired_expired, "test_tickms_expired_expired");
        run_test(test_tickms_expired_ms, "test_tickms_expired_ms");
        run_test(test_tickms_expired_sec, "test_tickms_expired_sec");
        run_test(test_tickms_expired_wrap_around,
                 "test_tickms_expired_wrap_around");
        run_test(test_tickms_remaining, "test_tickms_remaining");
        run_test(test_tickms_remaining_expired,
                 "test_tickms_remaining_expired");
        run_test(test_tickms_remaining_wrap_around,
                 "test_tickms_remaining_wrap_around");
        run_test(test_tickms_elapsed_ticks, "test_tickms_elapsed_ticks");
        run_test(test_tickms_has_elapsed_since,
                 "test_tickms_has_elapsed_since");
        run_test(test_tickms_ticks_to_ms, "test_tickms_ticks_to_ms");
        run_test(test_tickms_ticks_to_ms_saturated,
                 "test_tickms_ticks_to_ms_saturated");
        run_test(test_tickms_ticks_to_seconds_f,
                 "test_tickms_ticks_to_seconds_f");
        run_test(test_tickms_ms_to_ticks_floor,
                 "test_tickms_ms_to_ticks_floor");
        run_test(test_tickms_ms_to_ticks_ceil, "test_tickms_ms_to_ticks_ceil");
        run_test(test_tickms_sec_to_ticks_saturated,
                 "test_tickms_sec_to_ticks_saturated");
        run_test(test_tickms_sec_to_ticks_saturated_overflow,
                 "test_tickms_sec_to_ticks_saturated_overflow");
        run_test(test_tickms_seconds_to_ticks_nearest,
                 "test_tickms_seconds_to_ticks_nearest");
        run_test(test_tickms_seconds_to_ticks_nearest_rounding,
                 "test_tickms_seconds_to_ticks_nearest_rounding");
        run_test(test_tickms_seconds_to_ticks_nearest_saturated,
                 "test_tickms_seconds_to_ticks_nearest_saturated");
        run_test(test_tickms_seconds_to_ticks_nearest_float_boundary,
                 "test_tickms_seconds_to_ticks_nearest_float_boundary");
        run_test(test_tickms_seconds_to_ticks_nearest_nan,
                 "test_tickms_seconds_to_ticks_nearest_nan");
        run_test(test_tickms_current_ms, "test_tickms_current_ms");
        run_test(test_tickms_current_ms_saturated,
                 "test_tickms_current_ms_saturated");
        run_test(test_tickms_current_us, "test_tickms_current_us");
        run_test(test_tickms_current_us_large, "test_tickms_current_us_large");
        run_test(test_tickms_current_sec, "test_tickms_current_sec");
        run_test(test_tickms_current_sec_large,
                 "test_tickms_current_sec_large");
        run_test(test_tickms_current_sec_uint32max,
                 "test_tickms_current_sec_uint32max");
        run_test(test_tickms_elapsed_seconds_since,
                 "test_tickms_elapsed_seconds_since");
        run_test(test_tickms_wrap_around, "test_tickms_wrap_around");
        run_test(test_tickms_macro_ms_to_ticks,
                 "test_tickms_macro_ms_to_ticks");
        run_test(test_tickms_macro_ticks_to_ms,
                 "test_tickms_macro_ticks_to_ms");
        run_test(test_tickms_macro_sec_to_ms, "test_tickms_macro_sec_to_ms");
        run_test(test_tickms_init_null_start, "test_tickms_init_null_start");
        run_test(test_tickms_init_null_restart,
                 "test_tickms_init_null_restart");

        fprintf(stdout, "\n=== All tests passed ===\n\n");
        return EXIT_SUCCESS;
}
