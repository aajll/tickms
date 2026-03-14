/*
 * @file test_tickms_config_matrix.c
 * @brief Configuration matrix tests for tickms.
 */

#ifndef EXPECTED_TICKMS_MS_PER_TICK
#error "EXPECTED_TICKMS_MS_PER_TICK must be defined by the build"
#endif

#include "tickms.h"
#include "tickms_conf.h"

#include <stdint.h>
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

int
main(void)
{
        TEST_ASSERT(TICKMS_MS_PER_TICK == EXPECTED_TICKMS_MS_PER_TICK);
        TEST_ASSERT(TICKMS_TICKS_PER_SEC
                    == (1000u / EXPECTED_TICKMS_MS_PER_TICK));

        tickms_init(4u);
        TEST_ASSERT(tickms_get_ticks() == 4u);
        TEST_ASSERT(tickms_current_ms() == (4u * EXPECTED_TICKMS_MS_PER_TICK));

        TEST_ASSERT(
            tickms_ms_to_ticks_floor((EXPECTED_TICKMS_MS_PER_TICK * 3u)
                                     + (EXPECTED_TICKMS_MS_PER_TICK - 1u))
            == 3u);
        TEST_ASSERT(
            tickms_ms_to_ticks_ceil((EXPECTED_TICKMS_MS_PER_TICK * 3u) + 1u)
            == 4u);
        TEST_ASSERT(TICKMS_MS_TO_TICKS(EXPECTED_TICKMS_MS_PER_TICK + 1u) == 2u);

        tickms_init(3u);
        TEST_ASSERT(tickms_expired_ms(0u, (EXPECTED_TICKMS_MS_PER_TICK * 3u)));
        TEST_ASSERT(
            !tickms_expired_ms(0u, (EXPECTED_TICKMS_MS_PER_TICK * 3u) + 1u));

        return EXIT_SUCCESS;
}
