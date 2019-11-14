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
#include <cctype>
#include <cstring>

#include <lib/debug.h>
#include <lib/rcstring.h>
#include <lib/sector_io/apple.h>

#define BYTES_PER_SECTOR_SHIFT 8
#define BYTES_PER_SECTOR (1 << BYTES_PER_SECTOR_SHIFT)


static const unsigned char pattern[16] =
{
    0, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 15
};


static inline unsigned
map(unsigned sector_number)
{
    return (sector_number & ~15) + pattern[sector_number & 15];
}


sector_io_apple::~sector_io_apple()
{
}


sector_io_apple::sector_io_apple(const pointer &a_deeper) :
    deeper(a_deeper)
{
    deeper->bytes_per_sector_hint(BYTES_PER_SECTOR);
}


sector_io::pointer
sector_io_apple::create(const pointer &a_deeper)
{
    return pointer(new sector_io_apple(a_deeper));
}


int
sector_io_apple::read_sector(unsigned sector_number, void *data)
{
    unsigned deeper_sector_number = map(sector_number);
    off_t offset = (off_t)deeper_sector_number << BYTES_PER_SECTOR_SHIFT;
    int rc = deeper->read(offset, data, BYTES_PER_SECTOR);
    if (rc < 0)
        return rc;
    return 0;
}


int
sector_io_apple::write_sector(unsigned sector_number, const void *data)
{
    unsigned deeper_sector_number = map(sector_number);
    off_t offset = (off_t)deeper_sector_number << BYTES_PER_SECTOR_SHIFT;
    int rc = deeper->write(offset, data, BYTES_PER_SECTOR);
    if (rc < 0)
        return rc;
    return 0;
}


int
sector_io_apple::size_in_sectors(void)
{
    DEBUG(2, "sector_io_apple::size_in_sectors()");
    int rc = deeper->size_in_bytes();
    if (rc < 0)
        return rc;
    DEBUG(3, "rc = %d", rc);
    rc >>= BYTES_PER_SECTOR_SHIFT;
    DEBUG(3, "return %d", rc);
    return rc;
}


unsigned
sector_io_apple::bytes_per_sector(void)
    const
{
    DEBUG(2, "sector_io_apple::bytes_per_sector()");
    return BYTES_PER_SECTOR;
}


unsigned
sector_io_apple::size_multiple_in_bytes(void)
    const
{
    return (16 * BYTES_PER_SECTOR);
}


int
sector_io_apple::sync(void)
{
    return deeper->sync();
}


bool
sector_io_apple::is_read_only(void)
    const
{
    return deeper->is_read_only();
}


rcstring
sector_io_apple::get_filename(void)
    const
{
    return deeper->get_filename();
}
