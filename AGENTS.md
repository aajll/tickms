# AGENTS.md

---

## 1) Project-specific instructions

**Project:** `tickms`
**Primary goal:** A wrap-safe tick counter with timeouts and unit conversions, safe for one writer and concurrent readers on any C11 target.

### 1.1 Essential commands

#### Configure and build (library only)

```sh
meson setup build --wipe --buildtype=release -Dbuild_tests=false
meson compile -C build
```

#### Configure, build, and run tests

```sh
meson setup build --wipe --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

#### MISRA C:2023 (needs a configured build for `compile_commands.json`)

```sh
misch run --baseline
misch run --baseline --profile c11-atomics
misch run --baseline --profile no-atomics
```

#### Formatting (pinned clang-format through `uvx`)

```sh
scripts/format.sh          # check, as CI does
scripts/format.sh --fix    # rewrite in place
```

### 1.2 Notes

- `include/tickms.h` defines the version macros, and `meson setup` fails when they differ from `project(version:)`, so a release updates both. `include/tickms_version.h` is a deprecated header that includes `tickms.h`. Copy-in needs only `include/tickms.h`, `include/tickms_conf.h`, and `src/tickms.c`.
- The installed-consumer smoke test is skipped in sanitiser and coverage builds, because instrumented archives do not link into an uninstrumented consumer.
- The 16-bit lock-free probe in `tests/meson.build` runs only when `clang` is on the path.

---

## 2) Non-negotiable design invariants

- Elapsed time is `current - start` in `tickms_tick_t`. Never compare raw counts to decide a timeout.
- `tickms_tick_t` is exactly `uint32_t`. Conversions widen to `uint64_t` and saturate `uint32_t` results at `UINT32_MAX` rather than wrapping.
- `TICKMS_MS_PER_TICK` lies in 1 to 1000 and divides 1000, enforced by static assertions.
- One writer, any number of readers, relaxed ordering. Never add a lock, and never imply ordering for data other than the counter.
- Every counter access goes through the `TICKMS_ATOMIC_*` macros. All three backends must keep building and passing.
- The atomic backends must keep their lock-free static assertions. Never let a counter access fall through to libatomic.
- The single-core backend stays strictly standard C, with no compiler extensions.

---

## 3) CI / source of truth

- CI definitions live in `.github/workflows/ci.yml`.
- Prefer running the same commands locally as CI runs.
- Install Python tools (Meson, Ninja, gcovr, misch) with `uv tool install`, never `pip`; CI does the same through `astral-sh/setup-uv`.
- The CI jobs are: tests under ASan/UBSan, ThreadSanitizer, a release build, a TI C2000 cross-build (CGT 22.6.2 and 25.11.1, COFF and EABI, warnings as errors), `clang-format`, a 100 percent line and branch coverage gate, and `misch` against the baselines for all three backends.
- misch baselines are recorded with `MISRA_RULE_TEXTS` unset, because CI has no rule texts and the texts change the fingerprinted messages.

---

## 4) Docs / commit conventions

- Use Conventional Commits format when asked to commit.
- Keep commits focused and explain why in the message body.
- `README.md` is a short landing page: a quick start, a description, validated toolchains, and links. Put detail in `docs/`, not in the README.
- `docs/integration.md` covers installation, configuration, building, and cross builds.
- `docs/api.md` is the API reference. Use fenced C signatures followed by prose, never a table of functions. Update it with the header Doxygen whenever an interface changes.
- C code blocks in `README.md`, `CONTRIBUTING.md`, and `docs/` compile against the header in the `tickms docs examples` test. On the line directly above the fence, mark a block of statements with `<!-- snippet: body -->` and pseudo-code with `<!-- snippet: skip -->`.
- `docs/design.md` is the single authoritative design document. Keep it standalone and in step with behaviour.
- Write prose in British English and do not hard-wrap markdown.

---

## 5) C style expectations

### Build and configuration

- Use the Meson build system. Do not introduce CMake, Make, or other systems.
- Update `meson.build` when adding or removing library source files, and `tests/meson.build` when adding or removing tests.

### Formatting

- `.clang-format` is present and mandatory. Run `scripts/format.sh --fix` before committing. It runs the clang-format release pinned in the script through `uvx`. Do not use a system `clang-format`: another release can format the same code differently.
- Do not reformat unrelated code.
- Key settings: 8-space indent, `BreakBeforeBraces: Linux`, column limit 80.

### Style and correctness

- Match conventions in the existing files (indentation, braces, naming).
- Validate pointer arguments at every public API boundary.
- No heap allocation (`malloc`, `free`, VLAs).
- Never compute in plain `int`; it can be 16 bits wide. Do not use `uint8_t`, and do not assume a C byte is an octet.

### Comment placement (Doxygen)

- Inline trailing annotations (`/**< ... */`) on members are allowed only when the line fits 80 columns. If a member would overrun, move all of that aggregate's member docs into a `@details` list above the type.
- Never mix inline and block forms within one aggregate.

### Testing

- Run `meson test -C build` after every change.
- Add a test case for each bug fix and each new behaviour.
- Keep `src/` and `include/` at 100 percent line and branch coverage; the CI gate enforces it.
- Tests live in `tests/test_*.c`. A backend-specific test compiles `src/tickms.c` with the same `TICKMS_USE_*` selector.

---
