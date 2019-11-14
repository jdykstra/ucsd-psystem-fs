//
// UCSD p-System filesystem in user space
// Copyright (C) 2007 Peter Miller
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

#include <lib/sector_io/imd.h>
#include <lib/sector_io/mmap.h>
#include <lib/sector_io/raw.h>
#include <lib/sector_io/td0.h>


sector_io::pointer
sector_io::factory(const rcstring &filename, bool read_only)
{
    if (sector_io_imd::candidate(filename))
        return sector_io_imd::create(filename, read_only);
    if (sector_io_td0::candidate(filename))
        return sector_io_td0::create(filename, read_only);
    if (sector_io_mmap::available(filename, read_only))
        return sector_io_mmap::create(filename, read_only);
    return sector_io_raw::create(filename, read_only);
}
