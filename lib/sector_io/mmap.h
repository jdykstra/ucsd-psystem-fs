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

#ifndef LIB_SECTOR_IO_MMAP_H
#define LIB_SECTOR_IO_MMAP_H

#include <lib/config.h>
#include <lib/rcstring.h>
#include <lib/sector_io.h>

/**
  * The sector_io_mmap class is used to represent a raw disk image to be
  * accessed via a memory mapped file.  In theory this will be faster
  * than sector_io_raw, but these are such small file systems it hardly
  * matters.
  */
class sector_io_mmap:
    public sector_io
{
public:
    /**
      * The destructor.
      */
    virtual ~sector_io_mmap();

    /**
      * The available class method is used to determine whether or not
      * memory mapping is available for the given file.
      *
      * @param filename
      *     The name of the Unix file containing the UCSD p-System
      *     filesystem image to be memory mapped.
      * @param read_only
      *     whether the file system may only be read (true) or whether
      *     it may be both read and written (false).
      * @returns
      *     true if memory mapping is available for this file,
      *     and false if it is not.
      */
    static bool available(const rcstring &filename, bool read_only = false);

private:
    /**
      * The constructor.
      *
      * @param filename
      *     The name of the Unix file containing the UCSD p-System
      *     filesystem image to be memory mapped.
      * @param read_only
      *     whether the file system may only be read (true) or whether
      *     it may be both read and written (false).
      */
    sector_io_mmap(const rcstring &filename, bool read_only);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param filename
      *     The name of the Unix file containing the UCSD p-System
      *     filesystem image to be memory mapped.
      * @param read_only
      *     whether the file system may only be read (true) or whether
      *     it may be both read and written (false).
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
    int size_in_sectors();

    // See base class for documentation.
    int sync();

    // See base class for documentation.
    bool is_read_only() const;

    // See base class for documentation.
    unsigned bytes_per_sector() const;

    // See base class for documentation.
    unsigned size_multiple_in_bytes() const;

    // See base class for documentation.
    void bytes_per_sector_hint(unsigned nbytes);

    // See base class for documentation.
    rcstring get_filename(void) const;

private:
    /**
      * The filename instance variable is used to remember the name of
      * the file being accessed.
      */
    rcstring filename;

    /**
      * The read_only instance variable is used to remember if the file
      * system may be read and written (false) or may only be read
      * (true).
      */
    bool read_only;

    /**
      * The base instance variable is used to remember the address of
      * the beginning of the memory array mapped over the file contents.
      */
    unsigned char *base;

    /**
      * The length instance variable is used to remember the size
      * in bytes of the memory mapped region pointed to by the base
      * instance variable.
      */
    size_t length;

    /**
      * The fake_bytes_per_sector instance variable is used to remember
      * the current bytes per sector hint.  It has no effect on our I/O
      * performance, unless we are below in interleaving filter.
      */
    unsigned fake_bytes_per_sector;

    /**
      * The default constructor.  Do not use.
      */
    sector_io_mmap();

    /**
      * The copy constructor.  Do not use.
      */
    sector_io_mmap(const sector_io_mmap &);

    /**
      * The assignment operator.  Do not use.
      */
    sector_io_mmap &operator=(const sector_io_mmap &);
};

#endif // LIB_SECTOR_IO_MMAP_H
