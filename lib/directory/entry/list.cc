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

#include <cstring>
#include <cstdlib>

#include <lib/directory/entry.h>
#include <lib/directory/entry/list.h>


directory_entry_list::~directory_entry_list()
{
    delete [] list;
    length = 0;
    maximum = 0;
    list = 0;
}


directory_entry_list::directory_entry_list() :
    length(0),
    maximum(0),
    list(0)
{
}


void
directory_entry_list::push_back(directory_entry::pointer dep)
{
    if (length >= maximum)
    {
        size_t new_maximum = maximum * 2 + 8;
        directory_entry::pointer *new_list =
            new directory_entry::pointer [new_maximum];
        for (size_t j = 0; j < length; ++j)
            new_list[j] = list[j];
        delete [] list;
        list = new_list;
        maximum = new_maximum;
    }
    list[length++] = dep;
}


directory_entry::pointer
directory_entry_list::nth(size_t n)
    const
{
    if (n >= length)
        return directory_entry::pointer();
    return list[n];
}


directory_entry::pointer
directory_entry_list::find(const rcstring &filename)
    const
{
    for (size_t j = 0; j < length; ++j)
    {
        directory_entry::pointer dep = list[j];
        if
        (
            0
        ==
            strncasecmp
            (
                filename.c_str(),
                dep->get_name().c_str(),
                dep->get_name_maxlen()
            )
        )
        {
            return dep;
        }
    }
    return directory_entry::pointer();
}


directory_entry::pointer
directory_entry_list::back()
    const
{
    if (length == 0)
        return directory_entry::pointer();
    return list[length - 1];
}


size_t
directory_entry_list::index_of(directory_entry::pointer dep)
    const
{
    for (size_t j = 0; j < length; ++j)
    {
        if (list[j] == dep)
            return j;
    }
    return (size_t)(-1);
}


size_t
directory_entry_list::index_of(directory_entry *dep)
    const
{
    for (size_t j = 0; j < length; ++j)
    {
        if (list[j].get() == dep)
            return j;
    }
    return (size_t)(-1);
}


bool
directory_entry_list::erase(directory_entry *dep)
{
    if (!dep)
        return false;
    for (size_t j = 0; j < length; ++j)
    {
        if (list[j].get() == dep)
        {
            //
            // DO NOT get the files out of order.  This means we MUST
            // shuffle down, rather than just grabbing the last item in
            // the list and dropping it in the empty slot.
            //
            for (size_t k = j + 1; k < length; ++k)
                list[k - 1] = list[k];
            --length;
            list[length].reset();
            return true;
        }
    }
    return false;
}


bool
directory_entry_list::erase(directory_entry::pointer dep)
{
    return erase(dep.get());
}


static int
cmp(const void *va, const void *vb)
{
    const directory_entry::pointer a = *(const directory_entry::pointer *)va;
    const directory_entry::pointer b = *(const directory_entry::pointer *)vb;
    int x = a->get_first_block() - b->get_first_block();
    if (x < 0)
        return -1;
    if (x > 0)
        return 1;
    return strcasecmp(a->get_name().c_str(), b->get_name().c_str());
}


void
directory_entry_list::sort_by_first_block()
{
    qsort(list, length, sizeof(list[0]), cmp);
}
