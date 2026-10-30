/*
   Copyright (c) 2016, Facebook, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* This C++ file's header */
#include "./rdb_utils.h"

/* C++ standard header files */
#include <algorithm>
#include <array>
#include <new>
#include <sstream>
#include <string>
#include <vector>

/* C standard header files */
#include <ctype.h>

/* ICU header files */
#include <unicode/regex.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

/* MySQL header files */
#include "my_dir.h"
#include "sql/mysqld.h"

/* MyRocks header files */
#include "./ha_rocksdb.h"

/* RocksDB includes */
#include "rocksdb/env.h"
#include "rocksdb/file_system.h"
#include "rocksdb/io_status.h"

namespace myrocks {

/*
  Skip past any spaces in the input
*/
const char *rdb_skip_spaces(const CHARSET_INFO *const cs, const char *str) {
  assert(cs != nullptr);
  assert(str != nullptr);

  while (my_isspace(cs, *str)) {
    str++;
  }

  return str;
}

/*
  Compare (ignoring case) to see if str2 is the next data in str1.
  Note that str1 can be longer but we only compare up to the number
  of characters in str2.
*/
bool rdb_compare_strings_ic(const char *const str1, const char *const str2) {
  assert(str1 != nullptr);
  assert(str2 != nullptr);

  // Scan through the strings
  size_t ii;
  for (ii = 0; str2[ii]; ii++) {
    if (toupper(static_cast<int>(str1[ii])) !=
        toupper(static_cast<int>(str2[ii]))) {
      return false;
    }
  }

  return true;
}

/*
  Scan through an input string looking for pattern, ignoring case
  and skipping all data enclosed in quotes.
*/
const char *rdb_find_in_string(const char *str, const char *pattern,
                               bool *const succeeded) {
  char quote = '\0';
  bool escape = false;

  assert(str != nullptr);
  assert(pattern != nullptr);
  assert(succeeded != nullptr);

  *succeeded = false;

  for (; *str; str++) {
    /* If we found a our starting quote character */
    if (*str == quote) {
      /* If it was escaped ignore it */
      if (escape) {
        escape = false;
      }
      /* Otherwise we are now outside of the quoted string */
      else {
        quote = '\0';
      }
    }
    /* Else if we are currently inside a quoted string? */
    else if (quote != '\0') {
      /* If so, check for the escape character */
      escape = !escape && *str == '\\';
    }
    /* Else if we found a quote we are starting a quoted string */
    else if (*str == '"' || *str == '\'' || *str == '`') {
      quote = *str;
    }
    /* Else we are outside of a quoted string - look for our pattern */
    else {
      if (rdb_compare_strings_ic(str, pattern)) {
        *succeeded = true;
        return str;
      }
    }
  }

  // Return the character after the found pattern or the null terminateor
  // if the pattern wasn't found.
  return str;
}

/*
  See if the next valid token matches the specified string
*/
const char *rdb_check_next_token(const CHARSET_INFO *const cs, const char *str,
                                 const char *const pattern,
                                 bool *const succeeded) {
  assert(cs != nullptr);
  assert(str != nullptr);
  assert(pattern != nullptr);
  assert(succeeded != nullptr);

  // Move past any spaces
  str = rdb_skip_spaces(cs, str);

  // See if the next characters match the pattern
  if (rdb_compare_strings_ic(str, pattern)) {
    *succeeded = true;
    return str + strlen(pattern);
  }

  *succeeded = false;
  return str;
}

/*
  Parse id
*/
const char *rdb_parse_id(const CHARSET_INFO *const cs, const char *str,
                         std::string *const id) {
  assert(cs != nullptr);
  assert(str != nullptr);

  // Move past any spaces
  str = rdb_skip_spaces(cs, str);

  if (*str == '\0') {
    return str;
  }

  char quote = '\0';
  if (*str == '`' || *str == '"') {
    quote = *str++;
  }

  size_t len = 0;
  const char *start = str;

  if (quote != '\0') {
    for (;;) {
      if (*str == '\0') {
        return str;
      }

      if (*str == quote) {
        str++;
        if (*str != quote) {
          break;
        }
      }

      str++;
      len++;
    }
  } else {
    while (!my_isspace(cs, *str) && *str != '(' && *str != ')' && *str != '.' &&
           *str != ',' && *str != '\0') {
      str++;
      len++;
    }
  }

  // If the user requested the id create it and return it
  if (id != nullptr) {
    *id = std::string("");
    id->reserve(len);
    while (len--) {
      *id += *start;
      if (*start++ == quote) {
        start++;
      }
    }
  }

  return str;
}

/*
  Skip id
*/
const char *rdb_skip_id(const CHARSET_INFO *const cs, const char *str) {
  assert(cs != nullptr);
  assert(str != nullptr);

  return rdb_parse_id(cs, str, nullptr);
}

/*
  Parses a given string into tokens (if any) separated by a specific delimiter.
*/
const std::vector<std::string> parse_into_tokens(const std::string &s,
                                                 const char delim) {
  std::vector<std::string> tokens;
  std::string t;
  std::stringstream ss(s);
  while (getline(ss, t, delim)) {
    rdb_trim_whitespace_from_edges(t);
    if (!t.empty()) tokens.push_back(t);
  }

  return tokens;
}

constexpr std::size_t rdb_hex_bytes_per_char = 2;
constexpr std::array<char, 16> rdb_hexdigit = {{'0', '1', '2', '3', '4', '5',
                                                '6', '7', '8', '9', 'a', 'b',
                                                'c', 'd', 'e', 'f'}};

/*
  Convert data into a hex string. If the data is larger than the maximum length
  trancate it and append "..". If maxsize is zero, the output length is
  unlimited.
*/
std::string rdb_hexdump(const char *data, std::size_t data_len,
                        std::size_t maxsize) {
  assert(data != nullptr);

  // Count the elements in the string
  std::size_t elems = data_len;
  // Calculate the amount of output needed
  std::size_t len = elems * rdb_hex_bytes_per_char;
  std::string str;

  if (maxsize != 0 && len > maxsize) {
    // If the amount of output is too large adjust the settings
    // and leave room for the ".." at the end
    elems = (maxsize - 2) / rdb_hex_bytes_per_char;
    len = elems * rdb_hex_bytes_per_char + 2;
  }

  // Reserve sufficient space to avoid reallocations
  str.reserve(len);

  // Loop through the input data and build the output string
  for (std::size_t ii = 0; ii < elems; ii++, data++) {
    uint8_t ch = (uint8_t)*data;
    str += rdb_hexdigit[ch >> 4];
    str += rdb_hexdigit[ch & 0x0F];
  }

  // If we can't fit it all add the ".."
  if (elems != data_len) {
    str += "..";
  }

  return str;
}

// Return dir + '/' + file
std::string rdb_concat_paths(const std::string &dir, const std::string &file) {
  std::string result;
  result.reserve(dir.length() + file.length() + 2);
  result = dir;
  result += FN_LIBCHAR;
  result += file;
  return result;
}

/*
  Attempt to access the database subdirectory to see if it exists
*/
bool rdb_database_exists(const std::string &db_name) {
  const auto dir = rdb_concat_paths(mysql_real_data_home, db_name);
  MY_DIR *const dir_info =
      my_dir(dir.c_str(), MYF(MY_DONT_SORT | MY_WANT_STAT));
  if (dir_info == nullptr) {
    return false;
  }

  my_dirend(dir_info);
  return true;
}

void rdb_log_status_error(const rocksdb::Status &s, const char *msg) {
  if (msg == nullptr) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Status error, code: %d, error message: %s", s.code(),
                    s.ToString().c_str());
    return;
  }

  LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "%s, Status Code: %d, Status: %s", msg, s.code(),
                  s.ToString().c_str());
}

/*
  Set the patterns string.  If there are invalid regex patterns they will
  be stored in m_bad_patterns and the result will be false, otherwise the
  result will be true.
*/
Regex_list_handler::Regex_list_handler(const PSI_rwlock_key &key,
                                       char delimiter)
    : m_key(key), m_delimiter(delimiter) {
  mysql_rwlock_init(key, &m_rwlock);
}

Regex_list_handler::~Regex_list_handler() { mysql_rwlock_destroy(&m_rwlock); }

bool Regex_list_handler::set_patterns(const std::string &pattern_str,
                                      uint32_t flags) {
  // Replace all delimiters with the '|' operator to create one regex. Note
  // that this means the delimiter can not be part of a regular expression.
  // This is currently not a problem as we are using the comma character as a
  // delimiter and commas are not valid in table names.
  std::string norm_pattern = pattern_str;
  std::replace(norm_pattern.begin(), norm_pattern.end(), m_delimiter, '|');

  // Strictly convert the pattern to UTF-16; invalid UTF-8 is an error.
  UErrorCode status = U_ZERO_ERROR;
  int32_t length16 = 0;
  u_strFromUTF8(nullptr, 0, &length16, norm_pattern.data(),
                static_cast<int32_t>(norm_pattern.size()), &status);
  if (status == U_BUFFER_OVERFLOW_ERROR) status = U_ZERO_ERROR;
  std::unique_ptr<char16_t[]> pattern16;
  if (U_SUCCESS(status)) {
    pattern16.reset(new (std::nothrow) char16_t[length16 + 1]);
    if (pattern16 == nullptr) status = U_MEMORY_ALLOCATION_ERROR;
  }
  if (U_SUCCESS(status)) {
    u_strFromUTF8(pattern16.get(), length16 + 1, nullptr, norm_pattern.data(),
                  static_cast<int32_t>(norm_pattern.size()), &status);
  }

  // ICU compiles iteratively, so deeply nested groups cannot overflow the
  // thread stack the way they do with std::regex; it rejects nesting deeper
  // than its parser stack instead. The UText overload is used because the
  // UnicodeString one does not check the allocation of its pattern copy.
  std::unique_ptr<icu::RegexPattern> pattern;
  if (U_SUCCESS(status)) {
    UText text_storage = UTEXT_INITIALIZER;
    UText *const text =
        utext_openUChars(&text_storage, pattern16.get(), length16, &status);
    UParseError parse_error;
    if (U_SUCCESS(status)) {
      pattern.reset(
          icu::RegexPattern::compile(text, flags, parse_error, status));
    }
    utext_close(text);
    if (U_SUCCESS(status) && pattern == nullptr)
      status = U_MEMORY_ALLOCATION_ERROR;
  }
  const bool pattern_valid = U_SUCCESS(status);

  // Make sure no one else is accessing the list while we are changing it.
  mysql_rwlock_wrlock(&m_rwlock);

  // Clear out any old error information
  m_bad_pattern_str.clear();

  if (pattern_valid) {
    m_pattern = std::move(pattern);
    m_pattern16 = std::move(pattern16);
    m_pattern_str = norm_pattern;
    m_match_error_logged.store(false, std::memory_order_relaxed);
    m_generation.fetch_add(1);
  } else {
    // Put the bad pattern into a member variable so it can be retrieved later.
    m_bad_pattern_str = pattern_str;
  }

  mysql_rwlock_unlock(&m_rwlock);

  return pattern_valid;
}

bool Regex_list_handler::matches(const std::string &str) const {
  // Make sure no one else changes the list while we are accessing it.
  mysql_rwlock_rdlock(&m_rwlock);
  assert(m_pattern != nullptr);

  UErrorCode status = U_ZERO_ERROR;
  bool found = false;
  UText text_storage = UTEXT_INITIALIZER;
  UText *const text = utext_openUTF8(&text_storage, str.data(),
                                     static_cast<int64_t>(str.size()), &status);
  {
    // Each match gets its own matcher, so concurrent matches share no
    // mutable state. The matcher refers to the text, so it goes first.
    std::unique_ptr<icu::RegexMatcher> matcher;
    if (U_SUCCESS(status)) {
      matcher.reset(m_pattern->matcher(status));
      if (U_SUCCESS(status) && matcher == nullptr)
        status = U_MEMORY_ALLOCATION_ERROR;
    }
    if (U_SUCCESS(status)) {
      matcher->reset(text);
      matcher->setTimeLimit(opt_regexp_time_limit, status);
      matcher->setStackLimit(opt_regexp_stack_limit, status);
    }
    if (U_SUCCESS(status)) found = matcher->matches(status);
  }
  utext_close(text);

  if (U_FAILURE(status)) {
    found = false;
    if (!m_match_error_logged.exchange(true, std::memory_order_relaxed)) {
      LogPluginErrMsg(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                      "Matching '%s' against the regular expression '%s' "
                      "failed (%s) and is treated as no match. Check "
                      "regexp_time_limit and regexp_stack_limit. Further "
                      "failures of this pattern are not logged.",
                      str.c_str(), m_pattern_str.c_str(), u_errorName(status));
    }
  }

  // Release the lock
  mysql_rwlock_unlock(&m_rwlock);

  return found;
}

void warn_about_bad_patterns(const Regex_list_handler *regex_list_handler,
                             const char *name) {
  // There was some invalid regular expression data in the patterns supplied

  LogPluginErrMsg(WARNING_LEVEL, ER_LOG_PRINTF_MSG, "Invalid pattern in %s: %s",
                  name, regex_list_handler->bad_pattern().c_str());
}

// Split a string based on a delimiter.  Two delimiters in a row will not add
// an empty string in the set.
std::vector<std::string> split_into_vector(const std::string &input,
                                           char delimiter) {
  size_t pos;
  size_t start = 0;
  std::vector<std::string> elems;

  // Find next delimiter
  while ((pos = input.find(delimiter, start)) != std::string::npos) {
    // If there is any data since the last delimiter add it to the list
    if (pos > start) elems.push_back(input.substr(start, pos - start));

    // Set our start position to the character after the delimiter
    start = pos + 1;
  }

  // Add a possible string since the last delimiter
  if (input.length() > start) elems.push_back(input.substr(start));

  // Return the resulting list back to the caller
  return elems;
}

bool rdb_has_rocksdb_corruption() {
  rocksdb::DBOptions *rocksdb_db_options = get_rocksdb_db_options();
  assert(rocksdb_db_options->env != nullptr);
  const std::string fileName(myrocks::rdb_corruption_marker_file_name());

  const auto &fs = rocksdb_db_options->env->GetFileSystem();
  rocksdb::IOStatus io_s =
      fs->FileExists(fileName, rocksdb::IOOptions(), nullptr);

  return io_s.ok();
}

void rdb_persist_corruption_marker() {
  rocksdb::DBOptions *rocksdb_db_options = get_rocksdb_db_options();
  assert(rocksdb_db_options->env != nullptr);
  const std::string fileName(myrocks::rdb_corruption_marker_file_name());

  const auto &fs = rocksdb_db_options->env->GetFileSystem();
  std::unique_ptr<rocksdb::FSWritableFile> file;
  rocksdb::IOStatus io_s =
      fs->NewWritableFile(fileName, rocksdb::FileOptions(), &file, nullptr);

  if (!io_s.ok()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Can't create file %s to mark rocksdb as corrupted.",
                    fileName.c_str());
  } else {
    LogPluginErrMsg(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                    "Creating the file %s to abort mysqld restarts. Remove "
                    "this file from the data directory after fixing the "
                    "corruption to recover. ",
                    fileName.c_str());
  }

  io_s = file->Close(rocksdb::IOOptions(), nullptr);
  if (!io_s.ok()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Error (%s) closing the file %s", io_s.ToString().c_str(),
                    fileName.c_str());
  }
}

}  // namespace myrocks
