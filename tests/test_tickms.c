/*
 * @file test_tickms.c
 * @brief Unit tests for tickms.
 */

#define TIMER_MS_PER_TICK 10u
#include "tickms.h"
#include <limits.h>
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

TEST_CASE(test_timer_init)
{
        timer_init(42u);
        TEST_ASSERT(timer_get_ticks() == 42u);
}

TEST_CASE(test_timer_set_ticks)
{
        timer_init(0u);
        timer_set_ticks(100u);
        TEST_ASSERT(timer_get_ticks() == 100u);
}

TEST_CASE(test_timer_tick_increment)
{
        timer_init(50u);
        timer_tick_t result = timer_tick_increment();
        TEST_ASSERT(result == 51u);
        TEST_ASSERT(timer_get_ticks() == 51u);
}

TEST_CASE(test_timer_tick_advance)
{
        timer_init(100u);
        timer_tick_t result = timer_tick_advance(25u);
        TEST_ASSERT(result == 125u);
        TEST_ASSERT(timer_get_ticks() == 125u);
}

TEST_CASE(test_timer_elapsed_ms)
{
        timer_init(100u);
        timer_tick_t start = 80u;
        uint32_t elapsed = timer_elapsed_ms(start);
        TEST_ASSERT(elapsed == 200u);
}

TEST_CASE(test_timer_elapsed_us)
{
        timer_init(100u);
        timer_tick_t start = 90u;
        uint64_t elapsed = timer_elapsed_us(start);
        TEST_ASSERT(elapsed == 100000ull);
}

TEST_CASE(test_timer_elapsed_sec)
{
        timer_init(200u);
        timer_tick_t start = 100u;
        uint32_t elapsed = timer_elapsed_sec(start);
        TEST_ASSERT(elapsed == 1u);
}

TEST_CASE(test_timer_start)
{
        timer_init(123u);
        timer_tick_t t;
        timer_start(&t);
        TEST_ASSERT(t == 123u);
}

TEST_CASE(test_timer_restart)
{
        timer_init(100u);
        timer_tick_t t = 50u;
        timer_restart(&t);
        TEST_ASSERT(t == 100u);
}

TEST_CASE(test_timer_expired_not_expired)
{
        timer_init(100u);
        timer_tick_t start = 80u;
        timer_tick_t timeout = 50u;
        TEST_ASSERT(!timer_expired(start, timeout));
}

TEST_CASE(test_timer_expired_expired)
{
        timer_init(150u);
        timer_tick_t start = 80u;
        timer_tick_t timeout = 50u;
        TEST_ASSERT(timer_expired(start, timeout));
}

TEST_CASE(test_timer_expired_ms)
{
        timer_init(150u);
        timer_tick_t start = 100u;
        TEST_ASSERT(timer_expired_ms(start, 500u));
        TEST_ASSERT(timer_expired_ms(start, 490u));
}

TEST_CASE(test_timer_expired_sec)
{
        timer_init(200u);
        timer_tick_t start = 100u;
        TEST_ASSERT(timer_expired_sec(start, 1u));
        TEST_ASSERT(!timer_expired_sec(start, 2u));
}

TEST_CASE(test_timer_remaining)
{
        timer_init(120u);
        timer_tick_t start = 100u;
        timer_tick_t timeout = 50u;
        timer_tick_t remaining = timer_remaining(start, timeout);
        TEST_ASSERT(remaining == 30u);
}

TEST_CASE(test_timer_remaining_expired)
{
        timer_init(200u);
        timer_tick_t start = 100u;
        timer_tick_t timeout = 50u;
        timer_tick_t remaining = timer_remaining(start, timeout);
        TEST_ASSERT(remaining == 0u);
}

TEST_CASE(test_timer_elapsed_ticks)
{
        timer_init(150u);
        timer_tick_t start = 100u;
        TEST_ASSERT(timer_elapsed_ticks(start) == 50u);
}

TEST_CASE(test_timer_has_elapsed_since)
{
        timer_init(150u);
        timer_tick_t start = 100u;
        TEST_ASSERT(timer_has_elapsed_since(start, 40u));
        TEST_ASSERT(!timer_has_elapsed_since(start, 60u));
}

TEST_CASE(test_timer_ticks_to_ms)
{
        TEST_ASSERT(timer_ticks_to_ms(100u) == 1000u);
        TEST_ASSERT(timer_ticks_to_ms(1u) == 10u);
}

TEST_CASE(test_timer_ticks_to_seconds_f)
{
        float sec = timer_ticks_to_seconds_f(100u);
        TEST_ASSERT(sec == 1.0f);
}

TEST_CASE(test_timer_ms_to_ticks_floor)
{
        TEST_ASSERT(timer_ms_to_ticks_floor(95u) == 9u);
        TEST_ASSERT(timer_ms_to_ticks_floor(100u) == 10u);
}

TEST_CASE(test_timer_ms_to_ticks_ceil)
{
        TEST_ASSERT(timer_ms_to_ticks_ceil(91u) == 10u);
        TEST_ASSERT(timer_ms_to_ticks_ceil(100u) == 10u);
}

TEST_CASE(test_timer_sec_to_ticks_saturated)
{
        TEST_ASSERT(timer_sec_to_ticks_saturated(1u) == 100u);
        TEST_ASSERT(timer_sec_to_ticks_saturated(0u) == 0u);
}

TEST_CASE(test_timer_seconds_to_ticks_nearest)
{
        TEST_ASSERT(timer_seconds_to_ticks_nearest(1.0f) == 100u);
        TEST_ASSERT(timer_seconds_to_ticks_nearest(0.0f) == 0u);
}

TEST_CASE(test_timer_current_ms)
{
        timer_init(500u);
        TEST_ASSERT(timer_current_ms() == 5000u);
}

TEST_CASE(test_timer_current_us)
{
        timer_init(100u);
        TEST_ASSERT(timer_current_us() == 1000000ull);
}

TEST_CASE(test_timer_current_sec)
{
        timer_init(150u);
        TEST_ASSERT(timer_current_sec() == 1u);
}

TEST_CASE(test_timer_elapsed_seconds_since)
{
        timer_init(150u);
        timer_tick_t start = 100u;
        float sec = timer_elapsed_seconds_since(start);
        TEST_ASSERT(sec == 0.5f);
}

TEST_CASE(test_timer_wrap_around)
{
        timer_init(UINT32_MAX);
        timer_tick_t start = UINT32_MAX - 100u;
        uint32_t elapsed = timer_elapsed_ms(start);
        TEST_ASSERT(elapsed == 1000u);
}

TEST_CASE(test_timer_macro_ms_to_ticks)
{
        TEST_ASSERT(TIMER_MS_TO_TICKS(95u) == 10u);
        TEST_ASSERT(TIMER_MS_TO_TICKS(100u) == 10u);
}

TEST_CASE(test_timer_macro_ticks_to_ms)
{
        TEST_ASSERT(TIMER_TICKS_TO_MS(10u) == 100u);
        TEST_ASSERT(TIMER_TICKS_TO_MS(100u) == 1000u);
}

TEST_CASE(test_timer_macro_sec_to_ms)
{
        TEST_ASSERT(TIMER_SEC_TO_MS(1u) == 1000u);
        TEST_ASSERT(TIMER_SEC_TO_MS(2u) == 2000u);
}

TEST_CASE(test_timer_init_null_start)
{
        timer_init(100u);
        timer_tick_t *null = NULL;
        timer_start(null);
        TEST_ASSERT(timer_get_ticks() == 100u);
}

TEST_CASE(test_timer_init_null_restart)
{
        timer_init(100u);
        timer_tick_t *null = NULL;
        timer_restart(null);
        TEST_ASSERT(timer_get_ticks() == 100u);
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

        run_test(test_timer_init, "test_timer_init");
        run_test(test_timer_set_ticks, "test_timer_set_ticks");
        run_test(test_timer_tick_increment, "test_timer_tick_increment");
        run_test(test_timer_tick_advance, "test_timer_tick_advance");
        run_test(test_timer_elapsed_ms, "test_timer_elapsed_ms");
        run_test(test_timer_elapsed_us, "test_timer_elapsed_us");
        run_test(test_timer_elapsed_sec, "test_timer_elapsed_sec");
        run_test(test_timer_start, "test_timer_start");
        run_test(test_timer_restart, "test_timer_restart");
        run_test(test_timer_expired_not_expired,
                 "test_timer_expired_not_expired");
        run_test(test_timer_expired_expired, "test_timer_expired_expired");
        run_test(test_timer_expired_ms, "test_timer_expired_ms");
        run_test(test_timer_expired_sec, "test_timer_expired_sec");
        run_test(test_timer_remaining, "test_timer_remaining");
        run_test(test_timer_remaining_expired, "test_timer_remaining_expired");
        run_test(test_timer_elapsed_ticks, "test_timer_elapsed_ticks");
        run_test(test_timer_has_elapsed_since, "test_timer_has_elapsed_since");
        run_test(test_timer_ticks_to_ms, "test_timer_ticks_to_ms");
        run_test(test_timer_ticks_to_seconds_f,
                 "test_timer_ticks_to_seconds_f");
        run_test(test_timer_ms_to_ticks_floor, "test_timer_ms_to_ticks_floor");
        run_test(test_timer_ms_to_ticks_ceil, "test_timer_ms_to_ticks_ceil");
        run_test(test_timer_sec_to_ticks_saturated,
                 "test_timer_sec_to_ticks_saturated");
        run_test(test_timer_seconds_to_ticks_nearest,
                 "test_timer_seconds_to_ticks_nearest");
        run_test(test_timer_current_ms, "test_timer_current_ms");
        run_test(test_timer_current_us, "test_timer_current_us");
        run_test(test_timer_current_sec, "test_timer_current_sec");
        run_test(test_timer_elapsed_seconds_since,
                 "test_timer_elapsed_seconds_since");
        run_test(test_timer_wrap_around, "test_timer_wrap_around");
        run_test(test_timer_macro_ms_to_ticks, "test_timer_macro_ms_to_ticks");
        run_test(test_timer_macro_ticks_to_ms, "test_timer_macro_ticks_to_ms");
        run_test(test_timer_macro_sec_to_ms, "test_timer_macro_sec_to_ms");
        run_test(test_timer_init_null_start, "test_timer_init_null_start");
        run_test(test_timer_init_null_restart, "test_timer_init_null_restart");

        fprintf(stdout, "\n=== All tests passed ===\n\n");
        return EXIT_SUCCESS;
}
