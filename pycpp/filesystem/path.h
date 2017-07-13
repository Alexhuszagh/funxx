//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Path type definitions.
 */

#pragma once

#include <pycpp/codec.h>
#include <pycpp/os.h>
#include <deque>
#include <string>

PYCPP_BEGIN_NAMESPACE

// ALIAS
// -----

#if defined(OS_WINDOWS)             // WINDOWS
   typedef wchar_t native_char_type;
   typedef char backup_char_type;
   typedef std::u16string path_t;
   typedef std::string backup_path_t;
   typedef std::deque<path_t> path_list_t;
   typedef std::deque<backup_path_t> backup_path_list_t;
   std::u16string ansi_to_utf16(const std::string&);
   std::string utf16_to_ansi(const std::u16string&);
#   define path_to_string(s) codec_utf16_utf8(s)
#   define backup_path_to_string(s) (s)
#   define path_to_backup_path(s) utf16_to_ansi(s)
#   define backup_path_to_path(s) ansi_to_utf16(s)
#   define string_to_path(s) codec_utf8_utf16(s)
#   define string_to_backup_path(s) (s)
#   define path_prefix(p) u##p
#else                               // POSIX
   typedef char native_char_type;
   typedef std::string path_t;
   typedef std::deque<path_t> path_list_t;
#   define path_to_string(s) (s)
#   define string_to_path(s) (s)
#   define path_prefix(p) (p)
#endif

#define path_extension path_prefix('.')
#define current_directory "."
#define parent_directory ".."
#if defined(OS_WINDOWS)                             // WINDOWS
#   define path_separator path_prefix('\\')
#   define path_separators path_t(path_prefix("/\\"))
#elif defined(OS_MSDOS)                             // MSDOS
#   define path_separator path_prefix('\\')
#   define path_separators path_t(path_prefix("\\"))
#elif defined(OS_MACOS) && OS_VERSION_MAJOR == 9    // MAC OS9
#   define path_separator path_prefix(':')
#   define path_separators path_t(path_prefix(":"))
#elif defined(OS_OS2)                               // OS2
#   define path_separator path_prefix('\\')
#   define path_separators path_t(path_prefix("/\\"))
#elif defined(OS_SYMBIAN)                           // SYMBIAN
#   define path_separator path_prefix('\\')
#   define path_separators path_t(path_prefix("\\"))
#elif defined(OS_VMS)                               // VMS
#   define path_separator path_prefix('.')
#   define path_separators path_t(path_prefix("."))
#elif defined(OS_VOS)                               // VOS
#   define path_separator path_prefix('>')
#   define path_separators path_t(path_prefix(">"))
#elif defined(OS_NONSTOP)                           // NONSTOP
#   define path_separator path_prefix('.')
#   define path_separators path_t(path_prefix("."))
#else                                               // POSIX, other
#   define path_separator path_prefix('/')
#   define path_separators path_t(path_prefix("/"))
#endif

PYCPP_END_NAMESPACE
