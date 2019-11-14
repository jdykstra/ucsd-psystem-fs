//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008, 2010 Peter Miller
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lib/debug.h>
#include <lib/directory.h>
#include <lib/directory/entry.h>


directory_entry::~directory_entry()
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
}


directory_entry::directory_entry(directory *a_parent) :
    parent(a_parent)
{
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
}


unsigned
directory_entry::get_word(const unsigned char *data)
    const
{
    return parent->get_word(data);
}


time_t
directory_entry::get_date(const unsigned char *data)
    const
{
    unsigned x = parent->get_word(data);
    if (x == 0)
        return 0;

    //
    // DATEREC = PACKED RECORD
    //         MONTH: 0..12;          (*0 IMPLIES DATE NOT MEANINGFUL*)
    //         DAY: 0..31;            (*DAY OF MONTH*)
    //         YEAR: 0..100           (*100 IS TEMP DISK FLAG*)
    //     END (*DATEREC*);
    //
    //   F   E   D   C   B   A   9   8   7   6   5   4   3   2   1   0
    // +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // |           year            |        day        |     month     |
    // +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //
    static struct tm zero;
    struct tm tmp = zero;
    tmp.tm_mon = (x & 0x0F) - 1;
    tmp.tm_mday = (x >> 4) & 0x1F;
    tmp.tm_year = (x >> 9) & 0x7F;
    if (tmp.tm_year < 70)
        tmp.tm_year += 100;
    return mktime(&tmp);
}


void
directory_entry::put_word(unsigned char *data, unsigned value)
    const
{
    parent->put_word(data, value);
}


void
directory_entry::put_date(unsigned char *data, time_t value)
    const
{
    //
    // DATEREC = PACKED RECORD
    //         MONTH: 0..12;          (*0 IMPLIES DATE NOT MEANINGFUL*)
    //         DAY: 0..31;            (*DAY OF MONTH*)
    //         YEAR: 0..100           (*100 IS TEMP DISK FLAG*)
    //     END (*DATEREC*);
    //
    //   F   E   D   C   B   A   9   8   7   6   5   4   3   2   1   0
    // +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // |           year            |        day        |     month     |
    // +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //
    struct tm *tmp = localtime(&value);
    unsigned x =
        (
            (tmp->tm_mon + 1)
        +
            (tmp->tm_mday << 4)
        +
            ((tmp->tm_year % 100) << 9)
        );
    parent->put_word(data, x);
}


int
directory_entry::getattr(struct stat *)
{
    return -ENOSYS;
}


int
directory_entry::readlink(char *, size_t)
{
    return -ENOSYS;
}


int
directory_entry::mknod(const rcstring &, mode_t, dev_t)
{
    return -ENOSYS;
}


int
directory_entry::mkdir(const rcstring &, mode_t)
{
    return -ENOSYS;
}


int
directory_entry::unlink()
{
    return -ENOSYS;
}


int
directory_entry::rmdir()
{
    return -ENOSYS;
}


int
directory_entry::symlink(const rcstring &, const char *)
{
    return -ENOSYS;
}


int
directory_entry::rename(const char *)
{
    return -ENOSYS;
}


int
directory_entry::link(const rcstring &, directory_entry::pointer )
{
    return -ENOSYS;
}


int
directory_entry::chmod(mode_t)
{
    return -ENOSYS;
}


int
directory_entry::chown(uid_t, gid_t)
{
    return -ENOSYS;
}


int
directory_entry::truncate(off_t size)
{
    return -ENOSYS;
}


int
directory_entry::utime_ns(const struct timespec *)
{
    return -ENOSYS;
}


int
directory_entry::open()
{
    return -ENOSYS;
}


int
directory_entry::read(off_t offset, void *data, size_t nbytes)
    const
{
    return -ENOSYS;
}


int
directory_entry::write(off_t offset, const void *data, size_t nbytes)
{
    return -ENOSYS;
}


int
directory_entry::statfs(struct statvfs *st)
{
    return parent->statfs(st);
}


int
directory_entry::flush()
{
    //
    // If they got this far, it means that open() is supported, so just
    // quietly do nothing when the system asks for a flush, and the
    // derived class doesn't override this method.
    //
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    return 0;
}


int
directory_entry::release()
{
    //
    // If they got this far, it means that open() is supported, so just
    // quietly do nothing when the system asks for a flush, and the
    // derived class doesn't override this method.
    //
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    return 0;
}


int
directory_entry::fsync(int)
{
    //
    // If they got this far, it means that open() is supported, so just
    // quietly do nothing when the system asks for a flush, and the
    // derived class doesn't override this method.
    //
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    return 0;
}


int
directory_entry::setxattr(const char *, const char *, size_t, int)
{
    return -ENOSYS;
}


int
directory_entry::getxattr(const char *, char *, size_t)
{
    return -ENOSYS;
}


int
directory_entry::listxattr(char *, size_t)
{
    return -ENOSYS;
}


int
directory_entry::removexattr(const char *)
{
    return -ENOSYS;
}


int
directory_entry::opendir()
{
    return -ENOTDIR;
}


int
directory_entry::get_directory_entry_names(rcstring_list &)
{
    return -ENOTDIR;
}


int
directory_entry::releasedir()
{
    //
    // If they got this far, it means that opendir() is supported, so
    // just quietly do nothing when the system asks for a releasedir,
    // and the derived class doesn't override this method.
    //
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    return 0;
}


int
directory_entry::fsyncdir(int)
{
    //
    // If they got this far, it means that mknod() or mkdir() or
    // unlink() or rmdir() or symlink() or rename() or link() is
    // supported, so just quietly do nothing when the system asks for a
    // fsyncdir, and the derived class doesn't override this method.
    //
    DEBUG(1, "%s", __PRETTY_FUNCTION__);
    return 0;
}


const char *
directory_entry::dfkind_name(dfkind_t dfkind)
{
    switch (dfkind)
    {
    case untypedfile:
        return "untypedfile";

    case xdskfile:
        return "xdskfile";

    case codefile:
        return "codefile";

    case textfile:
        return "textfile";

    case infofile:
        return "infofile";

    case datafile:
        return "datafile";

    case graffile:
        return "graffile";

    case fotofile:
        return "fotofile";

    case securedir:
        return "securedir";
    }
    return "????";
}


directory_entry::dfkind_t
directory_entry::dfkind_from_extension(const rcstring &name)
{
    DEBUG(1, "%s {", __PRETTY_FUNCTION__);
    DEBUG(2, "name = %s", name.quote_c().c_str());
    rcstring name_nc = name.downcase();

    struct table_t
    {
        rcstring extn;
        dfkind_t kind;
    };

    static const table_t table2[] =
    {
        { "6500.errors", datafile },
        { "6500.opcodes", datafile },
        { "6502.errors", datafile },
        { "6502.opcodes", datafile },
        { "system.apple", datafile },
        { "system.assmbler", codefile },
        { "system.charset", datafile },
        { "system.compiler", codefile },
        { "system.editor", codefile },
        { "system.filer", codefile },
        { "system.library", codefile },
        { "system.linker", codefile },
        { "system.miscinfo", datafile },
        { "system.pascal", codefile },
        { "system.syntax", textfile },
    };
    for (const table_t *tp = table2; tp < ENDOF(table2); ++tp)
    {
        if (name_nc == tp->extn)
        {
            DEBUG(1, "return %s; }", dfkind_name(tp->kind));
            return tp->kind;
        }
    }

    static const table_t table[] =
    {
        { ".a", codefile },
        { ".back", textfile },
        { ".backup", textfile },
        { ".bin", datafile },
        { ".binary", datafile },
        { ".bmp", fotofile },
        { ".c", textfile },
        { ".c++", textfile },
        { ".cc", textfile },
        { ".code", codefile },
        { ".conf", textfile },
        { ".cpp", textfile },
        { ".csv", textfile },
        { ".cxx", textfile },
        { ".dat", datafile },
        { ".data", datafile },
        { ".dll", codefile },
        { ".exe", codefile },
        { ".foto", fotofile },
        { ".gif", fotofile },
        { ".graf", graffile },
        { ".graph", graffile },
        { ".h", textfile },
        { ".h++", textfile },
        { ".hh", textfile },
        { ".hpp", textfile },
        { ".hxx", textfile },
        { ".ico", fotofile },
        { ".icon", fotofile },
        { ".info", infofile },
        { ".jpeg", fotofile },
        { ".jpg", fotofile },
        { ".lib", codefile },
        { ".miscinfo", datafile },
        { ".o", codefile },
        { ".obj", codefile },
        { ".p", textfile },
        { ".pas", textfile },
        { ".pascal", textfile },
        { ".photo", fotofile },
        { ".png", fotofile },
        { ".raw", datafile },
        { ".so", codefile },
        { ".svg", graffile },
        { ".text", textfile },
        { ".txt", textfile },
        { ".xdsk", xdskfile },
    };

    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (name_nc.ends_with(tp->extn))
        {
            DEBUG(1, "return %s; }", dfkind_name(tp->kind));
            return tp->kind;
        }
    }

    if (name_nc.starts_with("system."))
    {
        DEBUG(1, "return codefile; }");
        return codefile;
    }
    DEBUG(1, "return datafile; }");
    return datafile;
}


rcstring
directory_entry::get_full_name()
    const
{
    return (get_parent()->get_volume_name() + ":" + get_name());
}
