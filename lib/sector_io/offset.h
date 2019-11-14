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

#ifndef LIB_SECTOR_IO_OFFSET_H
#define LIB_SECTOR_IO_OFFSET_H

#include <lib/sector_io.h>

/**
  * The sector_io_offset class is used to represent a sector I/O filter
  * which offsets sector numbers to skip the leading portion of the
  * medium.
  */
class sector_io_offset:
    public sector_io
{
public:
    /**
      * The destructor.
      */
    virtual ~sector_io_offset();

private:
    /**
      * The constructor.
      *
      * @param deeper
      *     The sector I/O this filter operates upon.
      * @param byte_offset
      *     The number of bytes to skip at the start of the medium.
      */
    sector_io_offset(const pointer &deeper, off_t offset);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     The sector I/O this filter operates upon.
      * @param byte_offset
      *     The number of bytes to skip at the start of the medium.
      */
    static pointer create(const pointer &deeper, off_t offset);

protected:
    // See base class for documentation.
    int read_sector(unsigned sector_number, void *data);

    // See base class for documentation.
    int read(off_t byte_offset, void *data, size_t nbytes);

    // See base class for documentation.
    int write_sector(unsigned sector_number, const void *data);

    // See base class for documentation.
    int write(off_t byte_offset, const void *data, size_t nbytes);

    // See base class for documentation.
    int size_in_sectors();

    // See base class for documentation.
    int sync();

    // See base class for documentation.
    unsigned bytes_per_sector() const;

    // See base class for documentation.
    unsigned size_multiple_in_bytes() const;

    // See base class for documentation.
    bool is_read_only() const;

    // See base class for documentation.
    void bytes_per_sector_hint(unsigned nbytes);

    // See base class for documentation.
    rcstring get_filename(void) const;

private:
    /**
      * The offset instance variable is used to remember the sector I/O
      * this filter operates upon.
      */
    pointer deeper;

    /**
      * The byte_offset instance variable is used to remember the number
      * of bytes to skip at the start of the medium.
      */
    off_t byte_offset;

    /**
      * The default constructor.  Do not use.
      */
    sector_io_offset();

    /**
      * The copy constructor.  Do not use.
      */
    sector_io_offset(const sector_io_offset &);

    /**
      * The assignment operator.  Do not use.
      */
    sector_io_offset &operator=(const sector_io_offset &);
};

#endif // LIB_SECTOR_IO_OFFSET_H
