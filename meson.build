project(
    'xpt',
    'c',
    version: files('VERSION'),
    default_options: [
        'warning_level=3',
        'c_std=c89',
        'werror=true',
        'c_args=-Wall -Wextra',
    ],
    license: 'BSD-3-Clause',
    license_files: ['LICENSE', 'THIRD-PARTY-LICENSES'],
    meson_version: '>=1.12',
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
        subdir('include')
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

if not get_option('doxygen').disabled()
    doxygen = find_program(
        'doxygen',
        required: get_option('doxygen'),
    )

    if doxygen.found()
        config = configuration_data()
        config.set(
            'SOURCE_ROOT',
            meson.project_source_root(),
        )

        config.set(
            'BUILD_ROOT',
            meson.project_build_root(),
        )

        config.set(
            'PROJECT_VERSION',
            meson.project_version(),
        )

        doxyfile = configure_file(
            input: 'Doxyfile.in',
            output: 'Doxyfile',
            configuration: config,
        )

        custom_target(
            'docs',
            output: 'html',
            command: [doxygen, doxyfile],
            depends: [],
            install: true,
            install_dir: get_option('datadir') / 'doc' / 'xpt',
        )
    endif
endif

install_data(
    'LICENSE',
    'THIRD-PARTY-LICENSES',
    install_dir: get_option('datadir') / 'licenses' / 'xpt',
)

install_data(
    'README.md',
    'CHANGELOG.md',
    install_dir: get_option('datadir') / 'doc' / 'xpt',
)
