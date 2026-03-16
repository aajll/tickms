#!/usr/bin/env python3
"""Stage an install and compile a minimal external consumer."""

from __future__ import annotations

import os
import pathlib
import shutil
import subprocess
import sys
import tempfile


def find_single_path(root: pathlib.Path, pattern: str) -> pathlib.Path:
    matches = sorted(root.rglob(pattern))
    if len(matches) != 1:
        print(
            f"FAIL: expected exactly one match for {pattern!r} under {root}, "
            f"found {len(matches)}",
            file=sys.stderr,
        )
        for match in matches:
            print(f"  {match}", file=sys.stderr)
        raise SystemExit(1)
    return matches[0]


def main() -> int:
    if len(sys.argv) != 5:
        print(
            "usage: run_smoke_install.py <source_root> <build_root> <prefix> <libdir>",
            file=sys.stderr,
        )
        return 2

    source_root = pathlib.Path(sys.argv[1]).resolve()
    build_root = pathlib.Path(sys.argv[2]).resolve()
    prefix_str = sys.argv[3]
    libdir = pathlib.Path(sys.argv[4])

    meson = shutil.which("meson")
    if meson is None:
        print("FAIL: meson not found in PATH", file=sys.stderr)
        return 1

    with tempfile.TemporaryDirectory(prefix="tickms-smoke-") as temp_dir:
        temp_path = pathlib.Path(temp_dir)
        destdir = temp_path / "destdir"
        consumer_src = temp_path / "consumer-src"
        consumer_build = temp_path / "consumer-build"

        consumer_src.mkdir()

        subprocess.run(
            [str(meson), "install", "-C", str(build_root), "--destdir", str(destdir)],
            check=True,
            stdout=subprocess.DEVNULL,
        )

        pkgconfig_dir = destdir / pathlib.PurePosixPath(
            prefix_str.lstrip("/") / libdir / "pkgconfig"
        )

        if not pkgconfig_dir.is_dir():
            print(f"Missing pkg-config directory: {pkgconfig_dir}", file=sys.stderr)
            return 1

        consumer_meson = consumer_src / "meson.build"
        consumer_meson.write_text(
            """project('tickms-smoke', 'c', default_options: ['c_std=c11'])

tickms_dep = dependency('tickms', method: 'pkg-config', required: true)

executable('tickms_smoke', 'main.c', dependencies: [tickms_dep])
"""
        )

        consumer_main = consumer_src / "main.c"
        consumer_main.write_text(
            """#define TICKMS_MS_PER_TICK 20u
#include <tickms/tickms_conf.h>
#include <tickms/tickms.h>
#include <tickms/tickms_version.h>

int
main(void)
{
        tickms_init(5u);

        if (TICKMS_MS_PER_TICK != 20u) {
                return 1;
        }

        if (tickms_current_ms() != 100u) {
                return 1;
        }

        if (TICKMS_VERSION_STRING[0] == '\0') {
                return 1;
        }

        return 0;
}
"""
        )

        env = os.environ.copy()
        env["PKG_CONFIG_PATH"] = str(pkgconfig_dir)
        env["PKG_CONFIG_SYSROOT_DIR"] = str(destdir)

        subprocess.run(
            [str(meson), "setup", str(consumer_build), str(consumer_src)],
            check=True,
            env=env,
            stdout=subprocess.DEVNULL,
        )

        subprocess.run(
            [str(meson), "compile", "-C", str(consumer_build)],
            check=True,
            env=env,
            stdout=subprocess.DEVNULL,
        )

        smoke_exe = consumer_build / "tickms_smoke"
        result = subprocess.run([str(smoke_exe)], check=False)

    if result.returncode != 0:
        print("FAIL: smoke test executable returned non-zero", file=sys.stderr)
        return result.returncode

    print("PASS: tickms_smoke")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
