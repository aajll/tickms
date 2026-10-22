# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

## [1.4.0] - 2026-10-22

### Added

- A threaded test with one producer and several readers for the `__atomic` and C11 backends, and a ThreadSanitizer CI job that runs it.
- A clang-format CI job with the formatter pinned in `scripts/format.sh`.
- MISRA C:2023 analysis with misch for each atomic backend. CI fails on findings that are not in the baselines under `analysis/baseline/`.
- A test that compiles every C example in `README.md` and `docs/` against the header.
- A CI job that cross-builds the library with warnings as errors for a 16-bit-MAU target (`CHAR_BIT == 16`) with two compiler releases, under both object formats.

### Changed

- `tickms.h` defines `TICKMS_VERSION_MAJOR`, `_MINOR`, `_PATCH`, `TICKMS_VERSION`, `TICKMS_VERSION_STRING` and a new `TICKMS_VERSION_NUMBER` (`long`, `MAJOR * 10000 + MINOR * 100 + PATCH`), so a copied-in header carries its version. Meson fails to configure when the header and `project(version:)` disagree, and the build needs Meson 0.57 or later.
- The README is a short landing page. Integration, configuration, API, and design detail is in `docs/integration.md`, `docs/api.md`, and `docs/design.md`.
- CI installs Meson, Ninja, gcovr and misch with `uv` instead of `pip`.
- The CI coverage gate is 100% line and 100% branch, up from 80% and 70%.
- The `__atomic` and C11 backends fail to compile when `tickms_tick_t` is not lock-free on the target, instead of routing every tick through libatomic or a user-supplied helper. 8-bit and 16-bit cores and ARMv6-M select `TICKMS_USE_NO_ATOMICS`.

### Removed

- `tickms_version.h`. `tickms.h` defines the version macros.
- The `extern "C"` guards in `tickms.h`. The library is C11 only and its headers make no C++ claim.

### Fixed

- `TICKMS_SEC_TO_MS` multiplies in 64 bits and saturates at `UINT32_MAX`. Where `int` is 16 bits wide, an `int` argument no longer wraps at 65 536, so `TICKMS_SEC_TO_MS(70)` is 70 000 rather than 4464. The macro evaluates its argument more than once.
- `tickms_get_ticks()` under `TICKMS_USE_NO_ATOMICS` re-reads the counter until two consecutive reads agree, so a tick that lands between the partial reads of an 8-bit or 16-bit core no longer returns a torn value.
- `TICKMS_TICKS_TO_MS` and `TICKMS_SEC_TO_TICKS` saturate at `UINT32_MAX` like `tickms_ticks_to_ms()` and `tickms_sec_to_ticks_saturated()`, and `TICKMS_MS_TO_TICKS` no longer wraps for inputs near `UINT32_MAX`. The two saturating macros evaluate their argument more than once.

## [1.3.0] - 2026-06-10

### Added

- Configurable atomic backend: auto-selects GCC/Clang `__atomic`, C11 `<stdatomic.h>`, or a `volatile` uniprocessor fallback, overridable via the `TICKMS_USE_*` macros. The fallback is strictly standard C, so toolchains with C11 but no `_Atomic` (e.g. TI C2000) build out of the box.
- Backend regression tests forcing the C11 and no-atomics paths.

### Changed

- Added changelog, contributor guide, security policy, SPDX headers, README badge, cleaned ignore rules, and a standard coverage gate.

## [1.2.2] - 2026-03-16

### Added

- Configurable tick-based timer system for time measurement, timeout handling, duration conversion, wrap-around-safe arithmetic, fixed-size storage, deterministic execution, and Meson subproject integration.

### Fixed

- Replaced the smoke-test shell script with a Python script to avoid subproject consumer test failures.
