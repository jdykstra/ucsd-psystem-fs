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

#include <lib/debug.h>
#include <lib/rcstring.h>
#include <lib/sector_io/pdp.h>

#define BYTES_PER_SECTOR_SHIFT 7
#define BYTES_PER_SECTOR (1 << BYTES_PER_SECTOR_SHIFT)

sector_io_pdp::~sector_io_pdp()
{
    DEBUG(3, "sector_io_pdp::~sector_io_pdp(this = %p)", this);
}


sector_io_pdp::sector_io_pdp(const pointer &arg) :
    deeper(arg)
{
    DEBUG(3, "sector_io_pdp::sector_io_pdp(this = %p)", this);
    deeper->bytes_per_sector_hint(BYTES_PER_SECTOR);
}


sector_io::pointer
sector_io_pdp::create(const pointer &arg)
{
    return pointer(new sector_io_pdp(arg));
}

#define SECTORS_PER_TRACK 26


/**
  * apply 2:1 sector interleave
  * then add skew offset of 6 based on track number
  */
static unsigned
map_sector(unsigned track, unsigned sector)
{
    static const unsigned char pattern[SECTORS_PER_TRACK] =
    {
        0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24,
        1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25,
    };

    //
    // logical           => disk
    // t1, sec 8,9,10,11 => t1, sec 16,18,20,22
    //

    DEBUG(3, "map_sector(track = %u, sector = %u)", track, sector);
    assert(sector < SECTORS_PER_TRACK);
    //
    // This code assumes that the deeper input has been passed through
    // sector_io_offset(128*26) to ignore the first track.  If not, the
    // mapping expression reads
    //
    //     (pattern[sector] + ((track + 12) % 13) * 6)
    //
    // That "12" is actually "-1 mod 13", but we want to keep everything
    // positive, or the % operator will give unhelpful results.
    //
    unsigned result =
        (
            (pattern[sector] + (track % 13) * 6)
        %
            SECTORS_PER_TRACK
        );
    DEBUG(3, "return %u", result);
    return result;
}


static unsigned
map(unsigned sector)
{
    DEBUG(3, "map(sector = %u)", sector);
    unsigned track = sector / SECTORS_PER_TRACK;
    unsigned result =
        (
            (track * SECTORS_PER_TRACK)
        +
            map_sector(track, sector % SECTORS_PER_TRACK)
        );
    DEBUG(3, "return %u", result);
    return result;
}


int
sector_io_pdp::read_sector(unsigned sector_number, void *data)
{
    DEBUG(3, "sector_io_pdp::read_sector(this = %p, sector_number = %u, "
        "data = %p)", this, sector_number, data);
    unsigned deeper_sector_number = map(sector_number);
    DEBUG(3, "deeper_sector_number = %u", deeper_sector_number);
    off_t offset = (off_t)deeper_sector_number << BYTES_PER_SECTOR_SHIFT;
    DEBUG(3, "deeper->read(offset = %ld, data = %p, size = %d)", (long)offset,
        data, BYTES_PER_SECTOR);
    int rc = deeper->read(offset, data, BYTES_PER_SECTOR);
    DEBUG(3, "rc = %d", rc);
    if (rc < 0)
        return rc;
    return 0;
}


int
sector_io_pdp::write_sector(unsigned sector_number, const void *data)
{
    DEBUG(3, "sector_io_pdp::write_sector(this = %p, sector_number = %u, "
        "data = %p)", this, sector_number, data);
    unsigned deeper_sector_number = map(sector_number);
    off_t offset = deeper_sector_number << BYTES_PER_SECTOR_SHIFT;
    int rc = deeper->write(offset, data, BYTES_PER_SECTOR);
    DEBUG(3, "rc = %d", rc);
    if (rc < 0)
        return rc;
    return 0;
}


int
sector_io_pdp::size_in_sectors()
{
    DEBUG(2, "sector_io_pdp::size_in_sectors()");
    int rc = deeper->size_in_bytes();
    DEBUG(3, "rc = %d", rc);
    if (rc < 0)
        return rc;
    rc >>= BYTES_PER_SECTOR_SHIFT;
    DEBUG(3, "return %d", rc);
    return rc;
}


unsigned
sector_io_pdp::bytes_per_sector()
    const
{
    DEBUG(2, "sector_io_pdp::bytes_per_sector() => %d", BYTES_PER_SECTOR);
    return BYTES_PER_SECTOR;
}


int
sector_io_pdp::sync()
{
    return deeper->sync();
}


bool
sector_io_pdp::is_read_only()
    const
{
    return deeper->is_read_only();
}


unsigned
sector_io_pdp::size_multiple_in_bytes()
    const
{
    return (SECTORS_PER_TRACK * BYTES_PER_SECTOR_SHIFT);
}


rcstring
sector_io_pdp::get_filename(void)
    const
{
    return deeper->get_filename();
}
