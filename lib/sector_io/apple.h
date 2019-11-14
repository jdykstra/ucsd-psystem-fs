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

#ifndef LIB_SECTOR_IO_APPLE_H
#define LIB_SECTOR_IO_APPLE_H

#include <lib/sector_io.h>

/**
  * The sector_io_apple class is used to represent a sector_io filter
  * which interleaves the sectors the way Apple ][ Pascal did it.  These
  * disks had 35 tracks of 16 sectors of 256 bytes.
  */
class sector_io_apple:
    public sector_io
{
public:
    /**
      * The destructor.
      */
    virtual ~sector_io_apple();

private:
    /**
      * The constructor.
      * It is private on purpose, use the create class method instead.
      *
      * @param deeper
      *     The sector I/O that this filter operates upon.
      */
    sector_io_apple(const pointer &deeper);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     The sector I/O that this filter operates upon.
      */
    static pointer create(const pointer &deeper);

protected:
    // See base class for documentation.
    int read_sector(unsigned sector_number, void *data);

    // See base class for documentation.
    int write_sector(unsigned sector_number, const void *data);

    // See base class for documentation.
    int size_in_sectors(void);

    // See base class for documentation.
    unsigned bytes_per_sector(void) const;

    // See base class for documentation.
    unsigned size_multiple_in_bytes(void) const;

    // See base class for documentation.
    int sync(void);

    // See base class for documentation.
    bool is_read_only(void) const;

    // See base class for documentation.
    rcstring get_filename(void) const;

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * sector_io device this filter operates upon.
      */
    pointer deeper;

    /**
      * The default constructor.  Do not use.
      */
    sector_io_apple();

    /**
      * The copy constructor.  Do not use.
      */
    sector_io_apple(const sector_io_apple &);

    /**
      * The assignment operator.  Do not use.
      */
    sector_io_apple &operator=(const sector_io_apple &);
};

#endif // LIB_SECTOR_IO_APPLE_H
