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

#include <lib/pretty_size.h>


rcstring
pretty_size(unsigned long nbytes)
{
    if (nbytes < 100000)
        return rcstring::printf("%ld", nbytes);

    double size = nbytes / 1024.;
    if (size < 9.995)
        return rcstring::printf("%5.3fK", size);
    if (size < 99.95)
        return rcstring::printf("%5.2fK", size);
    if (size < 1024)
        return rcstring::printf("%.0fK", size);

    size /= 1024.;
    if (size < 9.995)
        return rcstring::printf("%5.3fM", size);
    if (size < 99.95)
        return rcstring::printf("%5.2fM", size);

    //
    // The greatest size possible is 16MB, so don't worry overmuch about
    // anything bigger.
    //
    return rcstring::printf("%.1fM", size);
}
