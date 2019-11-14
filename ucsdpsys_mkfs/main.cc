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
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <getopt.h>
#include <libexplain/close.h>
#include <libexplain/fstat.h>
#include <libexplain/open.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <libexplain/write.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lib/debug.h>
#include <lib/directory.h>
#include <lib/mtype.h>
#include <lib/sector_io/raw.h>
#include <lib/sector_io/apple.h>
#include <lib/version.h>

//
// The MAX_DISK_SIZE_KB define is the maximum size of a disk volume, in
// kilobytes.
//
// The size of the disk, in 512-byte blocks, is stored in the volume
// label, in a 16-bit field called "deovblk".  Codeing practices being
// what they are (i.e. usually sloppy) it is unlikely that very much
// code can actually cope with deovblk of 65535, or any other value
// which looks negative (anything in the range 32768 to 65535).  Erring
// on the side of caution, we should aim for deovblk to be 32767 or
// smaller.  This gives a maximum disk size of 16384KB - 0.5KB.
//
// A second constraint is Apple ][ Pascal interleaved disks.  These
// operate within 16 256-byte sectors.  Thus, disk sizes must be a
// multiple of 4KB.  This gives a maximum disk size of 16384KB - 4KB =
// 16380KB.
//
#define MAX_DISK_SIZE_KB 16380


/**
  * The zero_whole_volume function is used to create an initial disk
  * image, completely filled with zero data (not just one big hole).
  *
  * @param filename
  *     The file to be written.
  * @param size_kb
  *     The size, in kilobytes, of the file to be written.
  */
static void
zero_whole_volume(const char *filename, unsigned size_kb)
{
    int fd = explain_open_or_die(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    struct stat st;
    explain_fstat_or_die(fd, &st);
    if (!S_ISREG(st.st_mode))
        explain_output_error_and_die("%s: not a regular file", filename);

    //
    // FIXME: At the moment, we only know how to build Apple volumes.
    // The interleaving code works on tracks of 16 sectors, each of 256
    // bytes.  The Apple ][ disks had 35 tracks.
    //
    char block[256 * 16];
    memset(block, 0, sizeof(block));
    size_t ntracks = size_kb / 4;
    for (unsigned track = 0; track < ntracks; ++track)
    {
        explain_write_or_die(fd, block, sizeof(block));
    }
    explain_close_or_die(fd);
}


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s <filename>\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


static int
parse_size(char *text)
{
    char *ep = 0;
    double d = strtod(text, &ep);
    if (ep == text)
    {
        yuck:
        explain_output_error_and_die
        (
            "unable to turn \"%s\" into a disk size in KB",
            text
        );
    }
    while (*ep && isspace((unsigned char)*ep))
        ++ep;

    double mult = 0;
    // acceptable suffixes are
    //     ""
    //     "b"
    //     "kb"
    //     "kib"
    //     "m"
    //     "mb"
    //     "mib"
    switch (*ep++)
    {
    default:
        goto yuck;

    case 'b':
    case 'B':
        if (*ep)
            goto yuck;
        mult = 1 / 1024.;
        break;

    case '\0':
        mult = 1;
        break;

    case 'k':
    case 'K':
        mult = 1;
        ib:
        switch (*ep++)
        {
        default:
            goto yuck;

        case '\0':
            break;

        case 'b':
        case 'B':
            break;

        case 'i':
        case 'I':
            switch (*ep++)
            {
            default:
                goto yuck;

            case 'b':
            case 'B':
                break;
            }
            break;
        }
        break;

    case 'm':
    case 'M':
        mult = 1024.;
        goto ib;

    case 'g':
    case 'G':
        mult = 1024. * 1024.;
        goto ib;
    }
    assert(mult > 0);

    return ceil(mult * d);
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);
    rcstring volid;
    bool twin = false;
    const char *interleave = "none";

    unsigned size_kb = 0;

    mtype_t mtype = mtype_undefined;
    byte_sex_t byte_sex = little_endian;
    const char *boot_file = 0;
    for (;;)
    {
        static const struct option options[] =
        {
            { "architecture", 1, 0, 'A' },
            { "boot", 1, 0, 'b' },
            { "debug", 0, 0, 'D' },
            { "host", 1, 0, 'A' },
            { "interleave", 1, 0, 'I' },
            { "label", 1, 0, 'L' },
            { "machine", 1, 0, 'A' },
            { "size", 1, 0, 'B' },
            { "twin", 0, 0, 't' },
            { "version", 0, 0, 'V' },
            { 0, 0, 0, 0 },
        };
        int c = getopt_long(argc, argv, "B:b:DI:iL:tV", options, 0);
        if (c == EOF)
            break;
        switch (c)
        {
        case 'A':
            {
                rcstring name(optarg);
                mtype = mtype_from_name(name);
                if (mtype == mtype_undefined)
                {
                    mtype = mtype_from_name_fuzzy(name);
                    if (mtype != mtype_undefined)
                    {
                        explain_output_error_and_die
                        (
                            "architecture %s unknown, did you mean %s instead?",
                            name.quote_c().c_str(),
                            mtype_name(mtype).quote_c().c_str()
                        );
                    }
                    explain_output_error_and_die
                    (
                        "architecture %s unknown",
                        name.quote_c().c_str()
                    );
                }
                byte_sex = byte_sex_from_mtype(mtype);
            }
            break;

        case 'B':
            size_kb = parse_size(optarg);
            if (size_kb & 3)
            {
                explain_output_error_and_die
                (
                    "the size given (%dKB) is not a multiple of 4KB",
                    size_kb
                );
            }
            if (size_kb < 4 || size_kb > MAX_DISK_SIZE_KB)
            {
                explain_output_error_and_die
                (
                    "the size given (%dKB) is not in the range 4 to %d",
                    size_kb,
                    MAX_DISK_SIZE_KB
                );
            }
            break;

        case 'b':
            boot_file = optarg;
            break;

        case 'D':
            ++debug_level;
            break;

        case 'i':
            interleave = "apple";
            break;

        case 'I':
            interleave = optarg;
            break;

        case 'L':
            volid = optarg;
            break;

        case 't':
            twin = true;
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
    // Default the file system size from the machine type.
    //
    if (size_kb == 0)
    {
        switch (mtype)
        {
        case mtype_undefined:
            // that's what it was first coded as
            size_kb = 140;
            break;

        case mtype_pdp11:
            // The old 8-inch floppies
            size_kb = 800;
            break;

        case mtype_6502:
            // Apple ][ Pascal disks were this size
            size_kb = 140;
            break;

        default:
            size_kb = 256;
            break;
        }
    }
    assert(size_kb >= 4);

    zero_whole_volume(filename, size_kb);

    sector_io::pointer raw = sector_io_raw::create(filename);
    sector_io::pointer cooked = sector_io::interleave_factory(interleave, raw);
    directory dir(cooked, byte_sex);
    dir.mkfs(volid.upcase(), twin);
    dir.meta_sync();
    if (boot_file)
        dir.set_boot_blocks(boot_file);

    return 0;
}
