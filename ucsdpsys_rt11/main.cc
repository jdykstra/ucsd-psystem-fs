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
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <getopt.h>
#include <libexplain/close.h>
#include <libexplain/creat.h>
#include <libexplain/fclose.h>
#include <libexplain/fopen.h>
#include <libexplain/fstat.h>
#include <libexplain/lseek.h>
#include <libexplain/open.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <libexplain/read.h>
#include <libexplain/utime.h>
#include <libexplain/write.h>
#include <unistd.h>
#include <utime.h>

#include <lib/endof.h>
#include <lib/rcstring.h>
#include <lib/version.h>


static unsigned
get_word(const unsigned char *data)
{
    return (data[0] | (data[1] << 8));
}


struct filentry
{
    enum
    {
        tentative = 1,
        empty = 2,
        permanent = 4,
        endmark = 8
    };

    unsigned char stat_lo;
    unsigned char stat_hi;
    short first;
    short second;
    short extension;
    short lgth;
    unsigned char job;
    unsigned char ce;
    unsigned char day;
    unsigned char month;
    unsigned char year;

    unsigned block;

    rcstring get_title(void) const;

    size_t decode(const unsigned char *data, unsigned &block);
    void list(void) const;
    time_t get_mtime(void) const;
};


size_t
filentry::decode(const unsigned char *data, unsigned &a_block)
{
    stat_lo = data[0];
    stat_hi = data[1];
    first = get_word(data + 2);
    second = get_word(data + 4);
    extension = get_word(data + 6);
    lgth = get_word(data + 8);
    job = data[10];
    ce = data[11];
    unsigned date = get_word(data + 12);
    month = (date >> 10) & 15;
    day = (date >> 5) & 31;
    year = date & 31;
    year += 0110;
    block = a_block;
    a_block += lgth;
    return 14;
}


struct direc
{
    short segsavail;
    short nextseg;
    short highseg;
    short extra;
    unsigned short beginseg;
    filentry entry[72];

    enum { min_size = 10 + 72 * 14 };

    size_t decode(const unsigned char *data);
    void list(void) const;
    const filentry *find(const rcstring &name) const;
};


size_t
direc::decode(const unsigned char *data)
{
    segsavail = get_word(data);
    nextseg = get_word(data + 2);
    highseg = get_word(data + 4);
    extra = get_word(data + 6);
    beginseg = get_word(data + 8);

    unsigned block = beginseg;
    size_t result = 10;
    for (unsigned j = 0; j < 72; ++j)
        result += entry[j].decode(data + result, block);
    return result;
}


const filentry *
direc::find(const rcstring &name)
    const
{
    for (const filentry *fep = entry; fep < ENDOF(entry); ++fep)
    {
        if (fep->get_title().downcase() == name)
            return fep;
    }
    return 0;
}


static char
derad50(unsigned x)
{
#if 0
    // PDP-6, PDP-10, DECsystem-10, DECSYSTEM-20
    static const char pdp6_rad50[050] =
    {
        ' ', '0', '1', '2', '3', '4', '5', '6',
        '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
        'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
        'V', 'W', 'X', 'Y', 'Z', '.', '$', '%',
    };
#endif
    // PDP-11, VAX
    static const char pdp11_rad50[050] =
    {
        ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', '$', '.', '%', '0', '1',
        '2', '3', '4', '5', '6', '7', '8', '9',
    };
    assert(x < 050);
    return pdp11_rad50[x];
}


static void
derad50(unsigned short word, char *store)
{
    store[2] = derad50(word % 050);
    word /= 050;
    store[1] = derad50(word % 050);
    word /= 050;
    store[0] = derad50(word % 050);
}


rcstring
filentry::get_title(void)
    const
{
    char stem6[6];
    derad50(first, stem6);
    derad50(second, stem6 + 3);
    rcstring stem = rcstring(stem6, sizeof(stem6)).trim();

    char ext3[3];
    derad50(extension, ext3);
    rcstring ext = rcstring(ext3, sizeof(ext3)).trim();

    rcstring title = stem;
    if (!ext.empty())
        title += "." + ext;
    return title;
}


void
direc::list(void)
    const
{
    printf
    (
        "avail = %d, next = %d, high = %d, extra = %d, begin = %d\n",
        segsavail,
        nextseg,
        highseg,
        extra,
        beginseg
    );

    printf("\nRes Typ Title      Start  Size Job Chn Date\n"
    "--- --- ---------- ----- ----- --- --- -----------\n");
    for (size_t i = 0; i < SIZEOF(entry); ++i)
    {
        const filentry *fep = &entry[i];
        if (fep->stat_hi == filentry::endmark)
            break;
        fep->list();
    }
}


static rcstring
file_type_name(int x)
{
    switch (x)
    {
    case filentry::empty:
        return "Empty";

    case filentry::tentative:
        return "Tentative";

    case filentry::permanent:
        return "Permanent";

    case filentry::endmark:
        return "End";
    }
    return rcstring::printf("%d", x);
}


void
filentry::list(void)
    const
{
    rcstring title = "<unused>";
    if (stat_hi != filentry::empty)
        title = get_title();
    rcstring typ = file_type_name(stat_hi);
    printf
    (
        "%3d %3.3s %-10.10s %5d %5d %3d %3d",
        stat_lo,
        typ.c_str(),
        title.c_str(),
        block,
        lgth,
        job,
        ce
    );
    if (month || day)
    {
        static const char *mon[16] =
        {
            "000",
            "Jan",
            "Feb",
            "Mar",
            "Apr",
            "May",
            "Jun",
            "Jul",
            "Aug",
            "Sep",
            "Oct",
            "Nov",
            "Dec",
            "013",
            "014",
            "015",
        };

        printf
        (
            " 19%02d-%s-%02d",
            year,
            mon[month],
            day
        );
    }
    printf("\n");
}


time_t
filentry::get_mtime(void)
    const
{
    if (month == 0)
        return 0;
    if (day == 0)
        return 0;
    struct tm tm;
    tm.tm_year = year;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 12;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    return mktime(&tm);
}


static void
extract_one_file(const direc &rt11, const rcstring &dsk, const rcstring &get)
{
    const filentry *fep = rt11.find(get);
    if (!fep)
    {
        explain_output_error_and_die
        (
            "file %s not found",
            get.quote_c().c_str()
        );
    }

    rcstring dsk_fil = dsk + ".files";
    int ifd = explain_open_or_die(dsk_fil.c_str(), O_RDONLY, 0666);

    assert(fep->block >= rt11.beginseg);
    unsigned block = fep->block - rt11.beginseg;

    explain_lseek_or_die(ifd, block << 9, SEEK_SET);
    bool binary = false;
    {
        unsigned char data[512];
        if (explain_read_or_die(ifd, data, 512) != 512)
            explain_output_error_and_die("%s: short file", dsk_fil.c_str());
        for (size_t j = 0; j < 512; ++j)
        {
            unsigned char c = data[j];
            switch (c)
            {
            case '\0':
            case '\t':
            case '\r':
            case '\n':
            case ' ':
                continue;

            default:
                if (c >= 128 || !isprint(c))
                {
                    binary = true;
                    break;
                }
                continue;
            }
            break;
        }
    }
    explain_lseek_or_die(ifd, block << 9, SEEK_SET);

    if (binary)
    {
        int ofd = explain_creat_or_die(get.c_str(), 0666);
        for (int j = 0; j < fep->lgth; ++j)
        {
            char data[512];
            if (explain_read_or_die(ifd, data, 512) != 512)
                explain_output_error_and_die("%s: short file", dsk_fil.c_str());
            explain_write_or_die(ofd, data, 512);
        }
        explain_close_or_die(ofd);
    }
    else
    {
        FILE *ofp = explain_fopen_or_die(get.c_str(), "w");
        for (int j = 0; j < fep->lgth; ++j)
        {
            char data[512];
            if (explain_read_or_die(ifd, data, 512) != 512)
                explain_output_error_and_die("%s: short file", dsk_fil.c_str());
            for (size_t k = 0; k < 512; ++k)
            {
                unsigned char c = data[k];
                switch (c)
                {
                case '\0':
                case '\r':
                    break;

                default:
                    fputc(c, ofp);
                    break;
                }
            }
        }
        explain_fclose_or_die(ofp);
    }
    explain_close_or_die(ifd);

    time_t when = fep->get_mtime();
    if (when > 0)
    {
        struct utimbuf buf;
        buf.actime = when;
        buf.modtime = when;
        explain_utime_or_die(get.c_str(), &buf);
    }
}


static void
usage(void)
{
    const char *prog = "ucsdpsys_rt11";
    fprintf(stderr, "Usage: %s -f <disk> --list\n", prog);
    fprintf(stderr, "       %s -f <disk> --get <name>\n", prog);
    fprintf(stderr, "       %s -f <disk> --extract-all\n", prog);
    fprintf(stderr, "       %s --version\n", prog);
    exit(1);
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);
    rcstring dsk;
    rcstring get;
    bool list = false;
    bool extract_all = false;
    for (;;)
    {
        static const struct option options[] =
        {
            { "disk", 1, 0, 'f' },
            { "extract-all", 0, 0, 'x' },
            { "file", 1, 0, 'f' },
            { "get", 1, 0, 'g' },
            { "get-all", 0, 0, 'x' },
            { "list", 0, 0, 'l' },
            { "version", 0, 0, 'V' },
            { 0, 0, 0, 0 }
        };
        int c = getopt_long(argc, argv, "f:g:lVx", options, 0);
        if (c < 0)
            break;
        switch (c)
        {
        case 'f':
            dsk = optarg;
            if (dsk.ends_with(".dir"))
                dsk = dsk.substring(0, dsk.size() - 4);
            if (dsk.ends_with(".files"))
                dsk = dsk.substring(0, dsk.size() - 6);
            break;

        case 'g':
            get = rcstring(optarg).downcase();
            break;

        case 'l':
            list = true;
            break;

        case 'V':
            version_print();
            return 0;

        case 'x':
            extract_all = true;
            break;

        default:
            usage();
        }
    }
    if (dsk.empty())
        usage();
    if (optind != argc)
        usage();
    if (get.empty() && !extract_all && !list)
        list = true;
    if (!get.empty() + extract_all + list != 1)
        usage();

    direc rt11;
    {
        unsigned char *data = 0;
        size_t data_size = 0;
        {
            rcstring dsk_dir = dsk + ".dir";
            int fd = explain_open_or_die(dsk_dir.c_str(), O_RDONLY, 0666);
            struct stat st;
            explain_fstat_or_die(fd, &st);
            data_size = st.st_size;
            if (data_size > (1 << 20))
            {
                explain_output_error_and_die
                (
                    "%s: file too big",
                    dsk_dir.c_str()
                );
            }
            if (data_size < direc::min_size)
            {
                explain_output_error_and_die
                (
                    "%s: file too small (%d < %d)",
                    dsk_dir.c_str(),
                    (int)data_size,
                    direc::min_size
                );
            }
            data = (unsigned char *)malloc(data_size);
            explain_read_or_die(fd, data, data_size);
            explain_close_or_die(fd);
        }

        rt11.decode(data);
        free(data);
    }

    if (!get.empty())
    {
        extract_one_file(rt11, dsk, get);
        return 0;
    }
    if (extract_all)
    {
        for (const filentry *fep = rt11.entry; fep < ENDOF(rt11.entry); ++fep)
        {
            if (fep->stat_hi == filentry::endmark)
                break;
            extract_one_file(rt11, dsk, fep->get_title().downcase());
        }
        return 0;
    }

    rt11.list();
    return 0;
}
