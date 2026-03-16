project(
  'tickms',
  'c',
  version: '1.2.2',
  default_options: [
    'c_std=c11',
    'warning_level=3',
  ],
  license: 'MIT',
)

project_description = 'Configurable tick-based timer system for time measurement, timeout handling, and, duration calculation.'

ver = meson.project_version().split('.')
major = ver[0].to_int()
minor = ver[1].to_int()
patch = ver[2].to_int()

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
conf.set('MAJOR', major)
conf.set('MINOR', minor)
conf.set('PATCH', patch)

generated_version_header = configure_file(
  input: 'config' / 'tickms_version.h.in',
  output: 'tickms_version.h',
  configuration: conf,
  install: true,
  install_dir: get_option('includedir') / 'tickms',
)

public_headers = include_directories('include')
generated_headers = include_directories('.')

library_sources = files('src/tickms.c')

tickms_lib = static_library(
  'tickms',
  library_sources,
  include_directories: [public_headers, generated_headers],
  install: true,
)

install_headers('include/tickms.h', 'include/tickms_conf.h', subdir: 'tickms')

tickms_dep = declare_dependency(
  include_directories: [public_headers, generated_headers],
  link_with: tickms_lib,
  sources: generated_version_header,
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
