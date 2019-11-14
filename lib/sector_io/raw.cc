//
// UCSD p-System filesystem in user space
// Copyright (C) 2006, 2007, 2010 Peter Miller
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
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <libexplain/open.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lib/debug.h>
#include <lib/sector_io/raw.h>


sector_io_raw::~sector_io_raw()
{
    DEBUG(2, "sector_io_raw::~sector_io_raw(this = %p)", this);
    if (fd >= 0)
        close(fd);
    fd = -1;
    err = 0;
}


sector_io_raw::sector_io_raw(const rcstring &a_filename, bool a_read_only) :
    filename(a_filename),
    fd(-1),
    err(0),
    read_only(a_read_only),
    fake_bytes_per_sector(512)
{
    DEBUG(2, "sector_io_raw::sector_io_raw(this = %p, filename = %s, "
        "read_only = %d)", this, filename.quote_c().c_str(), read_only);
    int mode = (read_only ? O_RDONLY : (O_RDWR | O_CREAT));
    fd = explain_open_or_die(filename.c_str(), mode, 0666);
    DEBUG(3, "fd = %d", fd);
}


sector_io::pointer
sector_io_raw::create(const rcstring &a_filename, bool a_read_only)
{
    return pointer(new sector_io_raw(a_filename, a_read_only));
}


int
sector_io_raw::read_sector(unsigned sector_number, void *data)
{
    DEBUG(2, "sector_io_raw::read_sector(this = %p, sector_number = %u, "
        "data = %p)", this, sector_number, data);
    off_t offset = (off_t)sector_number * fake_bytes_per_sector;
    return read(offset, data, fake_bytes_per_sector);
}


int
sector_io_raw::read(off_t offset, void *data, size_t size)
{
    DEBUG(2, "sector_io_raw::read(this = %p, offset = 0x%lX, data = %p, "
        "size = 0x%lX)", this, (long)offset, data, (long)size);
    if (fd < 0)
        return -err;
    if (offset < 0)
        return -EINVAL;
    DEBUG(3, "pread(fd = %d, data = %p, size = 0x%lX, offset = 0x%lX)", fd,
        data, (long)size, (long)offset);
    ssize_t n = ::pread(fd, data, size, offset);
    if (n < 0)
    {
        err = errno;
        return -err;
    }
    if ((size_t)n != size)
    {
        DEBUG(3, "read %ld, expected %ld", (long)n, (long)size);
        return -ENOSPC;
    }
    return size;
}


int
sector_io_raw::write_sector(unsigned sector_number, const void *data)
{
    DEBUG(2, "sector_io_raw::write_sector(this = %p, sector_number = %u, "
        "data = %p)", this, sector_number, data);
    off_t offset = (off_t)sector_number * fake_bytes_per_sector;
    return write(offset, data, fake_bytes_per_sector);
}


int
sector_io_raw::write(off_t offset, const void *data, size_t size)
{
    DEBUG(2, "sector_io_raw::write(this = %p, offset = 0x%lX, data = %p, "
        "size = 0x%lX)", this, (long)offset, data, (long)size);
    if (fd < 0)
        return -err;
    if (read_only)
        return -EACCES;
    if (offset < 0)
        return -EINVAL;
    DEBUG(3, "pwrite(fd = %d, data = %p, size = 0x%lX, offset = 0x%lX)", fd,
        data, (long)size, (long)offset);
    ssize_t n = ::pwrite(fd, data, size, offset);
    if (n < 0)
    {
        err = errno;
        return -err;
    }
    if ((size_t)n != size)
    {
        DEBUG(3, "wrote %ld, expected %ld", (long)n, (long)size);
        return -ENOSPC;
    }
    return size;
}


int
sector_io_raw::size_in_sectors()
{
    if (fd < 0)
        return -err;
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        err = errno;
        return -err;
    }
    return (st.st_size / fake_bytes_per_sector);
}


unsigned
sector_io_raw::bytes_per_sector()
    const
{
    return fake_bytes_per_sector;
}


unsigned
sector_io_raw::size_multiple_in_bytes()
    const
{
    return fake_bytes_per_sector;
}


int
sector_io_raw::sync()
{
    if (read_only)
        return 0;
    if (fd < 0)
        return -err;
    int rslt = fsync(fd);
    if (rslt < 0)
        return -errno;
    return 0;
}


bool
sector_io_raw::is_read_only()
    const
{
    return read_only;
}


void
sector_io_raw::bytes_per_sector_hint(unsigned n)
{
    // Must always be a power of two.
    assert(n != 0 && n == (n & -n));
    if (n < fake_bytes_per_sector)
        fake_bytes_per_sector = n;
}


rcstring
sector_io_raw::get_filename(void)
    const
{
    return filename;
}
