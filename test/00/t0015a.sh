#!/bin/sh
#
# UCSD p-System filesystem in user space
# Copyright (C) 2006-2008 Peter Miller
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

TEST_SUBJECT="ucsdpsys_disk new=old"
. test_prelude

#
# Make sure we can insert file into disk images where the Unix name and
# the UCSD p-System file name are different.
#
ucsdpsys_mkfs test.vol
test $? -eq 0 || no_result

echo this is a test > junk
test $? -eq 0 || no_result

ucsdpsys_disk -f test.vol -p GWEN.TEXT=junk
test $? -eq 0 || fail

ucsdpsys_disk -f test.vol -l > test.out
test $? -eq 0 || no_result

grep 'GWEN' test.out > /dev/null || fail

#
# and make sure we can pull it out the same way
#
ucsdpsys_disk -f test.vol -g get=GWEN.TEXT
test $? -eq 0 || fail

test -f get || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
