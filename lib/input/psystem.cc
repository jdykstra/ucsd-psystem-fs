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

#include <lib/config.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <libexplain/output.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <lib/input/psystem.h>
#include <lib/directory/entry.h>


input_psystem::~input_psystem()
{
    assert(dep);
    int err = dep->flush();
    if (err < 0)
    {
        explain_output_error_and_die
        (
            "flush %s: %s",
            name().quote_c().c_str(),
            strerror(-err)
        );
    }
    err = dep->release();
    if (err < 0)
    {
        explain_output_error_and_die
        (
            "release %s: %s",
            name().quote_c().c_str(),
            strerror(-err)
        );
    }
    dep = 0;
}


input_psystem::input_psystem(directory_entry *a_dep) :
    dep(a_dep),
    address(0)
{
    assert(dep);
    int err = dep->open();
    if (err < 0)
    {
        explain_output_error_and_die
        (
            "open %s: %s",
            name().quote_c().c_str(),
            strerror(-err)
        );
    }
}


input::pointer
input_psystem::create(directory_entry::pointer a_dep)
{
    return pointer(new input_psystem(a_dep.get()));
}


input::pointer
input_psystem::create(directory_entry *a_dep)
{
    return pointer(new input_psystem(a_dep));
}


long
input_psystem::read_inner(void *data, size_t size)
{
    long n = dep->read(address, data, size);
    if (n < 0)
    {
        // EINVAL usually means truncated file system image
        explain_output_error_and_die
        (
            "read %s: %s",
            name().quote_c().c_str(),
            strerror(-n)
        );
    }
    address += n;
    return n;
}


void
input_psystem::fstat(struct stat &st)
{
    int rc = dep->getattr(&st);
    if (rc < 0)
    {
        explain_output_error_and_die
        (
            "fstat %s: %s",
            name().quote_c().c_str(),
            strerror(-rc)
        );
    }
}


int
input_psystem::fpathconf_name_max()
{
    return dep->get_name_maxlen();
}


rcstring
input_psystem::name()
{
    return dep->get_full_name();
}


long
input_psystem::length()
{
    struct stat st;
    fstat(st);
    return st.st_size;
}
