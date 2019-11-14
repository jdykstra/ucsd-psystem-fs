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
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <libexplain/close.h>
#include <libexplain/open.h>
#include <libexplain/utimes.h>
#include <libexplain/write.h>

#include <lib/output/file.h>


output_file::~output_file()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    if (fd >= 0)
        explain_close_or_die(fd);
    fd = -1;
    pos = 0;
}


output::pointer
output_file::create(const rcstring &a_file_name, bool a_binary)
{
    return pointer(new output_file(a_file_name, a_binary));
}


rcstring
output_file::filename()
{
    return file_name;
}


void
output_file::write_inner(const void *data, size_t len)
{
    explain_write_or_die(fd, data, len);
    if (len > 0)
        bol = (((char *)data)[len - 1] == '\n');
    pos += len;
}


void
output_file::flush_inner()
{
}


output_file::output_file(const rcstring &fn, bool binary) :
    file_name(fn),
    fd(-1),
    bol(true),
    pos(0)
{
    int mode = O_WRONLY | O_CREAT | O_TRUNC;
#ifdef O_BINARY
    if (binary)
        mode |= O_BINARY;
#endif
    fd = explain_open_or_die(fn.c_str(), mode, 0666);
}


void
output_file::utime_ns(const struct timespec *utb)
{
    struct timeval tv2[2];
    tv2[0].tv_sec = utb[0].tv_sec;
    tv2[0].tv_usec = utb[0].tv_nsec / 1000;
    tv2[1].tv_sec = utb[1].tv_sec;
    tv2[1].tv_usec = utb[1].tv_nsec / 1000;

    explain_utimes_or_die(file_name.c_str(), tv2);
}
