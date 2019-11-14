//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008 Peter Miller
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

#include <lib/debug.h>
#include <lib/input.h>
#include <lib/output.h>


void
output::write(const input::pointer &is)
{
    for (;;)
    {
        unsigned char temp[1 << 14];
        DEBUG(3, "is->read(%p, 0x%lX);", temp, (long)sizeof(temp));
        long n = is->read(temp, sizeof(temp));
        if (!n)
            break;
        DEBUG(3, "write(%p, 0x%lX);", temp, n);
        write(temp, n);
    }
}
