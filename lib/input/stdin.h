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

#ifndef LIB_INPUT_STDIN_H
#define LIB_INPUT_STDIN_H

#include <lib/input.h>

/**
  * The input_stdin class is used to represent an input source which
  * reads from the standard input.
  */
class input_stdin:
    public input
{
public:
    /**
      * The destructor.
      */
    virtual ~input_stdin();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * #create class method instead.
      */
    input_stdin();

public:
    /**
      * The craete class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create();

protected:
    // See base class for documentation.
    rcstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    void keepalive();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    int fpathconf_name_max();

    // See base class for documentation.
    void fstat(struct stat &);

private:
    long pos;
    bool unbuffered;

    /**
      * The copy constructor.  Do not use.
      */
    input_stdin(const input_stdin &arg);

    /**
      * The default constructor.  Do not use.
      */
    input_stdin &operator=(const input_stdin &arg);
};

#endif // LIB_INPUT_STDIN_H
