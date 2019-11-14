//
// UCSD p-System filesystem in user space
// Copyright (C) 2007, 2010 Peter Miller
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

#ifndef LIB_SECTOR_IO_IMD_H
#define LIB_SECTOR_IO_IMD_H

#include <cstdio>

#include <lib/rcstring.h>
#include <lib/sector_io.h>

/**
  * The sector_io_imd class is used to represent access to a disk image
  * in "IMageDisk" format.
  *
  * Here is source code http://www.classiccmp.org/dunfield/img/imd117sc.zip
  * Here is documentation http://www.classiccmp.org/dunfield/img/imd117.zip
  */
class sector_io_imd:
    public sector_io
{
public:
    /**
      * The destructor.
      */
    virtual ~sector_io_imd();

private:
    /**
      * The constructor.
      * It si private op purpose, use the creat class method instead.
      */
    sector_io_imd(const rcstring &filename);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(const rcstring &filename, bool read_only);

    /**
      * The candidate class method is used to determine if the given file
      * is a candidate for being in the IMD format.
      */
    static bool candidate(const rcstring &filename);

protected:
    // See base class for documentation
    int read_sector(unsigned sector_number, void *o_data);

    // See base class for documentation
    int read(off_t offset, void *o_data, size_t size);

    // See base class for documentation
    int write_sector(unsigned sector_number, const void *i_data);

    // See base class for documentation
    int write(off_t offset, const void *i_data, size_t size);

    // See base class for documentation
    int size_in_sectors(void);

    // See base class for documentation
    unsigned bytes_per_sector(void) const;

    // See base class for documentation
    unsigned size_multiple_in_bytes(void) const;

    // See base class for documentation
    int sync(void);

    // See base class for documentation
    bool is_read_only(void) const;

    // See base class for documentation
    rcstring get_filename(void) const;

private:
    rcstring filename;

    struct track
    {
        track();
        ~track();
        void read(FILE *fp, const rcstring &filename);

        unsigned cylinder;
        unsigned head;
        unsigned sectors;
        unsigned sector_size;
        unsigned char *sector_map;
        size_t data_size_in_bytes;
        unsigned char *data;
    };

    track **tracks;
    size_t tracks_count;
    size_t tracks_count_max;

    size_t data_size_in_bytes;
    unsigned char *data;

    bool read_track(FILE *fp);

    /**
      * The default constructor.  Do not use.
      */
    sector_io_imd();

    /**
      * The copy constructor.  Do not use.
      */
    sector_io_imd(const sector_io_imd &);

    /**
      * The assignment operator.  Do not use.
      */
    sector_io_imd &operator=(const sector_io_imd &);
};

#endif // LIB_SECTOR_IO_IMD_H
