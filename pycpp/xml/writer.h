//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief XML generic writer.
 */

#pragma once

#include <pycpp/config.h>
#include <pycpp/xml/core.h>
#include <pycpp/stream/fstream.h>
#include <pycpp/view/string.h>

#include <sstream>

PYCPP_BEGIN_NAMESPACE

// OBJECTS
// -------


/**
 *  \brief Generic writer for an XML document.
 */
struct xml_writer
{
    virtual void start_element(const string_view&);
    virtual void end_element();
    virtual void write_attribute(const string_view&, const string_view&);
    virtual void write_text(const string_view&);
    virtual void write_comment(const string_view&);
    virtual void start_element_ns(const string_view&, const string_view&, const string_view&);
    virtual void end_element_ns();
    virtual void write_attribute_ns(const string_view&, const string_view&, const string_view&, const string_view&);
    virtual void flush() const;
};


/**
 *  \brief Writer for stream-based document.
 */
struct xml_stream_writer: xml_writer
{
public:
    xml_stream_writer(char = ' ', int = 4);
    xml_stream_writer(std::ostream&, char = ' ', int = 4);
    ~xml_stream_writer();

    void open(std::ostream&);
    void set_indent(char = ' ', int = 4);

    virtual void start_element(const string_view&) override;
    virtual void end_element() override;
    virtual void write_attribute(const string_view&, const string_view&) override;
    virtual void write_text(const string_view&) override;
    virtual void write_comment(const string_view&) override;
    virtual void start_element_ns(const string_view&, const string_view&, const string_view&) override;
    virtual void end_element_ns() override;
    virtual void write_attribute_ns(const string_view&, const string_view&, const string_view&, const string_view&) override;
    virtual void flush() const override;

private:
    char indent_character_ = ' ';
    int indent_width_ = 0;
    void* stream_ = nullptr;
    void* writer_ = nullptr;
};


/**
 *  \brief Writer for stream-based document.
 */
struct xml_file_writer: xml_stream_writer
{
public:
    xml_file_writer();
    xml_file_writer(const string_view& name);
    void open(const string_view& name);

#if defined(HAVE_WFOPEN)                        // WINDOWS
    xml_file_writer(const wstring_view& name);
    void open(const wstring_view& name);
    xml_file_writer(const u16string_view& name);
    void open(const u16string_view& name);
#endif                                          // WINDOWS
    virtual void flush() const override;

private:
    mutable ofstream file_;
};


/**
 *  \brief Writer for string-based document.
 */
struct xml_string_writer: xml_stream_writer
{
public:
    xml_string_writer();
    std::string str() const;

private:
    std::ostringstream sstream_;
};

PYCPP_END_NAMESPACE
