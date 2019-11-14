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

#include <lib/sector_io.h>


int
sector_io::relocate_bytes(off_t to, off_t from, size_t nbytes)
{
    unsigned sizeof_sector = bytes_per_sector();
    assert(to % sizeof_sector == 0);
    assert(from % sizeof_sector == 0);
    assert(nbytes % sizeof_sector == 0);
    return
        relocate_sectors
        (
            to / sizeof_sector,
            from / sizeof_sector,
            nbytes / sizeof_sector
        );
}


int
sector_io::relocate_sectors(unsigned to_sector_num, unsigned from_sector_num,
    unsigned num_sectors)
{
    if (to_sector_num == from_sector_num)
        return 0;
    if (num_sectors == 0)
        return 0;
    if (to_sector_num < from_sector_num)
    {
        for (;;)
        {
            assert(bytes_per_sector() <= 512);
            char buffer[512];
            int err = read_sector(from_sector_num, buffer);
            if (err < 0)
                return err;
            err = write_sector(to_sector_num, buffer);
            if (err < 0)
                return err;
            ++to_sector_num;
            ++from_sector_num;
            --num_sectors;
            if (num_sectors == 0)
                break;
        }
    }
    else
    {
        for (;;)
        {
            assert(bytes_per_sector() <= 512);
            char buffer[512];
            int err = read_sector(from_sector_num + num_sectors - 1, buffer);
            if (err < 0)
                return err;
            err = write_sector(to_sector_num + num_sectors - 1, buffer);
            if (err < 0)
                return err;
            --num_sectors;
            if (num_sectors == 0)
                break;
        }
    }
    return 0;
}
