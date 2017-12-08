//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief JSON SAX parser.
 */

#pragma once

#include <pycpp/json/core.h>
#include <pycpp/stream/fstream.h>
#include <pycpp/view/string.h>

#include <sstream>

PYCPP_BEGIN_NAMESPACE

// OBJECTS
// -------


/**
 *  \brief SAX handler for a JSON document.
 */
struct json_sax_handler
{
    // SAX EVENTS
    virtual void start_document();
    virtual void end_document();
    virtual void start_object();
    virtual void end_object(size_t);
    virtual void start_array();
    virtual void end_array(size_t);
    virtual void key(const string_view&);
    virtual void null();
    virtual void boolean(bool);
    virtual void number(double);
    virtual void string(const string_view&);
};


/**
 *  \brief Reader for stream-based document.
 */
struct json_stream_reader
{
public:
    json_stream_reader();

    void open(std::istream&);
    void set_handler(json_sax_handler&);

private:
    std::istream* stream_ = nullptr;
    json_sax_handler* handler_ = nullptr;
};


/**
 *  \brief Reader for file-based document.
 */
struct json_file_reader: json_stream_reader
{
public:
    json_file_reader();
    json_file_reader(const string_view& name);
    void open(const string_view& name);

#if defined(HAVE_WFOPEN)                        // WINDOWS
    json_file_reader(const wstring_view& name);
    void open(const wstring_view& name);
    json_file_reader(const u16string_view& name);
    void open(const u16string_view& name);
#endif                                          // WINDOWS

private:
    ifstream file_;
};


/**
 *  \brief Reader for string-based document.
 */
struct json_string_reader: json_stream_reader
{
public:
    json_string_reader();
    json_string_reader(const string_view& str);
    void open(const string_view& str);

private:
    std::istringstream sstream_;
};

PYCPP_END_NAMESPACE
