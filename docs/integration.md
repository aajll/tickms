# Integration guide

This guide shows how to add `tickms` to a project, configure it, build it, and run its tests. The API is described in [api.md](api.md), and the counter model and concurrency rules in [design.md](design.md).

## Requirements

- A C11 compiler with `uint32_t` and `uint64_t`.
- For the tests: Meson, Ninja, Python 3, and `pkg-config` or `pkgconf`. The threaded tests need POSIX threads.

## Copy-in

Copy-in is the recommended method for embedded targets. Copy three files into your source tree:

```
include/tickms.h
include/tickms_conf.h
src/tickms.c
```

Compile `tickms.c` with your other sources and include the header:

```c
#include "tickms.h"
```

No generated header is needed. `tickms_version.h` exists only in Meson builds.

## Meson subproject

Put the repository in `subprojects/tickms`, as a checkout or through a wrap file, then declare the dependency:

```meson
tickms_dep = dependency('tickms', fallback: ['tickms', 'tickms_dep'])
```

The build calls `meson.override_dependency('tickms', ...)`, so other subprojects in the same build find `tickms` by name. Tests are off by default, so a subproject build compiles only the library. Include the header as `"tickms.h"`.

## Installed package

```sh
meson setup build --buildtype=release -Dbuild_tests=false --prefix=/usr/local
meson compile -C build
meson install -C build
cc app.c $(pkg-config --cflags --libs tickms) -o app
```

After installation the headers live in a `tickms/` subdirectory:

<!-- snippet: skip -->
```c
#include <tickms/tickms.h>
#include <tickms/tickms_version.h>
```

## Configuration

Every translation unit that includes `tickms.h`, and the build of `tickms.c` itself, must see the same configuration. Set it with compiler flags, or in a project header included before `tickms.h`.

| Macro | Default | Meaning |
| --- | --- | --- |
| `TICKMS_MS_PER_TICK` | `10u` | Tick period in milliseconds. It must lie in 1 to 1000 and divide 1000, or the header fails to compile. |
| `TICKMS_USE_GNU_ATOMICS` | selected on GCC and Clang | GCC/Clang `__atomic` builtins. |
| `TICKMS_USE_C11_ATOMICS` | selected on other C11 compilers with atomics | C11 `<stdatomic.h>`. |
| `TICKMS_USE_NO_ATOMICS` | selected otherwise | A `volatile` counter for single-core targets. |

Define at most one `TICKMS_USE_*` macro. Without one, the header picks the first available backend in the order above. Select `TICKMS_USE_NO_ATOMICS` on a target whose 32-bit atomics are not lock-free, such as an 8-bit or 16-bit core or ARMv6-M; the atomic backends fail to compile there. [design.md](design.md#atomic-backends) explains the choice.

The tick source must advance the counter once per `TICKMS_MS_PER_TICK` milliseconds. With the default of 10 ms, a 1 kHz timer interrupt that calls `tickms_tick_increment()` makes every time conversion ten times too long.

## Build and test

```sh
# Library only
meson setup build --buildtype=release -Dbuild_tests=false
meson compile -C build

# Unit, threaded, and integration tests
meson setup build --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

The suite tests each atomic backend, three tick periods, compile-time rejection of invalid periods, torn reads in the `volatile` backend, and an installed consumer that finds the package through `pkg-config`. When Clang is available, configuration also checks that the atomic backends refuse a 16-bit target. Sanitiser, coverage, formatting, and MISRA commands are in [CONTRIBUTING.md](../CONTRIBUTING.md).

## Cross builds

The consuming firmware owns its cross file, linker command file, and memory placement. No cross file ships with this project.

- CI compiles the library with warnings as errors using two releases of a vendor C11 compiler for a 16-bit-MAU DSP target, where `CHAR_BIT` is 16, under both of its object formats. That compiler has no C11 atomics, so the header selects `TICKMS_USE_NO_ATOMICS`.
- Configuration checks the lock-free rule with Clang for a 16-bit `msp430` target, without a cross toolchain.

These results cover the source dialect and type assumptions only. Linking, execution, interrupt timing, and the final map-file footprint remain checks for the consumer.
