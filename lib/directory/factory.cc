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
#include <cerrno>
#include <cstring>
#include <libexplain/output.h>

#include <lib/directory.h>


/**
  * The interleaved_raw_sector_io function is used to figure out what
  * interleaving (if any) is needed for the disk image.
  *
  * We have to sniff the disk image to see if we need to interleave
  * it or not.  This is all very heuristic.  I have only got Apple ][
  * Pascal images to work with.  Some are interleaved, and some are
  * not.
  *
  * Patches to extend this code to understand more disk image formats
  * are most welcome.
  *
  * @param filename
  *     The name of the file containing the disk image.
  * @returns
  *     pointer to sector i/o for accessing disk image
  */
static sector_io::pointer
interleaved_raw_sector_io(const rcstring &filename, bool read_only)
{
    //
    // Open the file.
    //
    sector_io::pointer raw = sector_io::factory(filename, read_only);

    //
    // Sniff the file for interleaving
    //
    sector_io::pointer io = sector_io::guess_interleaving(raw);
    if (!io)
    {
        //
        // No luck.
        //
        explain_output_error_and_die
        (
            "the %s file does not appear to have a UCSD p-System volume label",
            filename.quote_c().c_str()
        );
        return raw;
    }

    //
    // report success
    //
    return io;
}


directory *
directory::factory(const rcstring &filename, bool read_only, concern_t level)
{
    if (read_only && level > concern_check)
        level = concern_check;
    sector_io::pointer disk =
        interleaved_raw_sector_io(filename, read_only);
    directory *dir = new directory(disk);
    int err = dir->meta_read(level);
    if (err < 0)
    {
        //
        // If we can't read the volume, make sure the mount fails as well.
        //
        explain_output_error_and_die
        (
            "read %s: %s",
            filename.c_str(),
            strerror(-err)
        );
    }
    if (err > 0)
    {
        if (level >= concern_repair)
        {
            explain_output_error_and_die
            (
                "%s: repaired %d format error%s",
                filename.c_str(),
                err,
                (err == 1 ? "" : "s")
            );
            // NOTREACHED
        }
        explain_output_error
        (
            "%s: warning: found %d format error%s",
            filename.c_str(),
            err,
            (err == 1 ? "" : "s")
        );
    }
    return dir;
}
