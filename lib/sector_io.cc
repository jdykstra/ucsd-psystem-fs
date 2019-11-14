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
#include <assert.h>

#include <lib/sector_io.h>


sector_io::~sector_io()
{
}


sector_io::sector_io()
{
}


int
sector_io::size_in_bytes()
{
    int rc = size_in_sectors();
    if (rc < 0)
        return rc;
    return (bytes_per_sector() * rc);
}


void
sector_io::bytes_per_sector_hint(unsigned)
{
    // Do nothing.
}
