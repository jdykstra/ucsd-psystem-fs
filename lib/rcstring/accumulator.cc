//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#pragma implementation "rcstring_accumulator"

#include <cstring>
#include <lib/rcstring/accumulator.h>


rcstring_accumulator::~rcstring_accumulator()
{
    delete [] buffer;
    buffer = 0;
    length = 0;
    maximum = 0;
}


rcstring_accumulator::rcstring_accumulator() :
    length(0),
    maximum(0),
    buffer(0)
{
}


rcstring_accumulator::rcstring_accumulator(const rcstring_accumulator &arg) :
    length(0),
    maximum(0),
    buffer(0)
{
    push_back(arg);
}


rcstring_accumulator &
rcstring_accumulator::operator=(const rcstring_accumulator &arg)
{
    if (this != &arg)
    {
        clear();
        push_back(arg);
    }
    return *this;
}


rcstring
rcstring_accumulator::mkstr()
    const
{
    return rcstring(buffer, length);
}


void
rcstring_accumulator::overflow(char c)
{
    grow(length + 1);
    buffer[length++] = c;
}


void
rcstring_accumulator::push_back(const rcstring_accumulator &arg)
{
    push_back(arg.buffer, arg.length);
}


void
rcstring_accumulator::push_back(const void *data, size_t n)
{
    if (!n)
        return;
    grow(length + n);
    const char *cp = (const char *)data;
    memcpy(buffer + length, cp, n);
    length += n;
}


void
rcstring_accumulator::push_back(const char *s)
{
    push_back(s, strlen(s));
}
