# Contributing to tickms

`tickms` is a small C11 library for a wrap-safe tick counter with timeouts and unit conversions. Two contracts carry most of the design weight: elapsed time is unsigned subtraction that stays correct across the wrap, and the counter has one writer and lock-free readers. Keep both in mind when you change anything.

## Getting started

Install a C11 compiler and install the Python tools with [uv](https://docs.astral.sh/uv/), as CI does. The MISRA analysis also needs `cppcheck` from the system package manager. The same commands CI runs, locally:

```sh
# Python tools
uv tool install meson
uv tool install ninja
uv tool install gcovr
uv tool install misch

# Configure with tests and sanitisers (CI default)
meson setup build --buildtype=debug -Dbuild_tests=true -Db_sanitize=address,undefined
meson compile -C build
meson test -C build --verbose

# ThreadSanitizer
meson setup build_tsan --buildtype=debug -Dbuild_tests=true -Db_sanitize=thread
meson compile -C build_tsan && meson test -C build_tsan

# Coverage (CI gate is 100% line and 100% branch for src/ and include/)
meson setup build_cov --buildtype=debug -Dbuild_tests=true -Db_coverage=true -Dc_args=-fprofile-update=atomic
meson compile -C build_cov && meson test -C build_cov
gcovr --root . --filter 'src/' --filter 'include/' --print-summary

# MISRA C:2023 against the recorded baselines, one per atomic backend
meson setup build --buildtype=debug -Dbuild_tests=true   # compile_commands.json
misch run --baseline
misch run --baseline --profile c11-atomics
misch run --baseline --profile no-atomics

# Formatting (pinned clang-format through uvx)
scripts/format.sh --fix
```

## Source style

- Formatting is mandatory. Run `scripts/format.sh --fix` before submitting, or `scripts/format.sh` to check without writing, as CI does. The script pins the clang-format release and runs it through `uvx`, so an editor formatter gives the same result only when it runs the same release.
- 8-space indent, Linux brace style, 80-column limit. Match the existing conventions and do not reformat unrelated code.
- The Meson build system is the single source of truth. Update `meson.build` and `tests/meson.build` when you add or remove files.
- No CMake, no Make, and no other build systems.

## C language rules

- C11 only. Use fixed-width types from `<stdint.h>` and `<stdbool.h>`. Do not use `uint8_t`.
- No heap allocation (`malloc`, `free`, VLAs).
- Compute conversions in `tickms_tick_t` or `uint64_t`, never in `int`, which can be 16 bits wide.
- Route every counter access through the `TICKMS_ATOMIC_*` macros in `tickms_conf.h`, and keep all three backends building.
- Keep the single-core backend in strictly standard C, with no compiler extensions.
- Validate pointer arguments at every public API boundary.

## MISRA C:2023

The library is analysed with `misch` (cppcheck-backed MISRA C:2023 analysis), configured by `misra.toml`. The default run covers the GCC/Clang builtins backend, and the `c11-atomics` and `no-atomics` profiles cover the other two. Each has a baseline in `analysis/baseline/`, and CI fails on any finding that is not in it.

A change must not add findings. If it removes one, regenerate the affected baselines with `MISRA_RULE_TEXTS` unset, because the rule texts change the messages the baseline records:

```sh
env -u MISRA_RULE_TEXTS misch baseline
env -u MISRA_RULE_TEXTS misch baseline --profile c11-atomics
env -u MISRA_RULE_TEXTS misch baseline --profile no-atomics
```

Full tool-driven compliance beyond this analysis needs a certified static analyser, which this repository does not vendor.

## Tests and coverage

- Add a test for every bug fix and every new behaviour.
- CI enforces 100 percent line and branch coverage of `src/` and `include/`.
- Tests live in `tests/test_*.c` and are registered in `tests/meson.build`. Backend-specific tests compile `src/tickms.c` with the same `TICKMS_USE_*` selector as the test.
- The `tickms docs examples` test compiles every C code block in `README.md`, `CONTRIBUTING.md`, and `docs/` against the public header, through `tests/docs/check_snippets.py`. A block compiles at file scope by default. On the line directly above the fence, put `<!-- snippet: body -->` for a block of statements, or `<!-- snippet: skip -->` for pseudo-code or a copy of a header definition.

## Documentation

| File | Purpose |
| --- | --- |
| `README.md` | Landing page: quick start, description, validated toolchains, and links. Keep it short. |
| `docs/integration.md` | Installation, configuration, building, and cross builds. |
| `docs/api.md` | Reference for every public function and macro. |
| `docs/design.md` | Counter model, wrap and saturation rules, concurrency, portability, and verification. |

Update `docs/api.md` and the header Doxygen together when an interface changes. Update `docs/design.md` when behaviour changes. Keep it standalone: describe how the library works, not how it was built or which other projects use it. Write prose in British English, one line per paragraph.

## Commits

Use Conventional Commits:

- `feat: ...` new feature
- `fix: ...` bug fix
- `doc: ...` documentation only
- `test: ...` test-only changes
- `chore: ...` build, CI, release work
- `refactor: ...` a change that neither fixes a bug nor adds a feature

Keep the subject under about 70 characters. Use the body to explain why the change is needed.

## Pull requests

- Open an issue first for non-trivial changes so the design can be agreed before implementation.
- Keep pull requests focused: one feature or one fix.
- All CI checks must pass.

## When in doubt

Open an issue. The counter is shared by every timeout in a program, so a small change to its contract reaches everything that uses it.
