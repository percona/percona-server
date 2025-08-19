#include "components/bulk_load/rows_reader/text_row.h"

#include <unordered_set>
#include <iostream>
#include <string>

namespace Bulk_load {

enum class CsvParserState {
  Ok,
  Error,
  EndOfBuffer
};

class CsvParser final {
 public:

  using buffer_iterator = char *;
  using const_buffer_iterator = const char *;
  using character_predicate = bool (CsvParser::*)(char);

  CsvParser(TextRow *text_row,
            const std::string &column_terminator,
            const std::string &row_terminator,
            const std::string &escape_char,
            const std::string &column_enclose_char)
    : m_text_row{text_row},
      m_column_terminator{column_terminator}, m_row_terminator{row_terminator},
      m_escape_char{escape_char}, m_column_enclose_char{column_enclose_char},
      m_state{CsvParserState::Ok} {
//    m_character_predicates = {'\r', '\n', ',', '"'};
    m_character_predicates = {'"'};
    m_separator_predicates = {','};
  }

  // <line> ::= <value> <separator_and_value>* <eol>
  CsvParserState parse_line(buffer_iterator &it, const_buffer_iterator end) {
    m_state = CsvParserState::Ok;
    m_text_row->reset();
    buffer_iterator it_backup{it};

    if (!parse_value_with_action(it, end) ||
        !parse_star(it, end,
                    &CsvParser::separator_predicate,
                    &CsvParser::parse_separator_and_value) ||
        !parse_row_terminator(it, end)) {
      it = it_backup;
    }

    return m_state;
  }

 private:
  bool parse_value_with_action(buffer_iterator &it, const_buffer_iterator end) {
    if (!parse_column_enclose(it, end)) {
      return false;
    }

    buffer_iterator value_begin{it};

    if (!parse_value(it, end)) {
      return false;
    }

    buffer_iterator value_end{it};

    if (!parse_column_enclose(it, end)) {
      return false;
    }

    m_text_row->set_column(value_begin, value_end);

    return true;
  }

  // <value> ::= <value_character>*
  bool parse_value(buffer_iterator &it, const_buffer_iterator end) {
    return parse_star(it, end,
                      &CsvParser::value_character_predicate,
                      &CsvParser::parse_value_character);
  }

  bool parse_character(buffer_iterator &it, const_buffer_iterator end, char ch) {
    if (it == end) {
      m_state = CsvParserState::EndOfBuffer;
      return false;
    }
    if (*it != ch) {
      std::cout << "!!! parse_character expected: " << ch << std::endl;
      m_state = CsvParserState::Error;
      return false;
    }

    ++it;

    return true;
  }

  template<typename Predicate>
  bool parse_character_predicate(buffer_iterator &it, const_buffer_iterator end,
                                 const Predicate &predicate) {
    if (it == end) {
      m_state = CsvParserState::EndOfBuffer;
      return false;
    }
    if (!(this->*predicate)(*it)) {
      std::cout << "!!! parse_character_predicate expected <predicate>: " << std::endl;
      m_state = CsvParserState::Error;
      return false;
    }

    ++it;

    return true;
  }

  // a helper for handling <rule>* (0 or more <rule>)
  template<typename Predicate, typename Rule>
  bool parse_star(buffer_iterator &it, const_buffer_iterator end,
                  const Predicate &predicate, const Rule &rule) {
    while ((this->*predicate)(*it)) {
      if (!(this->*rule)(it, end)) {
        return false;
      }
    }

    return true;
  }

  // a helper for handling <rule>+ (1 or more <rule>)
  template<typename Predicate, typename Rule>
  void parse_plus(buffer_iterator &it, const_buffer_iterator end,
                  const Predicate &predicate, const Rule &rule) {
    rule(it, end);
    parse_star(it, end, predicate, rule);
  }

  // <separator> ::= ','
  bool parse_column_terminator(buffer_iterator &it, const_buffer_iterator end) {
    return parse_seq(it, end, m_column_terminator);
  }

  // <eol> ::= '\r' '\n'
  bool parse_row_terminator(buffer_iterator &it, const_buffer_iterator end) {
    return parse_seq(it, end, m_row_terminator);
  }

  bool parse_column_enclose(buffer_iterator &it, const_buffer_iterator end) {
    return parse_seq(it, end, m_column_enclose_char);
  }

  bool parse_seq(buffer_iterator &it, const_buffer_iterator end,
                 const std::string &seq) {
    for (const auto ch : seq) {
      if (!parse_character(it, end, ch)) {
        return false;
      }
    }

    return true;
  }

  // <value_character> ::= [^\r\n,""]
  bool parse_value_character(buffer_iterator &it, const_buffer_iterator end) {
    return parse_character_predicate(it, end,
                                     &CsvParser::value_character_predicate);
  }

  // <separator_and_value> ::= <separator><value>
  bool parse_separator_and_value(buffer_iterator &it, const_buffer_iterator end) {
    return parse_column_terminator(it, end) &&
           parse_value_with_action(it, end);
  }

  bool value_character_predicate(char ch) const noexcept {
    return !m_character_predicates.contains(ch);
  }

  bool separator_predicate(char ch) const noexcept {
    return m_separator_predicates.contains(ch);
  }

  TextRow *m_text_row;
  const std::string m_column_terminator;
  const std::string m_row_terminator;
  const std::string m_escape_char;
  const std::string m_column_enclose_char;
  CsvParserState m_state;
  std::unordered_set<char> m_character_predicates;
  std::unordered_set<char> m_separator_predicates;
};

}  // namespace Bulk_load
