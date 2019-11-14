#!/bin/sh
#
# UCSD p-System filesystem in user space
# Copyright (C) 2011 Peter Miller
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

TEST_SUBJECT="ucsdpsys_disk --system-volume"
. test_prelude

# create a pseudo system volume
mkdir system || no_result
date > system/system.pascal
test $? -eq 0 || no result
date > system/system.filer
test $? -eq 0 || no result
date > system/system.editor
test $? -eq 0 || no result
date > system/system.compiler
test $? -eq 0 || no result
ucsdpsys_mkfs -B256 --label=system system.vol
test $? -eq 0 || no result
ucsdpsys_disk -f system.vol -p system
test $? -eq 0 || no result

ucsdpsys_disk -f system.vol --system-volume > LOG 2>&1
if test $? -ne 0
then
    if test -s LOG
    then
        cat LOG
        no_result
    fi
    echo "was supposed to silently exit success" 1>&2
    fail
fi

ucsdpsys_mkfs -B256 --label=bupkis bupkis.vol
test $? -eq 0 || no result

ucsdpsys_disk -f bupkis.vol --system-volume > LOG 2>&1
if test $? -eq 0
then
    echo "was supposed to fail" 1>&2
    fail
else
    if test -s LOG
    then
        cat LOG
        no_result
    fi
fi

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
