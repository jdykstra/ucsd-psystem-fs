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

#ifndef LIB_DIRECTORY_ENTRY_H
#define LIB_DIRECTORY_ENTRY_H

#include <lib/config.h>
#include <ctime>
#include <sys/types.h>
#include <boost/shared_ptr.hpp>

#include <lib/concern.h>
#include <lib/endof.h>
#include <lib/rcstring.h>

class directory; // forward
struct stat; // forward
class rcstring_list; // forward

/**
  * The directory_entry virtual base class is used to represent an entry
  * in a directory.
  */
class directory_entry
{
public:
    /**
      * The pointer typedef allows the pointers to be replaced with
      * different sorts of smart pointers with one line of code.
      */
    typedef boost::shared_ptr<directory_entry> pointer;

    /**
      * The destructor.
      */
    virtual ~directory_entry();

protected:
    /**
      * The default constructor.
      *
      * @param parent
      *     The directory containing this object.
      */
    directory_entry(directory *parent);

public:
    /**
      * Get file attributes.
      *
      * Similar to the stat() system call.  The 'st_dev' and
      * 'st_blksize' fields are ignored.  The 'st_ino' field is ignored
      * except if the 'use_ino' mount option is given.
      *
      * @param stbuf
      *     Where to put the file meta-date
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int getattr(struct stat *stbuf);

    /**
      * Read the target of a symbolic link
      *
      * The buffer should be filled with a null terminated string.  The
      * buffer size argument includes the space for the terminating
      * null character.  If the linkname is too long to fit in the
      * buffer, it should be truncated.
      *
      * @param buf
      *     The buffer should be filled with a null terminated string.
      * @param size
      *     The buffer size argument includes the space for the
      *     terminating null character.  If the linkname is too long to
      *     fit in the buffer, it should be truncated.
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int readlink(char *buf, size_t size);

    /**
      * Create a file node
      *
      * There is no create() operation, mknod() will be called for
      * creation of all non-directory, non-symlink nodes.
      *
      * @param name
      *     The name of the node to create in this directory.
      *     (If invoked on a non-directory, should return -ENOTDIR).
      * @param mode
      *     The permissions mode for the file, AND the file type.
      * @param dev
      *     The device information, if a block or special device node is
      *     being created.
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int mknod(const rcstring &name, mode_t mode, dev_t dev);

    /**
      * Create a directory.
      *
      * @param name
      *     The name of the node to create in this directory.
      *     (If invoked on a non-directory, should return -ENOTDIR).
      * @param mode
      *     The permissions mode for the file, AND the file type.
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int mkdir(const rcstring &name, mode_t mode);

    /**
      * Remove a file.
      *
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int unlink(void);

    /**
      * Remove a directory.
      */
    virtual int rmdir(void);

    /**
      * Create a symbolic link.
      *
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int symlink(const rcstring &name, const char *value);

    /**
      * Rename a file.
      *
      * @param new_name
      *     The new name for the file.
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int rename(const char *new_name);

    /**
      * Create a hard link to a file.
      *
      * @param name
      *     The name to be created in the directory.
      *     (Return -ENOTDIR if this isn't a directory.)
      * @param other
      *     The existing file being linked to this new name.
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int link(const rcstring &name, directory_entry::pointer other);

    /**
      * Change the permission bits of a file.
      *
      * @param mode
      *     The mode to change the file to (low 12 bits only).
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int chmod(mode_t mode);

    /**
      * Change the owner and group of a file.
      *
      * @param uid
      *     The user ID for the new ownership.
      * @param gid
      *     The group ID for the new ownership.
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int chown(uid_t uid, gid_t gid);

    /**
      * The truncate method is used to change the length of a file.
      *
      * @param size
      *     The new size of the file.
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int truncate(off_t size);

    /**
      * Change the access and/or modification times of a file.
      *
      * @param buf
      *     The data to set the times from.
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int utime_ns(const struct timespec *buf);

    /**
      * File open operation
      *
      * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
      * will be passed to open().  Open should check if the operation
      * is permitted for the given flags.  Optionally open may also
      * return an arbitrary filehandle in the fuse_file_info structure,
      * which will be passed to all file operations.
      *
      * @returns
      *     zero for success, or -errno on error.
      */
    virtual int open(void);

    /**
      * Read data from an open file
      *
      * Read should return exactly the number of bytes requested
      * except on EOF or error, otherwise the rest of the data will
      * be substituted with zeroes.  An exception to this is when the
      * 'direct_io' mount option is specified, in which case the return
      * value of the read system call will reflect the return value of
      * this operation.
      *
      * @param offset
      *     How far into the file to start reading.
      * @param data
      *     Where to place the data read from the file.
      * @param nbytes
      *     How much data to read from the file.
      * @returns
      *     0 on EOF, positive when data read successfully, -errno on error.
      */
    virtual int read(off_t offset, void *data, size_t nbytes) const;

    /**
      * Write data to an open file
      *
      * Write should return exactly the number of bytes requested except
      * on error.  An exception to this is when the 'direct_io' mount
      * option is specified (see read operation).
      *
      * @param offset
      *     How far into the file to start reading.
      * @param data
      *     Where to obtain the data to write to the file.
      * @param nbytes
      *     How much data to write to the file.
      * @returns
      *     0 on EOF, positive when data wrote successfully, -errno on error.
      */
    virtual int write(off_t offset, const void *data, size_t nbytes);

    /**
      * Get file system statistics
      *
      * The 'f_type' and 'f_fsid' fields are ignored
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int statfs(struct statvfs *);

    /**
      * Possibly flush cached data
      *
      * BIG NOTE: This is not equivalent to fsync().  It's not a
      * request to sync dirty data.
      *
      * Flush is called on each close() of a file descriptor.  So if a
      * filesystem wants to return write errors in close() and the file
      * has cached dirty data, this is a good place to write back data
      * and return any errors.  Since many applications ignore close()
      * errors this is not always useful.
      *
      * NOTE: The flush() method may be called more than once for each
      * open().  This happens if more than one file descriptor refers
      * to an opened file due to dup(), dup2() or fork() calls.  It is
      * not possible to determine if a flush is final, so each flush
      * should be treated equally.  Multiple write-flush sequences are
      * relatively rare, so this shouldn't be a problem.
      *
      * Filesystems shouldn't assume that flush will always be called
      * after some writes, or that if will be called at all.
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int flush(void);

    /**
      * Release an open file
      *
      * Release is called when there are no more references to an open
      * file: all file descriptors are closed and all memory mappings
      * are unmapped.
      *
      * For every open() call there will be exactly one release() call
      * with the same flags and file descriptor.  It is possible to
      * have a file opened more than once, in which case only the last
      * release will mean, that no more reads/writes will happen on the
      * file.  The return value of release is ignored.
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int release(void);

    /**
      * Synchronize file contents
      *
      * If the datasync parameter is non-zero, then only the user data
      * should be flushed, not the meta data.
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int fsync(int);

    /**
      * Set extended attributes.
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int setxattr(const char *, const char *, size_t, int);

    /**
      * Get extended attributes.
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int getxattr(const char *, char *, size_t);

    /**
      * List extended attributes.
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int listxattr(char *, size_t);

    /**
      * Remove extended attributes.
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int removexattr(const char *);

    /**
      * Open directory
      *
      * This method should check if the open operation is permitted for
      * this  directory
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int opendir(void);

    /**
      * The get_directory_entry_names method is used to obtain the names
      * of the directory entries.
      *
      * @param results
      *     The names of the directory entries are returned in this list.
      * @returns
      *     zero on success, or -errno on error
      */
    virtual int get_directory_entry_names(rcstring_list &results);

    /**
      * Release directory
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int releasedir(void);

    /**
      * Synchronize directory contents
      *
      * If the datasync parameter is non-zero, then only the user data
      * should be flushed, not the meta data
      *
      * @returns
      *     zero on success, -errno on error.
      */
    virtual int fsyncdir(int);

    enum dfkind_t
    {
        untypedfile,
        xdskfile,
        codefile,
        textfile,
        infofile,
        datafile,
        graffile,
        fotofile,
        securedir
    };

    /**
      * The dfkind_name class method is used to translate a numeric
      * dfkind value into the equivalent string.
      *
      * @param dfkind
      *     The value to be stranslated.
      * @returns
      *     a string equivalent of the value
      */
    static const char *dfkind_name(dfkind_t dfkind);

    /**
      * The dfkind_from_extension class method is used to map a file name
      * extension to a dfkind.
      *
      * @param name
      *     The file name to look at the extension.
      * @returns
      *     an approximate kind, or datafile by default.
      */
    static dfkind_t dfkind_from_extension(const rcstring &name);

    /**
      * The get_file_kind method is used to determine the kind of the
      * directory.
      */
    virtual dfkind_t get_file_kind(void) const = 0;

    /**
      * The is_text_kind is used to determine whether or not this
      * directory entry is a text file.
      */
    bool is_text_kind(void) const { return (textfile == get_file_kind()); }

    /**
      * The get_name method is used to obtain the name of this directory entry.
      */
    virtual rcstring get_name(void) const = 0;

    /**
      * The get_full_name method is used to obtain the fully qualified
      * name of the file including the volume name.  This is useful for
      * error messages.
      */
    rcstring get_full_name(void) const;

    /**
      * The get_name_maxlen method is used to obtain the maximum number
      * of characters in the name of a directory entry.
      */
    virtual size_t get_name_maxlen(void) const = 0;

    /**
      * The get_size_in_bytes method may be used to obtain the size of
      * the file, in bytes.
      */
    virtual size_t get_size_in_bytes(void) const = 0;

    /**
      * The meta_write method is used to encode the meta data for this
      * directory entry into the on-disk format.
      *
      * @param data
      *     Where to write the on-disk data.
      */
    virtual void meta_write(unsigned char *data) const = 0;

    /**
      * The get_first_block method is used to obtain the block number of
      * the first block of this file.
      *
      * @returns
      *     block number
      */
    virtual int get_first_block(void) const = 0;

    /**
      * The get_last_block method is used to obtain the number of the
      * block beyond the end of this file.
      *
      * (last_block - first_block) = number of blocks in the file.
      *
      * @returns
      *     block number
      */
    virtual int get_last_block(void) const = 0;

    /**
      * The size_in_blcoks method is used to obtain the number of blocks
      * used by the file.
      */
    int
    size_in_blocks(void)
        const
    {
        return get_last_block() - get_first_block();
    }

    /**
      * The relocate method is used to move the contents of a disk file
      * to another location on the medium.
      *
      * @param to_block
      *     The new start-of-file block
      * @returns
      *     zero on success, or -errno on error
      */
    virtual int relocate(unsigned to_block) = 0;

    /**
      * The fsck method is used to perform file system consistency
      * checks on this directory entry.
      *
      * @param concern_level
      *     How much concern to exhibit.
      * @returns
      *     zero if no errors found, negative errno if a system level
      *     error occurred, the problem count if problems wer encounted
      *     (and possibly fixed).
      */
    virtual int fsck(concern_t concern_level) = 0;

    /**
      * The fsck_first_block is used by directory::fsck to adjust the
      * first block location of the file IF it finds a problem.  No
      * other code should call this method.
      *
      * @param blknum
      *     The block number to be set.
      */
    virtual void fsck_first_block(int blknum) = 0;

    /**
      * The fsck_last_block is used by directory::fsck to adjust the
      * last block location of the file IF it finds a problem.  No
      * other code should call this method.
      *
      * @param blknum
      *     The block number to be set.
      */
    virtual void fsck_last_block(int blknum) = 0;

    /**
      * The print_listing method is used to print a directory listing,
      * independent of mounting the file system.  The listing is printed
      * on the standard output stream.
      */
    virtual void print_listing(bool verbose = false) = 0;

    virtual time_t get_mtime(void) const = 0;

protected:
    /**
      * The get_word method is used to translate two bytes into an
      * unsigned value, taking endian-ness into account.
      *
      * @param data
      *     The bytes to be translated.
      * @returns
      *     The unsigend 16-bit value.
      */
    unsigned get_word(const unsigned char *data) const;

    /**
      * The get_date method is used to translate two bytes into a
      * date-time value.
      *
      * @param data
      *     The bytes to be translated.
      * @returns
      *     The 32-bit time value.
      */
    time_t get_date(const unsigned char *data) const;

    /**
      * The put_word method is used to translate an unsignmed value into
      * two bytes, taking endian-ness into account.
      *
      * @param data
      *     The bytes once translated.
      * @param value
      *     The unsigend 16-bit value.
      */
    void put_word(unsigned char *data, unsigned) const;

    /**
      * The put_date method is used to translate a date-time valu into
      * two bytes.
      *
      * @param data
      *     The bytes once translated.
      * @param value
      *     The 32-bit time value.
      */
    void put_date(unsigned char *data, time_t) const;

    /**
      * The get_parent method is used to obtain the containing directory
      * of this directory entry.
      */
    directory *get_parent(void) const { return parent; }

private:
    /**
      * The parent instance variable is used to remember the containing
      * directory for this directory entry.
      */
    directory *parent;

    /**
      * The copy constructor.  Do no tuse.
      */
    directory_entry(const directory_entry &);

    /**
      * The assignment operator.  Do no tuse.
      */
    directory_entry &operator=(const directory_entry &);
};

#endif // LIB_DIRECTORY_ENTRY_H
