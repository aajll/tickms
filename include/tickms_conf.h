/**
 * SPDX-License-Identifier: MIT
 *
 * @file: tickms_conf.h
 *
 * @brief
 *    Public configuration header for tickms.
 *
 * @details
 *    This header provides configuration options for the tickms module.
 *    It is included automatically by tickms.h, but applications may also
 *    include it directly before tickms.h to keep configuration in one place.
 *    Default values are provided if not overridden.
 *
 * @note
 *    Override any configuration option by defining it before including
 *    this file. Example:
 *    @code
 *    #define TICKMS_MS_PER_TICK 1u
 *    #include "tickms_conf.h"
 *    #include "tickms.h"
 *    @endcode
 *
 * @note
 *    To force an atomic backend instead of relying on auto-selection (e.g.
 *    `-DTICKMS_USE_NO_ATOMICS=1` on uniprocessor targets whose toolchain
 *    lacks `_Atomic` support, such as TI C2000), define exactly one
 *    `TICKMS_USE_*` selector identically for the tickms library build and
 *    every consumer translation unit.
 */
#ifndef TICKMS_CONF_H_
#define TICKMS_CONF_H_

/* ================ CONFIGURATION =========================================== */

/* ---------------- Tick Period --------------------------------------------- */

#ifndef TICKMS_MS_PER_TICK
/**
 * @def TICKMS_MS_PER_TICK
 * @brief Duration of one tick (in milliseconds).
 *
 * @details
 *    The tick period determines the resolution of the tick-based timer.
 *    Typical values are 1, 5, 10, 20, 50 ms. The value must divide 1000
 *    evenly for integer time conversions to work correctly.
 *
 * @note
 *    The following values are valid (divide 1000 evenly):
 *    1, 2, 4, 5, 10, 20, 25, 50, 100, 200, 250, 500, 1000 ms
 */
#define TICKMS_MS_PER_TICK (10u)
#endif

/* ================ ATOMIC BACKEND SELECTION ================================ */

/**
 * @def TICKMS_USE_GNU_ATOMICS
 * @brief Force the GCC/Clang `__atomic` backend.
 *
 * This is the default backend on GCC and Clang. Define exactly one of
 * TICKMS_USE_GNU_ATOMICS, TICKMS_USE_C11_ATOMICS, or
 * TICKMS_USE_NO_ATOMICS consistently for the tickms library build and all
 * consumers to override automatic backend selection.
 */

/**
 * @def TICKMS_USE_C11_ATOMICS
 * @brief Force the C11 `<stdatomic.h>` backend.
 *
 * Use when the toolchain provides C11 atomics and the project wants to avoid
 * compiler-specific `__atomic` builtins. Define exactly one backend-selection
 * macro consistently for the tickms library build and all consumers.
 */

/**
 * @def TICKMS_USE_NO_ATOMICS
 * @brief Force the degenerate uniprocessor backend.
 *
 * Correct only when readers and the writer cannot run concurrently on separate
 * cores, or on targets with equivalent ordering guarantees. This backend uses
 * a volatile counter and no hardware fences. Define exactly one backend-
 * selection macro consistently for the tickms library build and all consumers.
 */

/*
 * Pick exactly one backend unless the consumer forced one. Selection order:
 *   GCC/Clang __atomic  ->  C11 <stdatomic.h>  ->  degenerate no-op (uniproc).
 */
#if !defined(TICKMS_USE_GNU_ATOMICS) && !defined(TICKMS_USE_C11_ATOMICS)       \
    && !defined(TICKMS_USE_NO_ATOMICS)
#if defined(__GNUC__) || defined(__clang__)
#define TICKMS_USE_GNU_ATOMICS 1
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)               \
    && !defined(__STDC_NO_ATOMICS__)
#define TICKMS_USE_C11_ATOMICS 1
#else
#define TICKMS_USE_NO_ATOMICS 1
#endif
#endif

#if (defined(TICKMS_USE_GNU_ATOMICS) + defined(TICKMS_USE_C11_ATOMICS)         \
     + defined(TICKMS_USE_NO_ATOMICS))                                         \
    != 1
#error "tickms: define exactly one atomic backend"
#endif

#if defined(TICKMS_USE_GNU_ATOMICS)

/** Qualifier applied to the counter member (none needed for __atomic). */
#define TICKMS_ATOMIC_QUAL
#define TICKMS_ATOMIC_LOAD(ptr) __atomic_load_n((ptr), __ATOMIC_RELAXED)
#define TICKMS_ATOMIC_STORE(ptr, val)                                          \
        __atomic_store_n((ptr), (val), __ATOMIC_RELAXED)
#define TICKMS_ATOMIC_FETCH_ADD(ptr, delta)                                    \
        __atomic_fetch_add((ptr), (delta), __ATOMIC_RELAXED)

#elif defined(TICKMS_USE_C11_ATOMICS)

#include <stdatomic.h>

#define TICKMS_ATOMIC_QUAL _Atomic
#define TICKMS_ATOMIC_LOAD(ptr)                                                \
        atomic_load_explicit((ptr), memory_order_relaxed)
#define TICKMS_ATOMIC_STORE(ptr, val)                                          \
        atomic_store_explicit((ptr), (val), memory_order_relaxed)
#define TICKMS_ATOMIC_FETCH_ADD(ptr, delta)                                    \
        atomic_fetch_add_explicit((ptr), (delta), memory_order_relaxed)

#else /* TICKMS_USE_NO_ATOMICS */

/*
 * Degenerate single-core fallback: `volatile` defeats compiler reordering of
 * the counter but provides NO hardware ordering or atomicity for read-modify-
 * write operations. Correct only on a uniprocessor (or where readers/writer
 * cannot run on separate cores). The fetch-add is a plain compound
 * assignment, which is sufficient when no concurrent access can occur.
 *
 * This backend is strictly standard C (no compiler extensions): it is the
 * path taken by toolchains without `__atomic` builtins or `<stdatomic.h>`
 * support, e.g. TI C2000 in C11 mode, so it must not assume GNU extensions.
 *
 * MISRA C:2012 deviation: TICKMS_ATOMIC_FETCH_ADD uses the value of an
 * assignment expression (Rule 13.4, advisory). Justified: returning the
 * pre-add value as a single expression requires it, and the alternative (a
 * GNU statement expression) is a language extension unavailable on the
 * strictly conforming toolchains this backend exists for. Subtracting the
 * delta recovers the old value exactly because tickms_tick_t is unsigned,
 * so wraparound is well defined.
 */
#define TICKMS_ATOMIC_QUAL                  volatile
#define TICKMS_ATOMIC_LOAD(ptr)             (*(ptr))
#define TICKMS_ATOMIC_STORE(ptr, val)       ((void)(*(ptr) = (val)))
#define TICKMS_ATOMIC_FETCH_ADD(ptr, delta) ((*(ptr) += (delta)) - (delta))

#endif

#endif /* TICKMS_CONF_H_ */
