#!/bin/sh
#
# UCSD p-System filesystem in user space
# Copyright (C) 2010, 2012 Peter Miller
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

TEST_SUBJECT="ucsdpsys_disk --crunch"
. test_prelude

dt=`date '+%e-%b-%y'`
dt2=`date '+%Y-%m-%d'`
cat > ok << fubar
FRED:
EXAMPLE.TEXT       6  10   2048 $dt textfile
1 of 77 files
10 of 280 blocks, 96.4% free
Last mounted $dt2
fubar
test $? -eq 0 || no_result

ucsdpsys_mkfs -L fred fred.vol
test $? -eq 0 || no_result

date > example.text
test $? -eq 0 || no_result

ucsdpsys_disk -f fred.vol -p example.text -k
test $? -eq 0 || fail

ucsdpsys_disk -f fred.vol -ll > test.out
test $? -eq 0 || fail

diff ok test.out
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
# vim: set ts=8 sw=4 et :
