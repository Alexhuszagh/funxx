//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief CSV base writer.
 */

#pragma once

#include <pycpp/csv/punct.h>
#include <pycpp/stl/fstream.h>
#include <pycpp/stl/memory.h>
#include <pycpp/stl/ostream.h>
#include <pycpp/stl/sstream.h>
#include <pycpp/stl/string_view.h>

PYCPP_BEGIN_NAMESPACE

// OBJECTS
// -------

/**
 *  \brief Generic writer for CSV file.
 *
 *  This reader is mostly analogous to Python's `csv.writer` object, with
 *  one notable exception: it overloads `operator()()` to write a row,
 *  rather than `writerow()`.
 *
 *  The punctation can be altered similar to an STL locale, The
 *  CSV reader takes ownership of the punct object.
 *
 *  The quoting preferences may also similarly be changed, from quoting all fields
 *  (CSV_QUOTE_ALL), to only quoting fields as required (CSV_QUOTE_MINIMAL),
 *  to quoting all non-numeric fields
 */
struct csv_stream_writer
{
public:
    // MEMBER TYPES
    // ------------
    using value_type = csv_row;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;

    // MEMBER FUNCTIONS
    // ----------------
    csv_stream_writer(csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
    csv_stream_writer(const csv_stream_writer&) = delete;
    csv_stream_writer& operator=(const csv_stream_writer&) = delete;
    csv_stream_writer(csv_stream_writer&&);
    csv_stream_writer& operator=(csv_stream_writer&&);

    // STREAM
    csv_stream_writer(ostream&, csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
    void open(ostream&, csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);

    // PROPERTIES/MODIFIERS
    void punctuation(csvpunct_impl*);
    const csvpunct_impl* punctuation() const;
    void quoting(csv_quoting);
    const csv_quoting quoting() const;
    void swap(csv_stream_writer&);

    // DATA
    void operator()(const value_type& row);

protected:
    friend struct csv_dict_file_writer;
    friend struct csv_dict_string_writer;
    ostream* stream_ = nullptr;
    csv_quoting quoting_ = CSV_QUOTE_MINIMAL;
    shared_ptr<csvpunct_impl> punct_;
};


/**
 *  \brief Writer for file-based document.
 */
struct csv_file_writer: csv_stream_writer
{
public:
    csv_file_writer(csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
    csv_file_writer(const csv_file_writer&) = delete;
    csv_file_writer& operator=(const csv_file_writer&) = delete;
    csv_file_writer(csv_file_writer&&);
    csv_file_writer& operator=(csv_file_writer&&);

    // STREAM
    csv_file_writer(const string_view& name, csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
    void open(const string_view& name, csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
#if defined(HAVE_WFOPEN)                        // WINDOWS
    csv_file_writer(const wstring_view& name, csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
    void open(const wstring_view& name, csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
    csv_file_writer(const u16string_view& name, csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
    void open(const u16string_view& name, csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
#endif                                          // WINDOWS

    // MODIFIERS
    void swap(csv_file_writer&);

private:
    ofstream file_;
};


/**
 *  \brief Writer for string-based document.
 */
struct csv_string_writer: csv_stream_writer
{
public:
    csv_string_writer(csv_quoting = CSV_QUOTE_MINIMAL, csvpunct_impl* = nullptr);
    csv_string_writer(const csv_string_writer&) = delete;
    csv_string_writer& operator=(const csv_string_writer&) = delete;
    csv_string_writer(csv_string_writer&&);
    csv_string_writer& operator=(csv_string_writer&&);
    string str() const;

    // MODIFIERS
    void swap(csv_string_writer&);

private:
    ostringstream sstream_;
};

PYCPP_END_NAMESPACE
