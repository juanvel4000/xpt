project(
    'xpt',
    'c',
    version: '0.5.2',
    default_options: ['warning_level=3', 'c_std=c11'],
    meson_version: '>=1.1',
)

inc = include_directories('include')

xpt_c_args = ['-D_GNU_SOURCE', '-D_XOPEN_SOURCE=700']


subdir('libfetch')
subdir('libxpt')
subdir('xpt')
if not get_option('docs').disabled()
    subdir('docs')
endif
if not get_option('ebt').disabled()
    subdir('ebt')
endif

