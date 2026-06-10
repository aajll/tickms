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
 * @par MISRA C:2012 deviation record
 *    The library is written against MISRA C:2012 principles. Known deviations,
 *    all advisory and justified:
 *      - Compiler atomic builtins (`__atomic_load_n`, `__atomic_store_n`,
 *        `__atomic_fetch_add`) are language extensions (Dir 1.1 / Rule 1.2).
 *        Justified: portable lock-free atomics; isolated behind
 *        `tickms_conf.h` macros and swappable for the C11 `<stdatomic.h>`
 *        path or the uniprocessor volatile fallback. Applies only when the
 *        GNU backend is selected.
 *      - The no-atomics fallback's `TICKMS_ATOMIC_FETCH_ADD` uses the value
 *        of an assignment expression (Rule 13.4, advisory). Justified at the
 *        macro definition in `tickms_conf.h`. Applies only when the
 *        no-atomics backend is selected.
 */

/* ================ INCLUDES ================================================ */

#include "tickms.h"

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
 *    @note This storage is private to the module and accessed via the atomic
 *          backend selected in tickms_conf.h (GCC/Clang __atomic, C11
 *          <stdatomic.h>, or volatile uniprocessor fallback).
 */
static TICKMS_ATOMIC_QUAL tickms_tick_t tickms_ticks = 0u;

/* ================ MACROS ================================================== */

/* ================ STATIC FUNCTIONS ======================================== */

/* ================ GLOBAL FUNCTIONS ======================================== */

void
tickms_init(tickms_tick_t initial_ticks)
{
        TICKMS_ATOMIC_STORE(&tickms_ticks, initial_ticks);
}

void
tickms_set_ticks(tickms_tick_t ticks)
{
        TICKMS_ATOMIC_STORE(&tickms_ticks, ticks);
}

tickms_tick_t
tickms_get_ticks(void)
{
        return TICKMS_ATOMIC_LOAD(&tickms_ticks);
}

tickms_tick_t
tickms_tick_increment(void)
{
        return TICKMS_ATOMIC_FETCH_ADD(&tickms_ticks, 1u) + 1u;
}

tickms_tick_t
tickms_tick_advance(tickms_tick_t delta)
{
        return TICKMS_ATOMIC_FETCH_ADD(&tickms_ticks, delta) + delta;
}
