//
// UCSD p-System filesystem in user space
// Copyright (C) 2006-2008 Peter Miller
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

#ifndef LIB_DIRECTORY_ENTRY_LIST_H
#define LIB_DIRECTORY_ENTRY_LIST_H

#include <cstddef>

#include <lib/directory/entry.h>

class rcstring; // forward

/**
  * The directory_entry_list class is used to represent an ordered list
  * of directory entries.
  */
class directory_entry_list
{
public:
    /**
      * The destructor.
      */
    virtual ~directory_entry_list();

    /**
      * The default constructor.
      */
    directory_entry_list();

    /**
      * The size method is used to obtain the number of entries in the
      * list.
      */
    size_t size() const { return length; }

    /**
      * The empty method is used to determine whther or not the
      * directory entry list is empty (has no entries).
      */
    bool empty() const { return (length == 0); }

    /**
      * The push_back method is used to add another directory entry to
      * the end of the list.
      *
      * @param dep
      *     Pointer to the directory entry to add.  Must be in dynamic
      *     memory.  Will be deleted by the destructor.
      */
    void push_back(directory_entry::pointer dep);

    /**
      * The nth function is used to obtain the N'th entry in the list.
      *
      * @param n
      *     The ordinal number of the entry of interest.
      * @returns
      *     a pointer to the directory entry, or NULL if you have asked
      *     for an entry beyond the end of the list.
      */
    directory_entry::pointer nth(size_t n) const;

    /**
      * The find method is used to locate a drectory entry by name.
      *
      * @param filename
      *     The name of the file to look for.
      * @returns
      *     a pointer to the directory entry of interest, or the NULL
      *     pointer if the named file is not in the list.
      */
    directory_entry::pointer find(const rcstring &filename) const;

    directory_entry::pointer operator[](size_t n) const { return nth(n); }

    /**
      * The back method is used to obtain the last entry in the list.
      *
      * @returns
      *     a pointer to the directory entry, or NULL if the list is empty.
      */
    directory_entry::pointer back() const;

    /**
      * The index_of method is used to obtain the position in the list
      * of the given entry.
      *
      * @param dep
      *     The directory entry to look for.
      * @returns
      *     the index of the entry.
      * @note
      *     If the entry is not present in the list, the result is undefined.
      */
    size_t index_of(directory_entry::pointer dep) const;

    /**
      * The index_of method is used to obtain the position in the list
      * of the given entry.
      *
      * @param dep
      *     The directory entry to look for.
      * @returns
      *     the index of the entry.
      * @note
      *     If the entry is not present in the list, the result is undefined.
      */
    size_t index_of(directory_entry *dep) const;

    /**
      * The erase method is used to remove the given directory entry
      * from the list.  The entry will not be deleted, only removed from
      * the list.
      *
      * @param dep
      *     The directory entry of interest.
      * @returns
      *     true if the entry was in the list (and has been removed),
      *     and false if the entry was not in the list (and the list has
      *     not been changed).
      */
    bool erase(directory_entry::pointer dep);

    /**
      * The erase method is used to remove the given directory entry
      * from the list.  The entry will not be deleted, only removed from
      * the list.
      *
      * @param dep
      *     The directory entry of interest.
      * @returns
      *     true if the entry was in the list (and has been removed),
      *     and false if the entry was not in the list (and the list has
      *     not been changed).
      */
    bool erase(directory_entry *dep);

    /**
      * The sort_by_first_block method is used to sort the directory
      * entries by the number of the forst block of each directory
      * entry.
      */
    void sort_by_first_block();

private:
    /**
      * The length instance variable is used to remember how many
      * entries htere are in the list at present.
      */
    size_t length;

    /**
      * The maximum instance variable is used to remember how large the
      * \a list array was when it was allocated from dynamic memory.
      *
      * assert(length <= maximum);
      */
    size_t maximum;

    /**
      * The list instance variable is used to remember the base address
      * of a dynamically allocated array of pointers to directory
      * entries.
      */
    directory_entry::pointer *list;

    /**
      * The copy constructor.
      */
    directory_entry_list(const directory_entry_list &);

    /**
      * The assignment operator.
      */
    directory_entry_list &operator=(const directory_entry_list &);
};

#endif // LIB_DIRECTORY_ENTRY_LIST_H
