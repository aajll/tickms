/**
 * SPDX-License-Identifier: MIT
 *
 * @file: int16_probe.c
 *
 * @brief
 *    Translation unit for the 16-bit int checks in tests/meson.build and the
 *    TI C2000 CI job. An int argument to a conversion macro must not wrap in
 *    the width of int.
 */

#include "tickms.h"

_Static_assert(TICKMS_SEC_TO_MS(70) == 70000ul,
               "TICKMS_SEC_TO_MS wraps in the width of int");
_Static_assert(TICKMS_SEC_TO_TICKS(70) == (70ul * TICKMS_TICKS_PER_SEC),
               "TICKMS_SEC_TO_TICKS wraps in the width of int");

tickms_tick_t tickms_int16_probe_value;
