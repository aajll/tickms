# tickms

[![CI](https://github.com/aajll/tickms/actions/workflows/ci.yml/badge.svg)](https://github.com/aajll/tickms/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**A wrap-safe millisecond tick counter with timeouts and unit conversions, for embedded C.**

## Quick start

Copy `include/tickms.h`, `include/tickms_conf.h` and `src/tickms.c` into your project, and advance the counter from a periodic interrupt. For a Meson subproject or an installed `pkg-config` package, see the [integration guide](docs/integration.md).

```c
/* Build every file, and tickms.c, with -DTICKMS_MS_PER_TICK=1u. */
#include "tickms.h"

/* Called every TICKMS_MS_PER_TICK milliseconds by a timer interrupt. */
void
systick_handler(void)
{
        (void)tickms_tick_increment();
}

bool
wait_for_ready(bool (*ready)(void))
{
        tickms_tick_t start;

        tickms_start(&start);
        while (!ready()) {
                if (tickms_expired_ms(start, 500u)) {
                        return false;
                }
        }
        return true;
}
```

## Description

`tickms` keeps one 32-bit tick counter that a timer interrupt advances, and answers the questions firmware asks of it: how long since a start tick, has a timeout expired, how much time remains. Elapsed-time arithmetic is unsigned and wraps correctly, so a timeout that spans the counter overflow still works.

- Tick period from 1 ms to 1000 ms, set at compile time, which must divide 1000; invalid periods fail to compile
- Wrap-safe elapsed time, expiry and remaining-time helpers in ticks, milliseconds, microseconds and seconds
- Tick-to-millisecond and second-to-tick conversions that saturate at `UINT32_MAX` instead of wrapping
- One writer and any number of concurrent readers, through GCC/Clang `__atomic`, C11 `<stdatomic.h>` or a uniprocessor `volatile` backend
- The atomic backends require a lock-free counter at compile time, so they never call into libatomic
- No heap and no dependencies beyond the C standard headers
- 100% line and branch coverage, sanitiser-clean tests, and a MISRA C:2023 baseline from [`misch`](https://pypi.org/project/misch/)

The counter is a single module-level instance. Writers (the tick source and any call that sets the count) must not run concurrently with each other.

## Validated toolchains

| Toolchain               | Target                        | Validation                                 |
| ----------------------- | ----------------------------- | ------------------------------------------ |
| GCC, Clang              | Linux hosts                   | Unit, threaded, and sanitiser tests        |
| Clang                   | 16-bit `msp430`, freestanding | Lock-free checks reject the atomic backends |
| DSP vendor C11 compiler | 16-bit MAU (`CHAR_BIT == 16`) | Library compile, two object formats        |

Target linking, execution, and timing belong to the consuming firmware. See [cross builds](docs/integration.md#cross-builds).

## Documentation

- [Integration guide](docs/integration.md) — copy-in, Meson subproject, installed package, configuration, and cross builds
- [API reference](docs/api.md) — every function and macro, with its rules
- [Design](docs/design.md) — counter model, wrap and saturation rules, atomic backends, and verification
- [Changelog](CHANGELOG.md) — release history

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) before you open a pull request. Report security issues as described in [SECURITY.md](.github/SECURITY.md).

## License

MIT. See [LICENSE](LICENSE).
