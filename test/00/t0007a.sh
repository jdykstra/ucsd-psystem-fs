#!/bin/sh
#
# UCSD p-System filesystem in user space
# Copyright (C) 2006-2008, 2010 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# you option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>
#

TEST_SUBJECT="unlink"
. test_prelude

if test -w /dev/fuse -a -r /etc/mtab
then
    : ok
else
    echo
    echo "    This system does not appear to provide the /dev/fuse device,"
    echo "    needed to run the FUSE tests, probably because it is in a build"
    echo "    chroot.  This test is declared to pass by default."
    echo
    pass
fi

#
# Create a disk image
#
ucsdpsys_mkfs -B800 disk.image
test $? -eq 0 || fail

mkdir mnt
test $? -eq 0 || no_result

#
# Mount the disk image.
#
ucsdpsys_mount $testdir/disk.image $testdir/mnt
test $? -eq 0 || fail

umount_and_fail() {
    ucsdpsys_umount $testdir/mnt
    fail
}

umount_and_no_result() {
    ucsdpsys_umount $testdir/mnt
    no_result
}

trap "umount_and_no_result" 1 2 3 15

#
# write files into the disk image
#
date > $testdir/mnt/abcd
test $? -eq 0 || umount_and_no_result

date > $testdir/mnt/efgh
test $? -eq 0 || umount_and_no_result

#
# delete the first one
#
rm $testdir/mnt/abcd
test $? -eq 0 || umount_and_fail

#
# See how many are left
#
cat > test.ok << 'fubar'
EFGH
fubar
test $? -eq 0 || umount_and_no_result

ls $testdir/mnt > test.out
test $? -eq 0 || umount_and_no_result

diff test.ok test.out
test $? -eq 0 || umount_and_fail

ucsdpsys_umount $testdir/mnt
test $? -eq 0 || no_result

#
# Make sure the file system is OK.
#
ucsdpsys_fsck disk.image
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
