//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.

#include <pycpp/csv/reader.h>
#include <pycpp/string/getline.h>

#include <cassert>

PYCPP_BEGIN_NAMESPACE

// FUNCTIONS
// ---------

static std::string readline(std::istream& stream)
{
    std::string line;
    getline(stream, line);
    return line;
}

static csv_row parse_csv_row(std::istream& stream, csvpunct_impl& punct, size_t size)
{
    csv_row row;
    std::string line = readline(stream);
    row.reserve(size);

    bool quote = false;
    bool escape = false;
    std::vector<char> word(line.size());

    int j = 0;
    int k = 0;
    for (const char c : line) {
        if (escape) {               // escape letter and undo escaping
            escape = false;
            word[j] = c;
            j++;
        }  else if (c == punct.escape()) {
            escape = true;          // escape next character
        } else if (c == punct.quote()) {
            quote ^= true;          // opening/ending quote
        } else if (quote) {
            word[j] = c;            // append quoted character to word
            j++;
        } else if (c == punct.delimiter()) {
            row.emplace_back(std::string(word.data(), j));
            j = 0;
            k++;
        } else {
            word[j] = c;            // append unquoted word
            j++;
        }
    }

    row.emplace_back(std::string(word.data(), j));

    return row;
}

// OBJECTS
// -------

csv_stream_reader::csv_stream_reader(csvpunct_impl* punct):
    punct_(punct ? punct : new csvpunct)
{}


csv_stream_reader::csv_stream_reader(std::istream& stream, size_t skip, csvpunct_impl* punct)
{
    open(stream, skip, punct);
}


void csv_stream_reader::open(std::istream& stream, size_t skip, csvpunct_impl* punct)
{
    // skip lines
    std::string line;
    while (skip--) {
        getline(stream, line);
    }
    stream_ = &stream;
    punctuation(punct);
}


void csv_stream_reader::punctuation(csvpunct_impl* punct)
{
    punct_.reset(punct ? punct : new csvpunct);
}


const csvpunct_impl* csv_stream_reader::punctuation() const
{
    return punct_.get();
}


auto csv_stream_reader::operator()() -> value_type
{
    assert(stream_ && "Stream cannot be null.");

    value_type row;
    row = parse_csv_row(*stream_, *punct_, row_length_);
    row_length_ = row.size();
    return row;
}


bool csv_stream_reader::eof() const
{
    assert(stream_ && "Stream cannot be null.");
    return (stream_->eof() || stream_->peek() == EOF || stream_->peek() == '\0');
}


csv_stream_reader::operator bool() const
{
    assert(stream_ && "Stream cannot be null.");
    return !(stream_->bad() || eof());
}


auto csv_stream_reader::begin() -> iterator
{
    return iterator(*this);
}


auto csv_stream_reader::end() -> iterator
{
    return iterator();
}


csv_file_reader::csv_file_reader(csvpunct_impl* punct):
    csv_stream_reader(punct)
{}


csv_file_reader::csv_file_reader(const std::string &name, size_t skip, csvpunct_impl* punct)
{
    open(name, skip, punct);
}


void csv_file_reader::open(const std::string &name, size_t skip, csvpunct_impl* punct)
{
    file_.open(name, std::ios_base::in | std::ios_base::binary);
    csv_stream_reader::open(file_, skip, punct);
}


#if defined(PYCPP_HAVE_WFOPEN)


csv_file_reader::csv_file_reader(const std::wstring &name, size_t skip, csvpunct_impl* punct)
{
    open(name, skip, punct);
}


void csv_file_reader::open(const std::wstring &name, size_t skip, csvpunct_impl* punct)
{
    file_.open(name, std::ios_base::in | std::ios_base::binary);
    csv_stream_reader::open(file_, skip, punct);
}

#endif


csv_string_reader::csv_string_reader(csvpunct_impl* punct):
    csv_stream_reader(punct)
{}


csv_string_reader::csv_string_reader(const std::string &str, size_t skip, csvpunct_impl* punct)
{
    open(str, skip, punct);
}


void csv_string_reader::open(const std::string &str, size_t skip, csvpunct_impl* punct)
{
    sstream_ = std::istringstream(str, std::ios_base::in | std::ios_base::binary);
    csv_stream_reader::open(sstream_, skip, punct);
}

PYCPP_END_NAMESPACE
