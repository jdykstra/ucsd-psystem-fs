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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/statvfs.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <libexplain/statvfs.h>

#include <lib/version.h>


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>\n", prog);
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
        int c = getopt(argc, argv, "V");
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
    const char *filename = argv[optind];

    struct statvfs st;
    explain_statvfs_or_die(filename, &st);

    printf("f_fsid = 0x%08lX\n", (long)st.f_fsid);
    printf("f_bsize = %9ld\n", (long)st.f_bsize);
    printf("f_blocks = %8ld\n", (long)st.f_blocks);
    printf("f_bfree = %9ld\n", (long)st.f_bfree);
    printf("f_bavail = %8ld\n", (long)st.f_bavail);
    printf("f_files = %9ld\n", (long)st.f_files);
    printf("f_ffree = %9ld\n", (long)st.f_ffree);
    printf("f_namemax = %7ld\n", (long)st.f_namemax);

    return 0;
}
