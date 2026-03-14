# tickms

Configurable tick-based timer system for time measurement, timeout handling, and, duration calculation.

## Features

_Add a short list of the projects feature here_

- **<Topic>**: <description>

## Using the Library

### As a Meson subproject

```meson
tickms_dep = dependency('tickms', fallback: ['tickms', 'tickms_dep'])
```

The template also exports `meson.override_dependency('tickms', ...)`
so downstream Meson builds can resolve the subproject dependency by name.

For subproject builds, include the public header directly:

```c
#include "tickms.h"
```

### As an installed dependency

If the library is installed system-wide, include the namespaced header path:

```c
#include <tickms/tickms.h>
```

If `pkg-config` files are installed in your environment, downstream builds can
also discover the package as `tickms`.

The generated version header is available as `tickms_version.h` in the
build tree and as `<tickms/tickms_version.h>` after install.

## Building

```sh
# Library only (release)
meson setup build --buildtype=release -Dbuild_tests=false
meson compile -C build

# With unit tests
meson setup build --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

## Quick Start

_Add a code snippet for a quick start use for the project_

## API Reference

_Add the API reference here_

## Use Cases

_Add a small list of use case examples_

- **<Topic>**: <description>

## Notes

| Topic | Note |
|-------|------|
| **<Topic>** | <description> |
