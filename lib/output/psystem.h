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

#ifndef LIB_OUTPUT_PSYSTEM_H
#define LIB_OUTPUT_PSYSTEM_H

#include <lib/output.h>

class directory_entry; // forward

/**
  * The output_psystem class is used to represent an output destination
  * which writes into a file in a UCSD p-System disk image.
  */
class output_psystem:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_psystem();

private:
    /**
      * The constructor.  It is private on purpose, use the #create
      * class method instead.
      *
      * @param dep
      *     The deeper output stream on which to write the filtered output.
      */
    output_psystem(directory_entry::pointer dep);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param dep
      *     The deeper output stream on which to write the filtered output.
      */
    static pointer create(directory_entry::pointer dep);

protected:
    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    rcstring filename();

    // See base class for documentation.
    void flush_inner();

    // See base class for documentation.
    void utime_ns(const struct timespec *utb);

private:
    directory_entry::pointer dep;
    long address;

    /**
      * The default constructor.  Do not use.
      */
    output_psystem();

    /**
      * The copy constructor.  Do not use.
      */
    output_psystem(const output_psystem &);

    /**
      * The assignment operator.  Do not use.
      */
    output_psystem &operator=(const output_psystem &);
};

#endif // LIB_OUTPUT_PSYSTEM_H
