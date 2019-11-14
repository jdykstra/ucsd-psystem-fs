//
// UCSD p-System filesystem in user space
// Copyright (C) 2008 Peter Miller
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

#ifndef LIB_OUTPUT_MEMORY_H
#define LIB_OUTPUT_MEMORY_H

#include <lib/output.h>
#include <lib/rcstring/accumulator.h>

/**
  * The output_memory class is used to represent an output destination
  * which stores the result in memory.
  */
class output_memory:
    public output
{
public:
    typedef boost::shared_ptr<output_memory> mpointer;

    /**
      * The destructor.
      */
    virtual ~output_memory();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * #create class method instead.
      */
    output_memory();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static mpointer create();

    /**
      * The truncate_to method may be used to change the data to be of
      * the given size.  If the data is longer, it will be truncated.
      * If the data is shorter, it will be padded.
      *
      * @param size
      *     The size to adjust the data to.
      * @param padding
      *     the character to use to dat the data
      */
    void truncate_to(size_t size, char padding = '\0');

    const char *get_data() const { return buffer.get_data(); }

    size_t size() const { return buffer.size(); }

    /**
      * The overwrite method is used to replace existing data in the
      * memory buffer.
      *
      * @param offset
      *     Where to place the data
      * @param data
      *     pointer to the base of the data to be copied
      * @param nbytes
      *     the amount of data to be copied
      */
    void overwrite(size_t offset, const void *data, size_t nbytes);

protected:
    // See base class for documentation.
    rcstring filename();

    // See base class for documentation.
    void utime_ns(const struct timespec *utb);

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void flush_inner();

private:
    /**
      * The buffer instance variable is used to remember the data which
      * has been output to this output stream.
      */
    rcstring_accumulator buffer;

    /**
      * The copy constructor.  Do not use.
      */
    output_memory(const output_memory &);

    /**
      * The assignment operator.  Do not use.
      */
    output_memory &operator=(const output_memory &);
};

#endif // LIB_OUTPUT_MEMORY_H
