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
#include <lib/directory/entry/volume_label.h>
#include <lib/rcstring/list.h>


directory_entry_volume_label::~directory_entry_volume_label()
{
}


directory_entry_volume_label::directory_entry_volume_label(
    directory *a_parent,
    const unsigned char *data,
    const sector_io::pointer &a_deeper
) :
    directory_entry(a_parent),
    deeper(a_deeper),
    dfirstblock(0),
    dlastblock(0),
    padding4(0),
    deovblk(0),
    dnumfiles(0),
    dloadtime(0),
    when(0),
    padding22(0),
    padding24(0),
    max_dir_ents(0)
{
    meta_read(data);
}


directory_entry_volume_label::vlptr
directory_entry_volume_label::create(directory *a_parent,
    const unsigned char *data, const sector_io::pointer &a_deeper)
{
    return vlptr(new directory_entry_volume_label(a_parent, data, a_deeper));
}


directory_entry_volume_label::directory_entry_volume_label(
    directory *a_parent,
    const rcstring &a_name,
    const sector_io::pointer &a_deeper,
    bool twin
) :
    directory_entry(a_parent),
    deeper(a_deeper),
    dfirstblock(0),
    dlastblock(twin ? 10 : 6),
    padding4(0),
    name(a_name),
    deovblk(deeper->size_in_bytes() >> 9),
    dnumfiles(0),
    dloadtime(0),
    when(time(0)),
    padding22(0),
    padding24(0),
    max_dir_ents(0)
{
    calc_max_dir_ents();
}


directory_entry_volume_label::vlptr
directory_entry_volume_label::create(directory *a_prnt,
    const rcstring &a_nm, const sector_io::pointer &a_deep, bool twin)
{
    return vlptr(new directory_entry_volume_label(a_prnt, a_nm, a_deep, twin));
}


void
directory_entry_volume_label::meta_read(const unsigned char *data)
{
    //
    // The data layout for directories (volume labels) is...
    //
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //  0 |            dfirstblock (1st physical disk address)            |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //  2 |       dlastblock (points to block after last used block)      |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //  4 |                            Ignore                             |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //  6 |            vidlen             |                               |
    //    +---+---+---+---+---+---+---+---+--                           --+
    //  8 |                                                               |
    //    +--              vid (Name of volume, 7 bytes)                --+
    // 10 |                                                               |
    //    +--                                                           --+
    // 12 |                                                               |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // 14 |                            deovblk                            |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // 16 |                           dnumfiles                           |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // 18 |                           dloadtime                           |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // 20 |        Year               |     Month     |        Day        |
    //    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //
    dfirstblock = get_word(data);
    dlastblock = get_word(data + 2);
    padding4 = get_word(data + 4);
    name = rcstring((const char *)(data + 7), data[6]);
    name = name.replace("/", "_");
    deovblk = get_word(data + 14);
    dnumfiles = get_word(data + 16);
    dloadtime = get_word(data + 18);
    when = get_date(data + 20);
    padding22 = get_word(data + 22);
    padding24 = get_word(data + 24);
    calc_max_dir_ents();
}


void
directory_entry_volume_label::meta_write(unsigned char *data)
    const
{
    put_word(data, dfirstblock);
    put_word(data + 2, dlastblock);
    put_word(data + 4, padding4);
    size_t len = name.size();
    if (len > 7)
        len = 7;
    data[6] = len;
    memcpy(data + 7, name.c_str(), len);
    put_word(data + 14, deovblk);
    put_word(data + 16, dnumfiles);
    put_word(data + 18, dloadtime);
    put_date(data + 20, when);
    put_word(data + 22, padding22);
    put_word(data + 24, padding24);
}


rcstring
directory_entry_volume_label::get_name(void)
    const
{
    return name;
}


size_t
directory_entry_volume_label::get_name_maxlen(void)
    const
{
    return 7;
}


int
directory_entry_volume_label::getattr(struct stat *stbuf)
{
    stbuf->st_dev = 0;
    stbuf->st_rdev = 0;
    stbuf->st_mode = S_IFDIR | (deeper->is_read_only() ? 0555 : 0777);
    stbuf->st_nlink = 2;
    stbuf->st_blocks = dlastblock - dfirstblock;
    stbuf->st_blksize = 512;
    stbuf->st_size = stbuf->st_blocks * stbuf->st_blksize;
    stbuf->st_ino = 2;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = when;
    stbuf->st_mtime = when;
    stbuf->st_ctime = when;
    return 0;
}


int
directory_entry_volume_label::utime_ns(const struct timespec *buf)
{
    if (deeper->is_read_only())
        return -EROFS;
    // ignore buf[0] (st_ctime)
    when = buf[1].tv_sec; // st_mtime
    return 0;
}


int
directory_entry_volume_label::mknod(const rcstring &ename, mode_t mode, dev_t)
{
    if (deeper->is_read_only())
        return -EROFS;
    if (!S_ISREG(mode))
        return -ENOSYS;
    if (!get_parent()->has_room_for_new_file())
        return -ENOSPC;
    dfkind_t kind = dfkind_from_extension(ename);
    directory_entry::pointer dep =
        directory_entry_file::create
        (
            get_parent(),
            ename.upcase(),
            kind,
            deovblk,    // start block
            0,          // number of blocks
            deeper
        );
    return get_parent()->add_new_file(dep);
}


int
directory_entry_volume_label::opendir(void)
{
    return 0;
}


int
directory_entry_volume_label::get_directory_entry_names(rcstring_list &results)
{
    int n = 0;
    for (;;)
    {
        directory_entry::pointer dep = get_parent()->nth(n);
        if (!dep)
            break;
        results.push_back(dep->get_name());
    }
    return 0;
}


int
directory_entry_volume_label::releasedir(void)
{
    return 0;
}


void
directory_entry_volume_label::set_num_files(size_t n)
{
    assert(n <= max_dir_ents);
    dnumfiles = n;
}


void
directory_entry_volume_label::update_timestamp(void)
{
    if (!deeper->is_read_only())
        time(&when);
}


void
directory_entry_volume_label::calc_max_dir_ents(void)
{
    int num_blocks = dlastblock - dfirstblock - 2;
    assert(num_blocks > 0);
    size_t available_bytes = (unsigned)num_blocks << 9;
    assert(available_bytes >= 2 * 26);
    // The -1 is to leave room for the volume header.
    max_dir_ents = (available_bytes / 26) - 1;
    DEBUG(3, "max_dir_ents = %ld", (long)max_dir_ents);
}


int
directory_entry_volume_label::get_first_block(void)
    const
{
    return dfirstblock;
}


int
directory_entry_volume_label::get_last_block(void)
    const
{
    return dlastblock;
}


int
directory_entry_volume_label::relocate(unsigned to_block)
{
    if (deeper->is_read_only())
    {
        // Should have caught this error before now.
        assert(0);
        return -EROFS;
    }
    if ((unsigned)dfirstblock == to_block)
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
directory_entry_volume_label::fsck(concern_t concern_level)
{
    if (concern_level == concern_blithe)
        return 0;
    int number_of_errors = 0;
    if (dfirstblock != 0)
    {
        explain_output_error
        (
            "%s: volume label: first block not zero (%d)",
            deeper->get_filename().c_str(),
            dfirstblock
        );
        if (concern_level >= concern_repair)
            dfirstblock = 0;
        ++number_of_errors;
    }
    if (dlastblock != 6 && dlastblock != 10)
    {
        explain_output_error
        (
            "%s: volume label: last block not six (%d)",
            deeper->get_filename().c_str(),
            dlastblock
        );
        if (concern_level >= concern_repair)
            dlastblock = 0;
        ++number_of_errors;
    }
    if (padding4 != 0)
    {
        explain_output_error
        (
            "%s: volume label: padding4 not zero (%04X)",
            deeper->get_filename().c_str(),
            padding4
        );
        if (concern_level >= concern_repair)
            padding4 = 0;
        ++number_of_errors;
    }
    if (name.size() < 0)
    {
        explain_output_error
        (
            "%s: volume label: name too short",
            deeper->get_filename().c_str()
        );
        // always fix this error
        name = "NO-NAME";
        ++number_of_errors;
    }
    else if (name.size() > 7)
    {
        // FIXME: test for illegal characters?
        explain_output_error
        (
            "%s: volume label: name too long",
            deeper->get_filename().c_str()
        );
        // always fix this error
        name = name.substring(0, 7);
        ++number_of_errors;
    }
    int actual_blocks = deeper->size_in_bytes() >> 9;
    if (deovblk != actual_blocks)
    {
        explain_output_error
        (
            "%s: volume label: end-of-volume block incorrect (was %d, "
                "expected %d)",
            deeper->get_filename().c_str(),
            deovblk,
            actual_blocks
        );
        // always fix this error
        deovblk = actual_blocks;
        ++number_of_errors;
    }
    if (dloadtime != 0)
    {
        explain_output_error
        (
            "%s: volume label: load time not zero (%04X)",
            deeper->get_filename().c_str(),
            dloadtime
        );
        if (concern_level >= concern_repair)
            dloadtime = 0;
        ++number_of_errors;
    }
    if (padding22 != 0)
    {
        explain_output_error
        (
            "%s: volume label: padding22 not zero (%04X)",
            deeper->get_filename().c_str(),
            padding22
        );
        if (concern_level >= concern_repair)
            padding22 = 0;
        ++number_of_errors;
    }
    if (padding24 != 0)
    {
        explain_output_error
        (
            "%s: volume label: padding24 not zero (%04X)",
            deeper->get_filename().c_str(),
            padding24
        );
        if (concern_level >= concern_repair)
            padding24 = 0;
        ++number_of_errors;
    }
    if (dnumfiles < 0 || dnumfiles > int(max_dir_ents))
    {
        explain_output_error
        (
            "%s: number of files absurd (got %ld, maximum %ld)",
            deeper->get_filename().c_str(),
            (long)dnumfiles, (long)max_dir_ents
        );
        // make this sane no matter what concern level is given
        dnumfiles = max_dir_ents;
        ++number_of_errors;
    }
    return number_of_errors;
}


void
directory_entry_volume_label::fsck_first_block(int blknum)
{
    dfirstblock = blknum;
    if (blknum > dlastblock)
        dlastblock = blknum;
}


void
directory_entry_volume_label::fsck_last_block(int blknum)
{
    dlastblock = blknum;
    if (blknum < dfirstblock)
        dfirstblock = blknum;
}


void
directory_entry_volume_label::print_listing(bool verbose)
{
    struct tm *tmp = localtime(&when);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", tmp);
    printf("Last mounted %s\n", buffer);
}


directory_entry::dfkind_t
directory_entry_volume_label::get_file_kind(void)
    const
{
    return (dfkind_t)0;
}


time_t
directory_entry_volume_label::get_mtime(void)
    const
{
    return when;
}


size_t
directory_entry_volume_label::get_size_in_bytes(void)
    const
{
    return (deovblk << 9);
}
