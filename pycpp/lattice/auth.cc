//  :copyright: (c) 2015 Huu Nguyen.
//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.

#include <pycpp/lattice/auth.h>


PYCPP_BEGIN_NAMESPACE

// OBJECTS
// -------


authentication_t::authentication_t(const char* username, const char* password):
    authentication_t(string_view(username), string_view(password))
{}


authentication_t::authentication_t(std::string &&username, std::string &&password):
    username(std::forward<std::string>(username)),
    password(std::forward<std::string>(password))
{}


authentication_t::authentication_t(const string_view& username, const string_view& password):
    username(username),
    password(password)
{}


const std::string authentication_t::string() const noexcept
{
    return username + ":" + password;
}


authentication_t::operator bool() const
{
    return !(username.empty() || password.empty());
}

PYCPP_END_NAMESPACE
