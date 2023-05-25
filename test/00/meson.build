env = environment()
env.prepend('PATH', fs.parent(disk_exe.full_path()))
env.prepend('PATH', fs.parent(fsck_exe.full_path()))
env.prepend('PATH', fs.parent(interleave_exe.full_path()))
env.prepend('PATH', fs.parent(logo_exe.full_path()))
env.prepend('PATH', fs.parent(mkfs_exe.full_path()))
env.prepend('PATH', fs.parent(mount_exe.full_path()))
env.prepend('PATH', fs.parent(rt11_exe.full_path()))
env.prepend('PATH', fs.parent(test_rdwr_exe.full_path()))
env.prepend('PATH', fs.parent(test_statfs_exe.full_path()))
env.prepend('PATH', fs.parent(text_exe.full_path()))
env.prepend('PATH', fs.parent(umount_exe.full_path()))
env.prepend('PATH', meson.project_source_root() / 'script')

cases = [
  ['t0001a', [fsck_exe, mkfs_exe]],
  ['t0002a', [fsck_exe, mkfs_exe, mount_exe, umount_exe]],
  ['t0003a', [test_rdwr_exe, fsck_exe, mkfs_exe, mount_exe, umount_exe]],
  ['t0005a', [test_statfs_exe, fsck_exe, mkfs_exe, mount_exe, umount_exe]],
  ['t0006a', [text_exe]],
  ['t0007a', [fsck_exe, mkfs_exe, mount_exe, umount_exe]],
  ['t0008a', [fsck_exe, mkfs_exe, mount_exe, umount_exe]],
  ['t0009a', [test_rdwr_exe, fsck_exe, mkfs_exe, mount_exe, umount_exe]],
  ['t0010a', [test_rdwr_exe, fsck_exe, mkfs_exe, mount_exe, umount_exe]],
  ['t0011a', [disk_exe, fsck_exe, mkfs_exe]],
  ['t0015a', [disk_exe, mkfs_exe]],
  ['t0016a', [disk_exe, mkfs_exe]],
  ['t0017a', [disk_exe, mkfs_exe]],
  ['t0018a', [disk_exe, mkfs_exe, text_exe]],
  ['t0019a', [disk_exe, mkfs_exe]],
  ['t0020a', [disk_exe, mkfs_exe]],
  ['t0021a', [disk_exe, mkfs_exe]],
  ['t0022a', [mkfs_exe]],
  ['t0023a', [disk_exe, mkfs_exe]],
  ['t0025a', [disk_exe, mkfs_exe]],
  ['t0026a', [disk_exe]],
  ['t0027a', [disk_exe, mkfs_exe]],
  ['t0028a', [mkfs_exe]],
  ['t0029a', [disk_exe, mkfs_exe]],
  ['t0030a', [disk_exe, mkfs_exe]],
  ['t0031a', [disk_exe, mkfs_exe]],
]

foreach case : cases
  name = case[0]
  depends = case[1]
  test(name, sh, args : files(name + '.sh'), env : env, depends : depends)
endforeach
