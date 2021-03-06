//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.

#include <pycpp/filesystem.h>
#include <pycpp/filesystem/exception.h>
#include <pycpp/preprocessor/architecture.h>
#include <pycpp/stream/mmap.h>
#include <assert.h>

#if defined(HAVE_MMAP) || defined(OS_WINDOWS)           // MMAP

#if defined(HAVE_MMAP)
#   include <sys/mman.h>
#   include <pycpp/preprocessor/sysstat.h>
#elif defined(OS_WINDOWS)
#   include <pycpp/windows/mman.h>
#   include <io.h>
#endif

PYCPP_BEGIN_NAMESPACE

// HELPERS
// -------


#if defined(OS_POSIX)                   // POSIX

static size_t file_length(fd_t fd)
{
    struct stat sb;
    if (::fstat(fd, &sb) == -1) {
        throw filesystem_error(filesystem_unexpected_error);
    }
    return sb.st_size;
}

#elif defined(OS_WINDOWS)               // WINDOWS

static size_t file_length(fd_t fd)
{
    LARGE_INTEGER bytes;
    if (!::GetFileSizeEx(fd, &bytes)) {
        throw filesystem_error(filesystem_unexpected_error);
    }
    return static_cast<size_t>(bytes.QuadPart);
}

#else                                   // UNKNOWN
#   error "Unsupported operating system."
#endif                                  // POSIX


static int convert_prot(ios_base::openmode mode)
{
    int prot = 0;
    if (mode & ios_base::in) {
        prot |= PROT_READ;
    }
    if (mode & ios_base::out) {
        prot |= PROT_WRITE;
    }

    return prot;
}


static void* open_memory_view(fd_t fd, ios_base::openmode mode, size_t offset, size_t length)
{
#if defined(OS_WINDOWS)
    int fd_ = _open_osfhandle((intptr_t) fd, 0);
    void* addr = ::mmap(nullptr, length, convert_prot(mode), MAP_SHARED, fd_, offset);
#else
    void* addr = ::mmap(nullptr, length, convert_prot(mode), MAP_SHARED, fd, offset);
#endif
    if (addr == MAP_FAILED) {
        return nullptr;
    }
    return addr;
}


static int memory_sync(void *addr, size_t length, bool async)
{
    // on modern Linux, MS_ASYNC is a no-op, however,
    // it still should be used for futureproofing
    int flags = async ? MS_ASYNC : MS_SYNC;
    return ::msync(addr, length, flags);
}


static int close_memory_view(void *addr, size_t length)
{
    return ::munmap(addr, length);
}

// OBJECTS
// -------

// MMAP FSTREAM

mmap_fstream::mmap_fstream():
    buffer(ios_base::in | ios_base::out, INVALID_FD_VALUE),
    iostream(&buffer)
{}


mmap_fstream::~mmap_fstream()
{
    close();
}


mmap_fstream::mmap_fstream(mmap_fstream&& rhs):
    mmap_fstream()
{
    swap(rhs);
}


mmap_fstream & mmap_fstream::operator=(mmap_fstream&& rhs)
{
    swap(rhs);
    return *this;
}


mmap_fstream::mmap_fstream(const string_view& name, ios_base::openmode mode):
    buffer(ios_base::in | ios_base::out, INVALID_FD_VALUE),
    iostream(&buffer)
{
    open(name, mode);
}


void mmap_fstream::open(const string_view& name, ios_base::openmode mode)
{
    close();
    mode |= ios_base::in | ios_base::out;
    buffer.fd(fd_open(name, mode, S_IWR_USR_GRP, access_random));
}

#if defined(HAVE_WFOPEN)                        // WINDOWS

mmap_fstream::mmap_fstream(const wstring_view& name, ios_base::openmode mode):
    buffer(ios_base::in | ios_base::out, INVALID_FD_VALUE),
    iostream(&buffer)
{
    open(name, mode);
}


void mmap_fstream::open(const wstring_view& name, ios_base::openmode mode)
{
    open(reinterpret_cast<const char16_t*>(name.data()), mode);
}


mmap_fstream::mmap_fstream(const u16string_view& name, ios_base::openmode mode):
    buffer(ios_base::in | ios_base::out, INVALID_FD_VALUE),
    iostream(&buffer)
{
    open(name, mode);
}


void mmap_fstream::open(const u16string_view& name, ios_base::openmode mode)
{
    close();
    mode |= ios_base::in | ios_base::out;
    buffer.fd(fd_open(name, mode, S_IWR_USR_GRP, access_random));
}

#endif                                          // WINDOWS

bool mmap_fstream::is_open() const
{
    return buffer.is_open();
}


bool mmap_fstream::has_mapping() const
{
    return data_ != nullptr;
}


void mmap_fstream::close()
{
    unmap();
    if (buffer.fd() != INVALID_FD_VALUE) {
        buffer.close();
        fd_close(buffer.fd());
        buffer.fd(INVALID_FD_VALUE);
    }
}


void mmap_fstream::swap(mmap_fstream& rhs)
{
    using PYCPP_NAMESPACE::swap;
    iostream::swap(rhs);
    swap(buffer, rhs.buffer);
    swap(data_, rhs.data_);
    swap(length_, rhs.length_);
}


char& mmap_fstream::operator[](size_t index)
{
    assert(data_ && "Memory address cannot be null.");
    assert(index < length_ && "Index must be less than buffer length.");
    return data_[index];
}


const char& mmap_fstream::operator[](size_t index) const
{
    assert(data_ && "Memory address cannot be null.");
    assert(index < length_ && "Index must be less than buffer length.");
    return data_[index];
}


char* mmap_fstream::data() const
{
    return data_;
}


size_t mmap_fstream::size() const
{
    return length();
}


size_t mmap_fstream::length() const
{
    return length_;
}


void mmap_fstream::map(size_t o)
{
    map(o, file_length(buffer.fd()));
}


void mmap_fstream::map(size_t o, size_t l)
{
    // cleanup existing memory
    unmap();

    // allocate if space is required
    size_t fd_length = file_length(buffer.fd());
    if (l > fd_length) {
        if (fd_allocate(buffer.fd(), l) < 0) {
            return;
        }
    }

    // map memory
    ios_base::openmode mode = ios_base::in | ios_base::out;
    data_ = reinterpret_cast<char*>(open_memory_view(buffer.fd(), mode, o, l));
    if (data_) {
        length_ = l;
    }
}


void mmap_fstream::unmap()
{
    // unmap
    if (data_) {
        close_memory_view(data_, length_);
        data_ = nullptr;
        length_ = 0;
    }
}


void mmap_fstream::flush(bool async)
{
    assert(data_ && "Memory address cannot be null.");
    memory_sync(data_, length_, async);
}

// MMAP IFSTREAM


mmap_ifstream::mmap_ifstream():
    buffer(ios_base::in, INVALID_FD_VALUE),
    istream(&buffer)
{}


mmap_ifstream::~mmap_ifstream()
{
    close();
}


mmap_ifstream::mmap_ifstream(mmap_ifstream&& rhs):
    mmap_ifstream()
{
    swap(rhs);
}


mmap_ifstream & mmap_ifstream::operator=(mmap_ifstream&& rhs)
{
    swap(rhs);
    return *this;
}


mmap_ifstream::mmap_ifstream(const string_view& name, ios_base::openmode mode):
    buffer(ios_base::in, INVALID_FD_VALUE),
    istream(&buffer)
{
    open(name, mode);
}


void mmap_ifstream::open(const string_view& name, ios_base::openmode mode)
{
    close();
    mode |= ios_base::in;
    buffer.fd(fd_open(name, mode, S_IWR_USR_GRP, access_random));
}

#if defined(HAVE_WFOPEN)                        // WINDOWS

mmap_ifstream::mmap_ifstream(const wstring_view& name, ios_base::openmode mode):
    buffer(ios_base::in, INVALID_FD_VALUE),
    istream(&buffer)
{
    open(name, mode);
}


void mmap_ifstream::open(const wstring_view& name, ios_base::openmode mode)
{
    open(reinterpret_cast<const char16_t*>(name.data()), mode);
}


mmap_ifstream::mmap_ifstream(const u16string_view& name, ios_base::openmode mode):
    buffer(ios_base::in, INVALID_FD_VALUE),
    istream(&buffer)
{
    open(name, mode);
}


void mmap_ifstream::open(const u16string_view& name, ios_base::openmode mode)
{
    close();
    mode |= ios_base::in;
    buffer.fd(fd_open(name, mode, S_IWR_USR_GRP, access_random));
}

#endif                                          // WINDOWS

bool mmap_ifstream::is_open() const
{
    return buffer.is_open();
}


bool mmap_ifstream::has_mapping() const
{
    return data_ != nullptr;
}


void mmap_ifstream::close()
{
    unmap();
    if (buffer.fd() != INVALID_FD_VALUE) {
        buffer.close();
        fd_close(buffer.fd());
        buffer.fd(INVALID_FD_VALUE);
    }
}


void mmap_ifstream::swap(mmap_ifstream& rhs)
{
    using PYCPP_NAMESPACE::swap;
    istream::swap(rhs);
    swap(buffer, rhs.buffer);
    swap(data_, rhs.data_);
    swap(length_, rhs.length_);
}


const char& mmap_ifstream::operator[](size_t index) const
{
    assert(data_ && "Memory address cannot be null.");
    assert(index < length_ && "Index must be less than buffer length.");
    return data_[index];
}


const char* mmap_ifstream::data() const
{
    return data_;
}


size_t mmap_ifstream::size() const
{
    return length();
}


size_t mmap_ifstream::length() const
{
    return length_;
}


void mmap_ifstream::map(size_t o)
{
    map(o, file_length(buffer.fd()));
}


void mmap_ifstream::map(size_t o, size_t l)
{
    // cleanup
    unmap();

    // Note: read-only, cannot map beyond file.
    // map memory
    ios_base::openmode mode = ios_base::in;
    data_ = reinterpret_cast<char*>(open_memory_view(buffer.fd(), mode, o, l));
    if (data_) {
        length_ = l;
    }
}


void mmap_ifstream::unmap()
{
    // unmap
    if (data_) {
        close_memory_view(data_, length_);
        data_ = nullptr;
        length_ = 0;
    }
}


void mmap_ifstream::flush(bool async)
{
    assert(data_ && "Memory address cannot be null.");
    memory_sync(data_, length_, async);
}

// MMAP OFSTREAM

mmap_ofstream::mmap_ofstream():
    // Linux and Windows require read/write access for mmap
    // Lie about the underlying fd and just provide write methods
    buffer(ios_base::in | ios_base::out, INVALID_FD_VALUE),
    ostream(&buffer)
{}


mmap_ofstream::~mmap_ofstream()
{
    close();
}


mmap_ofstream::mmap_ofstream(mmap_ofstream&& rhs):
    mmap_ofstream()
{
    swap(rhs);
}


mmap_ofstream & mmap_ofstream::operator=(mmap_ofstream&& rhs)
{
    swap(rhs);
    return *this;
}


mmap_ofstream::mmap_ofstream(const string_view& name, ios_base::openmode mode):
    // Linux and Windows require read/write access for mmap
    // Lie about the underlying fd and just provide write methods
    buffer(ios_base::in | ios_base::out, INVALID_FD_VALUE),
    ostream(&buffer)
{
    open(name, mode);
}


void mmap_ofstream::open(const string_view& name, ios_base::openmode mode)
{
    close();
    mode |= ios_base::in | ios_base::out;
    buffer.fd(fd_open(name, mode, S_IWR_USR_GRP, access_random));
}

#if defined(HAVE_WFOPEN)                        // WINDOWS

mmap_ofstream::mmap_ofstream(const wstring_view& name, ios_base::openmode mode):
    // Linux and Windows require read/write access for mmap
    // Lie about the underlying fd and just provide write methods
    buffer(ios_base::in | ios_base::out, INVALID_FD_VALUE),
    ostream(&buffer)
{
    open(name, mode);
}


void mmap_ofstream::open(const wstring_view& name, ios_base::openmode mode)
{
    open(reinterpret_cast<const char16_t*>(name.data()), mode);
}


mmap_ofstream::mmap_ofstream(const u16string_view& name, ios_base::openmode mode):
    // Linux and Windows require read/write access for mmap
    // Lie about the underlying fd and just provide write methods
    buffer(ios_base::in | ios_base::out, INVALID_FD_VALUE),
    ostream(&buffer)
{
    open(name, mode);
}


void mmap_ofstream::open(const u16string_view& name, ios_base::openmode mode)
{
    close();
    mode |= ios_base::in | ios_base::out;
    buffer.fd(fd_open(name, mode, S_IWR_USR_GRP, access_random));
}

#endif                                          // WINDOWS

bool mmap_ofstream::is_open() const
{
    return buffer.is_open();
}


bool mmap_ofstream::has_mapping() const
{
    return data_ != nullptr;
}


void mmap_ofstream::close()
{
    unmap();
    if (buffer.fd() != INVALID_FD_VALUE) {
        buffer.close();
        fd_close(buffer.fd());
        buffer.fd(INVALID_FD_VALUE);
    }
}


void mmap_ofstream::swap(mmap_ofstream& rhs)
{
    using PYCPP_NAMESPACE::swap;
    ostream::swap(rhs);
    swap(buffer, rhs.buffer);
    swap(data_, rhs.data_);
    swap(length_, rhs.length_);
}


char& mmap_ofstream::operator[](size_t index)
{
    assert(data_ && "Memory address cannot be null.");
    assert(index < length_ && "Index must be less than buffer length.");
    return data_[index];
}


char* mmap_ofstream::data() const
{
    return data_;
}


size_t mmap_ofstream::size() const
{
    return length();
}


size_t mmap_ofstream::length() const
{
    return length_;
}


void mmap_ofstream::map(size_t o)
{
    map(o, file_length(buffer.fd()));
}


void mmap_ofstream::map(size_t o, size_t l)
{
    // cleanup
    unmap();

    // allocate if space is required
    size_t fd_length = file_length(buffer.fd());
    if (l > fd_length) {
        if (fd_allocate(buffer.fd(), l) < 0) {
            return;
        }
    }

    // map memory
    // Linux and Windows require read/write access for mmap
    // Lie about the underlying fd and just provide write methods
    ios_base::openmode mode = ios_base::in | ios_base::out;
    data_ = reinterpret_cast<char*>(open_memory_view(buffer.fd(), mode, o, l));
    if (data_) {
        length_ = l;
    }
}


void mmap_ofstream::unmap()
{
    // unmap
    if (data_) {
        close_memory_view(data_, length_);
        data_ = nullptr;
        length_ = 0;
    }
}


void mmap_ofstream::flush(bool async)
{
    assert(data_ && "Memory address cannot be null.");
    memory_sync(data_, length_, async);
}

PYCPP_END_NAMESPACE

#endif                                                  // MMAP
