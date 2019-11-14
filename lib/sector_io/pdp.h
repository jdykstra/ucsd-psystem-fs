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

#ifndef LIB_SECTOR_IO_PDP_H
#define LIB_SECTOR_IO_PDP_H

#include <lib/sector_io.h>

/**
  * The sector_io_pdp class is used to represent a sector_io filter
  * which interleaves the sectors the way PDP11 did it.  These disks had
  * N tracks of 26 sectors of 128 bytes.
  */
class sector_io_pdp:
    public sector_io
{
public:
    /**
      * The destructor.
      */
    virtual ~sector_io_pdp();

private:
    /**
      * The constructor.
      */
    sector_io_pdp(const pointer &deeper);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
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
    int sync(void);

    // See base class for documentation.
    bool is_read_only(void) const;

    // See base class for documentation.
    unsigned size_multiple_in_bytes(void) const;

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
    sector_io_pdp();

    /**
      * The copy constructor.  Do not use.
      */
    sector_io_pdp(const sector_io_pdp &);

    /**
      * The assignment operator.  Do not use.
      */
    sector_io_pdp &operator=(const sector_io_pdp &);
};

#endif // LIB_SECTOR_IO_PDP_H
