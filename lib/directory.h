//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008, 2010, 2011 Peter Miller
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

#ifndef LIB_DIRECTORY_H
#define LIB_DIRECTORY_H

#include <lib/byte_sex.h>
#include <lib/concern.h>
#include <lib/directory/entry/list.h>
#include <lib/directory/entry/volume_label.h>
#include <lib/rcstring.h>
#include <lib/sector_io.h>

class directory_entry_list; // forward
struct statvfs; // forward


/**
  * The directory class is used to represent a UCSD p-System directory
  * on a medium.
  */
class directory
{
public:
    /**
      * The destructor.
      */
    virtual ~directory();

    /**
      * The constructor.
      *
      * @param deeper
      *     The sector I/O to use to access this directory instance.
      */
    directory(const sector_io::pointer &deeper,
        byte_sex_t byte_sex = little_endian);

    /**
      * The find method is used to locate a file within the directory.
      *
      * @param filename
      *     The name of the file to look for.
      * @returns
      *     directory entry for the file, or the NULL pointer if the
      *     file is not found.
      */
    directory_entry::pointer find(const rcstring &filename);

    /**
      * The get_word method is used to decode two bytes into a 16-bit
      * word, taking byte sex into account.
      *
      * @param data
      *     The data to be decoded into a word value.
      * @returns
      *     an unsigned 16-bit value
      */
    unsigned get_word(const unsigned char *data) const;

    /**
      * The put_word method is used to encode two bytes into a 16-bit
      * word, taking byte sex into account.
      *
      * @param data
      *     The data to be decoded into a word value.
      * @param value
      *     an unsigned 16-bit value to be encoded
      */
    void put_word(unsigned char *data, unsigned value) const;

    /**
      * The nth method is used to get the "n"th directory entry.
      *
      * @param n
      *     The numer of the entry to get.
      * @returns
      *     Pointer to directory entry of interest, or NULL at the end
      *     of the directory.
      */
    directory_entry::pointer nth(int &n) const;

    /**
      * The mkfs method is used to create the initial volume ID in the
      * otherwise empty directory for the volume.
      *
      * @param volid
      *     The name of the volume.  Defaults to a string derived from
      *     the current time of not specified.
      * @param redundant_meta_data
      *     Have two copies of the directiry meta-data, not just one.
      *
      * @note
      *     You should call exactly ONE of the meta_read or mkfs methods
      *     as soon as possible after the constructor has run.
      */
    void mkfs(const rcstring &volid = "", bool redundant_meta_data = false);

    /**
      * The meta_sync method is used to update the meta data (contained
      * in the directory) to ensure it has been written to the disk
      * medium.
      *
      * @returns
      *     zero for success, or -errno on error.
      */
    int meta_sync(void);

    /**
      * The meta_read method is used to read the volume meta-data (the
      * volume directory) from the medium and into the instance variables.
      *
      * @param concern
      *     The level of concern to display about the data integrity of
      *     the disk image.  Defaults to "blithe" meaning no checking.
      * @returns
      *     zero for success, or -errno on error.
      *
      * @note
      *     You should call exactly ONE of the meta_read or mkfs methods
      *     as soon as possible after the constructor has run.
      */
    int meta_read(concern_t concern = concern_blithe);

    /**
      * The has_room_for_new_file method is used to determine whether or
      * not there is room to add a new file to the directory.
      *
      * @returns
      *     true if there is room, fale if there is no room.
      */
    bool has_room_for_new_file(void) const;

    /**
      * The add_new_file method is used to add a new file to the
      * directory.
      *
      * You MUST check with has_room_for_new_file first.
      *
      * @param dep
      *     The directory entry to add.
      */
    int add_new_file(directory_entry::pointer dep);

    /**
      * The delete_exiting_file method is used to delete a file from the
      * volume, and release the data blocks being used.
      *
      * The "hard_remove" mount option controls behaviour when unlinking
      * files, here is the FUSE documentation:
      *
      *   "The default behavior is that if an open file is deleted, the
      *   file is renamed to a hidden file (.fuse_hiddenXXX), and only
      *   removed when the file is finally released.  This relieves
      *   the filesystem implementation of having to deal with this
      *   problem.  This option disables the hiding behavior, and files
      *   are removed immediately in an unlink operation (or in a rename
      *   operation which overwrites an existing file).
      *
      *   "It is recommended that you not use the hard_remove
      *   option. When hard_remove is set, the following libc functions
      *   fail on unlinked files (returning errno of ENOENT): read(),
      *   write(), fsync(), close(), f*xattr(), ftruncate(), fstat(),
      *   fchmod() and fchown()."
      *
      * You will probably need to invoke the meta_sync method pretty
      * soon after calling this method.
      *
      * @param dep
      *     The directory entry to be deleted.
      */
    void delete_existing_file(directory_entry *dep);

    /**
      * The delete_exiting_file method is used to delete a file from the
      * volume, and release the data blocks being used.
      *
      * @param dep
      *     The directory entry to be deleted.
      */
    void delete_existing_file(directory_entry::pointer dep);

    /**
      * The first_empty block method is used to obtain the block number
      * of the first empty block on the volume.
      *
      * @returns
      *     the block number (if the disk is full, the block number past
      *     the end of the disk).
      */
    int first_empty_block(void) const;

    /**
      * The move_gap_after method is used to shuffle the files on this
      * disk so that a gap of unused blocks is immediately after the
      * given directory entry.
      *
      * The gap will be the largest possible gap.
      *
      * @param dep
      *     The directory entry in question.
      * @returns
      *     The gap size in blocks (it could be zero), or -errno on error.
      */
    int move_gap_after(directory_entry *dep);

    /**
      * The sizeof_gap_after method is used to determine how many blocks
      * are available beyond the end of the file's allocated extent.
      *
      * @param dep
      *     The directory entry in question.
      * @returns
      *     The gap size in blocks (it could be zero), or -errno on error.
      */
    int sizeof_gap_after(directory_entry *dep);

    /**
      * Get file system statistics
      *
      * The 'f_type' and 'f_fsid' fields are ignored
      *
      * @returns
      *     zero on success, -errno on error.
      */
    int statfs(struct statvfs *st);

    /**
      * The factor class method is used to open the disk image a file
      * and figure out how to access its contents.
      *
      * @param filename
      *     The file name of the file containing a UCSD p-System
      *     filesystem disk image.
      * @param concern
      *     The level of concern to display about the data integrity of
      *     the disk image.  Defaults to "blithe" meaning no checking.
      * @returns
      *     A pointer to a dynamically allocated directory.  Use the
      *     delete operator when you are done with it.
      *
      * @note
      *     This function does not return if there are any fatal errors.
      *     It will exit via the quitter.fatal_error mechanism.
      */
    static directory *factory(const rcstring &filaname, bool read_only = false,
        concern_t concern = concern_blithe);

    enum sort_by_t
    {
        sort_by_block,
        sort_by_size,
        sort_by_date,
        sort_by_name,
        sort_by_kind,
    };

    /**
      * The print_listing method is used to print a directory listing,
      * independent of mounting the file system.  The listing is printed
      * on the standard output stream.
      *
      * @param verbose
      *     Whether or not to issue a long (verbose) listing.
      * @param sort_by
      *     What criterion to sort the entries by.
      */
    void print_listing(bool verbose = false, sort_by_t sort_by = sort_by_block);

    /**
      * The get_volume_name method is used to obtain the name of the
      * volume this directory represents.
      */
    rcstring get_volume_name(void) const;

    /**
      * The convert_text_on_the_fly method is used to enable the
      * conversion of text files between Unix and UCSD formats
      * on-the-fly.
      */
    void convert_text_on_the_fly(void) { text_on_the_fly_flag = true; }

    /**
      * The text_on_the_fly method is used to determine whether or not
      * text files are to be converted on-the-fly between Unix and UCSD
      * formats.
      */
    bool text_on_the_fly(void) const { return text_on_the_fly_flag; }

    /**
      * The crunch method is used to move all of the files towards the
      * start of the disk image, so as to maximize the gap at the end of
      * the disk.
      */
    int crunch(void);

    /**
      * The wipe method is used to write zero bytes to all blocks not
      * accounted for in the directory, wiping any "left over" content.
      * Not only is this more secure (things you didn't intent to stay
      * on this disk don't) but this disk images compress better, too.
      */
    int wipe_unused(void);

    /**
      * The set_boot_blocks method is used to over-write the boot blocks
      * of the disk image (first four 512-byte blocks) with the raw
      * binary file given.
      *
      * @param filename
      *     The name of the binary file to read.
      */
    void set_boot_blocks(const rcstring &filename);

    /**
      * The get_boot_blocks method is used to read the boot
      * blocks of the disk image (first four 512-byte blocks)
      * and write them (as binary) to the file given.
      *
      * @param filename
      *     The name of the binary file to write.
      */
    void get_boot_blocks(const rcstring &filename) const;

    /**
      * The check_for_system_files method is used to verify that at
      * least the 'system.pascal' and 'system.filer' and 'system.editor' are
      * present in the directory.
      */
    bool check_for_system_files(void);

private:
    /**
      * The deeper instance variable is used to remember the sector I/O
      * to use to access this directory instance.
      */
    sector_io::pointer deeper;

    /**
      * The byte_sex instance variable is used to remember which byte
      * sex (endian coding) this volume / disk / directory uses.
      */
    byte_sex_t byte_sex;

    /**
      * The volume label instance variable is used to remember the
      * details of the label (name, size in blocks, etc) on the volume.
      */
    directory_entry_volume_label::vlptr volume_label;

    /**
      * The files instance variable is used to remember the details of
      * each directory entry.
      */
    directory_entry_list files;

    /**
      * The calc_used_blocks method is used to calculate how many blocks
      * are in use at present.  This method is used by the statfs
      * method.
      */
    int calc_used_blocks(void) const;

    /**
      * The text_on_the_fly_flag instance variable is used to remember
      * whether or not text files are to be converted to and from Unix
      * text representation.  Defaults to false, i.e. use the binary form.
      */
    bool text_on_the_fly_flag;

    /**
      * The default constructor.
      */
    directory();

    /**
      * The copy constructor.
      */
    directory(const directory &);

    /**
      * The assignment operator.
      */
    directory &operator=(const directory &);
};

#endif // LIB_DIRECTORY_H
