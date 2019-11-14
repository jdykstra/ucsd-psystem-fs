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
#include <cstdlib>
#include <getopt.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <libexplain/rename.h>
#include <libexplain/utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#include <lib/input/file.h>
#include <lib/input/stdin.h>
#include <lib/output/file.h>
#include <lib/output/stdout.h>
#include <lib/output/text_decode.h>
#include <lib/output/text_encode.h>
#include <lib/version.h>


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s -d [ <filename>... ]\n", prog);
    fprintf(stderr, "       %s -e [ <filename>... ]\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


static bool do_decode;
static bool do_encode;
static bool use_tabs = true;
static bool nul_guarantee = true;


static void
insitu(const rcstring &ifn)
{
    input::pointer in = input_file::create(ifn);
    int name_max = in->fpathconf_name_max();
    if (name_max <= 0)
        name_max = 15;

    //
    // Get the access time and modified time, so we can restore them later.
    //
    struct stat st;
    in->fstat(st);

    //
    // Now we have to create a temporary file to write the translated
    // text to, before we rename it over the top of the original.
    //
    rcstring dir(ifn.dirname());
    rcstring base(ifn.basename());
    rcstring ofn;
    for (int num = 1; ; ++num)
    {
        rcstring numstr = rcstring::printf(".t%d", num);
        ofn =
            rcstring::printf
            (
                "%s/.%*s%s",
                dir.c_str(),
                (int)(name_max - 1 - numstr.size()),
                base.c_str(),
                numstr.c_str()
            );
        if (access(ofn.c_str(), F_OK) < 0)
            break;
    }

    output::pointer out = output_file::create(ofn);
    if (do_decode)
        out = output_text_decode::create(out, use_tabs);
    if (do_encode)
        out = output_text_encode::create(out, use_tabs, nul_guarantee);

    //
    // Copy the input to the output.
    //
    out->write(in);
    out.reset();

    //
    // Now rename the new file over the top of the old file.
    //
    explain_rename_or_die(ofn.c_str(), ifn.c_str());

    //
    // Now restore the access time and modified time.
    //
    struct utimbuf amt;
    amt.actime = st.st_atime;
    amt.modtime = st.st_mtime;
    explain_utime_or_die(ifn.c_str(), &amt);
}


static void
run(int argc, char **argv)
{
    if (argc == 0)
    {
        input::pointer in = input_stdin::create();
        output::pointer out = output_stdout::create();
        if (do_decode)
            out = output_text_decode::create(out, use_tabs);
        if (do_encode)
            out = output_text_encode::create(out, use_tabs, nul_guarantee);

        //
        // Copy the input to the output.
        //
        out->write(in);
    }
    else
    {
        for (int j = 0; j < argc; ++j)
            insitu(argv[j]);
    }
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
            { "decode", 0, 0, 'd' },
            { "encode", 0, 0, 'e' },
            { "nul", 0, 0, 'N' },
            { "tabs", 0, 0, 't' },
            { "version", 0, 0, 'V' },
            { 0, 0, 0, 0 }
        };
        int c = getopt_long(argc, argv, "deNtV", options, 0);
        if (c == EOF)
            break;
        switch (c)
        {
        case 'd':
            do_decode = true;
            break;

        case 'e':
            do_encode = true;
            break;

        case 'N':
            nul_guarantee = false;
            break;

        case 't':
            use_tabs = false;
            break;

        case 'V':
            version_print();
            return 0;

        default:
            usage();
        }
    }
    if (do_decode + do_encode != 1)
        usage();
    run(argc - optind, argv + optind);
    return 0;
}
