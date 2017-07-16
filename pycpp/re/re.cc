//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.

#include <pycpp/cache/lru.h>
#include <pycpp/re/re.h>
#include <pycpp/re/regex.h>
#include <iostream>         // TODO: remove

PYCPP_BEGIN_NAMESPACE

// CONSTANTS
// ---------

#define REGEX_CACHE_SIZE 100

// GLOBALS
// -------

using regex_cache = lru_cache<std::string, regex_t>;
regex_cache REGEX_CACHE(REGEX_CACHE_SIZE);

// HELPERS
// -------


/**
 *  Compile regex if not previously present in the cache.
 */
static regex_t& compile(const std::string& pattern)
{
    auto it = REGEX_CACHE.find(pattern);
    if (it == REGEX_CACHE.end()) {
        it = REGEX_CACHE.insert(pattern, regex_t(pattern)).first;
    }

    return *it;
}

// FUNCTIONS
// ---------


match_t search(const std::string& pattern, const string_view& str)
{
    return compile(pattern).search(str);
}


match_t match(const std::string& pattern, const string_view& str)
{
    return compile(pattern).match(str);
}


std::string escape(const string_view& str)
{
    std::string output;
    output.reserve(str.size() * 2 + 1);

    for (const char c: str) {
        if ((c >= 0 && c <= 47) ||      // Null - /
            (c >= 58 && c <= 64) ||     // : - @
            (c >= 91 && c <= 96) ||     // [ - `]
            (c >= 123 && c <= 126)) {   // ( - ~
            output.push_back('\\');
        }
        output.push_back(c);
    }

    output.shrink_to_fit();
    return output;
}


void purge()
{
    REGEX_CACHE.clear();
}


PYCPP_END_NAMESPACE