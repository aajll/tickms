#!/bin/sh

set -eu

build_root=$1
prefix=$2
libdir=$3

tmpdir=$(mktemp -d)
cleanup() {
        rm -rf "$tmpdir"
}
trap cleanup EXIT INT TERM

destdir="$tmpdir/dest"
consumer_src="$tmpdir/consumer-src"
consumer_build="$tmpdir/consumer-build"
pkgconfig_dir="$destdir$prefix/$libdir/pkgconfig"

mkdir -p "$consumer_src"

meson install -C "$build_root" --destdir "$destdir" >/dev/null

if [ ! -d "$pkgconfig_dir" ]; then
        printf 'Missing pkg-config directory: %s\n' "$pkgconfig_dir" >&2
        exit 1
fi

cat >"$consumer_src/meson.build" <<'EOF'
project('tickms-smoke', 'c', default_options: ['c_std=c11'])

tickms_dep = dependency('tickms', method: 'pkg-config', required: true)

executable('tickms_smoke', 'main.c', dependencies: [tickms_dep])
EOF

cat >"$consumer_src/main.c" <<'EOF'
#define TICKMS_MS_PER_TICK 20u
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
EOF

PKG_CONFIG_PATH="$pkgconfig_dir${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
PKG_CONFIG_SYSROOT_DIR="$destdir" \
        meson setup "$consumer_build" "$consumer_src" >/dev/null
PKG_CONFIG_PATH="$pkgconfig_dir${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
PKG_CONFIG_SYSROOT_DIR="$destdir" \
        meson compile -C "$consumer_build" >/dev/null

"$consumer_build/tickms_smoke"
