#!/bin/sh
#
# UCSD p-System filesystem in user space
# Copyright (C) 2010 Peter Miller
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

TEST_SUBJECT="ucsdpsys_disk with -A"
. test_prelude

dt1=`date +%e-%b-%y`
dt2=`date +%Y-%m-%d`

cat > test.ok << fubar
TEST:
.FILE2.TEXT       2048 ${dt1} textfile
FILE1.TEXT        2048 ${dt1} textfile
2 of 77 files
14 of 280 blocks, 95.0% free
Last mounted ${dt2}
fubar
test $? -eq 0 || no_result

mkdir -p dir
test $? -eq 0 || no_result

date > dir/file1.text
test $? -eq 0 || no_result

date > dir/.file2.text
test $? -eq 0 || no_result

ucsdpsys_mkfs --label=test test.vol
test $? -eq 0 || no_result

ucsdpsys_disk -f test.vol -A --put dir
test $? -eq 0 || fail

ucsdpsys_disk -f test.vol --list --sort=name > test.out
test $? -eq 0 || no_result

diff test.ok test.out
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
