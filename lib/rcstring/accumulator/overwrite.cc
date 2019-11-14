//
// UCSD p-System filesystem in user space
// Copyright (C) 2008 Peter Miller
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

#include <cstring>

#include <lib/rcstring/accumulator.h>


void
rcstring_accumulator::overwrite(size_t offset, const void *data, size_t nbytes)
{
    size_t n = offset + nbytes;
    if (n > maximum)
        grow(n);
    memcpy(buffer + offset, data, nbytes);
    if (length < n)
        length = n;
}
