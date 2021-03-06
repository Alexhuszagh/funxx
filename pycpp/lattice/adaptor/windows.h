//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Windows socket adaptor.
 */

#pragma once

#include <pycpp/config.h>
#include <pycpp/preprocessor/os.h>

#if defined(OS_WINDOWS)

#include <pycpp/lattice/dns.h>
#include <pycpp/lattice/method.h>
#include <pycpp/lattice/ssl.h>
#include <pycpp/lattice/timeout.h>
#include <pycpp/lattice/url.h>
#include <pycpp/string/string.h>


PYCPP_BEGIN_NAMESPACE

// OBJECTS
// -------


/**
 *  \brief Adaptor for Win32 sockets.
 */
class win32_socket_adaptor_t
{
public:
    using self_t = win32_socket_adaptor_t;

    win32_socket_adaptor_t();
    win32_socket_adaptor_t(const self_t&) = delete;
    self_t& operator=(const self_t&) = delete;
    ~win32_socket_adaptor_t();

    // REQUESTS
    bool open(const addrinfo& info, const string_wrapper&);
    bool close();
    size_t write(const char *buf, size_t len);
    size_t read(char *buf, size_t count);

    // OPTIONS
    void set_reuse_address();
    void set_timeout(const timeout_t& timeout);
    void set_certificate_file(const certificate_file_t& certificate);
    void set_revocation_lists(const revocation_lists_t& revoke);
    void set_ssl_protocol(const ssl_protocol_t ssl);

    // DATA
    const SOCKET fd() const;

protected:
    SOCKET sock = INVALID_SOCKET;
};

PYCPP_END_NAMESPACE

#endif              // WIN32
