project(
    'xpt',
    'c',
    version: files('VERSION'),
    default_options: [
        'warning_level=3',
        'c_std=c99',
        'werror=true',
        'c_args=-Wall -Wextra',
    ],
    meson_version: '>=1.1',
)

inc = include_directories('include')

xpt_c_args = ['-D_XOPEN_SOURCE=700', '-D_DEFAULT_SOURCE']

ebt_only = get_option('ebt_only')
ebt = get_option('ebt')
networking = get_option('networking')
libxpt = get_option('libxpt')
xpt = get_option('xpt')
doc = get_option('doc')
tests = get_option('tests')

if ebt_only.enabled() and ebt.disabled()
    error('ebt_only requires ebt to not be disabled')
endif

if xpt.enabled() and libxpt.disabled()
    error('xpt requires libxpt')
endif

if ebt_only.disabled()
    if networking.enabled()
        subdir('libfetch')
    endif
    if libxpt.enabled()
        subdir('libxpt')
    endif
    if xpt.enabled()
        subdir('xpt')
    endif
endif
if doc.enabled()
    subdir('doc')
endif
if ebt.enabled()
    subdir('ebt')
endif
if tests.enabled()
    subdir('tests')
endif
