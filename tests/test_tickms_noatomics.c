/*
 * @file test_tickms_noatomics.c
 * @brief Exercises the degenerate uniprocessor (volatile) atomic backend.
 *
 * This backend uses a volatile counter with no hardware fences and is correct
 * only when readers and the writer cannot run concurrently on separate cores.
 * This executable forces TICKMS_USE_NO_ATOMICS for both this TU and the core
 * (the define is passed on the command line in tests/meson.build). A failure
 * here indicates the volatile fallback macros do not expand cleanly.
 */

#include "tickms.h"

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

TEST_CASE(test_noatomics_init)
{
        tickms_init(42u);
        TEST_ASSERT(tickms_get_ticks() == 42u);
}

TEST_CASE(test_noatomics_set_ticks)
{
        tickms_init(0u);
        tickms_set_ticks(100u);
        TEST_ASSERT(tickms_get_ticks() == 100u);
}

TEST_CASE(test_noatomics_tick_increment)
{
        tickms_init(50u);
        tickms_tick_t result = tickms_tick_increment();
        TEST_ASSERT(result == 51u);
        TEST_ASSERT(tickms_get_ticks() == 51u);
}

TEST_CASE(test_noatomics_tick_advance)
{
        tickms_init(100u);
        tickms_tick_t result = tickms_tick_advance(25u);
        TEST_ASSERT(result == 125u);
        TEST_ASSERT(tickms_get_ticks() == 125u);
}

TEST_CASE(test_noatomics_elapsed_ms)
{
        tickms_init(100u);
        tickms_tick_t start = 80u;
        uint32_t elapsed = tickms_elapsed_ms(start);
        TEST_ASSERT(elapsed == 200u);
}

TEST_CASE(test_noatomics_wrap_around)
{
        tickms_init(5u);
        TEST_ASSERT(tickms_expired(UINT32_MAX - 3u, 9u));
        TEST_ASSERT(!tickms_expired(UINT32_MAX - 3u, 10u));
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
        fprintf(stdout,
                "\n=== tickms no-atomics (volatile) backend tests ===\n\n");

        run_test(test_noatomics_init, "noatomics_init");
        run_test(test_noatomics_set_ticks, "noatomics_set_ticks");
        run_test(test_noatomics_tick_increment, "noatomics_tick_increment");
        run_test(test_noatomics_tick_advance, "noatomics_tick_advance");
        run_test(test_noatomics_elapsed_ms, "noatomics_elapsed_ms");
        run_test(test_noatomics_wrap_around, "noatomics_wrap_around");

        fprintf(stdout,
                "\n=== all no-atomics backend tests passed ===\n\n");
        return EXIT_SUCCESS;
}
