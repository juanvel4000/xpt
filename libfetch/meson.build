libfetch_sources = files('common.c', 'fetch.c', 'http.c')

openssl = dependency('openssl', static: get_option('static'))

libfetch = static_library(
    'fetch',
    libfetch_sources,
    c_args: ['-D_XOPEN_SOURCE=700', '-D_GNU_SOURCE'],
    dependencies: [openssl],
    install: false,
)

libfetch_dep = declare_dependency(
    link_with: libfetch,
    include_directories: include_directories('.'),
)
