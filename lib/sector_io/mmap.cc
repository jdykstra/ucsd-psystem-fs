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
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <libexplain/close.h>
#include <libexplain/fstat.h>
#include <libexplain/mmap.h>
#include <libexplain/munmap.h>
#include <libexplain/open.h>
#include <libexplain/output.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <lib/debug.h>
#include <lib/sector_io/mmap.h>


bool
sector_io_mmap::available(const rcstring &filename, bool read_only)
{
    DEBUG(2, "sector_io_mmap::available(filename = %s, read_only = %d)",
        filename.quote_c().c_str(), read_only);
#ifdef HAVE_MMAP
    int mode = O_RDWR;
    if (read_only)
        mode = O_RDONLY;
    int fd = open(filename.c_str(), mode);
    if (fd < 0)
        return false;
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        close_and_fail:
        close(fd);
        return false;
    }

    // Don't use memory mapped I/O for absurdly large disk images.
    // Sensable disk images are all less than 32K of 512 byte blocks.
    if (st.st_size > ((off_t)1 << (15 + 9)))
        goto close_and_fail;

    size_t length = st.st_size;
    int prot = PROT_READ;
    if (!read_only)
        prot |= PROT_WRITE;
    int flags = MAP_SHARED;
    off_t offset = 0;
    void *base = mmap(0, length, prot, flags, fd, offset);
    if (!base || base == (void *)(-1))
    {
        DEBUG(2, "mmap(base = 0, length = 0x%lX, prot = 0x%X, flags = 0x%X,"
            "fd = %d, offset = 0x%lX): %s\n", (long)length, prot, flags, fd,
            (long)offset, strerror(errno));
        goto close_and_fail;
    }
    DEBUG(2, "memory mapped I/O available");
    munmap(base, length);
    close(fd);
    return true;
#else
    return false;
#endif
}


sector_io_mmap::~sector_io_mmap()
{
#ifdef HAVE_MMAP
    if (base)
    {
        explain_munmap_or_die(base, length);
    }
#endif
    base = 0;
    length = 0;
}


sector_io_mmap::sector_io_mmap(const rcstring &a_filename, bool a_read_only) :
    filename(a_filename),
    read_only(a_read_only),
    base(0),
    length(0),
    fake_bytes_per_sector(512)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
#ifdef HAVE_MMAP
    int mode = O_RDWR;
    if (read_only)
        mode = O_RDONLY;
    int fd = explain_open_or_die(filename.c_str(), mode, 0666);
    DEBUG(2, "fd = %d", fd);
    struct stat st;
    explain_fstat_or_die(fd, &st);

    // Don't use memory mapped I/O for absurdly large disk images.
    // Sensable disk images are all less than 32K of 512 byte blocks.
    if (st.st_size > ((off_t)1 << (15 + 9)))
    {
        explain_output_error_and_die
        (
            "%s: too large for valid file system",
            filename.c_str()
        );
    }

    length = st.st_size;
    int prot = PROT_READ;
    if (!read_only)
        prot |= PROT_WRITE;
    int flags = MAP_SHARED;
    off_t offset = 0;
    base =
        (unsigned char *)
        explain_mmap_or_die(0, length, prot, flags, fd, offset);
    DEBUG(2, "base = %p", base);
    explain_close_or_die(fd);
#endif
}


sector_io::pointer
sector_io_mmap::create(const rcstring &a_filename, bool a_read_only)
{
    return pointer(new sector_io_mmap(a_filename, a_read_only));
}


int
sector_io_mmap::read_sector(unsigned sector_number, void *data)
{
    size_t offset = (size_t)sector_number * fake_bytes_per_sector;
    if (offset >= length)
        return -EINVAL;
#ifdef HAVE_MMAP
    memcpy(data, base + offset, fake_bytes_per_sector);
    return 0;
#else
    return -ENOSYS;
#endif
}


int
sector_io_mmap::read(off_t offset, void *data, size_t size)
{
    if (offset < 0 || offset >= (off_t)length)
        return -EINVAL;
    if (offset + size > length)
        return -EINVAL;
#ifdef HAVE_MMAP
    memcpy(data, base + (size_t)offset, size);
    return size;
#else
    return -ENOSYS;
#endif
}


int
sector_io_mmap::write_sector(unsigned sector_number, const void *data)
{
    if (read_only)
        return -EACCES;
    size_t offset = (size_t)sector_number * fake_bytes_per_sector;
    if (offset >= length)
        return -EINVAL;
#ifdef HAVE_MMAP
    memcpy(base + offset, data, fake_bytes_per_sector);
    return 0;
#else
    return -ENOSYS;
#endif
}


int
sector_io_mmap::write(off_t offset, const void *data, size_t size)
{
    if (read_only)
        return -EACCES;
    if (offset < 0 || offset >= (off_t)length)
        return -EINVAL;
    if (offset + size > length)
        return -EINVAL;
#ifdef HAVE_MMAP
    memcpy(base + (size_t)offset, data, size);
    return 0;
#else
    return -ENOSYS;
#endif
}


int
sector_io_mmap::size_in_sectors()
{
    DEBUG(2, "sector_io_mmap::size_in_sectors()");
    DEBUG(3, "length = %ld", (long)length);
    DEBUG(3, "return %ld", long(length / fake_bytes_per_sector));
    return (length / fake_bytes_per_sector);
}


int
sector_io_mmap::sync()
{
#ifdef HAVE_MMAP
    int flags = MS_SYNC;
    if (msync(base, length, flags) < 0)
        return -errno;
#endif
    return 0;
}


bool
sector_io_mmap::is_read_only()
    const
{
    return read_only;
}


unsigned
sector_io_mmap::bytes_per_sector()
    const
{
    DEBUG(2, "sector_io_mmap::bytes_per_sector()");
    return fake_bytes_per_sector;
}


unsigned
sector_io_mmap::size_multiple_in_bytes()
    const
{
    return fake_bytes_per_sector;
}


void
sector_io_mmap::bytes_per_sector_hint(unsigned x)
{
    // Must always be a power of two.
    assert(x != 0 && x == (x & -x));
    if (x < fake_bytes_per_sector)
        fake_bytes_per_sector = x;
}


rcstring
sector_io_mmap::get_filename(void)
    const
{
    return filename;
}
