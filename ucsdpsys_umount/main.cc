//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008, 2010, 2012 Peter Miller
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
#include <cstdlib>
#include <getopt.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <unistd.h>

#include <lib/rcstring.h>
#include <lib/version.h>


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s <mount-point>\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);
    for (;;)
    {
        static const struct option options[] =
        {
            { "version", 0, 0, 'V' },
            { 0, 0, 0, 0 }
        };
        int c = getopt_long(argc, argv, "V", options, 0);
        if (c == EOF)
            break;
        switch (c)
        {
        case 'V':
            version_print();
            return 0;

        default:
            usage();
        }
    }
    if (optind + 1 != argc)
        usage();

    // we may need to try more than once, because of the asynchronous
    // nature of the daemon process FUSE talks to.
    rcstring dir(argv[optind]);
    rcstring command = "fusermount -u " + dir.quote_shell();
    int ntries = 0;
    int maximum_ms = 3000;
    int sleep_ms = 20;
    for (int cumulative_ms = 0; ntries < 1000; )
    {
        if (cumulative_ms >= maximum_ms)
            explain_output_error_and_die("failed after %d attempts", ntries);
        if (ntries)
        {
#ifdef HAVE_USLEEP
            usleep(sleep_ms * 1000);
            cumulative_ms += sleep_ms;
#else
            int s = (sleep_ms + 999) / 1000;
            sleep(s);
            cumulative_ms += s * 1000;
#endif
            sleep_ms *= 2;
        }
        ++ntries;
        if (0 == system(command.c_str()))
            break;
    }
    if (ntries > 1)
        explain_output_error("succeeded after %d attempts", ntries);
    return 0;
}
