project(
  'xpt',
  'c',
  version: '0.5.1',
  default_options: [
    'warning_level=3',
    'c_std=c11'
  ],
  meson_version: '>=1.1',
)

inc = include_directories('include')

xpt_c_args = [
  '-D_GNU_SOURCE',
  '-D_XOPEN_SOURCE=700'
]


subdir('libfetch')
subdir('libxpt')
subdir('xpt')
subdir('docs')
