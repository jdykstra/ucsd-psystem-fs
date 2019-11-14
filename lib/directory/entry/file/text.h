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

#ifndef LIB_DIRECTORY_ENTRY_FILE_TEXT_H
#define LIB_DIRECTORY_ENTRY_FILE_TEXT_H

#include <lib/directory/entry/file.h>
#include <lib/output/memory.h>

/**
  * The directory_entry_text_file class is used to represent the
  * directory entry of a text file.  This will be automagically
  * translated between the Unix text file format and the UCSD p-System
  * text file format.
  */
class directory_entry_file_text:
    public directory_entry_file
{
public:
    /**
      * The destructor.
      */
    virtual ~directory_entry_file_text();

private:
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
    directory_entry_file_text(directory *parent, const unsigned char *data,
        const sector_io::pointer &deeper);

    /**
      * The constructor.  It is private on purpose, use the #create
      * class method instead.
      *
      * @param parent
      *     The directory containing this file.
      * @param name
      *     The name of the new file.
      * @param sector
      *     The first block of the file.
      * @param num_blocks
      *     The number of blocks in the new file.
      * @param deeper
      *     The data to be cropped to access data for this file.
      */
    directory_entry_file_text(directory *parent, const rcstring &name,
        int block, int num_blocks,
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
      * @param sector
      *     The first block of the file.
      * @param num_blocks
      *     The number of blocks in the new file.
      * @param deeper
      *     The data to be cropped to access data for this file.
      */
    static pointer create(directory *parent, const rcstring &name,
        int block, int num_blocks, const sector_io::pointer &deeper);

protected:
    // See base class for documentation
    int getattr(struct stat *stbuf);

    // See base class for documentation
    int truncate(off_t size);

    // See base class for documentation
    int read(off_t offset, void *data, size_t nbytes) const;

    // See base class for documentation
    int write(off_t offset, const void *data, size_t nbytes);

    // See base class for documentation
    size_t get_size_in_bytes(void) const;

    // See base class for documentation
    int release();

private:
    /**
      * The slurp method is used to read in the text file and convert it
      * from UCSD text into Unix text in memory.
      */
    output_memory::mpointer slurp(void) const;

    /**
      * The cache instance variable is used to remember the in-memory
      * Unix-format version of the text of the fil of the file.
      */
    mutable output_memory::mpointer cache;

    /**
      * The default constructor.  Do not use.
      */
    directory_entry_file_text();

    /**
      * The copy constructor.  Do not use.
      */
    directory_entry_file_text(const directory_entry_file_text &);

    /**
      * The assignment operator.  Do not use.
      */
    directory_entry_file_text &operator=(const directory_entry_file_text &);
};

#endif // LIB_DIRECTORY_ENTRY_FILE_TEXT_H
