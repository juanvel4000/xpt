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

libxpt = library(
  'xpt',
  libxpt_sources,
  version: meson.project_version(),
  soversion: '0',
  include_directories: inc,
  c_args: xpt_c_args,
  dependencies: libxpt_deps
)

libxpt_dep = declare_dependency(
    link_with: libxpt,
    include_directories: inc,
)
