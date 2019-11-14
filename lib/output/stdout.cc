//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008, 2010 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at
// you option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>
//

#include <lib/config.h>
#include <cstdio>
#include <unistd.h>
#include <libexplain/write.h>

#include <lib/output/stdout.h>
#include <lib/rcstring.h>


output_stdout::~output_stdout()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
}


output_stdout::output_stdout() :
    bol(true),
    pos(0)
{
}


output::pointer
output_stdout::create()
{
    return pointer(new output_stdout());
}


rcstring
output_stdout::filename()
{
    return "standard output";
}


void
output_stdout::write_inner(const void *data, size_t len)
{
    explain_write_or_die(fileno(stdout), data, len);
    if (len > 0)
        bol = (((const char *)data)[len - 1] == '\n');
    pos += len;
}


void
output_stdout::flush_inner()
{
}


void
output_stdout::utime_ns(const struct timespec *)
{
    // Do nothing.
}
