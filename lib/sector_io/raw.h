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

#ifndef LIB_SECTOR_IO_RAW_H
#define LIB_SECTOR_IO_RAW_H

#include <lib/rcstring.h>
#include <lib/sector_io.h>

/**
  * The sector_io_raw class is used to represent sector data to be read
  * from a Unix file or device.
  */
class sector_io_raw:
    public sector_io
{
public:
    /**
      * The destructor.
      */
    virtual ~sector_io_raw();

private:
    /**
      * The constructor.
      */
    sector_io_raw(const rcstring &filename, bool read_only);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(const rcstring &filename, bool read_only = false);

protected:
    // See base class for documentation.
    int read_sector(unsigned sector_number, void *data);

    // See base class for documentation.
    int read(off_t offset, void *data, size_t size);

    // See base class for documentation.
    int write_sector(unsigned sector_number, const void *data);

    // See base class for documentation.
    int write(off_t offset, const void *data, size_t size);

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
    void bytes_per_sector_hint(unsigned nbytes);

    // See base class for documentation.
    rcstring get_filename(void) const;

private:
    /**
      * The filename instance variable is used to remember the name of
      * the file containing the file system image.
      */
    rcstring filename;

    /**
      * The fd instance variable is used to remember the file descriptor
      * of the Unix file representing this I/O device.
      */
    int fd;

    /**
      * The err instance variable is used to remember the most recent error.
      */
    int err;

    /**
      * The read_only instance variable is used to remember whether or
      * not the file system is to be mounted read-only.  It defaults to
      * false (read and write).
      */
    bool read_only;

    /**
      * The fake_bytes_per_sector instance variable is used to remember
      * the current bytes per sector hint.  It has no effect on our I/O
      * performance, unless we are below in interleaving filter.
      */
    unsigned fake_bytes_per_sector;

    /**
      * The default constructor.  Do not use.
      */
    sector_io_raw();

    /**
      * The copy constructor.  Do not use.
      */
    sector_io_raw(const sector_io_raw &);

    /**
      * The assignment operator.  Do not use.
      */
    sector_io_raw &operator=(const sector_io_raw &);
};

#endif // LIB_SECTOR_IO_RAW_H
