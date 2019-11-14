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

#ifndef LIB_OUTPUT_STDOUT_H
#define LIB_OUTPUT_STDOUT_H

#include <lib/output.h>


/**
  * The output_stdout class is used to repersent the state of an
  * output stream being written to the standard output.
  */
class output_stdout:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_stdout();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * #create class method instead.
      */
    output_stdout();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create();

protected:
    // See base class for documentation.
    rcstring filename();

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void flush_inner();

    // See base class for documentation.
    void utime_ns(const struct timespec *utb);

private:
    /**
      * The bol instance variable is used to remember whether or not we
      * are at the beginning of a line.
      */
    bool bol;

    /**
      * The pos instance variable is used to remember our relative
      * position in the standard output stream.
      */
    long pos;

    /**
      * The copy constructor.  Do not use.
      */
    output_stdout(const output_stdout &);

    /**
      * The assignment operator.  Do not use.
      */
    output_stdout &operator=(const output_stdout &);
};

#endif // LIB_OUTPUT_STDOUT_H
