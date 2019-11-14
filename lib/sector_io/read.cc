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
sector_io::read(off_t byte_offset, void *data, size_t nbytes)
{
    DEBUG(2, "sector_io::read(this = %p, byte_offset = 0x%lX, data = %p, "
        "nbytes = 0x%lX)", this, (long)byte_offset, data, (long)nbytes);
    if (byte_offset < 0)
        return -EINVAL;
    unsigned sizeof_sector = bytes_per_sector();
    DEBUG(3, "sizeof_sector = %u", sizeof_sector);
    unsigned total = nbytes;
    unsigned secnum = byte_offset / sizeof_sector;

    //
    // Deal with unaligned start address.
    // Double handling is required.
    //
    unsigned remainder = byte_offset % sizeof_sector;
    if (remainder != 0)
    {
        DEBUG(3, "remainder = %u", remainder);
        assert(sizeof_sector <= 512);
        unsigned char partial[512];
        DEBUG(3, "read_sector(secnum = %d, partial = %p)", secnum, partial);
        int err = read_sector(secnum, partial);
        if (err < 0)
            return err;
        unsigned psize = sizeof_sector - remainder;
        if (psize > nbytes)
            psize = nbytes;
        memcpy(data, partial + remainder, psize);
        data = (char *)data + psize;
        nbytes -= psize;
        byte_offset += psize;
        ++secnum;
    }

    //
    // Read the rest of the sectors.
    //
    while (nbytes > 0)
    {
        DEBUG(3, "data = %p, nbytes = 0x%X, byte_offset = 0x%X, secnum = %d",
            data, (int)nbytes, (int)byte_offset, secnum);

        //
        // Deal with unaligned endings.
        //
        if (nbytes < sizeof_sector)
        {
            DEBUG(3, "unaligned ending");
            assert(sizeof_sector <= 512);
            unsigned char partial[512];
            DEBUG(3, "read_sector(secnum = %d, partial = %p)", secnum, partial);
            int err = read_sector(secnum, partial);
            if (err < 0)
                return err;
            DEBUG(3, "memcpy(data = %p, partial = %p, nbytes = 0x%lX)", data,
                partial, (long)nbytes);
            memcpy(data, partial, nbytes);
            break;
        }

        //
        // The simplest (and commonest) case.  We only do one sector at
        // a time, in case the deeper sectors are interleaved.
        //
        DEBUG(3, "read_sector(secnum = %d, data = %p)", secnum, data);
        int err = read_sector(secnum, data);
        if (err < 0)
            return err;
        DEBUG(3, "sizeof_sector = %d", sizeof_sector);
        assert(sizeof_sector == bytes_per_sector());
        data = (char *)data + sizeof_sector;
        nbytes -= sizeof_sector;
        byte_offset += sizeof_sector;
        ++secnum;
    }
    return total;
}
