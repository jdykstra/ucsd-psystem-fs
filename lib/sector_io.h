//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008, 2010 Peter Miller
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

#ifndef LIB_SECTOR_IO_H
#define LIB_SECTOR_IO_H

#include <lib/config.h>
#include <sys/types.h>

#include <boost/shared_ptr.hpp>

class rcstring; // forward

/**
  * The sector_io class is used to represent an abstract I/O in units of
  * sectors (256 bytes).
  */
class sector_io
{
public:
    typedef boost::shared_ptr<sector_io> pointer;

    /**
      * The destructor.
      */
    virtual ~sector_io();

    /**
      * The factory class method is used to create new dynamically
      * allocated instances, based on the characteristics of the named
      * file.
      *
      * @param filename
      *     The name of the file to open
      * @param read_only
      *     True if the file is to be opened read-only (some formats
      *     only support read-only).
      * @returns
      *     pointer to be sector_io instance, do NOT delete it, it will
      *     quietly disappear when the last pointer is destroyed.
      */
    static pointer factory(const rcstring &filename, bool read_only = true);

    /**
      * The guess_interleaving class method may be used to examine a
      * raw image and attempt to determine what sector interleaving and
      * disk offsets may be required.  It does this by looking for a
      * minimally valid volume label.
      *
      * @param deeper
      *     The raw data to be sniffed
      * @returns
      *     NULL on failure, or a suitable sector_io pointer for access
      *     to the data on success
      */
    static pointer guess_interleaving(pointer deeper);

    /**
      * The interleave_factory class method is used to add some disk
      * sector interleaving.  This is most often done when creating disk
      * images "from scratch".
      *
      * @param name
      *     The name of the interleave pattern, e.g. "apple", "pdp", "none".
      * @param deeper
      *     The disk image to be filtered.
      */
    static pointer interleave_factory(const char *name, const pointer &deeper);

    /**
      * The get_filename method is used to obtain the name of the file
      * being operated on.
      */
    virtual rcstring get_filename(void) const = 0;

protected:
    /**
      * The default constructor.
      */
    sector_io();

    /**
      * The read_sector method is used to read a sector of data from the
      * device.
      *
      * @param sector_number
      *     The number of the sector to be read (sector numbers are zero
      *     based).
      * @param data
      *     Pointer to a object of #sizeof_sector bytes into which the
      *     sector will be read.
      * @returns
      *     0 on success, or -errno on error.
      */
    virtual int read_sector(unsigned sector_number, void *data) = 0;

public:
    /**
      * The read method is used to read a data from the medium.  It does
      * not have to be sector aligned, and it does not have to be for a
      * whole number of sectors.
      *
      * @param offset
      *     The offset in bytes from the start of the medium.  It does
      *     not have to be sector aligned, although it is more efficient
      *     if it is.
      * @param data
      *     Pointer to the array into which the data will be read.
      * @param nbytes
      *     The number of bytes to be read.  It does not have to be a
      *     whole number of sectors, although it is more efficient if it
      *     is.
      * @returns
      *     \a nbytes on success, or -errno on error.
      */
    virtual int read(off_t byte_offset, void *data, size_t nbytes);

protected:
    /**
      * The write_sector method is used to write a sector of data to the
      * device.
      *
      * @param sector_number
      *     The number of the sector to be read (sector numbers are zero
      *     based).
      * @param data
      *     Pointer to a object of #sizeof_sector bytes from which the
      *     sector data will be written.
      * @returns
      *     0 on success, or -errno on error.
      */
    virtual int write_sector(unsigned sector_number, const void *data) = 0;

public:
    /**
      * The write method is used to write data to the medium.  It does
      * not have to be sector aligned, and it does not have to be for a
      * whole number of sectors.
      *
      * @param offset
      *     The offset in bytes from the start of the medium.  It does
      *     not have to be sector aligned, although it is more efficient
      *     if it is.
      * @param data
      *     Pointer to the array from which the data will be written.
      * @param nbytes
      *     The number of bytes to be written.  It does not have to be a
      *     whole number of sectors, although it is more efficient if it
      *     is.
      * @returns
      *     \a nbytes on success, or -errno on error.
      */
    virtual int write(off_t byte_offset, const void *data, size_t nbytes);

    /**
      * The write_zero method is used to zero data to the medium.  It
      * does not have to be sector aligned, and it does not have to be
      * for a whole number of sectors.
      *
      * @param offset
      *     The offset in bytes from the start of the medium.  It does
      *     not have to be sector aligned, although it is more efficient
      *     if it is.
      * @param nbytes
      *     The number of bytes to be written.  It does not have to be a
      *     whole number of sectors, although it is more efficient if it
      *     is.
      * @returns
      *     zero on success, or -errno on error.
      */
    virtual int write_zero(off_t byte_offset, size_t nbytes);

protected:
    /**
      * The size_in_sectors method is used to obtain the number of
      * sectors in the medium.
      *
      * @returns
      *     positive number on success, -errno on error.  (Should never
      *     return zero.)
      */
    virtual int size_in_sectors(void) = 0;

    /**
      * The bytes_per_sector method is used to determine the underlying
      * sector size.
      *
      * It will always be a power of two.
      * It will always be <= 512.
      */
    virtual unsigned bytes_per_sector(void) const = 0;

public:
    /**
      * The size_in_bytes method is used to obtain the full size of the
      * disk image, in bytes.
      */
    int size_in_bytes(void);

    /**
      * The size_multiple_in_bytes method is used to determine the size
      * multiple, in bytes, for valid disk image sizes.
      */
    virtual unsigned size_multiple_in_bytes(void) const = 0;

    /**
      * The bytes_per_sector_hint method is ucalled by interleavoing
      * filters to give a hit to the lower layer as to the size of
      * sector it will be using.  It is expected that some classes may
      * find this information useful.  The default implementation of
      * this method does nothing.
      *
      * @param nbytes
      *     The number of bytes per sector.
      */
    virtual void bytes_per_sector_hint(unsigned nbytes);

    /**
      * The sync method is used to flush all content to the diak medium.
      *
      * @returns
      *     zero for success, -errno on error.
      */
    virtual int sync(void) = 0;

private:
    /**
      * The relocate_sectors method is used to move ranges of blocks
      * back and forth within the medium.
      *
      * @param to_sector_num
      *     The number of the first destination sector.
      * @param from_sector_num
      *     The number of the first source sector.
      * @param size_in_sectors
      *     The number of sectors to move.
      * @returns
      *     zero on success, or -errno on error.
      */
    int relocate_sectors(unsigned to_sector_num, unsigned from_sector_num,
        unsigned size_in_sectors);

public:
    /**
      * The relocate_bytes method is used to move ranges of blocks back
      * and forth within the medium.
      *
      * @param to_sector_num
      *     The number of the first destination sector.
      * @param from_sector_num
      *     The number of the first source sector.
      * @param size_in_sectors
      *     The number of sectors to move.
      * @returns
      *     zero on success, or -errno on error.
      */
    int relocate_bytes(off_t to, off_t from, size_t nbytes);

    /**
      * The is_read_only method may be used to determine whether
      * the file system is read-only (true) or read-write (false).
      */
    virtual bool is_read_only(void) const = 0;

private:
    /**
      * The copy constructor.  Do not use.
      */
    sector_io(const sector_io &);

    /**
      * The assignment operator.  Do not use.
      */
    sector_io &operator=(const sector_io &);
};

#endif // LIB_SECTOR_IO_H
