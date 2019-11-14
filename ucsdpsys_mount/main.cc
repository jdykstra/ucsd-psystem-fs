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

#include <lib/config.h>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <unistd.h>

#include <lib/debug.h>
#include <lib/directory.h>
#include <lib/directory/entry.h>
#include <lib/fuse.h>
#include <lib/hexdump.h>
#include <lib/rcstring/list.h>
#include <lib/sector_io/raw.h>
#include <lib/version.h>


static directory *volume;


static int
getattr_callback(const char *path, struct stat *stbuf)
{
    DEBUG(1, "getattr(path = \"%s\", stbuf = %p)", path, stbuf);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->getattr(stbuf);
}


/**
  * Read the target of a symbolic link
  *
  * The buffer should be filled with a null terminated string.  The
  * buffer size argument includes the space for the terminating
  * null character.  If the linkname is too long to fit in the
  * buffer, it should be truncated.  The return value should be 0
  * for success.
  */
static int
readlink_callback(const char *path, char *buf, size_t size)
{
    DEBUG(1, "readlink(path = \"%s\", buf = %p, size = %d)", path, buf,
        (int)size);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->readlink(buf, size);
}


/**
  * Create a file node
  *
  * This is called for creation of all non-directory, non-symlink
  * nodes.  If the filesystem defines a create() method, then for
  * regular files that will be called instead.
  */
static int
mknod_callback(const char *s, mode_t mode, dev_t dev)
{
    DEBUG(1, "mknod(path = \"%s\", mode = 0%o, dev = %d)", s, (int)mode,
        (int)dev);
    rcstring path(s);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (dep)
        return -EEXIST;
    dep = volume->find(path.dirname());
    if (!dep)
        return -ENOENT;
    return dep->mknod(path.basename(), mode, dev);
}


static int
mkdir_callback(const char *s, mode_t mode)
{
    DEBUG(1, "mkdir(s = \"%s\", mode = 0%o)", s, mode);
    rcstring path(s);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (dep)
        return -EEXIST;
    dep = volume->find(path.dirname());
    if (!dep)
        return -ENOENT;
    return dep->mkdir(path.basename(), mode);
}


static int
unlink_callback(const char *path)
{
    DEBUG(1, "unlink(path = \"%s\")", path);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->unlink();
}


static int
rmdir_callback(const char *path)
{
    DEBUG(1, "rmdir(path = \"%s\")", path);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->rmdir();
}


static int
symlink_callback(const char *lhs, const char *rhs)
{
    DEBUG(1, "symlink(lhs = \"%s\", rhs = \"%s\")", lhs, rhs);
    rcstring path(lhs);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (dep)
        return -EEXIST;
    dep = volume->find(path.dirname());
    if (!dep)
        return -ENOENT;
    return dep->symlink(path.basename(), rhs);
}


static int
rename_callback(const char *lhs, const char *rhs)
{
    DEBUG(1, "rename(lhs = \"%s\", rhs = \"%s\")", lhs, rhs);
    assert(volume);
    directory_entry::pointer dep = volume->find(lhs);
    if (!dep)
        return -ENOENT;
    // FIXME: this isn't right, if moving between directories.
    return dep->rename(rhs);
}


static int
link_callback(const char *lhs, const char *rhs)
{
    DEBUG(1, "link(lhs = \"%s\", rhs = \"%s\")", lhs, rhs);
    rcstring path(lhs);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (dep)
        return -EEXIST;
    dep = volume->find(path.dirname());
    if (!dep)
        return -ENOENT;
    directory_entry::pointer rhs_dep = volume->find(rhs);
    if (!rhs)
        return -ENOENT;
    return dep->link(path.basename(), rhs_dep);
}


static int
chmod_callback(const char *path, mode_t mode)
{
    DEBUG(1, "chmod(path = \"%s\", mode = 0%o)", path, mode);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->chmod(mode);
}


static int
chown_callback(const char *path, uid_t uid, gid_t gid)
{
    DEBUG(1, "chown(path = \"%s\", uid = %d, gid = %d)", path, uid, gid);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->chown(uid, gid);
}


static int
truncate_callback(const char *path, off_t size)
{
    DEBUG(1, "truncate(path = \"%s\", size = %ld)", path, (long)size);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->truncate(size);
}


static int
utimens_callback(const char *path, const struct timespec *tv)
{
    DEBUG(1, "utimens(path = \"%s\", tv = %p)", path, tv);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->utime_ns(tv);
}


/**
  * File open operation
  *
  * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC) will be
  * passed to open().  Open should check if the operation is permitted
  * for the given flags.  Optionally open may also return an arbitrary
  * filehandle in the fuse_file_info structure, which will be passed to
  * all file operations.
  */
static int
open_callback(const char *path, fuse_file_info *fi)
{
    DEBUG(1, "open(path = \"%s\", fi = %p)", path, fi);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    int err = dep->open();
    if (err < 0)
        return err;
    fi->fh = (unsigned long)new directory_entry::pointer(dep);
    return 0;
}


/**
  * Read data from an open file
  *
  * Read should return exactly the number of bytes requested except
  * on EOF or error, otherwise the rest of the data will be
  * substituted with zeroes.      An exception to this is when the
  * 'direct_io' mount option is specified, in which case the return
  * value of the read system call will reflect the return value of
  * this operation.
  */
static int
read_callback(const char *path, char *buf, size_t size, off_t offset,
    fuse_file_info *fi)
{
    DEBUG(1, "read(path = \"%s\", buf = %p, size = %ld, offset = %ld, fi = %p)",
        path, buf, (long)size, (long)offset, fi);
    directory_entry::pointer *depp = (directory_entry::pointer *)fi->fh;
    directory_entry::pointer dep(depp ? *depp : directory_entry::pointer());
    if (!dep)
    {
        assert(volume);
        dep = volume->find(path);
        if (dep)
        {
            delete depp;
            fi->fh = (long)new directory_entry::pointer(dep);
        }
    }
    if (!dep)
        return -ENOENT;
    return dep->read(offset, buf, size);
}


/**
  * Write data to an open file
  *
  * Write should return exactly the number of bytes requested
  * except on error.  An exception to this is when the 'direct_io' mount
  * option is specified (see read operation).
  */
static int
write_callback(const char *path, const char *buf, size_t size, off_t offset,
    fuse_file_info *fi)
{
    DEBUG(1, "write(path = \"%s\", buf = %p, size = %ld, offset = %ld, "
        "fi = %p)", path, buf, (long)size, (long)offset, fi);
    directory_entry::pointer *depp = (directory_entry::pointer *)fi->fh;
    directory_entry::pointer dep(depp ? *depp : directory_entry::pointer());
    if (!dep)
    {
        assert(volume);
        dep = volume->find(path);
        if (dep)
        {
            delete depp;
            fi->fh = (long)new directory_entry::pointer(dep);
        }
    }
    if (!dep)
        return -ENOENT;
     int err = dep->write(offset, buf, size);
     if (err < 0)
         return err;
     return size;
}


/**
  * Get file system statistics
  *
  * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
  *
  * Replaced 'struct statfs' parameter with 'struct statvfs' in
  * FUSE version 2.5
  */
static int
statfs_callback(const char *path, struct statvfs *buf)
{
    DEBUG(1, "statfs(path = \"%s\", buf = %p)", path, buf);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->statfs(buf);
}


//
// Possibly flush cached data
//
// BIG NOTE: This is not equivalent to fsync().  It's not a request to
// sync dirty data.
//
// Flush is called on each close() of a file descriptor.  So if a
// filesystem wants to return write errors in close() and the file has
// cached dirty data, this is a good place to write back data and return
// any errors.  Since many applications ignore close() errors this is
// not always useful.
//
// NOTE: The flush() method may be called more than once for each
// open().  This happens if more than one file descriptor refers to an
// opened file due to dup(), dup2() or fork() calls.  It is not possible
// to determine if a flush is final, so each flush should be treated
// equally.  Multiple write-flush sequences are relatively rare, so this
// shouldn't be a problem.
//
// Filesystems shouldn't assume that flush will always be called after
// some writes, or that if will be called at all.
//
static int
flush_callback(const char *path, fuse_file_info *fi)
{
    DEBUG(1, "flush(path = \"%s\", fi = %p)", path, fi);
    directory_entry::pointer *depp = (directory_entry::pointer *)fi->fh;
    directory_entry::pointer dep(depp? *depp : directory_entry::pointer());
    if (!dep)
    {
        assert(volume);
        dep = volume->find(path);
    }
    if (!dep)
        return -ENOENT;
    return dep->flush();
}


//
// Release an open file
//
// Release is called when there are no more references to an open file:
// all file descriptors are closed and all memory mappings are unmapped.
//
// For every open() call there will be exactly one release() call with
// the same flags and file descriptor.  It is possible to have a file
// opened more than once, in which case only the last release will mean,
// that no more reads/writes will happen on the file.  The return value
// of release is ignored.
//
static int
release_callback(const char *path, fuse_file_info *fi)
{
    DEBUG(1, "release(path = \"%s\", fi = %p)", path, fi);
    directory_entry::pointer *depp = (directory_entry::pointer *)fi->fh;
    directory_entry::pointer dep(depp ? *depp : directory_entry::pointer());
    delete depp;
    fi->fh = 0;
    if (!dep)
    {
        assert(volume);
        dep = volume->find(path);
    }
    if (!dep)
        return -ENOENT;
    return dep->release();
}


/**
  * Synchronize file contents
  *
  * If the datasync parameter is non-zero, then only the user data
  * should be flushed, not the meta data.
  */
static int
fsync_callback(const char *path, int arg, fuse_file_info *fi)
{
    DEBUG(1, "fsync(path = \"%s\", arg = %d, fi = %p)", path, arg, fi);
    directory_entry::pointer *depp = (directory_entry::pointer *)fi->fh;
    directory_entry::pointer dep(depp ? *depp : directory_entry::pointer());
    if (!dep)
    {
        assert(volume);
        dep = volume->find(path);
        if (dep)
        {
            delete depp;
            fi->fh = (long)new directory_entry::pointer(dep);
        }
    }
    if (!dep)
        return -ENOENT;
    return dep->fsync(arg);
}


static int
setxattr_callback(const char *path, const char *name, const char *value,
    size_t size, int arg)
{
    DEBUG(1, "setxattr(path = \"%s\", name = \"%s\", value = %s, size = %ld, "
        "arg = %d)", path, name, rcstring(value, size).quote_c().c_str(),
        (long)size, arg);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->setxattr(name, value, size, arg);
}


static int
getxattr_callback(const char *path, const char *name, char *value, size_t size)
{
    DEBUG(1, "getxattr(path = \"%s\", name = \"%s\", value = %p, size = %ld)",
        path, name, value, (long)size);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->getxattr(name, value, size);
}


static int
listxattr_callback(const char *path, char *buf, size_t size)
{
    DEBUG(1, "listxattr(path = \"%s\", buf = %p, size = %ld)", path, buf,
        (long)size);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->listxattr(buf, size);
}


static int
removexattr_callback(const char *path, const char *name)
{
    DEBUG(1, "removexattr(path = \"%s\", name = \"%s\")", path, name);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    return dep->removexattr(name);
}


/**
  * Open directory
  *
  * This method should check if the open operation is permitted for
  * this  directory
  */
static int
opendir_callback(const char *path, fuse_file_info *fi)
{
    DEBUG(1, "opendir(path = \"%s\", fi = %p)", path, fi);
    assert(volume);
    directory_entry::pointer dep = volume->find(path);
    if (!dep)
        return -ENOENT;
    fi->fh = (long)new directory_entry::pointer(dep);
    return dep->opendir();
}


/**
  * Read directory
  *
  * This supersedes the old getdir() interface.  New applications
  * should use this.
  *
  * The filesystem may choose between two modes of operation:
  *
  * 1) The readdir implementation ignores the offset parameter,
  * and passes zero to the filler function's offset.  The filler
  * function will not return '1' (unless an error happens), so the
  * whole directory is read in a single readdir operation.  This
  * works just like the old getdir() method.
  *
  * 2) The readdir implementation keeps track of the offsets of the
  * directory entries.  It uses the offset parameter and always
  * passes non-zero offset to the filler function.  When the buffer
  * is full (or an error happens) the filler function will return
  * '1'.
  *
  * @returns
  *     zero on success, -errno on error.
  */
static int
readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t offset, fuse_file_info *fi)
{
    DEBUG(1, "readdir(path = \"%s\", buf = %p, filler = %p, offset = %ld, "
        "fi = %p)", path, buf, filler, (long)offset, fi);
    directory_entry::pointer *depp = (directory_entry::pointer *)fi->fh;
    directory_entry::pointer dep(depp ? *depp : directory_entry::pointer());
    if (!dep)
    {
        assert(volume);
        dep = volume->find(path);
        if (dep)
        {
            delete depp;
            fi->fh = (long)new directory_entry::pointer(dep);
        }
    }
    if (!dep)
        return -ENOENT;
    rcstring_list names;
    int err = dep->get_directory_entry_names(names);
    if (err < 0)
        return err;

    struct stat st;
    memset(&st, 0, sizeof(st));
    filler(buf, ".", &st, 0);
    filler(buf, "..", &st, 0);
    for (size_t j = 0; j < names.size(); ++j)
    {
        if (filler(buf, names[j].c_str(), &st, 0))
            break;
    }
    return 0;
}


static int
releasedir_callback(const char *path, fuse_file_info *fi)
{
    DEBUG(1, "releasedir(path = \"%s\", fi = %p)", path, fi);
    directory_entry::pointer *depp = (directory_entry::pointer *)fi->fh;
    directory_entry::pointer dep(depp ? *depp : directory_entry::pointer());
    delete depp;
    fi->fh = 0;
    if (!dep)
    {
        assert(volume);
        dep = volume->find(path);
    }
    if (!dep)
        return -ENOENT;
    return dep->releasedir();
}


/**
  * Synchronize directory contents
  *
  * If the datasync parameter is non-zero, then only the user data
  * should be flushed, not the meta data
  */
static int
fsyncdir_callback(const char *path, int arg, fuse_file_info *fi)
{
    DEBUG(1, "fsyncdir(path = \"%s\", arg = %d, fi = %p)", path, arg, fi);
    directory_entry::pointer *depp = (directory_entry::pointer *)fi->fh;
    directory_entry::pointer dep(depp ? *depp : directory_entry::pointer());
    if (!dep)
    {
        assert(volume);
        dep = volume->find(path);
    }
    if (!dep)
        return -ENOENT;
    return dep->fsyncdir(arg);
}


static fuse_operations ops =
{
    getattr_callback,
    readlink_callback,
    0, // getdir, deprecated
    mknod_callback,
    mkdir_callback,
    unlink_callback,
    rmdir_callback,
    symlink_callback,
    rename_callback,
    link_callback,
    chmod_callback,
    chown_callback,
    truncate_callback,
    0, // utime, deprecated
    open_callback,
    read_callback,
    write_callback,
    statfs_callback,
    flush_callback,
    release_callback,
    fsync_callback,
    setxattr_callback,
    getxattr_callback,
    listxattr_callback,
    removexattr_callback,
    opendir_callback,
    readdir_callback,
    releasedir_callback,
    fsyncdir_callback,
    0, // init
    0, // destroy
    0, // access
    0, // create
    0, // ftruncate
    0, // fgetattr
    0, // lock
    utimens_callback,
    0, // bmap
};


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <volume> <dir>\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);

    //
    // Parse the command line options.
    //
    rcstring_list subset;
    rcstring_list mount_options;
    subset.push_back(explain_program_name_get());
    bool read_only_flag = false;
    bool text_on_the_fly = false;
    bool foreground = false;
    for (;;)
    {
        static const struct option options[] =
        {
            { "debug", 0, 0, 'D' },
            { "fuse-debug", 0, 0, 'd' },
            { "foreground", 0, 0, 'f' },
            { "help", 0, 0, 'h' },
            { "options", 1, 0, 'o' },
            { "read-only", 0, 0, 'r' },
            { "text", 0, 0, 't' },
            { "version", 0, 0, 'V' },
            { 0, 0, 0, 0 }
        };
        int c = getopt_long(argc, argv, "Ddfho:rtV", options, 0);
        if (c < 0)
            break;
        switch (c)
        {
        case 'D':
            ++debug_level;
            break;

        case 'd':
            // debug
            subset.push_back("-d");
            break;

        case 'f':
            // foreground
            foreground = true;
            subset.push_back("-f");
            break;

        case 'h':
            // help - this is for compatibility with fusermount
            version_print();
            return 0;

        case 'o':
            // mount option
            mount_options.split(optarg, ",");
            break;

        case 'r':
            // read only
            subset.push_back("-r");
            read_only_flag = true;
            break;

        case 't':
            text_on_the_fly = true;
            break;

        case 'V':
            version_print();
            return 0;

        default:
            usage();
        }
    }
    if (optind + 2 != argc)
        usage();
    const char *filename = argv[optind];
    const char *mount_point = argv[optind + 1];

    //
    // Look in the mount options to see if there is a "ro" option, it
    // means the same as the -r option.
    //
    if (mount_options.member("ro"))
    {
        mount_options.remove("ro");
        if (!read_only_flag)
        {
            subset.push_back("-r");
            read_only_flag = true;
        }
    }

    //
    // Look in the mount options to see if there is a umask=NNN option.
    // If not, insert one based on the current process' umask.
    //
    bool have_umask = false;
    for (size_t j = 0; j < mount_options.size(); ++j)
    {
        if (0 == memcmp(mount_options[j].c_str(), "umask=", 6))
        {
            have_umask = true;
            break;
        }
    }
    if (!have_umask)
    {
        mode_t um = umask(0);
        umask(um);
        rcstring s = rcstring::printf("umask=%05o", int(um));
        mount_options.push_back(s);
    }

    //
    // Add the options to the command line subset to be passed to fusermount
    //
    subset.push_back("-o" + mount_options.unsplit(","));

    //
    // Always run single threaded, that we we don't need locks.  It's
    // a pretty small and simple file system, mainly intended for
    // retrieving historical files, so performance is unlikely to matter
    // much.
    //
    subset.push_back("-s");

    //
    // Add the mount point as the last command line option
    // to the fusermount call.
    //
    subset.push_back(mount_point);

    //
    // Open the volume, and make sure it has the right format.
    //
    volume = directory::factory(filename, read_only_flag);
    if (text_on_the_fly)
        volume->convert_text_on_the_fly();

    //
    // Send any future error messages to syslog
    //
    if (!foreground)
        explain_output_register(explain_output_syslog_new());

    //
    // Mount the file systems and start the fuse loop running to handle
    // requests from the kernel.
    //
    char **av = new char * [subset.size() + 1];
    for (size_t j = 0; j < subset.size(); ++j)
        av[j] = (char *)subset[j].c_str();
    av[subset.size()] = 0;
    fuse_main(subset.size(), av, &ops, 0);

    //
    // Close down the volume.
    // This may do essential flush operations.
    //
    delete volume;
    volume = 0;

    //
    // Report success.
    //
    return 0;
}
