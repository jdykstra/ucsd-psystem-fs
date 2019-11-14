//
// UCSD p-System filesystem in user space
// Copyright (C) 2006, 2007 Peter Miller
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

#include <cstring>
#include <lib/rcstring.h>


bool
rcstring::ends_with(const rcstring &rhs)
    const
{
    const char *haystack = c_str();
    size_t haystack_len = size();
    const char *needle = rhs.c_str();
    size_t needle_len = rhs.size();
    return
    (
        haystack_len >= needle_len
    &&
        0 == memcmp(haystack + haystack_len - needle_len, needle, needle_len)
    );
}
