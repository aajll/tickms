cc = meson.get_compiler('c')
python = find_program('python3')

test_exe = executable(
  'test_tickms',
  files('test_tickms.c'),
  dependencies: [tickms_dep],
  include_directories: [public_headers, generated_headers],
)

test('tickms module', test_exe)

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
# project that carries no sanitizer flags.  A sanitizer-instrumented .a
# embeds __asan_* / __ubsan_* symbols that the consumer linker cannot
# resolve, so the test is structurally incompatible with sanitized builds.
if get_option('b_sanitize') == 'none'
  test(
    'tickms installed consumer smoke',
    python,
    args: [
      files('run_smoke_install.py'),
      meson.project_source_root(),
      meson.project_build_root(),
      get_option('prefix'),
      get_option('libdir'),
    ],
  )
endif
