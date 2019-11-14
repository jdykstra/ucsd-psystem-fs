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

TEST_SUBJECT="ucsdpsys_mkfs"
. test_prelude

#
# Build a new file system.
#
ucsdpsys_mkfs -t testing.dsk
test $? -eq 0 || fail

echo hello > JUNK.TXT
test $? -eq 0 || no_result

ucsdpsys_disk -f testing.dsk -p JUNK.TXT
test $? -eq 0 || no_result

#
# Make sure the new file system.
#
ucsdpsys_fsck testing.dsk
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
