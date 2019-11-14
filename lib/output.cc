//
// UCSD p-System filesystem in user space
// Copyright (C) 2006, 2007, 2010 Peter Miller
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
#include <cstring>
#include <libexplain/output.h>

#include <lib/output.h>
#include <lib/rcstring.h>


output::~output()
{
    //
    // Note: calling the flush() method here is pointless, because
    // the derived class has already been destroyed.
    //
    if (buffer_position > buffer)
    {
        //
        // We couldn't call a type_name() method here, which would make
        // a much better error message, because the derived class has
        // already been destroyed.
        //
        // We can't call flush, for the same reason.
        //
        explain_output_error_and_die
        (
            "you forgot to call flush in the destructor of a derived "
            "class (bug)"
        );
    }
    assert(reference_count == 1);

    //
    // Now that no more reference count assertions should be reached, be
    // paranoid and set it to zero to trigger assertion failures if this
    // instance continues to be referenced.
    //
    reference_count = 0;

    assert(buffer);
    delete [] buffer;
    buffer = 0;
    buffer_size = 0;
    buffer_position = 0;
    buffer_end = 0;
}


output::output() :
    reference_count(1),
    buffer(0),
    buffer_size(0),
    buffer_position(0),
    buffer_end(0)
{
    buffer_size = (size_t)1 << 13;
    buffer = new unsigned char [buffer_size];
    buffer_position = buffer;
    buffer_end = buffer + buffer_size;
}


void
output::reference_count_up()
{
    assert(reference_count_valid());
    ++reference_count;
}


void
output::reference_count_down()
{
    assert(reference_count_valid());
    if (reference_count <= 1)
    {
        //
        // We don't let the reference count reach zero, otherwise
        // reference count assertions in the flush and write methods
        // will bomb.
        //
        delete this;
    }
    else
    {
        --reference_count;
    }
}


void
output::overflow(char c)
{
    assert(reference_count_valid());
    assert(buffer);
    assert(buffer_position >= buffer);
    assert(buffer_end == buffer + buffer_size);
    assert(buffer_position <= buffer_end);
    if (buffer_position >= buffer_end)
    {
        size_t nbytes = buffer_position - buffer;
        write_inner(buffer, nbytes);
        buffer_position = buffer;
    }
    *buffer_position++ = c;
}


void
output::fputs(const char *s)
{
    assert(reference_count_valid());
    size_t nbytes = strlen(s);
    if (nbytes)
        write(s, nbytes);
}


void
output::fputs(const rcstring &s)
{
    assert(reference_count_valid());
    if (!s.empty())
        write(s.c_str(), s.length());
}


void
output::write(const void *data, size_t len)
{
    assert(reference_count_valid());
    if (len)
    {
        if (buffer_position + len <= buffer_end)
        {
            memcpy(buffer_position, data, len);
            buffer_position += len;
        }
        else
        {
            size_t nbytes = buffer_position - buffer;
            if (nbytes)
            {
                write_inner(buffer, nbytes);
                buffer_position = buffer;
            }
            if (len < buffer_size)
            {
                memcpy(buffer, data, len);
                buffer_position += len;
            }
            else
                write_inner(data, len);
        }
    }
}


void
output::flush()
{
    assert(reference_count_valid());
    if (buffer_position > buffer)
    {
        size_t nbytes = buffer_position - buffer;
        write_inner(buffer, nbytes);
        buffer_position = buffer;
    }
    flush_inner();
}


void
output::fprintf(const char *fmt, ...)
{
    assert(reference_count_valid());
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fmt, ap);
    va_end(ap);
}


void
output::vfprintf(const char *fmt, va_list ap)
{
    //
    // We have to make a temporary copy of it, in case a deeper output
    // stream also uses output::fprintf to satisfy the virtual
    // write_inner call via output::fputs.
    //
    // The moral is: avoid output::fprintf.
    //
    assert(reference_count_valid());
    rcstring tmp(rcstring::printf(fmt, ap));
    fputs(tmp);
}


void
output::flush_inner()
{
    // Do nothing.
    assert(reference_count_valid());
}
