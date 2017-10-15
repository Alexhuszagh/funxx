//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  Although the C standard makes no guarantees a C FILE descriptor will
 *  be used internally, both GCC and Clang do so, with GCC providing
 *  an extension to create a basic_filebuf from an existing C FILE or
 *  POSIX file descriptor[1]. As basic_fstream is merely a wrapper around
 *  basic_iostream with a basic_filebuf, this provides a full API
 *  that should have native performance.
 *
 *  By using a GNU extension to solve a GNU-specific problem, or std::fstream
 *  elsewhere, this code should work with MSVC and MinGW on Windows, and
 *  with any compiler elsewhere.
 *
 *  1. http://cs.brown.edu/~jwicks/libstdc++/html_user/stdio__filebuf_8h-source.html
 *  2. https://github.com/llvm-mirror/libcxx/blob/master/include/fstream#L132
 */

#include <pycpp/filesystem.h>
#include <pycpp/stream/fstream.h>
#include <pycpp/string/codec.h>

PYCPP_BEGIN_NAMESPACE

#if defined(HAVE_GCC)                            // GCC

// MACROS
// ------

#define BASIC_FILEBUF(t) static_cast<std::basic_filebuf<char>>(t)

// FUNCTIONS
// ---------

/**
 *  \brief Convert C++ mode to C mode.
 */
template <typename String>
std::string c_ios_mode(const String& path, std::ios_base::openmode mode)
{
    using sios = std::ios_base;

    // Since "r+" and "r+b" do not create the file if it does not
    // exist,  we need to lazily check if the file exists only if
    // the mode corresponds to either mode. If the file does not exist,
    // use "w+" and "w+b", respectively.

    // static cast for clang warning, -WSwitch
    switch (static_cast<int>(mode & ~sios::ate)) {
        case sios::out:
        case sios::out | sios::trunc:
            return "w";
        case sios::out | sios::app:
        case sios::app:
            return "a";
        case sios::in:
            return "r";
        case sios::in | sios::out:
            return exists(path) ? "r+" : "w+";
        case sios::in | sios::out | sios::trunc:
            return "w+";
        case sios::in | sios::out | sios::app:
        case sios::in | sios::app:
            return "a+";
        case sios::out | sios::binary:
        case sios::out | sios::trunc | sios::binary:
            return "wb";
        case sios::out | sios::app | sios::binary:
        case sios::app | sios::binary:
            return "ab";
        case sios::in | sios::binary:
            return "rb";
        case sios::in | sios::out | sios::binary:
            return exists(path) ?  "r+b" : "w+b";
        case sios::in | sios::out | sios::trunc | sios::binary:
            return "w+b";
        case sios::in | sios::out | sios::app | sios::binary:
        case sios::in | sios::app | sios::binary:
            return "a+b";
        default:
            return "";
    }
}


/**
 *  \brief Get C FILE pointer from narrow filename.
 */
FILE * get_c_file(const std::string& narrow, std::ios_base::openmode mode)
{
    auto str = c_ios_mode(narrow, mode);
    std::cout << str << std::endl;
    if (str.size()) {
        return fopen(narrow.data(), str.data());
    }
    return nullptr;
}


#if defined(HAVE_WFOPEN)                        // WINDOWS


/**
 *  \brief Get C FILE pointer from wide filename.
 */
FILE * get_c_file(const std::u16string& wide, const std::ios_base::openmode mode)
{
    auto str = c_ios_mode(wide, mode);
    std::cout << str << std::endl;
    if (str.size()) {
        auto data = reinterpret_cast<const wchar_t*>(codec_utf8_utf16(str).data());
        return _wfopen(reinterpret_cast<const wchar_t*>(wide.data()), data);
    }
    return nullptr;
}

/**
 *  \brief Get C FILE pointer from wide filename.
 */
FILE * get_c_file(const std::wstring& wide, const std::ios_base::openmode mode)
{
    return get_c_file(reinterpret_cast<const char16_t*>(wide.data()), mode);
}

#endif                                          // HAVE_WFOPEN

// OBJECTS
// -------


// FSTREAM

fstream::fstream():
    std::iostream(&buffer)
{}


fstream::~fstream()
{
    close();
}


fstream::fstream(fstream &&other)
{
    swap(other);
}


fstream & fstream::operator=(fstream &&other)
{
    swap(other);
    return *this;
}


fstream::fstream(const std::string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void fstream::open(const std::string& name, std::ios_base::openmode mode)
{
#if defined(HAVE_WFOPEN)
    open(codec_utf8_utf16(name), mode);
#else
    mode |= std::ios_base::in | std::ios_base::out;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
#endif
}

#if defined(HAVE_WFOPEN)                        // WINDOWS

fstream::fstream(const std::wstring& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void fstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::in | std::ios_base::out;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
}


fstream::fstream(const std::u16string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void fstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::in | std::ios_base::out;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
}

#endif                                          // HAVE_WFOPEN


std::basic_filebuf<char>* fstream::rdbuf() const
{
    return const_cast<std::basic_filebuf<char>*>(&buffer);
}


void fstream::rdbuf(std::basic_filebuf<char> *buffer)
{
    std::ios::rdbuf(buffer);
}


bool fstream::is_open() const
{
    return buffer.is_open();
}


void fstream::close()
{
    if (file) {
        buffer.close();
        fclose(file);
        file = nullptr;
    }
}


void fstream::swap(fstream &other)
{
    // swap
    std::iostream::swap(other);
    std::swap(file, other.file);
    std::swap(buffer, other.buffer);

    // set filebuffers
    std::ios::rdbuf(&buffer);
    other.rdbuf(&other.buffer);
}


// IFSTREAM

ifstream::ifstream():
    std::istream(&buffer)
{}


ifstream::~ifstream()
{
    close();
}


ifstream::ifstream(ifstream &&other)
{
    swap(other);
}


ifstream & ifstream::operator=(ifstream &&other)
{
    swap(other);
    return *this;
}


ifstream::ifstream(const std::string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ifstream::open(const std::string& name, std::ios_base::openmode mode)
{
#if defined(HAVE_WFOPEN)
    open(codec_utf8_utf16(name), mode);
#else
    mode |= std::ios_base::in;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
#endif
}

#if defined(HAVE_WFOPEN)                        // WINDOWS


ifstream::ifstream(const std::wstring& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ifstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::in;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
}


ifstream::ifstream(const std::u16string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ifstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::in;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
}

#endif                                          // HAVE_WFOPEN


std::basic_filebuf<char>* ifstream::rdbuf() const
{
    return const_cast<std::basic_filebuf<char>*>(&buffer);
}


void ifstream::rdbuf(std::basic_filebuf<char> *buffer)
{
    std::ios::rdbuf(buffer);
}


bool ifstream::is_open() const
{
    return buffer.is_open();
}


void ifstream::close()
{
    if (file) {
        buffer.close();
        fclose(file);
        file = nullptr;
    }
}


void ifstream::swap(ifstream &other)
{
    // swap
    std::istream::swap(other);
    std::swap(file, other.file);
    std::swap(buffer, other.buffer);

    // set filebuffers
    std::ios::rdbuf(&buffer);
    other.rdbuf(&other.buffer);
}


ofstream::ofstream():
    std::ostream(&buffer)
{}


ofstream::~ofstream()
{
    close();
}


ofstream::ofstream(ofstream &&other)
{
    swap(other);
}


ofstream & ofstream::operator=(ofstream &&other)
{
    swap(other);
    return *this;
}


ofstream::ofstream(const std::string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}

void ofstream::open(const std::string& name, std::ios_base::openmode mode)
{
#if defined(HAVE_WFOPEN)
    open(codec_utf8_utf16(name), mode);
#else
    mode |= std::ios_base::out;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
#endif
}


#if defined(HAVE_WFOPEN)                        // WINDOWS

ofstream::ofstream(const std::wstring& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ofstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::out;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
}


ofstream::ofstream(const std::u16string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ofstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::out;
    file = get_c_file(name, mode);
    buffer = BASIC_FILEBUF(__gnu_cxx::stdio_filebuf<char>(file, mode));
    std::ios::rdbuf(&buffer);
}

#endif                                          // HAVE_WFOPEN


std::basic_filebuf<char>* ofstream::rdbuf() const
{
    return const_cast<std::basic_filebuf<char>*>(&buffer);
}


void ofstream::rdbuf(std::basic_filebuf<char> *buffer)
{
    std::ios::rdbuf(buffer);
}


bool ofstream::is_open() const
{
    return buffer.is_open();
}


void ofstream::close()
{
    if (file) {
        buffer.close();
        fclose(file);
        file = nullptr;
    }
}


void ofstream::swap(ofstream &other)
{
    // swap
    std::ostream::swap(other);
    std::swap(file, other.file);
    std::swap(buffer, other.buffer);

    // set filebuffers
    std::ios::rdbuf(&buffer);
    other.rdbuf(&other.buffer);
}

#elif defined(HAVE_MSVC)                // MSVC

// FSTREAM

fstream::fstream()
{}


fstream::~fstream()
{}


fstream::fstream(fstream &&other)
{
    std::fstream::swap(other);
}


fstream & fstream::operator=(fstream &&other)
{
    std::fstream::swap(other);
    return *this;
}


fstream::fstream(const std::string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void fstream::open(const std::string& name, std::ios_base::openmode mode)
{
#if defined(HAVE_WFOPEN)
    open(codec_utf8_utf16(name), mode);
#else
    mode |= std::ios_base::in | std::ios_base::out;
    std::fstream::open(name, mode);
#endif
}

#if defined(HAVE_WFOPEN)                        // WINDOWS


fstream::fstream(const std::wstring& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void fstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::in | std::ios_base::out;
    std::fstream::open(name, mode);
}


fstream::fstream(const std::u16string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void fstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::in | std::ios_base::out;
    std::fstream::open(reinterpret_cast<const wchar_t*>(name.data()), mode);
}

#endif                                          // HAVE_WFOPEN

// IFSTREAM

ifstream::ifstream()
{}


ifstream::~ifstream()
{}


ifstream::ifstream(ifstream &&other)
{
    std::ifstream::swap(other);
}


ifstream & ifstream::operator=(ifstream &&other)
{
    std::ifstream::swap(other);
    return *this;
}


ifstream::ifstream(const std::string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ifstream::open(const std::string& name, std::ios_base::openmode mode)
{
#if defined(HAVE_WFOPEN)
    open(codec_utf8_utf16(name), mode);
#else
    mode |= std::ios_base::in;
    std::ifstream::open(name, mode);
#endif
}

#if defined(HAVE_WFOPEN)                        // WINDOWS


ifstream::ifstream(const std::wstring& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ifstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::in;
    std::ifstream::open(name, mode);
}


ifstream::ifstream(const std::u16string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ifstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::in;
    std::ifstream::open(reinterpret_cast<const wchar_t*>(name.data()), mode);
}

#endif                                          // HAVE_WFOPEN

// OFSTREAM


ofstream::ofstream()
{}


ofstream::~ofstream()
{}


ofstream::ofstream(ofstream &&other)
{
    std::ofstream::swap(other);
}


ofstream & ofstream::operator=(ofstream &&other)
{
    std::ofstream::swap(other);
    return *this;
}


ofstream::ofstream(const std::string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ofstream::open(const std::string& name, std::ios_base::openmode mode)
{
#if defined(HAVE_WFOPEN)
    open(codec_utf8_utf16(name), mode);
#else
    mode |= std::ios_base::out;
    std::ofstream::open(name, mode);
#endif
}

#if defined(HAVE_WFOPEN)                        // WINDOWS


ofstream::ofstream(const std::wstring& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ofstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::out;
    std::ofstream::open(name, mode);
}


ofstream::ofstream(const std::u16string& name, std::ios_base::openmode mode)
{
    open(name, mode);
}


void ofstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    mode |= std::ios_base::out;
    std::ofstream::open(reinterpret_cast<const wchar_t*>(name.data()), mode);
}

#endif                                          // HAVE_WFOPEN
#endif                                          // GCC

PYCPP_END_NAMESPACE
