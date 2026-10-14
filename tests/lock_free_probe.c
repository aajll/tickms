/**
 * SPDX-License-Identifier: MIT
 *
 * @file: lock_free_probe.c
 *
 * @brief
 *    Translation unit for the cross-target lock-free checks in
 *    tests/meson.build. It compiles only if tickms.h accepts the target.
 */

#include "tickms.h"

tickms_tick_t tickms_probe_value;
