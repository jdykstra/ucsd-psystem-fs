//
// UCSD p-System filesystem in user space
// Copyright (C) 2008, 2010 Peter Miller
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lib/debug.h>
#include <lib/directory.h>
#include <lib/directory/entry/file/text.h>
#include <lib/input/psystem.h>
#include <lib/output/memory.h>
#include <lib/output/text_decode.h>
#include <lib/output/text_encode.h>


directory_entry_file_text::~directory_entry_file_text()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
}


directory_entry_file_text::directory_entry_file_text(directory *a_prnt,
        const rcstring &a_name, int a_blk, int a_nblks,
        const sector_io::pointer &a_deeper) :
    directory_entry_file(a_prnt, a_name, textfile, a_blk, a_nblks, a_deeper)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
}


directory_entry_file_text::directory_entry_file_text(directory *a_parent,
        const unsigned char *a_data, const sector_io::pointer &a_deeper) :
    directory_entry_file(a_parent, a_data, a_deeper)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
}


directory_entry::pointer
directory_entry_file_text::create(directory *a_parent,
    const unsigned char *a_data, const sector_io::pointer &a_deeper)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    return pointer(new directory_entry_file_text(a_parent, a_data, a_deeper));
}


directory_entry::pointer
directory_entry_file_text::create(directory *par, const rcstring &nm,
    int blk, int nblks, const sector_io::pointer &medium)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    return pointer(new directory_entry_file_text(par, nm, blk, nblks, medium));
}


output_memory::mpointer
directory_entry_file_text::slurp(void)
    const
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    // we can't use input_psystem because that will call our read
    // method, and not our parent's read method.
    off_t address = 0;
    output_memory::mpointer omp = output_memory::create();
    output::pointer op = output_text_decode::create(omp);
    for (;;)
    {
        char data[1 << 14];
        int n = directory_entry_file::read(address, data, sizeof(data));
        if (n < 0)
            return output_memory::mpointer();
        if (n == 0)
            break;
        op->write(data, n);
        address += n;
    }
    return omp;
}


int
directory_entry_file_text::getattr(struct stat *stbuf)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    directory_entry_file::getattr(stbuf);

    //
    // build the decoded text to extract the size
    //
    if (!cache)
        cache = slurp();
    if (!cache)
        return -ENOMEM;
    stbuf->st_size = cache->size();

    return 0;
}


int
directory_entry_file_text::truncate(off_t size)
{
    if (size < 0)
        return -EINVAL;
    DEBUG(2, "%s", __PRETTY_FUNCTION__);

    //
    // read in the whole text file
    //
    if (!cache)
       cache = slurp();

    //
    // truncate the memory image (or pad it as given) to make it the
    // desired size.
    //
    cache->truncate_to(size, '\n');

    //
    // Now re-encode the memory image to be the appropriate on-disk form.
    //
    output_memory::mpointer omp = output_memory::create();
    {
        output::pointer op = output_text_encode::create(omp);
        op->write(cache->get_data(), cache->size());
    }
    DEBUG(3, "omp->size() = %ld", long(omp->size()));
    omp->flush();

    //
    // truncate ourselves to oblivion
    //
    int err = directory_entry_file::truncate(0);
    if (err < 0)
        return err;

    //
    // Now write the in-memory UCSD form to disk.
    //
    DEBUG(2, "about to write");
    return directory_entry_file::write(0, omp->get_data(), omp->size());
}


int
directory_entry_file_text::read(off_t offset, void *data, size_t nbytes)
    const
{
    if (offset < 0)
        return -EINVAL;
    DEBUG(2, "%s", __PRETTY_FUNCTION__);

    //
    // read in the whole text file
    //
    if (!cache)
        cache = slurp();

    //
    // sanity check against in-memory copy
    //
    assert(offset >= 0);
    if ((size_t)offset > cache->size())
        return 0;
    if (offset + nbytes > cache->size())
        nbytes = cache->size() - offset;

    //
    // transfer data from in memory copy
    //
    memcpy(data, cache->get_data() + offset, nbytes);
    return nbytes;
}


int
directory_entry_file_text::write(off_t offset, const void *data, size_t nbytes)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    DEBUG(3, "offset = %ld", (long)offset);
    DEBUG(3, "nbytes = %ld", (long)nbytes);
    //
    // read in the whole text file
    //
    if (!cache)
        cache = slurp();
    DEBUG(3, "cache->size() = %ld", long(cache->size()));

    //
    // overwrite the text data at the specified location
    //
    cache->overwrite(offset, data, nbytes);
    DEBUG(3, "cache->size() = %ld", long(cache->size()));

    //
    // Now re-encode the memory image to be the appropriate on-disk form.
    //
    output_memory::mpointer omp = output_memory::create();
    {
        output::pointer op = output_text_encode::create(omp);
        op->write(cache->get_data(), cache->size());
    }
    DEBUG(3, "omp->size() = %ld", long(omp->size()));
    omp->flush();

    //
    // truncate ourselves to oblivion
    //
    int err = directory_entry_file::truncate(0);
    if (err < 0)
        return err;

    //
    // Now write the in-memory form to disk.
    //
    DEBUG(2, "about to write");
    int n2 = directory_entry_file::write(0, omp->get_data(), omp->size());
    if (n2 < 0)
        return n2;
    assert((size_t)n2 == omp->size());
    return nbytes;
}


size_t
directory_entry_file_text::get_size_in_bytes(void)
    const
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    if (!cache)
        cache = slurp();
    return cache->size();
}


int
directory_entry_file_text::release()
{
    cache.reset();
    return 0;
}
