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

TEST_SUBJECT="ucsdpsys_disk --boot"
. test_prelude

echo "Example" > boot.bin
test $? -eq 0 || no_result

ucsdpsys_mkfs -B140 --label=example example.vol
test $? -eq 0 || no_result

ucsdpsys_disk -f example.vol --put --boot=boot.bin
test $? -eq 0 || fail

cat > ok.1 << 'fubar'
0000000  45  78  61  6d  70  6c  65  0a  00  00  00  00  00  00  00  00
          E   x   a   m   p   l   e  \n  \0  \0  \0  \0  \0  \0  \0  \0
0000020  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
         \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0
*
0002000  00  00  06  00  00  00  07  45  58  41  4d  50  4c  45  18  01
         \0  \0 006  \0  \0  \0  \a   E   X   A   M   P   L   E 030 001
fubar
test $? -eq 0 || no_result

# this is a bug in old versions of GNU Coreutils
cat > ok.2 << 'fubar'
0000000 45 78 61 6d 70 6c 65 0a 00 00 00 00 00 00 00 00
          E   x   a   m   p   l   e  \n  \0  \0  \0  \0  \0  \0  \0  \0
0000020 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
         \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0
*
0002000 00 00 06 00 00 00 07 45 58 41 4d 50 4c 45 18 01
         \0  \0 006  \0  \0  \0  \a   E   X   A   M   P   L   E 030 001
fubar
test $? -eq 0 || no_result

od -tx1 -c example.vol > test.out.2
test $? -eq 0 || no_result

head -7 test.out.2 > test.out
test $? -eq 0 || no_result

diff ok.2 test.out > /dev/null 2> /dev/null && pass

diff ok.1 test.out
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
