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
#include <libexplain/output.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lib/debug.h>
#include <lib/directory.h>
#include <lib/directory/entry/file.h>
#include <lib/directory/entry/file/text.h>
#include <lib/pretty_size.h>


directory_entry_file::~directory_entry_file()
{
}


directory_entry_file::directory_entry_file(
    directory *a_parent,
    const rcstring &a_name,
    dfkind_t a_dfkind,
    int a_block,
    int a_num_blocks,
    const sector_io::pointer &a_deeper
) :
    directory_entry(a_parent),
    deeper(a_deeper),
    dfirstblock(a_block),
    dlastblock(a_block + a_num_blocks),
    dfkind(a_dfkind),
    status(false),
    name(a_name.substring(0, 15)),
    dlastbyte(512),
    when(time(0))
{
}


directory_entry_file::directory_entry_file(
    directory *a_parent,
    const unsigned char *a_data,
    const sector_io::pointer &a_deeper
) :
    directory_entry(a_parent),
    deeper(a_deeper),
    dfirstblock(0),
    dlastblock(0),
    dfkind(untypedfile),
    status(false),
    dlastbyte(512),
    when(0)
{
    meta_read(a_data);
}


directory_entry::pointer
directory_entry_file::create(directory *a_parent, const unsigned char *a_data,
    const sector_io::pointer &a_deeper)
{
    // This is where to do it if you want to create different class
    // instances by filetype.
    if
    (
        a_parent->text_on_the_fly()
    &&
        (
            // little endian
            dfkind_t(a_data[4] & 7) == textfile
        ||
            // big endian
            dfkind_t(a_data[5] & 7) == textfile
        )
    )
        return directory_entry_file_text::create(a_parent, a_data, a_deeper);

    return pointer(new directory_entry_file(a_parent, a_data, a_deeper));
}


directory_entry::pointer
directory_entry_file::create(directory *a_parent, const rcstring &a_name,
    dfkind_t a_dfkind, int a_block, int a_num_blocks,
    const sector_io::pointer &a_deeper)
{
    //
    // It is possible for text files to be translated on-the-fly.
    // We do it with a different class of directory entry.
    //
    if (a_parent->text_on_the_fly() && a_dfkind == textfile)
    {
        return
            directory_entry_file_text::create
            (
                a_parent,
                a_name,
                a_block,
                a_num_blocks,
                a_deeper
            );
    }

    return
        pointer
        (
            new directory_entry_file
            (
                a_parent,
                a_name,
                a_dfkind,
                a_block,
                a_num_blocks,
                a_deeper
            )
        );
}


void
directory_entry_file::meta_read(const unsigned char *data)
{
    //
    // The data layout for simple files is...
    //
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //  0 |            dfirstblock (1st physical disk address)            |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //  2 |       dlastblock (points to block after last used block)      |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //  4 |Sta|                    Ignore                     |   dfkind  |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //  6 |            tidlen             |                               |
    //    +---+---+---+---+---+---+---+---+--                           --+
    //  8 |                                                               |
    //    +--                tid (Name of file, 15 bytes)               --+
    // 10 |                                                               |
    //    +--                                                           --+
    // 12 |                                                               |
    //    +--                                                           --+
    // 14 |                                                               |
    //    +--                                                           --+
    // 16 |                                                               |
    //    +--                                                           --+
    // 18 |                                                               |
    //    +--                                                           --+
    // 20 |                                                               |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // 22 |        Ignore         |               dlastbyte               |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // 24 |        Year               |     Month     |        Day        |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //
    dfirstblock = get_word(data);
    dlastblock = get_word(data + 2);
    padding4 = get_word(data + 4);
    dfkind = dfkind_t(padding4 & 7);
    status = (padding4 >> 15) & 1;
    name = rcstring((const char *)(data + 7), data[6]);
    name = name.replace("/", "_");
    padding22 = get_word(data + 22);
    dlastbyte = padding22 & 0x03FF;
    when = get_date(data + 24);
}


void
directory_entry_file::meta_write(unsigned char *data)
    const
{
    put_word(data, dfirstblock);
    put_word(data + 2, dlastblock);
    put_word(data + 4, (unsigned)dfkind + ((unsigned)status << 15));
    data[6] = name.size();
    memcpy(data + 7, name.c_str(), name.size());
    put_word(data + 22, dlastbyte);
    put_date(data + 24, when);
}


rcstring
directory_entry_file::get_name(void)
    const
{
    return name;
}


size_t
directory_entry_file::get_name_maxlen(void)
    const
{
    return 15;
}


int
directory_entry_file::getattr(struct stat *stbuf)
{
    stbuf->st_dev = 0;
    stbuf->st_rdev = 0;
    stbuf->st_mode = S_IFREG | (deeper->is_read_only() ? 0444 : 0666);
    stbuf->st_nlink = 1;
    stbuf->st_size = get_current_size();
    stbuf->st_blocks = dlastblock - dfirstblock;
    stbuf->st_ino = (long)(void *)this;
    stbuf->st_blksize = 512;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = when;
    stbuf->st_mtime = when;
    stbuf->st_ctime = when;
    return 0;
}


int
directory_entry_file::unlink(void)
{
    if (deeper->is_read_only())
        return -EROFS;
    get_parent()->delete_existing_file(this);
    return get_parent()->meta_sync();
}


int
directory_entry_file::rename(const char *new_name)
{
    if (deeper->is_read_only())
        return -EROFS;
    if (*new_name == '/')
        ++new_name;
    if (strchr(new_name, '/'))
        return -EINVAL;
    directory_entry::pointer old = get_parent()->find(new_name);
    if (old)
    {
        if (old.get() == this)
            return -EINVAL;
        get_parent()->delete_existing_file(old);
    }
    name = rcstring(new_name).substring(0, 15);
    return get_parent()->meta_sync();
}


int
directory_entry_file::chmod(mode_t mode)
{
    if (deeper->is_read_only())
        return -EROFS;
    if ((int)(mode & 07777) == 00666)
        return 0;
    return -EINVAL;
}


int
directory_entry_file::chown(uid_t uid, gid_t gid)
{
    if (deeper->is_read_only())
        return -EROFS;
    if
    (
        (uid == uid_t(-1) || uid == getuid())
    &&
        (gid == gid_t(-1) || gid == getgid())
    )
        return 0;
    return -EINVAL;
}


int
directory_entry_file::open(void)
{
    return 0;
}


int
directory_entry_file::truncate(off_t size)
{
    if (size < 0)
        return -EINVAL;
    if (deeper->is_read_only())
        return -EROFS;

    //
    // Deal with the cases where we are growing the file extent.
    //
    // First, look to see if the gap after this file is sufficient for
    // this new file size, and use the existing gap if at all possible.
    // This skips expensive reads and writes to the disk relocating the
    // files to make a gap that we don't need.
    //
    int gap_size = get_parent()->sizeof_gap_after(this);
    if (gap_size < 0)
        return gap_size;
    if (size > (off_t(dlastblock + gap_size - dfirstblock) << 9))
    {
        //
        // Ask parent to make a gap for us to write into, immediately
        // after our present extent.
        //
        // NOTE: when this returns, our dfirstblock and dlastblock may
        // have changed.
        //
        gap_size = get_parent()->move_gap_after(this);
        if (gap_size < 0)
            return gap_size;

        //
        // Make sure the write can succeed before we actually write anything
        // to the medium.
        //
        if (size > ((off_t)(dlastblock + gap_size - dfirstblock) << 9))
            return -ENOSPC;
    }

    //
    // If necessary, write zero to pad the file to the stated size.
    //
    long cur_size = get_current_size();
    if (size > (off_t)cur_size)
    {
        off_t pos = ((off_t)dfirstblock << 9) + cur_size;
        int err = deeper->write_zero(pos, size - cur_size);
        if (err < 0)
            return err;
    }

    //
    // Adjust the file length.
    //
    dlastblock = dfirstblock + ((size + 511) >> 9);
    dlastbyte = size & 511;
    if (dlastbyte == 0)
        dlastbyte = 512;
    time(&when);

    //
    // Be sure to write out new file size back out to the medium.
    //
    return get_parent()->meta_sync();
}


int
directory_entry_file::utime_ns(const struct timespec *buf)
{
    if (deeper->is_read_only())
        return -EROFS;
    // ignore buf[0] (st_atime)
    when = buf[0].tv_sec; // st_mtime
    return 0;
}


int
directory_entry_file::read(off_t offset, void *data, size_t nbytes)
    const
{
    if (offset < 0)
        return -EINVAL;
    long cur_size = get_current_size();
    assert(cur_size >= 0);
    if (offset >= (off_t)cur_size)
        return 0;
    if ((size_t)offset + nbytes > (size_t)cur_size)
        nbytes = cur_size - offset;
    off_t pos = ((off_t)dfirstblock << 9) + offset;
    return deeper->read(pos, data, nbytes);
}


int
directory_entry_file::write(off_t offset, const void *data, size_t nbytes)
{
    DEBUG(2, "directory_entry_file::write(offset = %ld, data = %p, "
        "nbytes = %ld)", (long)offset, data, (long)nbytes);
    if (deeper->is_read_only())
        return -EROFS;
    if (offset < 0)
        return -EINVAL;
    if (nbytes == 0)
        return 0;
    long cur_size = get_current_size();
    unsigned total = nbytes;

    //
    // If we are writing data entirely inside current extent, we can
    // avoid the call to relocate.
    //
    if ((size_t)offset + nbytes <= (size_t)cur_size)
    {
        DEBUG(2, "not growing");
        off_t pos = ((off_t)dfirstblock << 9) + offset;
        int err = deeper->write(pos, data, nbytes);
        if (err < 0)
            return err;

        // update the last-time-modified
        time(&when);

        //
        // We need to update the meta data at this point.
        //
        err = get_parent()->meta_sync();
        if (err < 0)
            return err;
        return total;
    }

    //
    // If we are writing inside the allocated extent, but past the end
    // of the file, we can still avoid the call to relocate.
    //
    long cur_ext_size = get_current_extent_size();
    if ((size_t)offset + nbytes <= (size_t)cur_ext_size)
    {
        if (offset <= (off_t)cur_size)
        {
            //
            // The only gap in the file is at the end, so we don't
            // have to construct a buffer for the last block (the
            // sector_io::write method does that for us).
            //
            off_t pos = ((off_t)dfirstblock << 9) + offset;
            int err = deeper->write(pos, data, nbytes);
            if (err < 0)
                return err;
        }
        else
        {
            //
            // There is a gap between the previous end-of-file and the
            // write offset.  We must will this gap with zero.
            //
            char rbuf[512];
            memset(rbuf, 0, 512);
            off_t pos = ((off_t)dfirstblock << 9) + (offset & -511);
            int err = deeper->read(pos, rbuf, 512);
            if (err < 0)
                return err;
            assert(nbytes < size_t(512 - (offset & 511)));
            memcpy(rbuf + (offset & 511), data, nbytes);
            err = deeper->write(pos, rbuf, 512);
            if (err < 0)
                return err;
        }
        dlastbyte = (offset + nbytes) & 511;
        if (dlastbyte == 0)
            dlastbyte = 512;
        time(&when);
        int err = get_parent()->meta_sync();
        if (err < 0)
            return err;
        return total;
    }

    //
    // We are growing the file extent.
    //
    // First, look to see if the gap after this file is sufficient for
    // this write, and use the existing gap if at all possible.  This
    // skips expensive reads and writes to the disk relocating the files
    // to make a gap that we don't need.
    //
    int gap_size = get_parent()->sizeof_gap_after(this);
    if (gap_size < 0)
        return gap_size;
    if
    (
        size_t(offset + nbytes)
    >
        size_t((dlastblock + gap_size - dfirstblock) << 9)
    )
    {
        //
        // Ask parent to make a gap for us to write into, immediately
        // after our present extent.
        //
        // NOTE: when this returns, our dfirstblock and dlastblock may have
        // changed.
        //
        gap_size = get_parent()->move_gap_after(this);
        if (gap_size < 0)
            return gap_size;

        //
        // Make sure the write can succeed before we actually write anything
        // to the medium.
        //
        if
        (
            size_t(offset + nbytes)
        >
            size_t((dlastblock + gap_size - dfirstblock) << 9)
        )
            return -ENOSPC;
    }

    //
    // We may need to pad with zeros between the current end-of-file and
    // the new write position.
    //
    if (offset > (off_t)cur_size)
    {
        off_t pos = ((off_t)dfirstblock << 9) + cur_size;
        int err = deeper->write_zero(pos, offset - cur_size);
        if (err < 0)
            return err;
    }

    // actually write the data
    off_t pos = ((off_t)dfirstblock << 9) + offset;
    int err = deeper->write(pos, data, nbytes);
    if (err < 0)
        return err;

    // calc new dlastblock
    dlastblock = dfirstblock + ((offset + nbytes + 511) >> 9);

    // calc new dlastbyte
    dlastbyte = (offset + nbytes) & 511;
    if (dlastbyte == 0)
        dlastbyte = 512;

    // update time-last-modified
    time(&when);

    //
    // The meta-data changed, so need to write it back out to disk.
    // Since all the meta-data is in the directory entry, that means
    // writing out the directory listing again.
    //
    err = get_parent()->meta_sync();
    if (err < 0)
        return -err;
    return total;
}


int
directory_entry_file::get_first_block(void)
    const
{
    return dfirstblock;
}


int
directory_entry_file::get_last_block(void)
    const
{
    return dlastblock;
}


int
directory_entry_file::relocate(unsigned to_block)
{
    if (deeper->is_read_only())
    {
        // Should have caught this error nefore now.
        assert(0);
        return -EROFS;
    }
    if (to_block == (unsigned)dfirstblock)
        return 0;
    int num_blocks = dlastblock - dfirstblock;
    int err =
        deeper->relocate_bytes
        (
            to_block << 9,
            dfirstblock << 9,
            num_blocks << 9
        );
    if (err < 0)
        return err;
    dfirstblock = to_block;
    dlastblock = to_block + num_blocks;
    // let the caller know something changed
    return 1;
}


int
directory_entry_file::fsck(concern_t concern_level)
{
    if (concern_level >= concern_blithe)
        return 0;
    int number_of_errors = 0;
    if (dlastblock < dfirstblock)
    {
        explain_output_error
        (
            "directory entry %s: last block wrong (was %d, expected >= %d)",
            name.quote_c().c_str(),
            dlastblock,
            dfirstblock
        );
        // always fix this
        dlastblock = dfirstblock;
        dlastbyte = 512;
        ++number_of_errors;
    }
    if ((padding4 & 0x7FF8) != 0)
    {
        explain_output_error
        (
            "directory entry %s: padding4 not zero (%04X)",
            name.quote_c().c_str(),
            padding4 & 0x7FF8
        );
        // This is automagically fixed when meta data is written.
        padding4 = 0;
        ++number_of_errors;
    }
    switch (dfkind)
    {
    case securedir:
    case untypedfile:
        explain_output_error
        (
            "directory entry %s: file kind %s (%d) not supported",
            name.quote_c().c_str(),
            dfkind_name(dfkind),
            (int)dfkind
        );
        if (concern_level >= concern_repair)
            dfkind = datafile;
        ++number_of_errors;
        break;

    case xdskfile:
    case codefile:
    case textfile:
    case infofile:
    case datafile:
    case graffile:
    case fotofile:
        break;
    }
    if (name.size() < 1)
    {
        explain_output_error
        (
            "directory entry %s: name too short",
            name.quote_c().c_str()
        );
        // always fix this error
        name = rcstring::printf("%p", this);
        ++number_of_errors;
    }
    else if (name.size() > 15)
    {
        // FIXME: test for illegal characters?
        explain_output_error
        (
            "directory entry %s: name too long",
            name.quote_c().c_str()
        );
        // always fix this error
        name = name.substring(0, 15);
        ++number_of_errors;
    }
    if (dlastblock < 1 || dlastblock > 512)
    {
        explain_output_error
        (
            "directory entry %s: dlastblock wrong (%d)",
            name.quote_c().c_str(),
            dlastblock
        );
        // Always fix this error.
        dlastblock = 12;
        ++number_of_errors;
    }
    if ((padding22 & 0xFC00) != 0)
    {
        explain_output_error
        (
            "directory entry %s: padding22 not zero (%04X)",
            name.quote_c().c_str(),
            padding22 & 0xFC00
        );
        // This is automagically fixed when meta data is written.
        ++number_of_errors;
    }
    return number_of_errors;
}


void
directory_entry_file::fsck_first_block(int blknum)
{
    dfirstblock = blknum;
    if (blknum > dlastblock)
        dlastblock = blknum;
    if (dfirstblock == dlastblock)
        dlastbyte = 512;
}


void
directory_entry_file::fsck_last_block(int blknum)
{
    dlastblock = blknum;
    if (blknum < dfirstblock)
        dfirstblock = blknum;
    if (dfirstblock == dlastblock)
        dlastbyte = 512;
}


void
directory_entry_file::print_listing(bool verbose)
{
    // The name of the file.
    printf("%-15.15s ", name.c_str());

    // The original printed the dlastblock-dfirstblock here, rather than
    // the file size in bytes.
    if (verbose)
        printf("%4d %3d ", dfirstblock, dlastblock);
    printf("%6.6s ", pretty_size(get_size_in_bytes()).c_str());

    // The date last modified of the file
    // (no time was actually tracked).
    struct tm *tmp = localtime(&when);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%e-%b-%y", tmp);
    printf("%9.9s ", buffer);

    // The original printed dfirstblock and dlastbyte here.  We omit
    // them completely (the size in bytes, above, is more useful).

    // Print the file kind.
    printf("%s\n", dfkind_name(dfkind));
}


size_t
directory_entry_file::get_size_in_bytes(void)
    const
{
    return get_current_size();
}


directory_entry::dfkind_t
directory_entry_file::get_file_kind(void)
    const
{
    return dfkind;
}


time_t
directory_entry_file::get_mtime(void)
    const
{
    return when;
}
