//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008, 2010, 2011 Peter Miller
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
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <getopt.h>
#include <libexplain/stat.h>
#include <libexplain/closedir.h>
#include <libexplain/opendir.h>
#include <libexplain/readdir.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <lib/debug.h>
#include <lib/directory.h>
#include <lib/directory/entry.h>
#include <lib/input/file.h>
#include <lib/input/psystem.h>
#include <lib/output/file.h>
#include <lib/output/psystem.h>
#include <lib/output/text_decode.h>
#include <lib/output/text_encode.h>
#include <lib/rcstring/list.h>
#include <lib/version.h>


static void
usage(void)
{
    const char *prog = explain_program_name_get();
    fprintf(stderr, "Usage: %s -f <disk.image> -l\n", prog);
    fprintf(stderr, "       %s -f <disk.image> -g <file.to.get>...\n", prog);
    fprintf(stderr, "       %s -f <disk.image> -p <file.to.put>...\n", prog);
    fprintf(stderr, "       %s -f <disk.image> -r <file.to.remove>...\n", prog);
    fprintf(stderr, "       %s -f <disk.image> --crunch\n", prog);
    fprintf(stderr, "       %s -f <disk.image> --system-volume\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


static mode_t
get_umask(void)
{
    mode_t um = umask(0);
    umask(um);
    return (um & 0777);
}


static bool all_binary;


static void
get(directory *volume, const rcstring &fspec)
{
    rcstring unix_filename(fspec);
    rcstring ucsd_filename(unix_filename.basename());
    const char *cp = strchr(fspec.c_str(), '=');
    if (cp)
    {
        unix_filename = rcstring(fspec.c_str(), cp - fspec.c_str());
        ucsd_filename = rcstring(cp + 1);
    }

    directory_entry::pointer dep = volume->find(ucsd_filename);
    if (!dep)
    {
        explain_output_error_and_die
        (
            "open %s:%s: %s",
            volume->get_volume_name().c_str(),
            ucsd_filename.c_str(),
            strerror(ENOENT)
        );
    }

    input::pointer in = input_psystem::create(dep);
    struct stat stbuf;
    in->fstat(stbuf);
    bool binary = all_binary || !dep->is_text_kind();
    output::pointer out = output_file::create(unix_filename, binary);
    if (!binary)
        out = output_text_decode::create(out);

    //
    // Copy the input to the output.
    //
    out->write(in);
    out->flush();

    //
    // Preserve the utimes (as far as possible, anyway).
    //
    struct timespec tv[2];
    tv[0].tv_sec = stbuf.st_atime;
    tv[0].tv_nsec = 0;
    tv[1].tv_sec = stbuf.st_mtime;
    tv[1].tv_nsec = 0;
    out->utime_ns(tv);
}


static void
get_dir(directory *volume, const rcstring &dir)
{
    directory_entry::pointer dep = volume->find("/");
    assert(dep);
    rcstring_list names;
    int err = dep->get_directory_entry_names(names);
    if (err < 0)
    {
        explain_output_error_and_die
        (
            "getdents %s: %s",
            volume->get_volume_name().c_str(),
            strerror(-err)
        );
    }
    for (size_t j = 0; j < names.size(); ++j)
    {
        rcstring name = names[j];
        rcstring path = dir + "/" + name.downcase();
        get(volume, path + "=" + name);
    }
}


static void
put(directory *volume, const rcstring &fspec)
{
    rcstring unix_filename(fspec);
    rcstring ucsd_filename(unix_filename.basename());
    const char *cp = strchr(fspec.c_str(), '=');
    if (cp)
    {
        ucsd_filename = rcstring(fspec.c_str(), cp - fspec.c_str());
        unix_filename = rcstring(cp + 1);
    }

    input::pointer in = input_file::create(unix_filename);
    struct stat st;
    in->fstat(st);

    directory_entry::pointer dep = volume->find(ucsd_filename);
    if (!dep)
    {
        dep = volume->find("/");
        assert(dep);
        mode_t mode = S_IFREG + (0666 & ~get_umask());
        int err = dep->mknod(ucsd_filename, mode, 0);
        if (err < 0)
        {
            explain_output_error_and_die
            (
                "create %s:%s: %s",
                volume->get_volume_name().c_str(),
                ucsd_filename.c_str(),
                strerror(-err)
            );
        }
        dep = volume->find(ucsd_filename);
        assert(dep);
    }

    output::pointer out = output_psystem::create(dep);
    if (!all_binary && dep->is_text_kind())
        out = output_text_encode::create(out);

    out->write(in);
    out->flush();

    //
    // Preserve the utimes (as far as possible, anyway).
    //
    struct timespec tv[2];
    tv[0].tv_sec = st.st_atime;
    tv[0].tv_nsec = 0;
    tv[1].tv_sec = st.st_mtime;
    tv[1].tv_nsec = 0;
    out->utime_ns(tv);
}


static void
put_dir(directory *volume, const rcstring &dir, bool skip_dot)
{
    DIR *dp = explain_opendir_or_die(dir.c_str());
    for (;;)
    {
        dirent *dep = explain_readdir_or_die(dp);
        if (!dep)
            break;
        rcstring name(dep->d_name);
        rcstring path = dir + "/" + name;
        struct stat st;
        explain_stat_or_die(path.c_str(), &st);
        if (skip_dot && name[0] == '.')
            continue;
        if (S_ISREG(st.st_mode))
            put(volume, name.upcase() + "=" + path);
    }
    explain_closedir_or_die(dp);
}


static void
remove(directory *volume, const rcstring &filename)
{
    directory_entry::pointer dep = volume->find(filename);
    if (!dep)
    {
        explain_output_error_and_die
        (
            "remove %s:%s: %s",
            volume->get_volume_name().c_str(),
            filename.c_str(),
            strerror(ENOENT)
        );
    }
    int err = dep->unlink();
    if (err < 0)
    {
        explain_output_error_and_die
        (
            "unlink %s:%s: %s",
            volume->get_volume_name().c_str(),
            filename.c_str(),
            strerror(-err)
        );
    }
}


static bool
is_a_directory(const rcstring &filename)
{
    struct stat st;
    if (stat(filename.c_str(), &st) < 0)
        return false;
    return S_ISDIR(st.st_mode);
}


static directory::sort_by_t
decode_sort_name(const rcstring &name)
{
    rcstring lc_name = name.downcase();
    if (lc_name == "block")
        return directory::sort_by_block;
    if (lc_name == "name")
        return directory::sort_by_name;
    if (lc_name == "date" || lc_name == "mtime")
        return directory::sort_by_date;
    if (lc_name == "size")
        return directory::sort_by_size;
    if (lc_name == "kind" || lc_name == "type")
        return directory::sort_by_kind;
    explain_output_error_and_die
    (
        "sort criterion %s unknown",
        name.quote_c().c_str()
    );
    return directory::sort_by_name;
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);
    int listing_flag = 0;
    bool get_flag = false;
    bool put_flag = false;
    bool crunch_flag = false;
    bool remove_flag = false;
    const char *disk_image_filename = 0;
    bool text_on_the_fly = false;
    bool skip_dot = true;
    bool wipe_flag = false;
    directory::sort_by_t sort_by = directory::sort_by_block;
    const char *boot_blocks = 0;
    bool check_for_system_volume = false;
    for (;;)
    {
        static struct option options[] =
        {
            { "all", 0, 0, 'A' }, // similar to ls(1)
            { "all-binary", 0, 0, 'B' },
            { "almost-all", 0, 0, 'A' }, // like ls(1)
            { "boot", 1, 0, 'b' },
            { "crunch", 0, 0, 'k' },
            { "debug", 0, 0, 'D' },
            { "defragment", 0, 0, 'k' },
            { "file", 1, 0, 'f' },
            { "get", 0, 0, 'g' },
            { "list", 0, 0, 'l' },
            { "no-skip-dot", 0, 0, 'A' },
            { "put", 0, 0, 'p' },
            { "remove", 0, 0, 'r' },
            { "sort", 1, 0, 's' },
            { "squeeze", 0, 0, 'k' },
            { "system-volume", 0, 0, 'S' },
            { "text", 0, 0, 't' },
            { "version", 0, 0, 'V' },
            { "wipe-unused", 0, 0, 'w' },
            { 0, 0, 0, 0 }
        };
        int c = getopt_long(argc, argv, "ABb:Df:gklprSs:tVw", options, 0);
        if (c == EOF)
            break;
        switch (c)
        {
        case 'A':
            skip_dot = false;
            break;

        case 'B':
            all_binary = true;
            break;

        case 'b':
            boot_blocks = optarg;
            break;

        case 'D':
            ++debug_level;
            break;

        case 'f':
            disk_image_filename = optarg;
            break;

        case 'g':
            get_flag = true;
            break;

        case 'k':
            crunch_flag = true;
            break;

        case 'l':
            ++listing_flag;
            break;

        case 'p':
            put_flag = true;
            break;

        case 'r':
            remove_flag = true;
            break;

        case 'S':
            check_for_system_volume = true;
            break;

        case 's':
            sort_by = decode_sort_name(optarg);
            break;

        case 't':
            // Have the file system implementation transparently translate
            // text files as they are read and written.
            text_on_the_fly = true;

            // Because the file system is doing the text file translations, no
            // further processed is to be done in this program.
            all_binary = true;
            break;

        case 'V':
            version_print();
            return 0;

        case 'w':
            wipe_flag = true;
            break;

        default:
            usage();
        }
    }
    if (!disk_image_filename)
    {
        if (optind >= argc)
            explain_output_error_and_die("no disk image file name specified");
        // Ugly, but probably what the user meant.
        disk_image_filename = argv[optind++];
    }
    if (boot_blocks && put_flag + get_flag != 1)
    {
        explain_output_error_and_die
        (
            "the --boot option must be accompanied by exactly one of the --get "
            "or --put options"
        );
    }
    if
    (
        !boot_blocks
    &&
        !listing_flag
    &&
        !get_flag
    &&
        !put_flag
    &&
        !remove_flag
    &&
        !crunch_flag
    &&
        !wipe_flag
    )
        listing_flag = 1;
    if (get_flag + put_flag + remove_flag > 1)
        usage();
    if
    (
        optind < argc
    &&
        !check_for_system_volume
    &&
        !get_flag
    &&
        !put_flag
    &&
        !remove_flag
    )
        usage();

    //
    // Open the volume, and make sure it has the right format.
    //
    bool read_only_flag =
        !put_flag && !remove_flag && !crunch_flag && !wipe_flag;
    directory *volume = directory::factory(disk_image_filename, read_only_flag);
    if (check_for_system_volume)
    {
        if (!volume->check_for_system_files())
            return EXIT_FAILURE;
        return EXIT_SUCCESS;
    }
    if (text_on_the_fly)
        volume->convert_text_on_the_fly();

    //
    // Chew on the volume, as requested.
    //
    while (optind < argc)
    {
        rcstring filename = argv[optind++];
        if (put_flag)
        {
            if (is_a_directory(filename))
                put_dir(volume, filename, skip_dot);
            else
                put(volume, filename);
        }
        if (get_flag)
        {
            if (is_a_directory(filename))
                get_dir(volume, filename);
            else
                get(volume, filename);
        }
        if (remove_flag)
            remove(volume, filename);
    }

    //
    // "Crunch" means to move all of the files as far forward in the
    // volume as possible.
    //
    if (crunch_flag)
        volume->crunch();

    //
    // The wipe-unused flags menas to make sure that all blocks not
    // accounted for in the directory are reset to zero, wiping any
    // "left over" content.  Not only is this more secure (things you
    // didn't intent to stay on this disk don't) but this disk images
    // compress better, too.
    //
    if (wipe_flag)
        volume->wipe_unused();

    //
    // Show the contents of the volume.
    //
    if (listing_flag)
        volume->print_listing(listing_flag >= 2, sort_by);

    //
    // Set the boot sectors (flat binary file).
    //
    if (boot_blocks)
    {
        if (put_flag)
            volume->set_boot_blocks(boot_blocks);
        else
            volume->get_boot_blocks(boot_blocks);
    }

    //
    // Close down the volume.
    // This may do essential flush operations.
    //
    delete volume;
    volume = 0;

    //
    // Report success
    //
    return 0;
}
