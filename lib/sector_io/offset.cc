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
#include <lib/rcstring.h>
#include <lib/sector_io/offset.h>


sector_io_offset::~sector_io_offset()
{
}


sector_io_offset::sector_io_offset(const pointer &a_deeper,
        off_t a_byte_offset) :
    deeper(a_deeper),
    byte_offset(a_byte_offset)
{
}


sector_io::pointer
sector_io_offset::create(const pointer &a_deeper, off_t a_byte_offset)
{
    return pointer(new sector_io_offset(a_deeper, a_byte_offset));
}


int
sector_io_offset::read_sector(unsigned sector_number, void *data)
{
    unsigned bps = bytes_per_sector();
    return deeper->read(sector_number * bps + byte_offset, data, bps);
}


int
sector_io_offset::read(off_t pos, void *data, size_t nbytes)
{
    return deeper->read(pos + byte_offset, data, nbytes);
}


int
sector_io_offset::write_sector(unsigned sector_number, const void *data)
{
    unsigned bps = bytes_per_sector();
    return deeper->write(sector_number * bps + byte_offset, data, bps);
}


int
sector_io_offset::write(off_t pos, const void *data, size_t nbytes)
{
    return deeper->write(pos + byte_offset, data, nbytes);
}


int
sector_io_offset::size_in_sectors()
{
    int n = deeper->size_in_bytes();
    if (n < 0)
        return n;
    n -= byte_offset;
    n /= bytes_per_sector();
    return n;
}


int
sector_io_offset::sync()
{
    return deeper->sync();
}


unsigned
sector_io_offset::bytes_per_sector()
    const
{
    // Pick a number.
    return 256;
}


unsigned
sector_io_offset::size_multiple_in_bytes()
    const
{
    return deeper->size_multiple_in_bytes();
}


bool
sector_io_offset::is_read_only()
    const
{
    return deeper->is_read_only();
}


void
sector_io_offset::bytes_per_sector_hint(unsigned nbytes)
{
    deeper->bytes_per_sector_hint(nbytes);
}


rcstring
sector_io_offset::get_filename(void)
    const
{
    return deeper->get_filename();
}
