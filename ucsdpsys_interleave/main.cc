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
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <unistd.h>

#include <lib/debug.h>
#include <lib/sector_io/apple.h>
#include <lib/sector_io/imd.h>
#include <lib/sector_io/offset.h>
#include <lib/sector_io/pdp.h>
#include <lib/sector_io/raw.h>
#include <lib/sector_io/td0.h>
#include <lib/version.h>


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s -T<name> -d <infile> <outfile>\n", prog);
    fprintf(stderr, "       %s -T<name> -e <infile> <outfile>\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


sector_io::pointer
filter_factory(const sector_io::pointer &fp, const char *name)
{
    if (0 == strcasecmp(name, "none") || 0 == strcasecmp(name, "raw"))
        return fp;
    if (0 == strcasecmp(name, "guess"))
    {
        sector_io::pointer io = sector_io::guess_interleaving(fp);
        if (io)
            return io;
        explain_output_error_and_die
        (
            "%s: unable to guess interleave format",
            fp->get_filename().c_str()
        );
    }
    if (0 == strcasecmp(name, "apple"))
        return sector_io_apple::create(fp);
    if (0 == strcasecmp(name, "pdp"))
    {
        sector_io::pointer tmp = sector_io_offset::create(fp, 26 * 128);
        return sector_io_pdp::create(tmp);
    }
    explain_output_error_and_die
    (
        "interleave format %s unknown",
        rcstring(name).quote_c().c_str()
    );
    return fp;
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);
    const char *interleave_type_name = 0;
    bool encode_flag = false;
    bool decode_flag = false;
    for (;;)
    {
        static const struct option options[] =
        {
            { "debug", 0, 0, 'D' },
            { "decode", 0, 0, 'd' },
            { "encode", 0, 0, 'e' },
            { "type", 1, 0, 'T' },
            { "version", 0, 0, 'V' },
            { 0, 0, 0, 0 }
        };
        int c = getopt_long(argc, argv, "DdeT:V", options, 0);
        if (c == EOF)
            break;
        switch (c)
        {
        case 'D':
            ++debug_level;
            break;

        case 'd':
            decode_flag = true;
            break;

        case 'e':
            encode_flag = true;
            break;

        case 'T':
            interleave_type_name = optarg;
            break;

        case 'V':
            version_print();
            return 0;

        default:
            usage();
        }
    }
    if (encode_flag + decode_flag != 1)
    {
        explain_output_error_and_die
        (
            "you must specify exactly one of the --encode or "
                "--decode options"
        );
    }
    if (argc - optind != 2)
        explain_output_error_and_die("two file names must be given");
    const char *infile = argv[optind];
    const char *outfile = argv[optind + 1];
    if (!interleave_type_name)
    {
        if (decode_flag)
            interleave_type_name = "guess";
        else
        {
            explain_output_error_and_die
            (
                "no interleave type (--type=name) specified"
            );
        }
    }

    DEBUG(1, "open input (%s)", infile);
    sector_io::pointer inp = sector_io::factory(infile);
    if (decode_flag)
        inp = filter_factory(inp, interleave_type_name);

    DEBUG(1, "open output (%s)", outfile);
    sector_io::pointer outp = sector_io_raw::create(outfile, false);
    if (encode_flag)
        outp = filter_factory(outp, interleave_type_name);

    //
    // Figure out how many bytes we are going to be processing.
    //
    int rc = inp->size_in_bytes();
    if (rc < 0)
    {
        explain_output_error_and_die("stat %s: %s", infile, strerror(-rc));
    }
    size_t size_in_bytes(rc);
    size_t bufsiz = (size_t)1 << 16;
    char *buffer = new char [bufsiz];

    //
    // Read chunks of input and write them to the output
    // until there is nothing more.
    //
    size_t address = 0;
    while (address < size_in_bytes)
    {
        // Slurp as much as possible.
        size_t nbytes = bufsiz;
        if (address + nbytes > size_in_bytes)
            nbytes = size_in_bytes - address;

        DEBUG(2, "inp->read(address = 0x%08lX, buffer = %p, size = %ld)",
            (long)address, buffer, (long)nbytes);
        int err = inp->read(address, buffer, nbytes);
        if (err < 0)
        {
            explain_output_error_and_die("read %s: %s", infile, strerror(-err));
        }

        DEBUG(2, "outp->write(address = 0x%08lX, buffer = %p, size = %ld)",
            (long)address, buffer, (long)nbytes);
        err = outp->write(address, buffer, nbytes);
        if (err < 0)
        {
            explain_output_error_and_die
            (
                "write %s: %s",
                infile,
                strerror(-err)
            );
        }

        address += nbytes;
    }
    rc = outp->sync();
    if (rc < 0)
    {
        explain_output_error_and_die("sync %s: %s", infile, strerror(-rc));
    }
    DEBUG(1, "close");
    delete buffer;
    inp.reset();
    outp.reset();
    return 0;
}
