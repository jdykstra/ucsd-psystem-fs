//
// UCSD p-System filesystem in user space
// Copyright (C) 2007, 2010 Peter Miller
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
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <libexplain/fopen.h>
#include <libexplain/getc.h>
#include <libexplain/output.h>

#include <lib/debug.h>
#include <lib/endof.h>
#include <lib/rcstring.h>
#include <lib/rcstring/accumulator.h>
#include <lib/sector_io/td0.h>


sector_io_td0::~sector_io_td0()
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    delete [] data;
}


static unsigned short
compute_crc(const unsigned char *p, size_t len, unsigned short crc)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    while (len)
    {
        --len;
        crc ^= (*p++ << 8);
        for (unsigned i = 0; i < 8; ++i)
            crc = (crc << 1) ^ ((crc & 0x8000) ? 0xA097 : 0);
    }
    return crc;
}

static rcstring fn;
static FILE *fp;

//
// LZSS decoder based in part on Haruhiko Okumura's LZHUF.C
//
#define SBSIZE      4096                // Size of Ring buffer
#define LASIZE      60                  // Size of Look-ahead buffer
#define THRESHOLD   2                   // Minimum match for compress

// Huffman coding parameters
#define N_CHAR   (256 - THRESHOLD + LASIZE)  // Character code (= 0..N_CHAR-1)
#define TSIZE    (N_CHAR * 2 - 1)            // Size of table
#define ROOT     (TSIZE - 1)                 // Root position
#define MAX_FREQ 0x8000    // Update when cumulativ frequency reaches this value

/// parent nodes (0..T-1) and leaf positions (rest)
static unsigned parent[TSIZE + N_CHAR];

/// pointers to child nodes (son[], son[] + 1)
static unsigned short son[TSIZE];

/// frequency table
static unsigned short freq[TSIZE + 1];

/// buffered bit count
static int Bits;

/// and left-aligned bit buffer
static int Bitbuff;

/// Getbyte check down-counter
static int GBcheck;

/// Ring buffer position
static int GBr;

/// Decoder index
static int GBi;

/// Decoder index
static int GBj;

/// Decoder index
static int GBk;

/// Decoder state
static bool GBstate;

/// End-of-file indicator
static bool Eof;

/// Advanced compression enabled
static bool Advcomp;

/// text buffer for match strings
static unsigned char ring_buff[SBSIZE + LASIZE - 1];

static unsigned char d_code[256] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0E, 0x0E, 0x0E, 0x0E,
    0x0F, 0x0F, 0x0F, 0x0F, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
    0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14, 0x14, 0x14,
    0x15, 0x15, 0x15, 0x15, 0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
    0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B, 0x1C, 0x1C, 0x1D, 0x1D,
    0x1E, 0x1E, 0x1F, 0x1F, 0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
    0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27, 0x28, 0x28, 0x29, 0x29,
    0x2A, 0x2A, 0x2B, 0x2B, 0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F
};

static unsigned char d_len[] =
{
    2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7
};

/**
  * Initialise the decompressor trees and state variables
  */
static void
init_decompress(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    unsigned i = 0;
    unsigned j = 0;
    for (size_t k = 0; k < SIZEOF(parent); ++k)
        parent[k] = 0;
    for (size_t k = 0; k < SIZEOF(son); ++k)
        son[k] = 0;
    for (size_t k = 0; k < SIZEOF(freq); ++k)
        freq[k] = 0;
    for (; i < N_CHAR; ++i)
    {
        // Walk up
        freq[i] = 1;
        son[i] = i + TSIZE;
        parent[i + TSIZE] = i;
    }

    while (i <= ROOT)
    {
        // Back down
        freq[i] = freq[j] + freq[j + 1];
        son[i] = j;
        parent[j] = parent[j + 1] = i++;
        j += 2;
    }

    memset(ring_buff, ' ', sizeof(ring_buff));
    Advcomp = true;
    freq[TSIZE] = 0xFFFF;
    parent[ROOT] = 0;
    Bitbuff = 0;
    Bits = 0;
    GBr = SBSIZE - LASIZE;
}

/**
 * Increment frequency tree entry for a given code
 */
static void
update(int c)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    unsigned i;
    unsigned j;
    unsigned k;
    unsigned f;
    unsigned l;

    if (freq[ROOT] == MAX_FREQ)
    {
        DEBUG(3, "tree full");
        // Tree is full - rebuild
        // Halve cumulative freq for leaf nodes
        for (i = j = 0; i < TSIZE; ++i)
        {
            if (son[i] >= TSIZE)
            {
                freq[j] = (freq[i] + 1) / 2;
                son[j] = son[i];
                ++j;
            }
        }

        // make a tree - first connect children nodes
        for (i = 0, j = N_CHAR; j < TSIZE; i += 2, ++j)
        {
            k = i + 1;
            f = freq[j] = freq[i] + freq[k];
            for (k = j - 1; f < freq[k]; --k);
            ++k;
            l = (j - k) * sizeof(freq[0]);

            memmove(&freq[k + 1], &freq[k], l);
            freq[k] = f;
            memmove(&son[k + 1], &son[k], l);
            son[k] = i;
        }

        // Connect parent nodes
        for (i = 0; i < TSIZE; ++i)
        {
            if ((k = son[i]) >= TSIZE)
                parent[k] = i;
            else
                parent[k] = parent[k + 1] = i;
        }
    }

    DEBUG(3, "c = 0x%02X", c);
    c = parent[c + TSIZE];
    for (;;)
    {
        DEBUG(3, "c = 0x%02X", c);
        ++freq[c];
        k = freq[c];
        // Swap nodes if necessary to maintain frequency ordering
        l = c + 1;
        if (k > freq[l])
        {
            while (k > freq[++l])
                ;
            --l;
            freq[c] = freq[l];
            freq[l] = k;
            i = son[c];
            parent[i] = l;
            if (i < TSIZE)
                parent[i + 1] = l;
            j = son[l];
            parent[j] = c;
            son[l] = i;
            if (j < TSIZE)
                parent[j + 1] = c;
            son[c] = j;
            c = l;
        }
        c = parent[c];
        if (c == 0)
            break;
    }
    DEBUG(3, "return");
}

/**
 * Get a byte from the input file and flag Eof at end
 */
static unsigned char
get_char_raw(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    int c = explain_getc_or_die(fp);
    if (c == EOF)
    {
        explain_output_error_and_die("%s: premature end-of-file", fn.c_str());
        c = 0;
        Eof = true;
    }
    DEBUG(3, "return 0x%02X", c);
    return c;
}

/**
 * Get a single bit from the input stream
 */
static unsigned
get_bit(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    if (!Bits--)
    {
        Bitbuff |= get_char_raw() << 8;
        Bits = 7;
    }

    unsigned short t = (Bitbuff >> 15) & 1;
    Bitbuff <<= 1;
    DEBUG(3, "return %d", t);
    return t;
}

/**
 * Get a byte from the input stream - NOT bit-aligned
 */
static unsigned char
get_unaligned_byte(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    if (Bits < 8)
        Bitbuff |= get_char_raw() << (8 - Bits);
    else
        Bits -= 8;

    unsigned char t = Bitbuff >> 8;
    Bitbuff <<= 8;
    DEBUG(3, "return 0x%02X", t);
    return t;
}

/**
 * Decode a character value from table
 */
static unsigned
decode_char(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);

    //
    // search the tree from the root to leaves.
    // choose node #(son[]) if input bit == 0
    // choose node #(son[]+1) if input bit == 1
    //
    unsigned short c = ROOT;
    for (;;)
    {
        c = son[c];
        if (c >= TSIZE)
            break;
        c += get_bit();
    }

    c -= TSIZE;
    update(c);
    DEBUG(3, "return 0x%02X", c);
    return c;
}

/**
 * Decode a compressed string index from the table
 */
static unsigned
decode_position(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    unsigned i;
    unsigned j;
    unsigned c;

    // Decode upper 6 bits from given table
    i = get_unaligned_byte();
    c = d_code[i] << 6;

    // input lower 6 bits directly
    j = d_len[i >> 4];
    while (--j)
        i = (i << 1) | get_bit();

    return (i & 0x3F) | c;
}


/**
 * Get a byte from the input file - decompress if required
 *
 * This implements a state machine to perform the LZSS decompression
 * allowing us to decompress the file "on the fly", without having to
 * have it all in memory.
 */
static unsigned char
get_byte(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    --GBcheck;
    if (!Advcomp)
    {
        // No compression
        return get_char_raw();
    }

    for (;;)
    {
        // Decompressor state machine
        if (Eof)
            return EOF;
        if (!GBstate)
        {
            // Not in the middle of a string
            unsigned c = decode_char();
            if (c < 256)
            {
                // Direct data extraction
                ring_buff[GBr++] = c;
                GBr &= (SBSIZE - 1);
                return c;
            }

            // Begin extracting a compressed string
            GBstate = true;
            GBi = (GBr - decode_position() - 1) & (SBSIZE - 1);
            GBj = c - 255 + THRESHOLD;
            GBk = 0;
        }
        if (GBk < GBj)
        {
            // Extract a compressed string
            unsigned c = ring_buff[(GBk + GBi) & (SBSIZE - 1)];
            ++GBk;
            ring_buff[GBr] = c;
            GBr = (GBr + 1) & (SBSIZE - 1);
            return c;
        }

        // Reset to non-string state
        GBstate = false;
    }
}


static unsigned char
get_byte(unsigned &crc)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    unsigned char uc = get_byte();
    crc = compute_crc(&uc, 1, crc);
    return uc;
}


static unsigned short
get_word(void)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    unsigned char c1 = get_byte();
    unsigned char c2 = get_byte();
    return (c1 | (c2 << 8));
}


static unsigned short
get_word(unsigned &crc)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    unsigned char c1 = get_byte(crc);
    unsigned char c2 = get_byte(crc);
    return (c1 | (c2 << 8));
}


sector_io_td0::sector_io_td0(const rcstring &a_filename) :
    filename(a_filename),
    data_size(0),
    data(0)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);

    //
    // Open the file
    //
    fn = filename;
    fp = explain_fopen_or_die(filename.c_str(), "rb");

    Advcomp = false;
    unsigned comp_crc = 0;
    int c1 = get_byte(comp_crc);
    int c2 = get_byte(comp_crc);
    int c3 = get_byte(comp_crc);
    bool enable_decompress = false;
    if (c1 == 'T' && c2 == 'D' && c3 == 0)
        enable_decompress = false;
    if (c1 == 't' && c2 == 'd' && c3 == 0)
        enable_decompress = true;
    else
    {
        explain_output_error_and_die
        (
            "%s: wrong magic number",
            filename.c_str()
        );
    }

    int check_sequence = get_byte(comp_crc);
    DEBUG(2, "Check sequence = 0x%02X\n", check_sequence);
    int td_version = get_byte(comp_crc);
    DEBUG(1, "Teledisk version = %x.%x\n", td_version >> 4, td_version & 15);
    int data_rate = get_byte(comp_crc);
    DEBUG(2, "Data rate = 0x%02X\n", data_rate);
    int drive_type = get_byte(comp_crc);
    DEBUG(2, "Drive type = 0x%02X\n", drive_type);
    int stepping = get_byte(comp_crc);
    DEBUG(2, "Stepping = 0x%02X\n", stepping);
    int dos_allocation_flag = get_byte(comp_crc);
    DEBUG(2, "DOS allocation flag = 0x%02X\n", dos_allocation_flag);
    int sides = get_byte(comp_crc);
    DEBUG(2, "Sides = 0x%02X\n", sides);
    unsigned file_crc = get_word();

    if (comp_crc != file_crc)
    {
        explain_output_error_and_die
        (
            "%s: header CRC mismatch (calculated 0x%04X, "
                "header contains 0x%04X",
            filename.c_str(),
            comp_crc,
            file_crc
        );
    }

    if (enable_decompress)
        init_decompress();

    //
    // We guess the data size from the drive type.
    // Is it only a *guess* we may have to adjust it.
    //
    switch (drive_type)
    {
    case 0:
        // 1.2MB "5.25 96 tpi disk in 48 tpi drive"
        data_size = 50 * 48 * 512;
        break;

    case 1:
        // 360kB "5.25"
        data_size = 20 * 36 * 512;
        break;

    case 2:
        // 1.2MB "5.25 48-tpi"
        data_size = 78 * 26 * 128;
        break;

    case 3:
        // 720kB "3.5"
        data_size = 40 * 36 * 512;
        break;

    case 4:
        // 1.44MB "3.5"
        data_size = 80 * 36 * 512;
        break;

    case 5:
        // "8-inch"
        data_size = 77 * 26 * 128;
        break;

    case 6:
        // "3.5"
        data_size = 80 * 36 * 512;
        break;

    default:
        explain_output_error_and_die
        (
            "%s: drive type %02X unknown",
            filename.c_str(),
            drive_type
        );
    }
    data = new unsigned char [data_size];
    data_pos = 0;

    //
    // Read the optional header comment.
    //
    if (stepping & 0x80)
    {
        DEBUG(2, "read header comment");
        file_crc = get_word();
        comp_crc = 0;
        unsigned len = get_word(comp_crc);
        get_byte(comp_crc);
        get_byte(comp_crc);
        get_byte(comp_crc);
        get_byte(comp_crc);
        get_byte(comp_crc);
        get_byte(comp_crc);
        rcstring_accumulator ac;
        while (len > 0)
        {
            unsigned char c = get_byte(comp_crc);
            if (c == 0)
                c = '\n';
            ac.push_back(c);
            --len;
        }
        DEBUG(2, "header comment = %s", ac.mkstr().quote_c().c_str());
        if (file_crc != comp_crc)
        {
            explain_output_error_and_die
            (
                "%s: comment CRC mismatch (calculated 0x%04X, "
                    "comment header contains 0x%04X",
                filename.c_str(),
                comp_crc,
                file_crc
            );
        }
    }

    //
    // Read all the tracks
    //
    while (read_track())
        ;
    fclose(fp);
    DEBUG(2, "data_pos = 0x%08X", int(data_pos));
    DEBUG(2, "data_size = 0x%08X", int(data_size));
    assert(data_pos <= data_size);
    if (data_pos < data_size)
    {
        unsigned char *new_data = new unsigned char [data_pos];
        memcpy(new_data, data, data_pos);
        data_size = data_pos;
        delete [] data;
        data = new_data;
    }
}


sector_io::pointer
sector_io_td0::create(const rcstring &a_filename, bool read_only)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    if (!read_only)
    {
        explain_output_error_and_die
        (
            "%s: the .TD0 format is only supported for read-only access",
            a_filename.c_str()
        );
    }
    return pointer(new sector_io_td0(a_filename));
}


bool
sector_io_td0::candidate(const rcstring &filnam)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    fp = fopen(filnam.c_str(), "rb");
    if (!fp)
        return false;
    int c1 = getc(fp);
    int c2 = getc(fp);
    int c3 = getc(fp);
    fclose(fp);
    fp = 0;
    return (((c1 == 'T' && c2 == 'D') || (c1 == 't' && c2 == 'd')) && c3 == 0);
}


bool
sector_io_td0::read_track(void)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);

    //
    // Read the track header
    //
    unsigned comp_crc = 0;
    int number_of_sectors = get_byte(comp_crc);
    if (number_of_sectors == 0xFF)
        return false;
    DEBUG(2, "Number of sectors = %d\n", number_of_sectors);
    int cylinder_number = get_byte(comp_crc);
    DEBUG(2, "Cylinder number = %d\n", cylinder_number);
    int side_head_number = get_byte(comp_crc);
    DEBUG(2, "Side/head number = %d\n", side_head_number);
    unsigned file_crc = get_byte();
    comp_crc &= 0xFF;
    if (file_crc != comp_crc)
    {
        explain_output_error_and_die
        (
            "%s: tracker header CRC error (calculated 0x%02X, "
                "track header has 0x%02X)",
            filename.c_str(),
            comp_crc,
            file_crc
        );
    }

    //
    // read track data
    //
    for (int j = 0; j < number_of_sectors; ++j)
        read_sector();

    return true;
}


void
sector_io_td0::read_sector(void)
{
    DEBUG(2, "%s", __PRETTY_FUNCTION__);
    unsigned comp_crc = 0;
    int cylinder_number = get_byte(comp_crc);
    DEBUG(2, "cylinder number = 0x%02X\n", cylinder_number);
    int side_head_number = get_byte(comp_crc);
    DEBUG(2, "side/head number = 0x%02X\n", side_head_number);
    int sector_number = get_byte(comp_crc);
    DEBUG(2, "sector number = 0x%02X\n", sector_number);
    int sector_size_code = get_byte(comp_crc);
    DEBUG(2, "sector_size_code = 0x%02X\n", sector_size_code);
    int flags = get_byte(comp_crc);
    DEBUG(2, "flags = 0x%02X\n", flags);
    unsigned char file_crc = get_byte();

    int sector_size = 128 << sector_size_code;
    DEBUG(2, "sector size = 0x%04X\n", sector_size);
    if (data_pos + sector_size > data_size)
    {
        //
        // our guess was wrong, double the size so that the chance of
        // needing to do it again is halved.  Sigma 2**-n == 1, so this
        // is actually an O(1) operation.
        //
        size_t new_data_size = data_size * 2;
        unsigned char *new_data = new unsigned char [new_data_size];
        memcpy(new_data, data, data_pos);
        delete [] data;
        data = new_data;
        data_size = new_data_size;
    }
    if ((flags & 0x30) == 0)
    {
        DEBUG(2, "sector data present");
        unsigned char *p = data + data_pos;
        unsigned data_block_size = get_word(comp_crc);
        DEBUG(2, "data_block_size = 0x%02X\n", data_block_size);
        unsigned encoding_method = get_byte(comp_crc);
        DEBUG(2, "encoding_method = 0x%02X\n", encoding_method);
        switch (encoding_method)
        {
        case 0:
            // Raw sector data
            DEBUG(2, "raw sector data");
            for (int j = 0; j < sector_size; ++j)
                *p++ = get_byte(comp_crc);
            break;

        case 1:
            // repeated 2-byte pattern
            DEBUG(2, "repeated 2-byte pattern");
            for (int j = sector_size; j > 0; )
            {
                int len = get_word(comp_crc);
                DEBUG(2, "len = %d", len);
                unsigned char d[2];
                d[0] = get_byte(comp_crc);
                d[1] = get_byte(comp_crc);
                DEBUG(2, "data = { 0x%02X, 0x%02X }", d[0], d[1]);
                while (len > 0)
                {
                    *p++ = d[0];
                    *p++ = d[1];
                    --len;
                    j -= 2;
                }
                assert(j >= 0);
            }
            break;

        case 2:
            // run length encoded data
            DEBUG(2, "run length encoded data");
            for (int j = sector_size; j > 0; )
            {
                unsigned char ctrl = get_byte(comp_crc);
                if (ctrl == 0)
                {
                    int n = get_byte(comp_crc);
                    while (n > 0)
                    {
                        *p++ = get_byte(comp_crc);
                        --j;
                        --n;
                    }
                }
                else
                {
                    int len = ctrl * 2;
                    int n = get_byte(comp_crc);
                    char buffer[512];
                    for (int k = 0; k < len; ++k)
                        buffer[k] = get_byte(comp_crc);
                    while (n > 0)
                    {
                        memcpy(p, buffer, len);
                        p += len;
                        j -= len;
                        --n;
                    }
                }
                assert(j >= 0);
            }
            break;

        default:
            explain_output_error_and_die
            (
                "%s: encoding method 0x%02X unknown",
                filename.c_str(),
                encoding_method
            );
        }
        DEBUG(2, "mark");
    }
    DEBUG(2, "mark");
    data_pos += sector_size;

#if 0
    if ((unsigned char)comp_crc != file_crc)
    {
        explain_output_error_and_die
        (
            "%s: sector CRC mismatch (calculated 0x%02X, "
                "sector header has 0x%02X)",
            filename.c_str(),
            (unsigned char)comp_crc,
            file_crc
        );
    }
#else
    (void)file_crc;
#endif
}


int
sector_io_td0::read(off_t offset, void *o_data, size_t size)
{
    DEBUG(2, "sector_io_td0::read(this = %p, offset = 0x%lX, data = %p, "
        "size = 0x%lX)", this, (long)offset, o_data, (long)size);
    if (offset < 0)
        return -EINVAL;
    if (offset + size > data_size)
        return -ENOSPC;
    memcpy(o_data, data + offset, size);
    return size;
}


int
sector_io_td0::read_sector(unsigned sector_number, void *o_data)
{
    DEBUG(2, "sector_io_td0::read_sector(this = %p, sector_number = %u, "
        "o_data = %p)", this, sector_number, o_data);
    off_t offset = (off_t)sector_number * 128;
    return read(offset, o_data, 128);
}


int
sector_io_td0::write_sector(unsigned sector_number, const void *o_data)
{
    DEBUG(2, "sector_io_td0::write_sector(this = %p, sector_number = %u, "
        "o_data = %p)", this, sector_number, o_data);
    return -EROFS;
}


int
sector_io_td0::write(off_t offset, const void *o_data, size_t size)
{
    DEBUG(2, "sector_io_td0::write(this = %p, offset = 0x%lX, o_data = %p, "
        "size = 0x%lX)", this, (long)offset, o_data, (long)size);
    return -EROFS;
}


int
sector_io_td0::size_in_sectors(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    DEBUG(3, "return %d\n", int(data_size >> 7));
    return (data_size >> 7);
}


unsigned
sector_io_td0::bytes_per_sector(void)
    const
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    return 128;
}


bool
sector_io_td0::is_read_only(void)
    const
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    return true;
}


unsigned
sector_io_td0::size_multiple_in_bytes(void)
    const
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    return 128;
}


int
sector_io_td0::sync(void)
{
    DEBUG(3, "%s", __PRETTY_FUNCTION__);
    return 0;
}


rcstring
sector_io_td0::get_filename(void)
    const
{
    return filename;
}
