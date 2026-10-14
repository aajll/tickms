/*
 * @file test_tickms_torn_read.c
 * @brief Torn-read rejection in the no-atomics backend.
 *
 * A host reads the 32-bit counter in one access, so a torn read cannot occur
 * here naturally. This TU replays the values successive counter reads would
 * observe on a narrower core and includes the core directly, so
 * tickms_get_ticks() sees a torn value followed by the settled one.
 */

#include <stddef.h>
#include <stdint.h>

static uint32_t script[3];
static size_t script_reads;

#define TICKMS__READ_COUNTER() (script[script_reads++])

#include "../src/tickms.c"

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

TEST_CASE(test_stable_counter_reads_twice)
{
        script[0] = 5u;
        script[1] = 5u;
        script_reads = 0u;

        TEST_ASSERT(tickms_get_ticks() == 5u);
        TEST_ASSERT(script_reads == 2u);
}

TEST_CASE(test_torn_read_is_discarded)
{
        /* 0x0000FFFF -> 0x00010000, low half read before the tick and high
         * half after it. */
        script[0] = 0x0001FFFFu;
        script[1] = 0x00010000u;
        script[2] = 0x00010000u;
        script_reads = 0u;

        TEST_ASSERT(tickms_get_ticks() == 0x00010000u);
        TEST_ASSERT(script_reads == 3u);
}

int
main(void)
{
        fprintf(stdout, "\n=== tickms no-atomics torn-read tests ===\n\n");

        test_stable_counter_reads_twice();
        TEST_PASS("stable_counter_reads_twice");
        test_torn_read_is_discarded();
        TEST_PASS("torn_read_is_discarded");

        fprintf(stdout, "\n=== all torn-read tests passed ===\n\n");
        return EXIT_SUCCESS;
}
