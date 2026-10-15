project(
  'tickms',
  'c',
  version: '1.3.0',
  meson_version: '>=0.57.0',
  default_options: [
    'c_std=c11',
    'warning_level=3',
  ],
  license: 'MIT',
)

project_description = 'Configurable tick-based timer system for time measurement, timeout handling, and duration calculation.'

# === Version ==================================================================
# include/tickms.h defines the version macros, so a copy-in build has them.
# Configuration fails when they differ from the project version.

fs = import('fs')
version_parts = meson.project_version().split('.')
version_macros = {
  'TICKMS_VERSION_MAJOR': version_parts[0],
  'TICKMS_VERSION_MINOR': version_parts[1],
  'TICKMS_VERSION_PATCH': version_parts[2],
  'TICKMS_VERSION': '"' + meson.project_version() + '"',
}
header_lines = []
foreach line : fs.read('include' / 'tickms.h').split('\n')
  header_lines += ' '.join(line.split())
endforeach
foreach name, value : version_macros
  if '#define @0@ @1@'.format(name, value) not in header_lines
    error('include/tickms.h must define @0@ as @1@'.format(name, value))
  endif
endforeach

public_headers = include_directories('include')

library_sources = files('src/tickms.c')

tickms_lib = static_library(
  'tickms',
  library_sources,
  include_directories: [public_headers],
  install: true,
)

install_headers(
  'include/tickms.h',
  'include/tickms_conf.h',
  subdir: 'tickms',
)

tickms_dep = declare_dependency(
  include_directories: [public_headers],
  link_with: tickms_lib,
)

meson.override_dependency('tickms', tickms_dep)

pkg = import('pkgconfig')
pkg.generate(
  tickms_lib,
  description: project_description,
  filebase: 'tickms',
  name: 'tickms',
)

if get_option('build_tests')
  subdir('tests')
endif
