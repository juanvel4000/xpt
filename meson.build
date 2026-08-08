project(
    'xpt',
    'c',
    version: run_command('cat', 'VERSION', check: true).stdout().strip(),
    default_options: ['warning_level=3', 'c_std=c11'],
    meson_version: '>=1.1',
)

inc = include_directories('include')

xpt_c_args = ['-D_GNU_SOURCE', '-D_XOPEN_SOURCE=700']

ebt_only = get_option('ebt_only')
ebt = get_option('ebt')

if ebt_only.enabled() and ebt.disabled()
    error('ebt_only requires ebt to not be disabled')
endif

if get_option('ebt_only').disabled()
    subdir('libfetch')
    subdir('libxpt')
    subdir('xpt')
endif
if get_option('docs').enabled()
    subdir('docs')
endif
if get_option('ebt').enabled()
    subdir('ebt')
endif
if get_option('tests').enabled()
    subdir('tests')
endif
