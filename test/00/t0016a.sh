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

TEST_SUBJECT="ucsdpsys_disk -p dir"
. test_prelude

#
# Test putting whole unix directories into a volume image.
#
mkdir test.dir test.dir/ignore-me
test $? -eq 0 || no_result
date > test.dir/a.text
test $? -eq 0 || no_result
date > test.dir/b.text
test $? -eq 0 || no_result
date > test.dir/c.text
test $? -eq 0 || no_result

ucsdpsys_mkfs test.vol
test $? -eq 0 || no_result

ucsdpsys_disk -f test.vol -p test.dir
test $? -eq 0 || fail

ucsdpsys_disk -f test.vol -l > test.out.raw
test $? -eq 0 || no_result

wc -l test.out.raw > test.out
test $? -eq 0 || no_result

cat > test.ok << 'fubar'
7 test.out.raw
fubar
test $? -eq 0 || no_result

diff test.ok test.out
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
