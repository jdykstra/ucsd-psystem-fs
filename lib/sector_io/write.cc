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

#include <lib/debug.h>
#include <lib/sector_io.h>


int
sector_io::write(off_t byte_offset, const void *data, size_t nbytes)
{
    DEBUG(2, "sector_io::write(byte_offset = %ld, data = %p, nbytes = %ld)",
            (long)byte_offset, data, (long)nbytes);
    if (byte_offset < 0)
        return -EINVAL;
    unsigned sizeof_sector = bytes_per_sector();
    DEBUG(3, "sizeof_sector = %u", sizeof_sector);
    unsigned total = nbytes;
    unsigned secnum = byte_offset / sizeof_sector;
    DEBUG(3, "secnum = %u", secnum);

    //
    // Deal with unaligned start address.
    // Double handling is required.
    //
    unsigned remainder = byte_offset % sizeof_sector;
    if (remainder != 0)
    {
        DEBUG(3, "remainder = %u", remainder);
        assert(sizeof_sector <= 512);
        char partial[512];
        int err = read_sector(secnum, partial);
        if (err < 0)
            return err;
        size_t psize = sizeof_sector - remainder;
        if (psize > nbytes)
            psize = nbytes;
        memcpy(partial + remainder, data, psize);
        err = write_sector(secnum, partial);
        if (err < 0)
            return err;
        data = (const char *)data + psize;
        nbytes -= psize;
        byte_offset += psize;
        ++secnum;
    }

    //
    // Write the rest of the sectors.
    //
    while (nbytes > 0)
    {
        //
        // Deal with unaligned endings.
        //
        if (nbytes < sizeof_sector)
        {
            assert(sizeof_sector <= 512);
            char partial[512];
            int err = read_sector(secnum, partial);
            if (err < 0)
                return err;
            memcpy(partial, data, nbytes);
            err = write_sector(secnum, partial);
            if (err < 0)
                return err;
            break;
        }

        //
        // The simple (and commonest) case.  We only do one sector at a
        // time, in case the deeper sectors are interleaved.
        //
        int err = write_sector(secnum, data);
        if (err < 0)
            return err;
        data = (const char *)data + sizeof_sector;
        nbytes -= sizeof_sector;
        byte_offset += sizeof_sector;
        ++secnum;
    }
    return total;
}
