//
// UCSD p-System filesystem in user space
// Copyright (C) 2008, 2010 Peter Miller
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
#include <lib/debug.h>
#include <lib/hexdump.h>
#include <lib/sector_io/apple.h>
#include <lib/sector_io/offset.h>
#include <lib/sector_io/pdp.h>


static bool
has_valid_signature(const sector_io::pointer &sio)
{
    unsigned char data[256];
    if (sio->read(1024, data, 256) < 0)
        return false;
    DEBUG(3, "test for signature\n%s", hexdump(data, 16).c_str());
    return
        (
            // dfirstblock == 0
            data[0] == 0
        &&
            data[1] == 0
        &&
            // dlastblock == 6 (single dir) or 10 (dup dir)
            // it could be big-endian or little-endian
            (
                (data[2] == 0 && (data[3] == 6 || data[3] == 10))
            ||
                ((data[2] == 6 || data[2] == 10) && data[3] == 0)
            )
        &&
            // volume name length is valid
            data[6] > 0
        &&
            data[6] < 8
        );
}


sector_io::pointer
sector_io::guess_interleaving(pointer raw)
{
    //
    // See if the raw disk image has a valid signature.
    // This is the ideal case.
    //
    DEBUG(3, "Interleaving: testing raw\n");
    if (has_valid_signature(raw))
    {
        DEBUG(2, "Interleaving: None");
        return raw;
    }

    //
    // See if it has Apple ][ Pascal sector interleaving.
    //
    DEBUG(3, "Interleaving: testing apple\n");
    sector_io::pointer trial = sector_io_apple::create(raw);
    if (has_valid_signature(trial))
    {
        DEBUG(2, "Interleaving: Apple ][ Pascal");
        return trial;
    }

    //
    // See if it has PDP11 interleaving.  There are three cases: offset,
    // offset and interleaved and interleaved by itself.  (There are
    // actually four cases, but we already tested the ideal case.)
    //
    // The PDP11 disks have 77 tracks of 26 sectors of 128 bytes, but
    // the first track is ignored.
    //
    DEBUG(3, "Interleaving: testing offset\n");
    trial = sector_io_offset::create(raw, 128 * 26);
    if (has_valid_signature(trial))
    {
        DEBUG(2, "Interleaving: PDP11 offset");
        return trial;
    }

    DEBUG(3, "Interleaving: testing offset pdp11\n");
    sector_io::pointer trial2 = sector_io_pdp::create(trial);
    if (has_valid_signature(trial2))
    {
        DEBUG(2, "Interleaving: PDP11 map, PDP11 offset");
        return trial2;
    }

    DEBUG(3, "Interleaving: testing pdp11\n");
    trial = sector_io_pdp::create(raw);
    if (has_valid_signature(trial))
    {
        DEBUG(2, "Interleaving: PDP11 map");
        return trial;
    }

    //
    // Try to brute force the offset
    //
    for (int offset = 1; offset < 128; ++offset)
    {
        int nbytes = offset << 8;
        trial = sector_io_offset::create(raw, nbytes);
        if (has_valid_signature(trial))
        {
            DEBUG(2, "Interleaving: offset 0x%04X", nbytes);
            return trial;
        }
    }

    DEBUG(2, "Interleaving: unable to find volume label");
    return pointer();
}
