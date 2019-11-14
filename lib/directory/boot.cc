//
// UCSD p-System filesystem in user space
// Copyright (C) 2010 Peter Miller
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
#include <cstring>
#include <fcntl.h>
#include <libexplain/close.h>
#include <libexplain/open.h>
#include <libexplain/read.h>
#include <libexplain/write.h>

#include <lib/directory.h>


#ifndef O_BINARY
#define O_BINARY 0
#endif


void
directory::get_boot_blocks(const rcstring &filename)
    const
{
    int flags = O_WRONLY | O_BINARY | O_TRUNC | O_CREAT;
    int fd = explain_open_or_die(filename.c_str(), flags, 0666);
    char data[0x400];
    // FIXME: error handling
    deeper->read(0, data, sizeof(data));
    explain_write_or_die(fd, data, sizeof(data));
    explain_close_or_die(fd);
}


void
directory::set_boot_blocks(const rcstring &filename)
{
    int flags = O_RDONLY | O_BINARY;
    int fd = explain_open_or_die(filename.c_str(), flags, 0666);
    char data[0x400];
    ssize_t n = explain_read_or_die(fd, data, sizeof(data));
    assert(n >= 0);
    if (size_t(n) < sizeof(data))
        memset(data + n, 0, sizeof(data) - n);
    // FIXME: error handling
    deeper->write(0, data, sizeof(data));
    explain_close_or_die(fd);
}
