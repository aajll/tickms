cc = meson.get_compiler('c')
python = find_program('python3')

test_exe = executable(
  'test_tickms',
  files('test_tickms.c'),
  dependencies: [tickms_dep],
  include_directories: [public_headers, generated_headers],
)

test('tickms module', test_exe)

# C11 <stdatomic.h> backend. The default on GCC/Clang is the __atomic path, so
# this forces the C11 backend for both the test and the core (compiled from
# library_sources with the same -D) to exercise that path.
test_c11_exe = executable(
  'test_tickms_c11',
  files('test_tickms_c11.c') + library_sources,
  include_directories: [public_headers, generated_headers],
  c_args: ['-DTICKMS_USE_C11_ATOMICS=1'],
)
test('tickms c11', test_c11_exe)

# Degenerate uniprocessor (volatile) backend. Forces the no-atomics path for
# both the test and the core to verify the volatile fallback macros expand
# cleanly and produce correct single-threaded results.
test_noatomics_exe = executable(
  'test_tickms_noatomics',
  files('test_tickms_noatomics.c') + library_sources,
  include_directories: [public_headers, generated_headers],
  c_args: ['-DTICKMS_USE_NO_ATOMICS=1'],
)
test('tickms noatomics', test_noatomics_exe)

foreach tick_ms : [1, 20, 250]
  cfg_test = executable(
    'test_tickms_cfg_@0@ms'.format(tick_ms),
    files('test_tickms_config_matrix.c'),
    dependencies: [tickms_dep],
    include_directories: [public_headers, generated_headers],
    c_args: [
      '-DTICKMS_MS_PER_TICK=@0@u'.format(tick_ms),
      '-DEXPECTED_TICKMS_MS_PER_TICK=@0@u'.format(tick_ms),
    ],
  )

  test('tickms config @0@ ms'.format(tick_ms), cfg_test)
endforeach

config_probe = '''
  #include "tickms.h"
  int main(void) { return 0; }
'''

foreach tick_ms : [1, 20, 250]
  assert(
    cc.compiles(
      config_probe,
      name: 'tickms valid config @0@ ms'.format(tick_ms),
      args: [
        '-I' + (meson.project_source_root() / 'include'),
        '-DTICKMS_MS_PER_TICK=@0@u'.format(tick_ms),
      ],
    ),
    'Expected TICKMS_MS_PER_TICK=@0@u to compile'.format(tick_ms),
  )
endforeach

foreach tick_ms : [0, 3, 1001]
  assert(
    not cc.compiles(
      config_probe,
      name: 'tickms invalid config @0@ ms'.format(tick_ms),
      args: [
        '-I' + (meson.project_source_root() / 'include'),
        '-DTICKMS_MS_PER_TICK=@0@u'.format(tick_ms),
      ],
    ),
    'Expected TICKMS_MS_PER_TICK=@0@u to fail compilation'.format(tick_ms),
  )
endforeach

# The smoke test installs libtickms.a and links it from a plain consumer
# project that carries no sanitizer or coverage flags. Instrumented static
# archives embed sanitizer/gcov runtime symbols that the consumer linker
# cannot resolve, so the test is structurally incompatible with sanitizer
# and coverage builds.
if get_option('b_sanitize') == 'none' and not get_option('b_coverage')
  test(
    'tickms installed consumer smoke',
    python,
    args: [
      files('run_smoke_install.py'),
      meson.project_source_root(),
      meson.global_build_root(),
      get_option('prefix'),
      get_option('libdir'),
    ],
  )
endif
