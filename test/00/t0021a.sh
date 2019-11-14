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

TEST_SUBJECT="ucsdpsys_disk --list --sort"
. test_prelude

DT1=`date +%e-%b-%y`
DT2=`date +%Y-%m-%d`

cat > ok << fubar
EXAMPLE:
D.DATA               2 ${DT1} datafile
C.DATA               3 ${DT1} datafile
B.DATA               4 ${DT1} datafile
A.DATA               5 ${DT1} datafile
4 of 77 files
10 of 280 blocks, 96.4% free
Last mounted ${DT2}
fubar
test $? -eq 0 || no_result

mkdir example
test $? -eq 0 || no_result

echo aaaa > example/a.data
test $? -eq 0 || no_result

echo bbb > example/b.data
test $? -eq 0 || no_result

echo cc > example/c.data
test $? -eq 0 || no_result

echo d > example/d.data
test $? -eq 0 || no_result

ucsdpsys_mkfs --label=example example.vol
test $? -eq 0 || no_result

ucsdpsys_disk -f example.vol --put example
test $? -eq 0 || fail

ucsdpsys_disk -f example.vol --sort=size > test.out
test $? -eq 0 || fail

diff ok test.out
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
