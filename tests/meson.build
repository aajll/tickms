test_exe = executable(
  'test_tickms',
  files('test_tickms.c'),
  dependencies: [tickms_dep],
  include_directories: [public_headers, generated_headers],
)

test('tickms module', test_exe)
