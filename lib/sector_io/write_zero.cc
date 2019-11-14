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

#include <lib/sector_io.h>


int
sector_io::write_zero(off_t byte_offset, size_t nbytes)
{
    if (byte_offset < 0)
        return -EINVAL;
    unsigned sizeof_sector = bytes_per_sector();
    unsigned secnum = byte_offset / sizeof_sector;

    //
    // Deal with unaligned start address.
    // Double handling is required.
    //
    unsigned remainder = byte_offset % sizeof_sector;
    if (remainder != 0)
    {
        assert(sizeof_sector <= 512);
        char partial[512];
        int err = read_sector(secnum, partial);
        if (err < 0)
            return err;
        size_t psize = sizeof_sector - remainder;
        if (psize > nbytes)
            psize = nbytes;
        memset(partial + remainder, 0, psize);
        err = write_sector(secnum, partial);
        if (err < 0)
            return err;
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
            memset(partial, 0, nbytes);
            err = write_sector(secnum, partial);
            if (err < 0)
                return err;
            break;
        }

        //
        // The simple (and commonest) case.  We only do one sector at a
        // time, in case the deeper sectors are interleaved.
        //
        assert(sizeof_sector <= 512);
        static char zero[512];
        int err = write_sector(secnum, zero);
        if (err < 0)
            return err;
        nbytes -= sizeof_sector;
        byte_offset += sizeof_sector;
        ++secnum;
    }
    return 0;
}
