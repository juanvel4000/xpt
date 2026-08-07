libfetch_sources = files('common.c', 'fetch.c', 'http.c', 'openssl-compat.c')

libfetch = static_library(
    'fetch',
    libfetch_sources,
    c_args: ['-D_XOPEN_SOURCE=700', '-D_GNU_SOURCE'],
    install: false,
)

libfetch_dep = declare_dependency(
    link_with: libfetch,
    include_directories: include_directories('.'),
)


