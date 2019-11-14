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
#include <cstdlib>
#include <fcntl.h>
#include <libexplain/close.h>
#include <libexplain/fopen.h>
#include <libexplain/fstat.h>
#include <libexplain/ftruncate.h>
#include <libexplain/getc.h>
#include <libexplain/lseek.h>
#include <libexplain/open.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <libexplain/read.h>
#include <libexplain/write.h>
#include <unistd.h>

#include <lib/version.h>


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


static void
fill_with_noise(unsigned char *buffer, size_t size)
{
    for (size_t j = 0; j < size; ++j)
        buffer[j] = rand() >> 7;
}


static void
test_write_then_read(const char *filename, size_t nbytes)
{
    int fd = explain_open_or_die(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);

    //
    // Write a partial block of data at the start of the file.
    //
    size_t nbytes_even = (nbytes + 1023) & -1023;
    unsigned char *buffer = new unsigned char [nbytes_even];
    fill_with_noise(buffer, sizeof(buffer));
    int n = explain_write_or_die(fd, buffer, nbytes);
    if ((size_t)n != nbytes)
    {
        explain_output_error_and_die
        (
            "write %s: wrong size: gave %ld, got %ld",
            filename,
            (long)nbytes,
            (long)n
        );
    }

    //
    // check that fstat has the same data as we expect.
    //
    struct stat st;
    explain_fstat_or_die(fd, &st);
    if ((size_t)st.st_size != nbytes)
    {
        explain_output_error_and_die
        (
            "fstat %s: wrong size: expected %d, got %d",
            filename,
            (int)nbytes,
            (int)st.st_size
        );
    }

    //
    // Now read it back and see if it is correct.
    //
    char *buf2 = new char [nbytes_even];
    explain_lseek_or_die(fd, 0, SEEK_SET);
    n = explain_read_or_die(fd, buf2, nbytes_even);
    if ((size_t)n != nbytes)
    {
        explain_output_error_and_die
        (
            "read %s: wrong size: expected %ld, got %ld",
            filename,
            (long)nbytes,
            (long)n
        );
    }

    explain_close_or_die(fd);

    delete buf2;
    delete buffer;
}


static void
test_basic_write_then_read(const char *filename)
{
    // Test less than one block
    test_write_then_read(filename, 107);
}


static void
test_block_write_then_read(const char *filename)
{
    // Test exact block multiple.
    test_write_then_read(filename, 1024);
}


static void
test_write_with_holes(const char *filename)
{
    int fd = explain_open_or_die(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);

    explain_lseek_or_die(fd, 521, SEEK_SET);

    unsigned char buffer[523];
    fill_with_noise(buffer, sizeof(buffer));
    explain_write_or_die(fd, buffer, sizeof(buffer));

    explain_lseek_or_die(fd, 1051, SEEK_SET);

    unsigned char buffer2[541];
    fill_with_noise(buffer2, sizeof(buffer2));
    explain_write_or_die(fd, buffer2, sizeof(buffer2));

    explain_close_or_die(fd);

    //
    // Now go back and make sure the data are correct.
    //
    FILE *fp = explain_fopen_or_die(filename, "rb");
    for (int addr = 0; ; ++addr)
    {
        int c = explain_getc_or_die(fp);
        if (c == EOF)
        {
            break;
        }
        if (addr < 521)
        {
            if (c != 0)
            {
                explain_output_error_and_die
                (
                    "%s: zero padding broken",
                    filename
                );
            }
        }
        else if (addr < 521 + 523)
        {
            if (c != buffer[addr - 521])
            {
                explain_output_error_and_die
                (
                    "%s: data wrong (addr %d, expected %d, got %d)",
                    filename,
                    addr,
                    buffer[addr - 521],
                    c
                );
            }
        }
        else if (addr < 1051)
        {
            if (c != 0)
            {
                explain_output_error_and_die
                (
                    "%s: zero padding broken",
                    filename
                );
            }
        }
        else if (addr < 1051 +  541)
        {
            if (c != buffer2[addr - 1051])
            {
                explain_output_error_and_die
                (
                    "%s: data wrong (addr %d, expected %d, got %d)",
                    filename,
                    addr,
                    buffer2[addr - 1051],
                    c
                );
            }
        }
        else
        {
            explain_output_error_and_die("%s: file too long", filename);
        }
    }
    fclose(fp);
}


static void
test_truncate_with_holes(const char *filename)
{
    int fd = explain_open_or_die(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);

    unsigned char buffer[523];
    fill_with_noise(buffer, sizeof(buffer));
    explain_write_or_die(fd, buffer, sizeof(buffer));

    explain_ftruncate_or_die(fd, 1051);

    explain_close_or_die(fd);

    //
    // Now go back and make sure the data are correct.
    //
    FILE *fp = explain_fopen_or_die(filename, "rb");
    for (int addr = 0; ; ++addr)
    {
        int c = explain_getc_or_die(fp);
        if (c == EOF)
        {
            break;
        }
        if (addr < 523)
        {
            if (c != buffer[addr])
            {
                explain_output_error_and_die
                (
                    "%s: data wrong (addr %d, expected %d, got %d)",
                    filename,
                    addr,
                    buffer[addr],
                    c
                );
            }
        }
        else if (addr < 1051)
        {
            if (c != 0)
            {
                explain_output_error_and_die
                (
                    "%s: zero padding broken",
                    filename
                );
            }
        }
        else
        {
            explain_output_error_and_die("%s: file too long", filename);
        }
    }
    fclose(fp);
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);
    srand(getpid());
    void (*func)(const char *) = 0;
    for (;;)
    {
        int c = getopt(argc, argv, "bceS:tV");
        if (c == EOF)
            break;
        switch (c)
        {
        case 'b':
            func = test_basic_write_then_read;
            break;

        case 'c':
            func = test_block_write_then_read;
            break;

        case 'e':
            func = test_write_with_holes;
            break;

        case 'S':
            srand(atoi(optarg));
            break;

        case 't':
            func = test_truncate_with_holes;
            break;

        case 'V':
            version_print();
            return 0;

        default:
            usage();
        }
    }
    if (!func)
        usage();
    if (optind + 1 != argc)
        usage();
    const char *filename = argv[optind];
    func(filename);
    return 0;
}
