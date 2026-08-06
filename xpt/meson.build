xpt_sources = files(
  'main.c',
)

xpt_link_args = []
if get_option('static')
  xpt_link_args += ['-static']
endif

xpt_exe = executable(
  'xpt',
  xpt_sources,
  dependencies: libxpt_dep,
  c_args: xpt_c_args,
  link_args: xpt_link_args,
  install: true
)

install_emptydir(join_paths(get_option('localstatedir'), 'lib', 'xpt'))
install_emptydir(join_paths(get_option('sysconfdir'), 'xpt.d'))
install_emptydir(join_paths(get_option('localstatedir'), 'cache', 'xpt.packages'))
