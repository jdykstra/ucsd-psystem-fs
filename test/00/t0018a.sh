#!/bin/sh
#
# UCSD p-System filesystem in user space
# Copyright (C) 2008 Peter Miller
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

TEST_SUBJECT="auto-convert text files"
. test_prelude

#
# Create a disk image
#
ucsdpsys_mkfs disk.image
test $? -eq 0 || fail

cat > guinee-pig.text << 'fubar'
This is a test.
fubar
test $? -eq 0 || no_result

ucsdpsys_disk -t -f disk.image -p guinee-pig.text
test $? -eq 0 || fail

ucsdpsys_disk -B -f disk.image -g test.out.raw=guinee-pig.text
test $? -eq 0 || fail

#
# make sure the text file was created as a 2048 byte binary file on disk.
#
ls -l test.out.raw > test.out
test $? -eq 0 || no_result

echo 2048 > ok
test $? -eq 0 || no_result

awk '{print $5}' test.out > test.out.2

diff ok test.out.2
test $? -eq 0 || fail

#
# decode the binary form of the file,
# and see if it survives the round trip
#
ucsdpsys_text -d < test.out.raw > test.out
test $? -eq 0 || no_result

diff guinee-pig.text test.out
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
