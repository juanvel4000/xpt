libxpt_sources = files(
  'common.c',
  'lockfile.c',
  'database.c',
  'manager.c',
  'packages.c',
  'manifests.c',
  'resolver.c',
  'repo.c'
)

sqlite3 = dependency('sqlite3')
libarchive = dependency('libarchive')
openssl = dependency('openssl')

libxpt_deps = [
    sqlite3,
    libarchive,
    openssl,
    libfetch_dep,
]

conf = configuration_data()

cc_id = meson.get_compiler('c')
compiler_str = '@0@ @1@'.format(cc_id.get_id(), cc_id.version())

conf.set_quoted('XPT_PREFIX',        get_option('prefix'))
conf.set_quoted('XPT_SYSCONFDIR',    get_option('sysconfdir'))
conf.set_quoted('XPT_LOCALSTATEDIR', get_option('localstatedir'))
conf.set_quoted('XPT_BUILDTYPE',     get_option('buildtype'))
conf.set_quoted('XPT_BUILD_COMPILER', compiler_str)
conf.set_quoted('XPT_SQLITE3_VERSION',   sqlite3.version())
conf.set_quoted('XPT_LIBARCHIVE_VERSION', libarchive.version())
conf.set_quoted('XPT_OPENSSL_VERSION',    openssl.version())

conf.set_quoted(
  'XPT_LOCALSTATEDIR',
  join_paths(get_option('prefix'), get_option('localstatedir')),
)

conf.set_quoted(
  'XPT_SYSCONFDIR',
  join_paths(get_option('prefix'), get_option('sysconfdir')),
)

config_h = configure_file(
  input: '../include/xpt/config.h.in',
  output: 'config.h',
  configuration: conf,
)

libxpt = library(
  'xpt',
  libxpt_sources,
  version: meson.project_version(),
  soversion: '0',
  include_directories: [
    inc,
    include_directories('.')
  ],
  c_args: xpt_c_args,
  dependencies: libxpt_deps,
  install: true
)

libxpt_dep = declare_dependency(
    link_with: libxpt,
    include_directories: [
      inc,
      include_directories('.'),
    ],
)

install_headers(
  '../include/xpt/xpt.h',
  subdir: 'xpt'
)

pkg = import('pkgconfig')

pkg.generate(
  libxpt,
  name: 'libxpt',
  description: 'XPT package management library',
  filebase: 'libxpt',
  version: meson.project_version()
)
