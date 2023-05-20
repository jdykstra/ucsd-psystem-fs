project('ucsd-psystem-fs', 'cpp')

fs = import('fs')

cpp = meson.get_compiler('cpp')
sh = find_program('sh', '/bin/sh')

libexplain_dep = dependency('libexplain')
fuse_dep = dependency('fuse')
boost_dep = dependency('boost')

root_inc = include_directories('.')

subdir('lib')

subdir('ucsdpsys_disk')
subdir('ucsdpsys_fsck')
subdir('ucsdpsys_interleave')
subdir('ucsdpsys_logo')
subdir('ucsdpsys_mkfs')
subdir('ucsdpsys_rt11')
subdir('ucsdpsys_text')

subdir('ucsdpsys_mount')
subdir('ucsdpsys_umount')

subdir('man')

subdir('test_rdwr')
subdir('test_statfs')
subdir('test/00')
