/**
 * SPDX-License-Identifier: MIT
 *
 * @file: test_tickms_mt.c
 *
 * @brief
 *    One producer advances the tick counter while reader threads poll it.
 *
 * @details
 *    Readers must only ever see a non-decreasing count that never exceeds the
 *    producer's final value, and the final value must equal the sum of every
 *    advance. Under ThreadSanitizer, a clean run also shows the counter
 *    accesses are race-free on the backend this executable is built with.
 */

#include "tickms.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#define MT_INCREMENTS 200000u
#define MT_ADVANCE    3u
#define MT_READERS    3

#define MT_FINAL      (MT_INCREMENTS * (1u + MT_ADVANCE))

static atomic_bool producer_done = false;

static void *
producer(void *arg)
{
        (void)arg;
        for (uint32_t i = 0u; i < MT_INCREMENTS; i++) {
                (void)tickms_tick_increment();
                (void)tickms_tick_advance(MT_ADVANCE);
        }
        atomic_store(&producer_done, true);
        return NULL;
}

static void *
reader(void *arg)
{
        unsigned long *violations = (unsigned long *)arg;
        tickms_tick_t last = 0u;
        bool done = false;

        while (!done) {
                done = atomic_load(&producer_done);
                const tickms_tick_t now = tickms_get_ticks();
                if ((now < last) || (now > MT_FINAL)) {
                        (*violations)++;
                }
                last = now;
        }
        return NULL;
}

int
main(void)
{
        pthread_t prod;
        pthread_t readers[MT_READERS];
        unsigned long violations[MT_READERS] = {0u};

        tickms_init(0u);

        for (int i = 0; i < MT_READERS; i++) {
                if (pthread_create(&readers[i], NULL, reader, &violations[i])
                    != 0) {
                        return EXIT_FAILURE;
                }
        }
        if (pthread_create(&prod, NULL, producer, NULL) != 0) {
                return EXIT_FAILURE;
        }

        (void)pthread_join(prod, NULL);
        for (int i = 0; i < MT_READERS; i++) {
                (void)pthread_join(readers[i], NULL);
        }

        int rc = EXIT_SUCCESS;
        for (int i = 0; i < MT_READERS; i++) {
                if (violations[i] != 0u) {
                        fprintf(stderr, "FAIL  reader %d saw %lu bad values\n",
                                i, violations[i]);
                        rc = EXIT_FAILURE;
                }
        }
        if (tickms_get_ticks() != MT_FINAL) {
                fprintf(stderr, "FAIL  final count %lu, expected %lu\n",
                        (unsigned long)tickms_get_ticks(),
                        (unsigned long)MT_FINAL);
                rc = EXIT_FAILURE;
        }
        if (rc == EXIT_SUCCESS) {
                fprintf(stdout, "PASS  test_tickms_mt\n");
        }
        return rc;
}
