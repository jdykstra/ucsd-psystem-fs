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

#include <lib/config.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <libexplain/output.h>
#include <sys/statvfs.h>

#include <lib/debug.h>
#include <lib/directory.h>
#include <lib/directory/entry/file.h>
#include <lib/directory/entry/volume_label.h>
#include <lib/hexdump.h>


directory::~directory()
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
}


directory::directory(
    const sector_io::pointer &a_deeper,
    byte_sex_t a_byte_sex
) :
    deeper(a_deeper),
    byte_sex(a_byte_sex),
    text_on_the_fly_flag(false)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
}


int
directory::meta_read(concern_t concern_level)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    int number_of_errors = 0;
    unsigned char buffer[2048];
    int err = deeper->read(0x400, buffer, sizeof(buffer));
    if (err < 0)
    {
        //
        // When the meta-data read fails, we must create a minimal
        // directory structure, otherwise things get very ugly.
        //
        mkfs();

        return err;
    }

    //
    // Figure out if the volume is little-endian or big-endian.
    //
    // We do this by looking at the "dlastblock" field, which is 16
    // bits, and assume the value will be less than 256 (it should be 6).
    //
    const unsigned char *bp = buffer;
    byte_sex = (bp[2] ? little_endian : big_endian);

    //
    // Slurp the volume label.
    //
    DEBUG(2, "Interpreting volume label...");
    DEBUG(3, "data:\n%s", hexdump(bp, 26).c_str());
    volume_label = directory_entry_volume_label::create(this, bp, deeper);
    bp += 26;
    number_of_errors += volume_label->fsck(concern_level);

    //
    // Slurp all of the directory entries.
    //
    size_t maxdents = volume_label->get_num_files();
    for (unsigned fnum = 0; fnum < maxdents; ++fnum, bp += 26)
    {
        DEBUG(2, "Reading entry %d...", fnum);
        DEBUG(3, "\n%s", hexdump(bp, 26).c_str());
        if (bp[6] == 0)
        {
            // slot empty
            // (volume_label->get_num_files() is wrong)
            explain_output_error_and_die
            (
                "number of files in volume label (%ld) does not agree "
                    "with number of actual directory entries (%ld)",
                (long)maxdents,
                (long)fnum
            );
            volume_label->set_num_files(fnum);
            ++number_of_errors;
            break;
        }
        directory_entry::pointer dep =
            directory_entry_file::create(this, bp, deeper);
        files.push_back(dep);
        number_of_errors += dep->fsck(concern_level);
    }

    if (concern_level >= concern_check)
    {
        // Make sure all files are in block order.
        bool out_of_block_order = false;
        directory_entry::pointer prev = files[0];
        for (size_t j = 1; j < files.size(); ++j)
        {
            directory_entry::pointer dep = files[j];
            if (prev->get_last_block() > dep->get_first_block())
            {
                out_of_block_order = true;
                break;
            }
        }
        if (out_of_block_order)
        {
            explain_output_error("directory entries not in block order");
            ++number_of_errors;
            // We always fix this error.
            files.sort_by_first_block();
        }

        //
        // Make sure there are no block overlaps across files.
        //
        int block_num = volume_label->get_last_block();
        for (size_t j = 0; j < files.size(); ++j)
        {
            directory_entry::pointer dep = files[j];
            if (dep->get_first_block() < block_num)
            {
                explain_output_error
                (
                    "directory entry %s: start block overlaps previous file: "
                        "was %d, should be %d",
                    dep->get_name().quote_c().c_str(),
                    dep->get_first_block(),
                    block_num
                );
                ++number_of_errors;
                if (concern_level >= concern_repair)
                    dep->fsck_first_block(block_num);
            }
        }

        //
        // Make sure all of the files fit on the disk.
        //
        for (size_t j = 0; j < files.size(); ++j)
        {
            directory_entry::pointer dep = files[j];
            if (dep->get_last_block() > volume_label->get_eov_block())
            {
                explain_output_error
                (
                    "directory entry %s: last block beyond disk: "
                        "was %d, should be %d",
                    dep->get_name().quote_c().c_str(),
                    dep->get_last_block(),
                    volume_label->get_eov_block()
                );
                ++number_of_errors;
                if (concern_level >= concern_repair)
                    dep->fsck_last_block(volume_label->get_eov_block());
            }
        }
    }

    //
    // If we repaired anything, write the meta data back out.
    //
    if (concern_level >= concern_repair && number_of_errors > 0)
    {
        err = meta_sync();
        if (err < 0)
            return err;
    }

    return number_of_errors;
}


unsigned
directory::get_word(const unsigned char *data)
    const
{
    return byte_sex_get_word(byte_sex, data);
}


void
directory::put_word(unsigned char *data, unsigned value)
    const
{
    byte_sex_put_word(byte_sex, data, value);
}


directory_entry::pointer
directory::find(const rcstring &filename)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    DEBUG(2, "filename = %s", filename.quote_c().c_str());
    static rcstring slash("/");
    if (filename == slash)
        return volume_label;
    rcstring fname(filename.substring(filename[0] == '/', 255));
    if (strchr(fname.c_str(), '/'))
        return directory_entry::pointer();
    return files.find(fname);
}


directory_entry::pointer
directory::nth(int &n)
    const
{
    if (n < 1)
        n = 1;
    directory_entry::pointer dep = files.nth((size_t)(n - 1));
    if (dep)
        ++n;
    return dep;
}


void
directory::mkfs(const rcstring &volid, bool twin)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    rcstring name =
        (
            volid.empty()
        ?
            rcstring::printf("V%06X", (unsigned)time(0) & 0x00FFFFFF)
        :
            volid.substring(0, 7)
        );
    assert(volume_label == 0);
    assert(files.empty());
    volume_label =
        directory_entry_volume_label::create(this, name, deeper, twin);
}


int
directory::meta_sync(void)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    if (deeper->is_read_only())
    {
        //
        // All read-only errors should be caught long before this.
        // It's too late to undo it if you get to here.
        //
        assert(0);
        return -EROFS;
    }

    unsigned char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    unsigned char *bp = buffer;
    assert(volume_label->get_last_block() >= 6);

    //
    // Write out the volume label.
    //
    assert(volume_label);
    volume_label->set_num_files(files.size());
    volume_label->meta_write(bp);
    bp += 26;

    //
    // Write out all of the directory entries.
    //
    for (unsigned fnum = 0; ; ++fnum, bp += 26)
    {
        directory_entry::pointer dep = files[fnum];
        if (!dep)
            break;
        dep->meta_write(bp);
    }

    //
    // Write the data to the disk.
    //
    assert(bp < buffer + sizeof(buffer));
    int erk = deeper->write(0x400, buffer, sizeof(buffer));
    if (erk < 0)
        return erk;

    //
    // It is possible to have a big volume label, containing two copies
    // of the directory and meta-data, rather then just one.  Probably
    // using the theory that you can recover after a directory write
    // problem, or something.  We don't try to do that, we just keep the
    // second copy up-to-date.
    //
    if (volume_label->get_last_block() == 10)
    {
        erk = deeper->write(0x400 + sizeof(buffer), buffer, sizeof(buffer));
        if (erk < 0)
            return erk;
    }

    //
    // Make sure it all arrives on the medium.
    //
    return deeper->sync();
}


bool
directory::has_room_for_new_file(void)
    const
{
    return (files.size() < volume_label->maximum_directory_entries());
}


int
directory::add_new_file(directory_entry::pointer dep)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    if (deeper->is_read_only())
    {
        //
        // All read-only errors should be caught long before this.
        // It's too late to undo it if you get to here.
        //
        assert(0);
        return -EROFS;
    }

    // You MUST check with has_room_for_new_file first.
    assert(files.size() < volume_label->maximum_directory_entries());
    volume_label->update_timestamp();
    files.push_back(dep);
    return meta_sync();
}


void
directory::delete_existing_file(directory_entry *dep)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    assert(!deeper->is_read_only());
    files.erase(dep);
    volume_label->update_timestamp();
}


void
directory::delete_existing_file(directory_entry::pointer dep)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    delete_existing_file(dep.get());
}


int
directory::first_empty_block(void)
    const
{
    if (files.empty())
        return volume_label->get_last_block();
    return files.back()->get_last_block();
}


int
directory::crunch(void)
{
    if (files.empty())
        return 0;
    return move_gap_after(files.back().get());
}


int
directory::move_gap_after(directory_entry *dep)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    if (deeper->is_read_only())
    {
        //
        // All read-only errors should be caught long before this.
        // It's too late to undo it if you get to here.
        //
        assert(!"can't move gap on read-only disk image");
        return -EROFS;
    }

    size_t idx = files.index_of(dep);
    assert(idx != (size_t)(-1));
    unsigned low_block = volume_label->get_last_block();
    unsigned high_block = volume_label->get_eov_block();
    bool changed = false;

    // Move stuff down
    for (size_t j = 0; j <= idx; ++j)
    {
        directory_entry::pointer mdep = files[j];
        int err = mdep->relocate(low_block);
        if (err < 0)
            return err;
        if (err > 0)
            changed = true;
        low_block = mdep->get_last_block();
    }

    // Move stuff up
    for (size_t k = files.size(); k > idx + 1; --k)
    {
        directory_entry::pointer mdep = files[k - 1];
        int err = mdep->relocate(high_block - mdep->size_in_blocks());
        if (err < 0)
            return err;
        if (err > 0)
            changed = true;
        high_block = mdep->get_first_block();
    }

    //
    // It is entirely possible that nothing has changed, because this
    // method will be called each time we are asked to write data to a
    // file, and it will probably (not always, but probably) be the same
    // file as the last time.
    //
    if (changed)
    {
        int err = meta_sync();
        if (err < 0)
            return err;
    }

    // Let them know how big the gap is.
    return (high_block - low_block);
}


int
directory::sizeof_gap_after(directory_entry *dep)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    if (deeper->is_read_only())
    {
        //
        // All read-only errors should be caught long before this.
        // It's too late to undo it if you get to here.
        //
        assert(!"should not reach here");
        return -EROFS;
    }

    size_t idx = files.index_of(dep);
    assert(idx != (size_t)(-1));
    if (idx == (size_t)(-1))
        return -ENOENT;
    assert(idx < files.size());
    unsigned low_block = dep->get_last_block();
    unsigned high_block =
        (
            idx == files.size() - 1
        ?
            volume_label->get_eov_block()
        :
            files[idx + 1]->get_first_block()
        );
    assert(low_block <= high_block);
    return (high_block - low_block);
}


int
directory::calc_used_blocks(void)
    const
{
    int nblocks = volume_label->size_in_blocks();
    for (size_t j = 0; j < files.size(); ++j)
        nblocks += files[j]->size_in_blocks();
    return nblocks;
}


int
directory::statfs(struct statvfs *st)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    memset(st, 0, sizeof(*st));
    st->f_bsize = 512;
    st->f_frsize = st->f_bsize;
    st->f_blocks = volume_label->get_eov_block();
    st->f_bfree = st->f_blocks - calc_used_blocks();
    st->f_bavail = st->f_bfree;
    st->f_files = volume_label->maximum_directory_entries();
    st->f_ffree = st->f_files - files.size();
    st->f_favail = st->f_ffree;
    st->f_fsid = 0x55435344; // "UCSD"
    st->f_flag = ST_NOSUID;
    if (deeper->is_read_only())
        st->f_flag |= ST_RDONLY;
    st->f_namemax = 15; // should there be a way to ask the class for this?
    return 0;
}


rcstring
directory::get_volume_name(void)
    const
{
    return volume_label->get_name();
}


int
directory::wipe_unused(void)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    if (deeper->is_read_only())
    {
        //
        // All read-only errors should be caught long before this.
        // It's too late to undo it if you get to here.
        //
        assert(!"can't wipe unused from read-only disk image");
        return -EROFS;
    }
    char zero[512];
    memset(zero, 0, sizeof(zero));
    int curblock = volume_label->size_in_blocks();
    for (size_t j = 0; j < files.size(); ++j)
    {
        directory_entry::pointer fp = files[j];

        // wipe unallocated blocks between the last file and this file
        int first_block = fp->get_first_block();
        while (curblock < first_block)
        {
            deeper->write(curblock << 9, zero, sizeof(zero));
            ++curblock;
        }

        // within the last block of the file, wipe any unused bytes
        unsigned partial = fp->get_size_in_bytes() & 511;
        if (partial)
        {
            unsigned blknum = curblock + fp->size_in_blocks() - 1;
            unsigned addr = (blknum << 9) + partial;
            unsigned tail_size = 512 - partial;
            deeper->write(addr, zero, tail_size);
        }

        // FIXME: within code files, the UCSD native compiler does not
        // bother to make sure unused bytes in the files are reset to
        // zero.  The result is that random memory from the cmpilation
        // is present, sometime as source code.  We should zero the tail
        // ends of each codefile segment, as well.

        curblock += fp->size_in_blocks();
    }
    int high_block = volume_label->get_eov_block();
    while (curblock < high_block)
    {
        deeper->write(curblock << 9, zero, sizeof(zero));
        ++curblock;
    }
    return 0;
}


bool
directory::check_for_system_files(void)
{
    //
    // Make sure there are enough files present to actually do something,
    // or start doing something.
    //
    return
        (
            find("SYSTEM.COMPILER")
        &&
            find("SYSTEM.EDITOR")
        &&
            find("SYSTEM.FILER")
        &&
            find("SYSTEM.PASCAL")
        );
}
