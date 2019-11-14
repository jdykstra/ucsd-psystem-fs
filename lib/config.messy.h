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

#ifndef LIB_CONFIG_MESSY_H
#define LIB_CONFIG_MESSY_H

//
// Make sure Solaris includes POSIX extensions.
//
#if (defined(__sun) || defined(__sun__) || defined(sun)) && \
        (defined(__svr4__) || defined(svr4))

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif

#endif // sun

//
// Cygwin's mingw mode...
//
#if defined(__MSVCRT__)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

//
// FUSE is fussy.  It needs to be.
//
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#endif // LIB_CONFIG_MESSY_H
