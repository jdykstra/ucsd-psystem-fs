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
#include <cstdio>
#include <getopt.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <unistd.h>

#include <lib/debug.h>
#include <lib/directory.h>
#include <lib/version.h>


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <volume>\n", prog);
    fprintf(stderr, "       %s --version\n", prog);
    exit(1);
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);

    //
    // Parse the command line options.
    //
    concern_t concern_level = concern_check;
    bool read_only_flag = false;
    for (;;)
    {
        static const struct option options[] =
        {
            { "debug", 0, 0, 'D' },
            { "fix", 0, 0, 'f' },
            { "read-only", 0, 0, 'r' },
            { "version", 0, 0, 'V' },
            { 0, 0, 0, 0 }
        };
        int c = getopt_long(argc, argv, "DfrV", options, 0);
        if (c < 0)
            break;
        switch (c)
        {
        case 'D':
            ++debug_level;
            break;

        case 'f':
            concern_level = concern_repair;
            break;

        case 'r':
            // read only
            read_only_flag = true;
            break;

        case 'V':
            version_print();
            return 0;

        default:
            usage();
        }
    }
    if (optind + 1 != argc)
        usage();
    const char *filename = argv[optind];

    //
    // Make sure we haven't been asked to do the impossible.
    //
    if (concern_level == concern_repair && read_only_flag)
    {
        explain_output_error_and_die
        (
            "unable to fix problems (--fix) if read only (--read-only)"
        );
    }

    //
    // Open the volume, and make sure it has the right format.
    //
    directory *volume =
        directory::factory(filename, read_only_flag, concern_level);
    assert(volume);

    //
    // Close down the volume.
    // This may do essential flush operations.
    //
    delete volume;
    volume = 0;

    //
    // Report success.
    //
    return 0;
}
