//
// UCSD p-System filesystem in user space
// Copyright (C) 2010 Peter Miller
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
#include <cstdio>
#include <cstring>
#include <libexplain/output.h>

#include <lib/endof.h>
#include <lib/rcstring.h>
#include <lib/sector_io/apple.h>
#include <lib/sector_io/pdp.h>


static sector_io::pointer help(const sector_io::pointer &iop); // forward


static sector_io::pointer
none(const sector_io::pointer &iop)
{
    return iop;
}

struct table_t
{
    const char *name;
    sector_io::pointer (*create)(const sector_io::pointer &deeper);
};

static const table_t table[] =
{
    { "apple", sector_io_apple::create },
    { "help", help },
    { "none", none },
    { "pdp", sector_io_pdp::create },
    { "raw", none },
};


static sector_io::pointer
help(const sector_io::pointer &iop)
{
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        printf("%s\n", tp->name);
    }
    exit(0);
    return iop;
}


sector_io::pointer
sector_io::interleave_factory(const char *name, const sector_io::pointer &iop)
{
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (0 == strcasecmp(name, tp->name))
            return tp->create(iop);
    }
    explain_output_error_and_die
    (
        "interleave pattern %s unknown",
        rcstring(name).quote_c().c_str()
    );
    return iop;
}
