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

TEST_SUBJECT="system.pascal"
. test_prelude

date1="`date '+%e-%b-%y'`"
date2="`date '+%Y-%m-%d'`"
cat > ok << fubar
WORK:
SYSTEM.PASCAL       29 ${date1} codefile
1 of 77 files
7 of 280 blocks, 97.5% free
Last mounted ${date2}
fubar
test $? -eq 0 || no_result

ucsdpsys_mkfs test.vol -L work
test $? -eq 0 || no_result

date > system.pascal
test $? -eq 0 || no_result

ucsdpsys_disk -f test.vol -p system.pascal
test $? -eq 0 || fail

ucsdpsys_disk -f test.vol -l > test.out
test $? -eq 0 || no_result

diff ok test.out
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
