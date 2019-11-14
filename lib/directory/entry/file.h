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

#ifndef LIB_DIRECTORY_ENTRY_FILE_H
#define LIB_DIRECTORY_ENTRY_FILE_H

#include <lib/directory/entry.h>
#include <lib/sector_io.h>

/**
  * The directory_entry_file class is used to represent the directory
  * entry of a simple file.
  */
class directory_entry_file:
    public directory_entry
{
public:
    /**
      * The destructor.
      */
    virtual ~directory_entry_file();

protected:
    /**
      * The constructor.  It is private on purpose, use the #create
      * class method instead.
      *
      * @param parent
      *     The directory containing this file.
      * @param data
      *     The data defining this directory entry.
      * @param deeper
      *     The data to be cropped to access data for this file.
      */
    directory_entry_file(directory *parent, const unsigned char *data,
        const sector_io::pointer &deeper);

    /**
      * The constructor.  It is private on purpose, use the #create
      * class method instead.
      *
      * @param parent
      *     The directory containing this file.
      * @param name
      *     The name of the new file.
      * @param dfkind
      *     The kind of the new file.
      * @param sector
      *     The first block of the file.
      * @param num_blocks
      *     The number of blocks in the new file.
      * @param deeper
      *     The data to be cropped to access data for this file.
      */
    directory_entry_file(directory *parent, const rcstring &name,
        dfkind_t dfkind, int block, int num_blocks,
        const sector_io::pointer &deeper);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param parent
      *     The directory containing this file.
      * @param data
      *     The data defining this directory entry.
      * @param deeper
      *     The data to be cropped to access data for this file.
      */
    static pointer create(directory *parent, const unsigned char *data,
        const sector_io::pointer &deeper);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param parent
      *     The directory containing this file.
      * @param name
      *     The name of the new file.
      * @param dfkind
      *     The kind of the new file.
      * @param sector
      *     The first block of the file.
      * @param num_blocks
      *     The number of blocks in the new file.
      * @param deeper
      *     The data to be cropped to access data for this file.
      */
    static pointer create(directory *parent, const rcstring &name,
        dfkind_t dfkind, int block, int num_blocks,
        const sector_io::pointer &deeper);

protected:
    // See base class for documentation.
    rcstring get_name() const;

    // See base class for documentation.
    int getattr(struct stat *stbuf);

    // See base class for documentation.
    int unlink();

    // See base class for documentation.
    int rename(const char *name);

    // See base class for documentation.
    int chmod(mode_t mode);

    // See base class for documentation.
    int chown(uid_t uid, gid_t gid);

    // See base class for documentation.
    int open();

    // See base class for documentation.
    int truncate(off_t size);

    // See base class for documentation.
    int utime_ns(const struct timespec *buf);

    // See base class for documentation.
    int read(off_t offset, void *data, size_t nbytes) const;

    // See base class for documentation.
    int write(off_t offset, const void *data, size_t nbytes);

    // See base class for documentation.
    size_t get_name_maxlen() const;

    // See base class for documentation.
    int get_first_block() const;

    // See base class for documentation.
    int get_last_block() const;

    // See base class for documentation.
    int relocate(unsigned to_block);

    // See base class for documentation.
    int fsck(concern_t concern_level);

    // See base class for documentation.
    void fsck_first_block(int blknum);

    // See base class for documentation.
    void fsck_last_block(int blknum);

    // See base class for documentation.
    void print_listing(bool verbose);

    // See base class for documentation.
    dfkind_t get_file_kind() const;

    // See base class for documentation.
    size_t get_size_in_bytes(void) const;

    // See base class for documentation.
    time_t get_mtime(void) const;

private:
    /**
      * The deeper instance variable is used to remember the sector I/O
      * we are to use when reading and writing our data.
      */
    sector_io::pointer deeper;

    /**
      * The dfistblock instance variable is used to remember the number
      * of the first 512-byte block containg the data associated with
      * this file.
      */
    int dfirstblock;

    /**
      * The dlastblock instance variable is used to remember the number
      * of the 512-byte block immediately beywond the end of the data
      * associated with this file.
      */
    int dlastblock;

    unsigned padding4;

    /**
      * The dfkind instance variable is used to remember what kinf od
      * file this is.  This information is not used by Unix.  When new
      * fiels are created, a guess is made (from the name) as to the
      * appropriate file type.
      */
    dfkind_t dfkind;

    /**
      * The status instance variable is used to remember the status bit
      * associate with the file.  I've never seen a file which did not
      * have this be false.  The UCSD p-System documentation isn't very
      * helpful in describing it, either.
      */
    bool status;

    /**
      * The name instance variable is used to remember the name of the
      * file.  The length <b>must</b> be in the range 1 to 15.
      */
    rcstring name;

    unsigned padding22;

    /**
      * The dlastbyte instance variable is used to remember how many
      * bytes have been used in the lost block of the file.  Values
      * range from 1 to 512.  A value if zero is invalid, however
      * logical it may appear to be.
      */
    int dlastbyte;

    /**
      * The when instance variable is used to remember the date the
      * file was last modified.  When in memory, the full Unix time
      * resolution is available.  When on disk, only the date portion is
      * available.  This is particularly noticable when you umount and
      * then remount the disk image.
      */
    time_t when;

    /**
      * The meta_read method is used to parse the on-disk 26-byte
      * representation of the meta data into our instance variables.
      *
      * @param data
      *     The data to be parsed.
      */
    void meta_read(const unsigned char *data);

    /**
      * The meta_write method is used to encode our instance variables
      * into the on-disk 26-byte representation.  Note that only the dat
      * portion of the "when" instance variable is saved on the disk.
      *
      * @param data
      *     Where to write the data.
      */
    void meta_write(unsigned char *data) const;

    /**
      * The get_current_size method is used to calculate the size in
      * bytes of the file.
      */
    long
    get_current_size()
        const
    {
        if (dfirstblock >= dlastblock)
            return 0;
        return (((long)(dlastblock - dfirstblock - 1) << 9) + dlastbyte);
    }

    /**
      * The get_current_extent_size method is used to calculate the size
      * in bytes of the extent allocated to the file.
      */
    long
    get_current_extent_size()
        const
    {
        if (dfirstblock >= dlastblock)
            return 0;
        return ((long)(dlastblock - dfirstblock) << 9);
    }

    /**
      * The default constructor.  Do not use.
      */
    directory_entry_file();

    /**
      * The copy constructor.  Do not use.
      */
    directory_entry_file(const directory_entry_file &);

    /**
      * The assignment operator.  Do not use.
      */
    directory_entry_file &operator=(const directory_entry_file &);
};

#endif // LIB_DIRECTORY_ENTRY_FILE_H
