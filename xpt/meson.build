xpt_sources = files(
  'main.c',
)

xpt_exe = executable(
  'xpt',
  xpt_sources,
  dependencies: libxpt_dep,
  c_args: xpt_c_args,
  install: true
)

install_emptydir(join_paths(get_option('localstatedir'), 'lib', 'xpt'))
install_emptydir(join_paths(get_option('sysconfdir'), 'xpt.d'))
install_emptydir(join_paths(get_option('localstatedir'), 'cache', 'xpt.packages'))
