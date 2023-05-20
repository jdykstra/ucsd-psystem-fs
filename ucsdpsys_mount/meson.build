mount_exe = executable(
  'ucsdpsys_mount',
  sources : 'main.cc',
  include_directories : root_inc,
  implicit_include_directories : false,
  dependencies : [libexplain_dep, fuse_dep],
  link_with : lib_lib,
  install : true,
)
