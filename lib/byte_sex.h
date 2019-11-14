//
// UCSD p-System filesystem in user space
// Copyright (C) 2010 Peter Miller
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

#ifndef LIB_BYTE_SEX_H
#define LIB_BYTE_SEX_H

#include <lib/rcstring.h>

enum byte_sex_t
{
    little_endian,
    big_endian
};

rcstring byte_sex_name(byte_sex_t value);

unsigned byte_sex_get_word(byte_sex_t bs, const unsigned char *data);
unsigned byte_sex_get_word_le(const unsigned char *data);
unsigned byte_sex_get_word_be(const unsigned char *data);
void byte_sex_put_word(byte_sex_t bs, unsigned char *data, unsigned value);
void byte_sex_put_word_le(unsigned char *data, unsigned value);
void byte_sex_put_word_be(unsigned char *data, unsigned value);

#endif // LIB_BYTE_SEX_H
