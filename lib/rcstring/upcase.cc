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

#include <cctype>
#include <lib/rcstring/accumulator.h>


rcstring
rcstring::upcase()
    const
{
    static rcstring_accumulator tmp;
    tmp.clear();
    const char *s = c_str();
    for (;;)
    {
        char c = *s++;
        if (!c)
            break;
        if (islower((unsigned char)c))
            c = toupper(c);
        tmp.push_back(c);
    }
    return tmp.mkstr();
}
