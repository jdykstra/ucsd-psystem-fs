//
// UCSD p-System filesystem in user space
// Copyright (C) 2007, 2008, 2010 Peter Miller
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
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <libexplain/fclose.h>
#include <libexplain/fopen.h>
#include <libexplain/getc.h>
#include <libexplain/output.h>

#include <lib/sector_io/imd.h>
#include <lib/debug.h>
#include <lib/hexdump.h>

// 6. IMAGE FILE FORMAT
//
// The overall layout of an ImageDisk .IMD image file is:
//
// IMD v.vv: dd/mm/yyyy hh:mm:ss          (ASCII Header)
// Comment (ASCII only - unlimited size)  (NOTE:  You can TYPE a .IMD)
// 1A byte - ASCII EOF character          (file to see header/comment)
// - For each track on the disk:
//   1 byte  Mode value                  (0-5)
//   1 byte  Cylinder                    (0-n)
//   1 byte  Head                        (0-1)   (see Note)
//   1 byte  number of sectors in track  (1-n)
//   1 byte  sector size                 (0-6)
//   sector numbering map                * number of sectors
//   sector cylinder map (optional)      * number of sectors
//   sector head map     (optional)      * number of sectors
//   sector data records                 * number of sectors
// <End of file>
//
// 6.1 Mode value
//
// This value indicates the data transfer rate and density  in  which
// the original track was recorded:
//
//   00 = 500 kbps FM   \   Note:   kbps indicates transfer rate,
//   01 = 300 kbps FM    >          not the data rate, which is
//   02 = 250 kbps FM   /           1/2 for FM encoding.
//   03 = 500 kbps MFM
//   04 = 300 kbps MFM
//   05 = 250 kbps MFM
//
// 6.2 Sector size
//
// The Sector Size value indicates the actual size of the sector data
// occuring on the track:
//
//   00 =  128 bytes/sector
//   01 =  256 bytes/sector
//   02 =  512 bytes/sector
//   03 = 1024 bytes/sector
//   04 = 2048 bytes/sector
//   05 = 4096 bytes/sector
//   06 = 8192 bytes/sector
//
// 6.3 Head value
//
// This value indicates the side of the disk on which this track
// occurs (0 or 1).
//
// Since HEAD can only be 0 or 1, ImageDisk uses the upper bits of
// this byte to indicate the presense of optional items in the track
// data:
//
//   Bit 7 (0x80) = Sector Cylinder Map
//   Bit 6 (0x40) = Sector Head     Map
//
// 6.4 Sector numbering map
//
// The sector numbering map contains one byte entry containing the
// physical ID for each sector that occurs in the track.
//
// Note that these entries may NOT be sequential.  A disk which uses
// sector interleave will have a sector numbering map in which the
// sector numbers occur in non-sequential order.
//
// If ImageDisk is unable to obtain all sector numbers in a single
// revolution of the disk, it will report "Unable to determine
// interleave" and rearrange the sector numbers into a simple
// sequential list.
//
// 6.5 Sector Cylinder Map
//
// This is an optional field.  It's presense is indicated by bit 7
// being set in the Head value for the track.
//
// When present, it means that the cylinder values written to the
// sectors do NOT match the physical cylinder of the track.
//
// The Sector Cylinder Map has one entry for each sector, and
// contains the logical Cylinder ID for the corresponding sector in
// the Sector Numbering Map.
//
// Reading a disk with non-standard Cylinder IDs will require the
// use of the FULL ANALYSIS setting.
//
// 6.6 Sector Head map
//
// This is an optional field.  It's presense is indicated by bit 6
// being set in the Head value for the track.
//
// When present, it means that the head values written to the sectors
// do NOT match the physical head selection of the track.
//
// The Sector Head Map has one entry for each sector, and contains
// the logical Head ID for the corresponding sector in the Sector
// Numbering Map.
//
// Reading a disk with non-standard Head ID's may require the use of
// the FULL ANALYSIS setting.
//
// 6.7 Sector Data Records
//
// For each sector ID occuring in the Sector Numbering Map, ImageDisk
// records a Sector Data Record - these records occur in the same
// order as the IDs in the Sector Numbering Map:
//
//   00      Sector data unavailable - could not be read
//   01 ...  Normal data: (Sector Size) bytes follow
//   02 xx   Compressed: All bytes in sector have same value (xx)
//   03 ...  Normal data with "Deleted-Data address mark"
//   04 xx   Compressed with "Deleted-Data address mark"
//   05 .... Normal data read with data error
//   06 xx   Compressed read with data error
//   07 .... Deleted data read with data error
//   08 xx   Compressed, Deleted read with data error
//

sector_io_imd::~sector_io_imd()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    for (size_t j = 0; j < tracks_count; ++j)
        delete tracks[j];
    delete [] tracks;
    delete [] data;
}


static void
    LIBEXPLAIN_FORMAT_PRINTF(2, 3)
file_not_in_imd_format(const rcstring &filename, const char *fmt, ...)
{
    DEBUG(2, "yuck");
    va_list ap;
    va_start(ap, fmt);
    char buf[200];
    snprintf(buf, sizeof(buf), fmt, ap);

    explain_output_error_and_die
    (
        "file %s not in .IMD format (%s)",
        filename.quote_c().c_str(),
        buf
    );
}


sector_io_imd::sector_io_imd(const rcstring &a_filename) :
    filename(a_filename),
    tracks(0),
    tracks_count(0),
    tracks_count_max(0),
    data_size_in_bytes(0),
    data(0)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    //
    // Open the file
    //
    FILE *fp = explain_fopen_or_die(filename.c_str(), "rb");

    //
    // Read the file magic number and comment.
    //
    char buffer[2048];
    char *bp = buffer;
    for (;;)
    {
        int c = explain_getc_or_die(fp);
        if (c == EOF)
        {
            file_not_in_imd_format(filename, "EOF before end of header");
        }
        if (c == 0x1A)
            break;
        if (bp < buffer + sizeof(buffer))
            *bp++ = c;
    }
    if (bp - buffer < 4 || 0 != memcmp(buffer, "IMD ", 4))
    {
        file_not_in_imd_format(filename, "wrong magic number");
    }

    //
    // Read all the tracks
    //
    while (read_track(fp))
        ;
    if (tracks_count == 0)
    {
        file_not_in_imd_format(filename, "disk image contains no tracks");
    }
    explain_fclose_or_die(fp);

    //
    // consolidate all of the tracks
    //
    DEBUG(2, "data_size_in_bytes = %d", int(data_size_in_bytes));
    assert(data_size_in_bytes > 0);
    DEBUG(2, "mark");
    assert((data_size_in_bytes & 127) == 0);
    DEBUG(2, "mark");
    data = new unsigned char [data_size_in_bytes];
    DEBUG(2, "data = %p", data);
    unsigned char *dp = data;
    DEBUG(2, "tracks = %p", tracks);
    for (size_t j = 0; j < tracks_count; ++j)
    {
        DEBUG(2, "dp = %p", dp);
        track *tp = tracks[j];
        DEBUG(2, "track %ld = %p", (long)j, tp);
        memcpy(dp, tp->data, tp->data_size_in_bytes);
        dp += tp->data_size_in_bytes;
    }
    DEBUG(2, "dp = %p", dp);
    DEBUG(2, "data + data_size_in_bytes = %p", data + data_size_in_bytes);
    assert(dp == data + data_size_in_bytes);

    DEBUG(2, "free tracks");
    for (size_t j = 0; j < tracks_count; ++j)
        delete tracks[j];
    delete [] tracks;
    tracks = 0;
    tracks_count = 0;
    tracks_count_max = 0;
}


sector_io::pointer
sector_io_imd::create(const rcstring &filename, bool read_only)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    if (!read_only)
    {
        explain_output_error_and_die
        (
            "%s: the .IMD format is only supported for read-only access",
            filename.c_str()
        );
    }
    return pointer(new sector_io_imd(filename));
}


bool
sector_io_imd::candidate(const rcstring &filename)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    FILE *fp = fopen(filename.c_str(), "rb");
    if (!fp)
        return false;
    char buffer[4];
    if (fread(buffer, 1, 4, fp) != 4)
    {
        fclose(fp);
        return false;
    }
    fclose(fp);
    DEBUG(2, "data = %s", hexdump(buffer, 4).c_str());
    return (0 == memcmp(buffer, "IMD ", 4));
}


bool
sector_io_imd::read_track(FILE *fp)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    int mode = explain_getc_or_die(fp);
    if (mode == EOF)
    {
        return false;
    }
    if (mode >= 6)
    {
        file_not_in_imd_format(filename, "unknown track mode (%d)", mode);
    }
    track *tp = new track;
    tp->read(fp, filename);
    data_size_in_bytes += tp->data_size_in_bytes;

    if (tracks_count >= tracks_count_max)
    {
        size_t new_tracks_count_max = tracks_count_max * 2 + 16;
        track **new_tracks = new track * [new_tracks_count_max];
        for (size_t j = 0; j < tracks_count; ++j)
            new_tracks[j] = tracks[j];
        delete [] tracks;
        tracks = new_tracks;
        tracks_count_max = new_tracks_count_max;
    }
    tracks[tracks_count++] = tp;
    DEBUG(2, "track read successful");
    return true;
}


sector_io_imd::track::~track()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    delete [] sector_map;
    delete [] data;
}


sector_io_imd::track::track() :
    cylinder(0),
    head(0),
    sectors(0),
    sector_size(0),
    sector_map(0),
    data_size_in_bytes(0),
    data(0)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
}


void
sector_io_imd::track::read(FILE *fp, const rcstring &fn)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    int c = explain_getc_or_die(fp);
    if (c == EOF)
    {
        file_not_in_imd_format(fn, "eof before cylinder number");
    }
    cylinder = c;
    DEBUG(2, "cylinder = %d", cylinder);

    c = explain_getc_or_die(fp);
    if (c == EOF)
    {
        file_not_in_imd_format(fn, "eof before head number");
    }
    head = c;
    bool cylinder_map_present = ((head & 0x80) != 0);
    DEBUG(2, "cylinder_map_present = %d", cylinder_map_present);
    bool head_map_present = ((head & 0x40) != 0);
    DEBUG(2, "head_map_present = %d", head_map_present);
    head &= 1;

    c = explain_getc_or_die(fp);
    if (c == EOF)
    {
        file_not_in_imd_format(fn, "eof before sector number");
    }
    sectors = c;
    DEBUG(2, "sectors = %d", sectors);

    c = explain_getc_or_die(fp);
    if (c == EOF)
        file_not_in_imd_format(fn, "eof before sector size code");
    if (c == EOF || c >= 7)
    {
        file_not_in_imd_format(fn, "bad sector size code (%d)", c);
    }
    sector_size = 128 << c;
    DEBUG(2, "sector_size = %d", sector_size);

    sector_map = new unsigned char [sectors];
    if (fread(sector_map, 1, sectors, fp) != sectors)
    {
        file_not_in_imd_format(fn, "eof before sector map");
    }
    for (unsigned j = 0; j < sectors; ++j)
    {
        if (sector_map[j] < 1 || sector_map[j] > sectors)
        {
            DEBUG(2, "weird sector map (%d not 1..%d)\n", sector_map[j],
                sectors);
        }
        sector_map[j] = (sector_map[j] + sectors - 1) % sectors;
        DEBUG(2, "sector_map[%d] = %d", j, sector_map[j]);
    }

    if (cylinder_map_present)
    {
        char junk[256];
        if (fread(junk, 1, sectors, fp) != sectors)
        {
            file_not_in_imd_format(fn, "eof before cylinder map");
        }
    }

    if (head_map_present)
    {
        char junk[256];
        if (fread(junk, 1, sectors, fp) != sectors)
        {
            file_not_in_imd_format(fn, "eof before head map");
        }
    }

    data_size_in_bytes = sectors * sector_size;
    DEBUG(2, "track data size = %d", int(data_size_in_bytes));
    data = new unsigned char [data_size_in_bytes];
    DEBUG(2, "track data = %p", data);

    for (unsigned j = 0; j < sectors; ++j)
    {
        if (sector_map[j] >= sectors)
        {
            file_not_in_imd_format
            (
                fn,
                "sector_map broken ([%d] = %d/%d)",
                j,
                sector_map[j],
                sectors
            );
        }
        unsigned char *p = data + sector_map[j] * sector_size;
        c = explain_getc_or_die(fp);
        switch (c)
        {
        case 0:
            // sector data unavailable
            DEBUG(1, "data for sector %d unavailable", j);
            memset(p, 0, sector_size);
            break;

        case 1:
            // normal data
            if (fread(p, 1, sector_size, fp) != sector_size)
            {
                file_not_in_imd_format(fn, "eof before sector data");
            }
            break;

        case 2:
            // compressed data
            c = explain_getc_or_die(fp);
            if (c == EOF)
            {
                file_not_in_imd_format(fn, "eof before sector data");
            }
            memset(p, c, sector_size);
            break;

        default:
            file_not_in_imd_format(fn, "unknown sector code %d", c);
        }
    }
    DEBUG(3, "data =\n%s", hexdump(data, data_size_in_bytes).c_str());
    delete [] sector_map;
    sector_map = 0;
}


int
sector_io_imd::read_sector(unsigned sector_number, void *o_data)
{
    DEBUG(2, "sector_io_imd::read_sector(this = %p, sector_number = %u, "
        "o_data = %p)", this, sector_number, o_data);
    off_t offset = (off_t)sector_number * 128;
    return read(offset, o_data, 128);
}


int
sector_io_imd::read(off_t offset, void *o_data, size_t size)
{
    DEBUG(2, "sector_io_imd::read(this = %p, offset = 0x%lX, data = %p, "
        "size = 0x%lX)", this, (long)offset, o_data, (long)size);
    if (offset < 0)
        return -EINVAL;
    if (offset + size > data_size_in_bytes)
        return -ENOSPC;
    memcpy(o_data, data + offset, size);
    return size;
}


int
sector_io_imd::write_sector(unsigned sector_number, const void *o_data)
{
    DEBUG(2, "sector_io_imd::write_sector(this = %p, sector_number = %u, "
        "o_data = %p)", this, sector_number, o_data);
    return -EROFS;
}


int
sector_io_imd::write(off_t offset, const void *o_data, size_t size)
{
    DEBUG(2, "sector_io_imd::write(this = %p, offset = 0x%lX, o_data = %p, "
        "size = 0x%lX)", this, (long)offset, o_data, (long)size);
    return -EROFS;
}


int
sector_io_imd::size_in_sectors()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    DEBUG(2, "return %d\n", (int)(data_size_in_bytes >> 7));
    return (data_size_in_bytes >> 7);
}


unsigned
sector_io_imd::bytes_per_sector()
    const
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    return 128;
}


bool
sector_io_imd::is_read_only()
    const
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    return true;
}


unsigned
sector_io_imd::size_multiple_in_bytes()
    const
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    return 128;
}


int
sector_io_imd::sync()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    return 0;
}


rcstring
sector_io_imd::get_filename(void)
    const
{
    return filename;
}
