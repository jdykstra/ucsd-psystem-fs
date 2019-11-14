//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008, 2010 Peter Miller
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

#include <cstdio>
#include <vector>

#include <lib/directory.h>
#include <lib/directory/entry/volume_label.h>


class sorter
{
public:
    sorter(directory::sort_by_t arg) :
        sort_by(arg)
    {
    }

    bool
    operator()(const directory_entry::pointer &lhs,
        const directory_entry::pointer &rhs)
    {
        switch (sort_by)
        {
        case directory::sort_by_block:
            return (lhs->get_first_block() < rhs->get_first_block());

        case directory::sort_by_name:
            return (lhs->get_name() < rhs->get_name());

        case directory::sort_by_size:
            if (lhs->get_size_in_bytes() != rhs->get_size_in_bytes())
                return (lhs->get_size_in_bytes() < rhs->get_size_in_bytes());
            break;

        case directory::sort_by_date:
            if (lhs->get_mtime() != rhs->get_mtime())
                return (lhs->get_mtime() < rhs->get_mtime());
            break;

        case directory::sort_by_kind:
            if (lhs->get_file_kind() != rhs->get_file_kind())
                return (lhs->get_file_kind() < rhs->get_file_kind());
            break;
        }
        return (lhs->get_name() < rhs->get_name());
    }

private:
    directory::sort_by_t sort_by;
};


void
directory::print_listing(bool verbose, sort_by_t sort_by)
{
    printf("%s:\n", volume_label->get_name().c_str());
    unsigned num_blocks = volume_label->get_last_block();
    int num_files = 0;
    typedef std::vector<directory_entry::pointer> entries_t;
    entries_t entries;
    for (;;)
    {
        directory_entry::pointer dep = nth(num_files);
        if (!dep)
            break;
        entries.push_back(dep);
        num_blocks += dep->size_in_blocks();
    }
    std::sort(entries.begin(), entries.end(), sorter(sort_by));
    for (entries_t::iterator it = entries.begin(); it != entries.end(); ++it)
        (*it)->print_listing(verbose);
    printf
    (
        "%ld of %ld files\n",
        long(num_files - 1),
        (long)volume_label->maximum_directory_entries()
    );
    unsigned tblks = volume_label->get_eov_block();
    printf
    (
        "%d of %d blocks, %3.1f%% free\n",
        num_blocks,
        tblks,
        100. * (tblks - num_blocks) / (double)tblks
    );
    volume_label->print_listing(verbose);
}
