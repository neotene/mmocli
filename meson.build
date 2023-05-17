project('mmocli', 'cpp', version : '0.1',
  default_options : ['cpp_std=c++17'])

boost       = dependency('boost')
gtest       = dependency('gtest')
gtest_main  = dependency('gtest_main')
neo         = dependency('neo')
json        = dependency('nlohmann_json')
base64      = dependency('cppbase64')
ssl         = dependency('libssl')
openssl     = dependency('openssl')
crypto      = dependency('libcrypto')
log         = dependency('spdlog')

deps = [boost, neo, json, base64, ssl, openssl, crypto, log]

libneo_cpp_args = []

if get_option('default_library') == 'static'
  libneo_cpp_args += '-DNEO_STATIC'
endif

common_src = ['utils.cpp']

incs = include_directories('.')

executable('mmocli-server',
  common_src + 'main_server.cpp',
  dependencies : deps,
  cpp_args : libneo_cpp_args,
  include_directories : incs,
)

executable('mmocli-client',
  common_src + 'main_client.cpp',
  dependencies : deps,
  cpp_args : libneo_cpp_args,
  include_directories : incs,
)

executable('mmocli-cli',
  common_src + 'main_cli.cpp',
  dependencies : deps,
  cpp_args : libneo_cpp_args,
  include_directories : incs,
)

tests = executable('mmocli-test',
  common_src + 'main_test.cpp',
  dependencies : deps + [gtest, gtest_main],
  cpp_args : libneo_cpp_args,
  include_directories : incs,
)

test('mmocli-test', tests)
