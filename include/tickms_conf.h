/**
 * @file
 *    tickms_conf.h
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

#endif /* TICKMS_CONF_H_ */
