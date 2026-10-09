if get_option('ebt_only').disabled()
  install_man('man/xpt.8')
  install_man('man/libxpt.3')
  install_man('man/xpt.manifest.5')
  install_man('man/xpt.tree.5')
  install_man('man/xpt.index.5')
endif
if get_option('ebt').enabled()
  install_man('man/mkfhs.1')
  install_man('man/ebs.1')
  install_man('man/ecw.8')
  install_man('man/ebs.5')
  install_man('man/rig.1')
endif

install_data(
  'examples/ebs.formula',
  install_dir: join_paths(get_option('datadir'), 'xpt'),
)
install_data(
  'examples/xpt.manifest',
  install_dir: join_paths(get_option('datadir'), 'xpt'),
)
install_data(
  'examples/xpt.index',
  install_dir: join_paths(get_option('datadir'), 'xpt'),
)
