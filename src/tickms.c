/**
 * SPDX-License-Identifier: MIT
 *
 * @file: tickms.c
 *
 * @brief
 *    Implementation of tickms.
 *
 * @details
 *    This file contains the implementation of non-inline tickms functions
 *    and the global tick counter variable.
 *
 */

/* ================ INCLUDES ================================================ */

#include "tickms.h"

#include <stdatomic.h>

/* ================ DEFINES ================================================= */

/* ================ STRUCTURES ============================================== */

/* ================ TYPEDEFS ================================================ */

/* ================ STATIC PROTOTYPES ======================================= */

/* ================ STATIC VARIABLES ======================================== */

/**
 * @brief Global tick counter.
 *
 * @details
 *    This counter must be incremented by the system every TICKMS_MS_PER_TICK
 *    milliseconds, typically from a periodic interrupt service routine or
 *    a high-resolution timer callback. The counter wraps around after
 *    reaching UINT32_MAX.
 *
 *    @note This storage is private to the module and accessed atomically.
 */
static _Atomic tickms_tick_t tickms_ticks = 0u;

/* ================ MACROS ================================================== */

/* ================ STATIC FUNCTIONS ======================================== */

/* ================ GLOBAL FUNCTIONS ======================================== */

void
tickms_init(tickms_tick_t initial_ticks)
{
        atomic_store_explicit(&tickms_ticks, initial_ticks,
                              memory_order_relaxed);
}

void
tickms_set_ticks(tickms_tick_t ticks)
{
        atomic_store_explicit(&tickms_ticks, ticks, memory_order_relaxed);
}

tickms_tick_t
tickms_get_ticks(void)
{
        return atomic_load_explicit(&tickms_ticks, memory_order_relaxed);
}

tickms_tick_t
tickms_tick_increment(void)
{
        return atomic_fetch_add_explicit(&tickms_ticks, 1u,
                                         memory_order_relaxed)
               + 1u;
}

tickms_tick_t
tickms_tick_advance(tickms_tick_t delta)
{
        return atomic_fetch_add_explicit(&tickms_ticks, delta,
                                         memory_order_relaxed)
               + delta;
}
