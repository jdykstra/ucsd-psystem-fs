//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008 Peter Miller
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

#ifndef LIB_INPUT_PSYSTEM_H
#define LIB_INPUT_PSYSTEM_H

#include <lib/directory/entry.h>
#include <lib/input.h>

/**
  * The input_psystem class is used to represent and input source which
  * comes from a file in a UCSD p-System disk image.
  */
class input_psystem:
    public input
{
public:
    /**
      * The destructor.
      */
    virtual ~input_psystem();

private:
    /**
      * The constructor.  It is private on purpose, use the #create
      * method instead.
      *
      * @param dep
      *     The directory entry to be read for input.
      */
    input_psystem(directory_entry *dep);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param dep
      *     The directory entry to be read for input.
      */
    static pointer create(directory_entry::pointer dep);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param dep
      *     The directory entry to be read for input.
      */
    static pointer create(directory_entry *dep);

protected:
    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    rcstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    int fpathconf_name_max();

    // See base class for documentation.
    void fstat(struct stat &);

private:
    directory_entry *dep;
    long address;

    /**
      * The default constructor.  Do not use.
      */
    input_psystem();

    /**
      * The copy constructor.  Do not use.
      */
    input_psystem(const input_psystem &);

    /**
      * The assignment operator.  Do not use.
      */
    input_psystem &operator=(const input_psystem &);
};

#endif // LIB_INPUT_PSYSTEM_H
