//
// UCSD p-System filesystem in user space
// Copyright (C) 2006, 2007, 2010 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIB_RCSTRING_H
#define LIB_RCSTRING_H

#pragma interface "rcstring"

#include <lib/format_printf.h>
#include <lib/rcstring/gizzards.h>

/**
  * The rcstring class is used to represent a reference counted narrow
  * string with fast equality comparison.
  */
class rcstring
{
public:
    /**
      * The destructor.
      *
      * This destructor is <b>not</b> virtual, do not derive from this class.
      */
    ~rcstring()
    {
        rcstring_gizzards_free(ref);
        ref = 0;
    }

    /**
      * The default constructor.
      */
    rcstring() :
        ref(get_empty_ref())
    {
    }

    /**
      * The constructor.
      */
    rcstring(const char *arg) :
        ref(rcstring_gizzards_from_c(arg))
    {
    }

    /**
      * The constructor.
      */
    rcstring(const char *data, size_t len) :
        ref(rcstring_gizzards_n_from_c(data, len))
    {
    }

    /**
      * The constructor.
      */
    explicit rcstring(long);

    /**
      * The copy constructor.
      */
    rcstring(const rcstring &arg) :
        ref(rcstring_gizzards_copy(arg.ref))
    {
    }

    /**
      * The assignment operator.
      */
    rcstring &
    operator=(const rcstring &arg)
    {
        // The usual test is (this != &arg) but that is a subset of the
        // test you see here.
        if (ref != arg.ref)
        {
            rcstring_gizzards_free(ref);
            ref = rcstring_gizzards_copy(arg.ref);
        }
        return *this;
    }

    /**
      * The c_str method is used to obtain a pointer to the underlying C
      * string (guaranteed to the NUL terminated).
      */
    const char *
    c_str()
        const
    {
        return ref->rcstring_gizzards_text;
    }

    bool
    empty()
        const
    {
        return (ref->rcstring_gizzards_length == 0);
    }

    size_t
    size()
        const
    {
        return ref->rcstring_gizzards_length;
    }

    size_t
    length()
        const
    {
        return ref->rcstring_gizzards_length;
    }

    /**
      * \brief
      * join two strings together
      *
      * The rcstring_gizzards_catenate function is used to join two
      * strings togther to form a new string.  The are joined in the
      * order given.
      *
      * \param arg
      *     A string to be joined.  Will not be modified.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use
      *     rcstring_gizzards_free() when finished with.  The contents
      *     of the structure pointed to <b>shall not</b> be altered.
      */
    rcstring catenate(const rcstring &arg) const;

    rcstring
    operator+(const rcstring &arg)
        const
    {
        return catenate(arg);
    }

    rcstring &
    operator+=(const rcstring &arg)
    {
        if (!arg.empty())
        {
            rcstring temp = catenate(arg);
            *this = temp;
        }
        return *this;
    }

    /**
      * \brief
      *     test a boolean
      *
      * The rcstring_gizzards_bool function is used to test the value of
      * a string, as if it contained a number.  If it doesn't contain a
      * number, it is as if the strings was "1".
      *
      * \return
      *     False if the numeric value in the strings was zero, or the
      *     empty string.  True if the numeric value in the string was
      *     non-zero, or the string was non-numeric.
      */
    bool  is_zero() const;

    operator bool() const { return is_zero(); }

    /**
      * The logical negation operator.
      * Returns the negation of the bool() operator.
      */
    bool operator!() const { return !is_zero(); }

    /**
      * \brief
      * convert to upper case
      *
      * The rcstring_gizzards_upcase function is used to create a new
      * string where the lower case characters in the input string are
      * converted to upper case.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use
      *     rcstring_gizzards_free() when finished with.  The contents
      *     of the structure pointed to <b>shall not</b> be altered.
      */
    rcstring upcase() const;

    /**
      * \brief
      * convert to lower case
      *
      * The rcstring_gizzards_downcase function is used to create a new
      * string where the upper case characters in the input string are
      * converted to lower case.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use
      *     rcstring_gizzards_free() when finished with.  The contents
      *     of the structure pointed to <b>shall not</b> be altered.
      */
    rcstring downcase() const;

    /**
      * \brief
      * convert to title case
      *
      * The rcstring_gizzards_capitalize function is used to create a
      * new string where the first letter or each word of the inopuyt
      * string are upper case, and the remaining letters in each word
      * are lower case.  (Sometimes called Title Case.)
      *
      * \returns
      *     a pointer to a string in dynamic memory.
      */
    rcstring capitalize() const;

    /**
      * \brief
      * extract a field
      *
      * The rcstring_gizzards_field function is used to extract the \a
      * nth field, where each field is separated by the \a sep string.
      *
      * \param sep
      *     The string which separates each field.
      * \param nth
      *     The number of the field to be extracted.  Zero based.
      *     If too high, the emtry string is returned.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use
      *     rcstring_gizzards_free() when finished with.  The contents
      *     of the structure pointed to <b>shall not</b> be altered.
      */
    rcstring field(char sep, int nth) const;

    /**
      * \brief
      * test string equality
      *
      * The rcstring_gizzards_equal function is used to test to see if
      * two strings are exactly the same.
      *
      * \param arg
      *     A string to be compared.  Will not be modified.
      *
      * \note
      *     Users shall always write code as if they did not know that a
      *     string equality test is a pointer equality test.
      *
      * \return
      *     Non-zero if the strings are equal,
      *     zero if the strings are unequal.
      */
    bool
    equal(const rcstring &arg)
        const
    {
        return rcstring_gizzards_equal(ref, arg.ref);
    }

    bool
    operator==(const rcstring &arg)
        const
    {
        return rcstring_gizzards_equal(ref, arg.ref);
    }

    bool
    operator!=(const rcstring &arg)
        const
    {
        return !rcstring_gizzards_equal(ref, arg.ref);
    }

    bool operator<(const rcstring &arg) const;
    bool operator<=(const rcstring &arg) const;
    bool operator>(const rcstring &arg) const;
    bool operator>=(const rcstring &arg) const;

    /**
      * \brief
      * quote shell meta-characters
      *
      * The quote_shell method is used to create a new string which
      * quotes the shell meta-characters in the input string.
      */
    rcstring quote_shell() const;

    /**
      * \brief
      * quote C meta-characters
      *
      * The quote_c method is used to create a new string which quotes
      * the C meta-characters in the input string, and adds double
      * quotes.
      */
    rcstring quote_c() const;

    /**
      * \brief
      * remove excess white space
      *
      * The rcstring_gizzards_trim function is used to remove white
      * space from the beginning and end of the string, and replace all
      * other runs of one or more white space characters with a single
      * space.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use
      *     rcstring_gizzards_free() when finished with.  The contents
      *     of the structure pointed to <b>shall not</b> be altered.
      */
    rcstring trim() const;

    /**
      * \brief
      * check is valid
      *
      * The rcstring_gizzards_validate function is used to confirm that
      * the given string pointer, \a str, points to a valid string.
      * Usually used for debugging, often in assert()s.
      *
      * \return
      *     Non-zero if valid, zero if invalid.
      */
    bool
    valid()
        const
    {
        return (ref && ref->validate());
    }

    /**
      * The starts_with method is used to test whether this string
      * starts with the given prefix.
      *
      * @param prefix
      *     The string to test for.
      */
    bool starts_with(const rcstring &prefix) const;

    /**
      * The starts_with_nc method is used to test whether this string
      * starts with the given prefix.  The comarison is perform base
      * in-sensitive.
      *
      * @param prefix
      *     The string to test for.
      */
    bool starts_with_nc(const rcstring &prefix) const;

    /**
      * The ends_with method is used to test whether this string
      * ends with the given suffix.
      *
      * @param suffix
      *     The string to test for.
      * @returns
      *     true if this string ends with the given suffix, false if it
      *     does not.
      */
    bool ends_with(const rcstring &suffix) const;

    /**
      * The ends_with_nc method is used to test whether this string
      * ends with the given suffix.  The comparison is case INsensitive.
      *
      * @param suffix
      *     The string to test for.
      * @returns
      *     true if this string ends with the given suffix, false if it
      *     does not.
      */
    bool ends_with_nc(const rcstring &suffix) const;

    /**
      * The gmatch function is used to match the string against a file
      * globbing pattern.
      */
    bool gmatch(const char *pattern) const;

    /**
      * The identifier method is used to convert all non-C-identifier
      * characters in the string to underscores.  The intention is to
      * create a valid C identifier from the string.
      */
    rcstring identifier() const;

    /**
      * The replace method may be used to alter a string by replacing
      * one constant substring with another.
      *
      * @note
      *     The replacement is <i>not</i> done <i>in situ</i>.  The original
      *     string is unaltered.
      *
      * @param lhs
      *     The substring to look for.
      * @param rhs
      *     The substring to replace \a lhs if found.
      * @param maximum
      *     The maximum number of times to perform the replacement.
      *     Defaults to "infinity".
      * @returns
      *     A new string with the replacements made.
      */
    rcstring replace(const rcstring &lhs, const rcstring &rhs, int maximum = -1)
        const;

    /**
      * The indexing operator is used to extract the nth character of a
      * string.  Indexes out of range will result in the NUL character
      * (0) being returned.
      *
      * @param n
      *     The character to extract.  Zero based.
      * @returns
      *     The character requested, or NUL (0) if the index is out
      *     of range.
      */
    char
    operator[](size_t n)
        const
    {
        return (n < size() ? ref->rcstring_gizzards_text[n] : '\0');
    }

    /**
      * The clear method is used to delete to contents of the string,
      * and replace it with the empty string.
      */
    void clear();

    typedef rcstring_gizzards::hash_t hash_t;

    hash_t
    get_hash()
        const
    {
        return ref->rcstring_gizzards_hash;
    }

    /**
      * The back method may be used to obtain the last character in the
      * string, or the NUL character if the string is empty.
      */
    char
    back()
        const
    {
        return (empty() ? '\0' : c_str()[size() - 1]);
    }

    /**
      * The front method may be used to obtain the first character in the
      * string, or the NUL character if the string is empty.
      */
    char
    front()
        const
    {
        return (c_str()[0]);
    }

    /**
      * The substring method is used to extract a substring from this
      * string.
      *
      * @param start
      *     This is the location that the substrings starts withing this
      *     string, counting from zero.  If negative, it is counted
      *     backwards from the end of the string.
      * @param length
      *     The length of the substring to extract.  If positiove,
      *     extends to the right.  If negative, extends to the left.
      */
    rcstring substring(long start, long length) const;

    /**
      * The dirname funtion extracts the directory component from
      * a pathname string.  In the usual case, dirname returns the
      * string up to, but not including, the final "/". and Trailing
      * "/"characters are not counted as part of the pathname.
      *
      * If path does not contain a slash, dirname returns the string "."
      * while If path is the string "/", then dirname returns the string
      * "/". If path is an empty string, then dirname returns the string
      * ".".
      */
    rcstring dirname() const;

    /**
      * The basename function extracts the terminal filename component
      * from a path string.  In the usual case, basename returns the
      * component following the final "/".  Trailing "/" characters are
      * not counted as part of the pathname.
      *
      * If path does not contain a slash, basename returns a copy of
      * path.  If path is the string "/", then basename returns the
      * string "/". If path is an empty string, then basename returns
      * ".".
      */
    rcstring basename() const;

    /**
      * The printf method is used to appand a formatted string to the
      * buffer.  See printf(3) for mor einformation.
      */
    static rcstring printf(const char *fmt, ...)            FORMAT_PRINTF(1, 2);

private:
    /**
      * The ref instance variable is used to remember the location of
      * the object common to all of the references.
      * The is <i>never</i> the NULL pointer.
      */
    rcstring_gizzards *ref;

    /**
      * The get_empty_ref() class method is used to get a
      * pointer to an underlying string object of length zero.
      */
    static rcstring_gizzards *get_empty_ref();
};

inline rcstring
operator+(const char *lhs, const rcstring &rhs)
{
    return rcstring(lhs).catenate(rhs);
}

inline rcstring
operator+(const rcstring &lhs, const char *rhs)
{
    return lhs.catenate(rcstring(rhs));
}

#endif // LIB_RCSTRING_H
