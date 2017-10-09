//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Hashmap-based CSV readers and writers.
 */

#pragma once

#include <pycpp/collections/ordereddict.h>
#include <pycpp/csv/reader.h>
#include <pycpp/csv/writer.h>

#include <unordered_map>

PYCPP_BEGIN_NAMESPACE

// ALIAS
// -----

using csv_indexes = ordered_map<std::string, size_t>;
using csv_map = std::unordered_map<std::string, std::string>;

// OBJECTS
// -------

// READER

/**
 *  \brief Hashmap-based reader for CSF file.
 *
 *  This reader is mostly analogous to Python's `csv.DictReader` object.
 */
struct csv_dict_stream_reader
{
public:
    // MEMBER TYPES
    // ------------
    using value_type = csv_map;
    using reference = value_type&;
    using iterator = input_iterator_facade<csv_dict_stream_reader>;

    // MEMBER FUNCTIONS
    // ----------------
    csv_dict_stream_reader();
    csv_dict_stream_reader(std::istream&, size_t skip = 0);
    void parse(std::istream&, size_t skip = 0);
    void punctuation(csvpunct_impl*);
    const csvpunct_impl* punctuation() const;

    // DATA
    value_type operator()();
    bool eof() const;
    explicit operator bool() const;

    // ITERATORS
    iterator begin();
    iterator end();

private:
    csv_stream_reader reader_;
    csv_indexes header_;
};


/**
 *  \brief Dict reader for file-based document.
 */
struct csv_dict_file_reader: csv_dict_stream_reader
{
public:
    csv_dict_file_reader();
    csv_dict_file_reader(const std::string &name);
    void open(const std::string &name);
    void parse(const std::string &name, size_t skip = 0);

#if defined(PYCPP_HAVE_WFOPEN)
    csv_dict_file_reader(const std::wstring &name);
    void open(const std::wstring &name);
    void parse(const std::wstring &name, size_t skip = 0);
#endif

    void parse(size_t skip = 0);

private:
    ifstream file_;
};


/**
 *  \brief Dict reader for string-based document.
 */
struct csv_dict_string_reader: csv_dict_stream_reader
{
public:
    csv_dict_string_reader();
    csv_dict_string_reader(const std::string &str);
    void open(const std::string &str);
    void parse(const std::string &str, size_t skip = 0);

    void parse(size_t skip = 0);

private:
    std::istringstream sstream_;
};

// WRITER


/**
 *  \brief Generic writer for CSV file.
 *
 *  This reader is mostly analogous to Python's `csv.DictWriter` object.
 */
struct csv_dict_stream_writer
{
public:
    // MEMBER TYPES
    // ------------
    using value_type = csv_map;
    using reference = value_type&;

    // MEMBER FUNCTIONS
    // ----------------
    csv_dict_stream_writer(csv_quoting = CSV_QUOTE_MINIMAL);
    csv_dict_stream_writer(std::ostream&, const csv_row&, csv_quoting = CSV_QUOTE_MINIMAL);
    void open(std::ostream&, const csv_row&);
    void punctuation(csvpunct_impl*);
    const csvpunct_impl* punctuation() const;
    void quoting(csv_quoting);
    const csv_quoting quoting() const;

    // DATA
    void operator()(const value_type& row);

private:
    csv_stream_writer writer_;
    csv_indexes header_;
};


/**
 *  \brief Writer for file-based document.
 */
struct csv_dict_file_writer: csv_dict_stream_writer
{
public:
    csv_dict_file_writer();
    csv_dict_file_writer(const std::string &name, const csv_row& header);
    void open(const std::string &name, const csv_row& header);

#if defined(PYCPP_HAVE_WFOPEN)
    csv_dict_file_writer(const std::wstring &name, const csv_row& header);
    void open(const std::wstring &name, const csv_row& header);
#endif

private:
    ofstream file_;
};


/**
 *  \brief Writer for string-based document.
 */
struct csv_dict_string_writer: csv_dict_stream_writer
{
public:
    csv_dict_string_writer();
    csv_dict_string_writer(const std::string &str, const csv_row& header);
    void open(const std::string &str, const csv_row& header);

private:
    std::ostringstream sstream_;
};

PYCPP_END_NAMESPACE
