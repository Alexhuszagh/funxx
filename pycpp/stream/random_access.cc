//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.

#include <pycpp/filesystem.h>
#include <pycpp/stream/random_access.h>

PYCPP_BEGIN_NAMESPACE

// OBJECTS
// -------


random_access_fstream::random_access_fstream():
    buffer(std::ios_base::in | std::ios_base::out, INVALID_FD_VALUE),
    std::iostream(&buffer)
{}


random_access_fstream::~random_access_fstream()
{
    close();
}


random_access_fstream::random_access_fstream(random_access_fstream &&other):
    buffer(std::ios_base::in | std::ios_base::out, INVALID_FD_VALUE),
    std::iostream(&buffer)
{
    swap(other);
}


random_access_fstream & random_access_fstream::operator=(random_access_fstream &&other)
{
    swap(other);
    return *this;
}


random_access_fstream::random_access_fstream(const std::string& name, std::ios_base::openmode mode):
    buffer(std::ios_base::in | std::ios_base::out, INVALID_FD_VALUE),
    std::iostream(&buffer)
{
    open(name, mode);
}


void random_access_fstream::open(const std::string& name, std::ios_base::openmode mode)
{
    buffer.fd(fd_open(name, mode, access_random));
}

#if defined(PYCPP_HAVE_WFOPEN)

random_access_fstream::random_access_fstream(const std::wstring& name, std::ios_base::openmode mode):
    buffer(std::ios_base::in | std::ios_base::out, INVALID_FD_VALUE),
    std::iostream(&buffer)
{
    open(name, mode);
}


void random_access_fstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    open(reinterpret_cast<const char16_t*>(name.data()), mode);
}


random_access_fstream::random_access_fstream(const std::u16string& name, std::ios_base::openmode mode):
    buffer(std::ios_base::in | std::ios_base::out, INVALID_FD_VALUE),
    std::iostream(&buffer)
{
    open(name, mode);
}


void random_access_fstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    buffer.fd(fd_open(name, mode, access_random));
}

#endif                              // PYCPP_HAVE_WFOPEN

bool random_access_fstream::is_open() const
{
    return buffer.is_open();
}

void random_access_fstream::close()
{
    if (buffer.fd() != INVALID_FD_VALUE) {
        buffer.close();
        fd_close(buffer.fd());
        buffer.fd(INVALID_FD_VALUE);
    }
}


void random_access_fstream::swap(random_access_fstream &other)
{
    // swap
    std::iostream::swap(other);
    std::swap(buffer, other.buffer);

    // set filebuffers
    std::ios::rdbuf(&buffer);
    other.rdbuf(&other.buffer);
}

// random_access IFSTREAM


random_access_ifstream::random_access_ifstream():
    buffer(std::ios_base::in, INVALID_FD_VALUE),
    std::istream(&buffer)
{}


random_access_ifstream::~random_access_ifstream()
{
    close();
}


random_access_ifstream::random_access_ifstream(random_access_ifstream &&other):
    buffer(std::ios_base::in, INVALID_FD_VALUE),
    std::istream(&buffer)
{
    swap(other);
}


random_access_ifstream & random_access_ifstream::operator=(random_access_ifstream &&other)
{
    swap(other);
    return *this;
}


random_access_ifstream::random_access_ifstream(const std::string& name, std::ios_base::openmode mode):
    buffer(std::ios_base::in, INVALID_FD_VALUE),
    std::istream(&buffer)
{
    open(name, mode);
}


void random_access_ifstream::open(const std::string& name, std::ios_base::openmode mode)
{
    buffer.fd(fd_open(name, mode, access_random));
}

#if defined(PYCPP_HAVE_WFOPEN)

random_access_ifstream::random_access_ifstream(const std::wstring& name, std::ios_base::openmode mode):
    buffer(std::ios_base::in, INVALID_FD_VALUE),
    std::istream(&buffer)
{
    open(name, mode);
}


void random_access_ifstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    open(reinterpret_cast<const char16_t*>(name.data()), mode);
}


random_access_ifstream::random_access_ifstream(const std::u16string& name, std::ios_base::openmode mode):
    buffer(std::ios_base::in, INVALID_FD_VALUE),
    std::istream(&buffer)
{
    open(name, mode);
}


void random_access_ifstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    buffer.fd(fd_open(name, mode, access_random));
}

#endif                              // PYCPP_HAVE_WFOPEN

bool random_access_ifstream::is_open() const
{
    return buffer.is_open();
}

void random_access_ifstream::close()
{
    if (buffer.fd() != INVALID_FD_VALUE) {
        buffer.close();
        fd_close(buffer.fd());
        buffer.fd(INVALID_FD_VALUE);
    }
}


void random_access_ifstream::swap(random_access_ifstream &other)
{
    // swap
    std::istream::swap(other);
    std::swap(buffer, other.buffer);

    // set filebuffers
    std::ios::rdbuf(&buffer);
    other.rdbuf(&other.buffer);
}

// random_access OFSTREAM

random_access_ofstream::random_access_ofstream():
    buffer(std::ios_base::out, INVALID_FD_VALUE),
    std::ostream(&buffer)
{}


random_access_ofstream::~random_access_ofstream()
{
    close();
}


random_access_ofstream::random_access_ofstream(random_access_ofstream &&other):
    buffer(std::ios_base::out, INVALID_FD_VALUE),
    std::ostream(&buffer)
{
    swap(other);
}


random_access_ofstream & random_access_ofstream::operator=(random_access_ofstream &&other)
{
    swap(other);
    return *this;
}


random_access_ofstream::random_access_ofstream(const std::string& name, std::ios_base::openmode mode):
    buffer(std::ios_base::out, INVALID_FD_VALUE),
    std::ostream(&buffer)
{
    open(name, mode);
}


void random_access_ofstream::open(const std::string& name, std::ios_base::openmode mode)
{
    buffer.fd(fd_open(name, mode, access_random));
}

#if defined(PYCPP_HAVE_WFOPEN)

random_access_ofstream::random_access_ofstream(const std::wstring& name, std::ios_base::openmode mode):
    buffer(std::ios_base::out, INVALID_FD_VALUE),
    std::ostream(&buffer)
{
    open(name, mode);
}


void random_access_ofstream::open(const std::wstring& name, std::ios_base::openmode mode)
{
    open(reinterpret_cast<const char16_t*>(name.data()), mode);
}


random_access_ofstream::random_access_ofstream(const std::u16string& name, std::ios_base::openmode mode):
    buffer(std::ios_base::out, INVALID_FD_VALUE),
    std::ostream(&buffer)
{
    open(name, mode);
}


void random_access_ofstream::open(const std::u16string& name, std::ios_base::openmode mode)
{
    buffer.fd(fd_open(name, mode, access_random));
}

#endif                              // PYCPP_HAVE_WFOPEN

bool random_access_ofstream::is_open() const
{
    return buffer.is_open();
}

void random_access_ofstream::close()
{
    if (buffer.fd() != INVALID_FD_VALUE) {
        buffer.close();
        fd_close(buffer.fd());
        buffer.fd(INVALID_FD_VALUE);
    }
}


void random_access_ofstream::swap(random_access_ofstream &other)
{
    // swap
    std::ostream::swap(other);
    std::swap(buffer, other.buffer);

    // set filebuffers
    std::ios::rdbuf(&buffer);
    other.rdbuf(&other.buffer);
}

PYCPP_END_NAMESPACE
