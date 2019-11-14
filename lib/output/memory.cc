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

#include <lib/debug.h>
#include <lib/output/memory.h>


output_memory::~output_memory()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    flush();
}


output_memory::output_memory()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
}


output_memory::mpointer
output_memory::create()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    return mpointer(new output_memory());
}


void
output_memory::truncate_to(size_t nbytes, char padding)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    buffer.set_size(nbytes, padding);
}


void
output_memory::overwrite(size_t offset, const void *data, size_t nbytes)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    buffer.overwrite(offset, data, nbytes);
}


rcstring
output_memory::filename()
{
    return "memory";
}


void
output_memory::utime_ns(const timespec *)
{
    // Do nothing
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
}


void
output_memory::write_inner(const void *data, size_t len)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    buffer.push_back(data, len);
}


void
output_memory::flush_inner()
{
    // Nothing to do.
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
}
