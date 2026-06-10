/*
 * @file test_tickms_c11.c
 * @brief Exercises the C11 `<stdatomic.h>` atomic backend.
 *
 * On GCC/Clang the default backend is the `__atomic` builtin path, so the C11
 * path would otherwise never be compiled in CI. This executable forces
 * TICKMS_USE_C11_ATOMICS for both this TU and the core (the define is passed
 * on the command line in tests/meson.build, so the separately compiled
 * tickms.c agrees on the backend). A failure here is most likely a compile
 * error: the C11 atomic macros not expanding cleanly.
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

TEST_CASE(test_c11_init)
{
        tickms_init(42u);
        TEST_ASSERT(tickms_get_ticks() == 42u);
}

TEST_CASE(test_c11_set_ticks)
{
        tickms_init(0u);
        tickms_set_ticks(100u);
        TEST_ASSERT(tickms_get_ticks() == 100u);
}

TEST_CASE(test_c11_tick_increment)
{
        tickms_init(50u);
        tickms_tick_t result = tickms_tick_increment();
        TEST_ASSERT(result == 51u);
        TEST_ASSERT(tickms_get_ticks() == 51u);
}

TEST_CASE(test_c11_tick_advance)
{
        tickms_init(100u);
        tickms_tick_t result = tickms_tick_advance(25u);
        TEST_ASSERT(result == 125u);
        TEST_ASSERT(tickms_get_ticks() == 125u);
}

TEST_CASE(test_c11_elapsed_ms)
{
        tickms_init(100u);
        tickms_tick_t start = 80u;
        uint32_t elapsed = tickms_elapsed_ms(start);
        TEST_ASSERT(elapsed == 200u);
}

TEST_CASE(test_c11_wrap_around)
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
        fprintf(stdout, "\n=== tickms C11-atomics backend tests ===\n\n");

        run_test(test_c11_init, "c11_init");
        run_test(test_c11_set_ticks, "c11_set_ticks");
        run_test(test_c11_tick_increment, "c11_tick_increment");
        run_test(test_c11_tick_advance, "c11_tick_advance");
        run_test(test_c11_elapsed_ms, "c11_elapsed_ms");
        run_test(test_c11_wrap_around, "c11_wrap_around");

        fprintf(stdout, "\n=== all C11-atomics backend tests passed ===\n\n");
        return EXIT_SUCCESS;
}
