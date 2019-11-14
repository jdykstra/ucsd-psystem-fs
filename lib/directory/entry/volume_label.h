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

#ifndef LIB_DIRECTORY_ENTRY_VOLUME_LABEL_H
#define LIB_DIRECTORY_ENTRY_VOLUME_LABEL_H

#include <lib/directory/entry.h>
#include <lib/sector_io.h>

/**
  * The directory_entry_volume_label class is used to represent the
  * meta-data for a volume label.
  */
class directory_entry_volume_label:
    public directory_entry
{
public:
    typedef boost::shared_ptr<directory_entry_volume_label> vlptr;

    /**
      * The destructor.
      */
    virtual ~directory_entry_volume_label();

private:
    /**
      * The constructor.  It is private on purpose, use the #create
      * class method instead.
      *
      * @param parent
      *     The containing directory
      * @param data
      *     The raw byte data in the disk image
      * @param deeper
      *     The raw disk data
      */
    directory_entry_volume_label(directory *parent, const unsigned char *data,
        const sector_io::pointer &deeper);

    /**
      * The constructor.  It is private on purpose, use the #create
      * class method instead.
      *
      * @param parent
      *     The containing directory
      * @param name
      *     The volume ID, the rest of the fields are folled automatically.
      * @param deeper
      *     The raw disk data
      * @param redundant_meta_data
      *     Have two copies of the directiry meta-data, not just one.
      */
    directory_entry_volume_label(directory *parent, const rcstring &name,
        const sector_io::pointer &deeper, bool redundant_meta_data = false);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param parent
      *     The containing directory
      * @param data
      *     The raw byte data in the disk image
      * @param deeper
      *     The raw disk data
      */
    static vlptr create(directory *parent, const unsigned char *data,
        const sector_io::pointer &deeper);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param parent
      *     The containing directory
      * @param name
      *     The volume ID, the rest of the fields are folled automatically.
      * @param deeper
      *     The raw disk data
      * @param redundant_meta_data
      *     Have two copies of the directiry meta-data, not just one.
      */
    static vlptr create(directory *parent, const rcstring &name,
        const sector_io::pointer &deeper, bool redundant_meta_data = false);

    /**
      * The meta_write method is used to write the instance variables
      * into the on-disk representation.
      *
      * @param data
      *     The data into which the instance variables are encoded.
      */
    void meta_write(unsigned char *data) const;

    /**
      * The set_num_files method is used to set the number of files
      * field of the volume label immediately before writing out the
      * meta-data.
      *
      * @param n
      *     The number of files.
      */
    void set_num_files(size_t n);

    /**
      * The get_num_files method is used to obtain the number of
      * files in the directory.  This is only useful when reading the
      * directory image into memory (directory::meta_read).
      */
    int get_num_files() const { return dnumfiles; }

    /**
      * The maximum_directory_entries function is used to obtain the
      * maximum number of directory entries which may be in this
      * directory.
      */
    size_t maximum_directory_entries() const { return max_dir_ents; }

    /**
      * The get_eov_block is used to obtain the number of the block
      * immediately beyond theend of the medium.  (The size in blocks of
      * the volume.)
      */
    int get_eov_block() const { return deovblk; }

    /**
      * The update_timestamp method is used to touch the "when" field
      * with the current system time.
      */
    void update_timestamp();

    // See base class for documentation.
    int getattr(struct stat *stbuf);

    // See base class for documentation.
    int utime_ns(const struct timespec *buf);

    // See base class for documentation.
    int mknod(const rcstring &name, mode_t mode, dev_t dev);

    // See base class for documentation.
    int opendir();

    // See base class for documentation.
    int get_directory_entry_names(rcstring_list &results);

    // See base class for documentation.
    int releasedir();

    // See base class for documentation.
    rcstring get_name() const;

    // See base class for documentation.
    size_t get_name_maxlen() const;

    // See base class for documentation.
    dfkind_t get_file_kind() const;

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
    size_t get_size_in_bytes(void) const;

    // See base class for documentation.
    time_t get_mtime(void) const;

private:
    /**
      * The deeper instance variable is used to remember the medium for
      * all disk I/O accesses.
      */
    sector_io::pointer deeper;

    /**
      * The dfistblock instance variable is used to remember the block
      * number of the start of the volume.
      * Blocks are 512 bytes long.
      */
    int dfirstblock;

    /**
      * The dlastblock instance variable is used to remember the block
      * number of the block beyond the contents of the directory.
      * Blocks are 512 bytes long.
      */
    int dlastblock;

    unsigned padding4;

    /**
      * The name instance variable is used to remember the name of this
      * volume.  Must be at least one byte long, must be at most 7 bytes
      * long.
      */
    rcstring name;

    /**
      * The deobblk instance variable is used to remember the block
      * number of the blok beyond the end of the volume.  Blocks are 512
      * bytes long.
      */
    int deovblk;

    /**
      * The dnumfiles instance variable is used to remember how many
      * files there are in this volume, not counting the directory itself.
      */
    int dnumfiles;

    /**
      * The dloadtime instance variable is used to remember the time the
      * volume was mounted.
      *
      * @note
      *     The documentation is not at all clear as to what this field
      *     what this should hold, on disk.
      */
    int dloadtime;

    /**
      * The when instance variable is used to remember the last time the
      * directory contents changed.
      */
    time_t when;

    unsigned padding22;
    unsigned padding24;

    /**
      * The max_dir_ents instance variable is used to remember the
      * maximum number of directory entries for this directory.
      */
    size_t max_dir_ents;

    /**
      * The meta_read method is used to read the on-disk data about the
      * directory, and load the instance variables from it.
      *
      * @param data
      *     The data to decode into the instance variables.
      */
    void meta_read(const unsigned char *data);

    /**
      * The calc_max_dir_ents is used to calculate the maximum number of
      * directory entries, once the first and last block are known.
      */
    void calc_max_dir_ents();

    /**
      * The default constructor.
      */
    directory_entry_volume_label();

    /**
      * The copy constructor.
      */
    directory_entry_volume_label(const directory_entry_volume_label &);

    /**
      * The assignment operator.
      */
    directory_entry_volume_label &operator=(
        const directory_entry_volume_label &);
};

#endif // LIB_DIRECTORY_ENTRY_VOLUME_LABEL_H
