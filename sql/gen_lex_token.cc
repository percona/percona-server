/*
   Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* We only need the tokens here */
#define YYSTYPE_IS_DECLARED

#include "sql/lex.h"
#include "sql/lex_symbol.h"
#include "sql/sql_yacc.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

/*
  MAINTAINER:

  Tokens printed in sql/lex_token.h do come from several sources:
  - tokens from sql_yacc.yy
  - tokens from sql_hints.yy
  - fake tokens for digests.

  All the token values are mapped in the same space,
  indexed by the token value directly.

  To account for enhancements and new tokens,
  gap are created, so that adding a token from one source
  does not change values of tokens from other sources.

  This is done to ensure stability in digest computed values.

  As of now (8.0.0), the mapping looks like this:
<<<<<<< HEAD
  - [0 .. 255] character terminal tokens.
  - [256 .. 907] non terminal tokens from sql_yacc.yy
  - [908 .. 999] reserved for sql_yacc.yy new tokens
  - [1000 .. 1017] non terminal tokens from sql_hints.yy
  - [1018 .. 1099] reserved for sql_hints.yy new tokens
  - [1100 .. 1299] non terminal tokens from sql_yacc.yy_
  - [1300 .. ] Percona tokens from sql_yacc.yy_
||||||| ea7d2e2d16a
  - [0 .. 255] character terminal tokens.
  - [256 .. 907] non terminal tokens from sql_yacc.yy
  - [908 .. 999] reserved for sql_yacc.yy new tokens
  - [1000 .. 1017] non terminal tokens from sql_hints.yy
  - [1018 .. 1099] reserved for sql_hints.yy new tokens
  - [1100 .. 1111] non terminal tokens for digests
=======
  - PART 1: [0 .. 255] tokens of single-character lexemes
  - PART 2: [256 .. ...] tokens < NOT_A_TOKEN_SYM from sql_yacc.yy
  - PART 3: [... .. 999] reserved for sql_yacc.yy new tokens < NOT_A_TOKEN_SYM
  - PART 4: [1000 .. ...] tokens from sql_hints.yy
  - PART 5: [... .. 1099] reserved for sql_hints.yy new tokens
  - PART 6: [1100 .. ...] digest special fake tokens
  - PART 7: [... .. 1149] reserved for new digest special fake tokens
  - PART 8: [1150 .. ...] tokens > NOT_A_TOKEN_SYM from sql_yacc.yy
>>>>>>> mysql-8.0.20

  Should gen_lex_token fail when tokens are exhausted
  (maybe you are reading this comment because of a fprintf(stderr) below),
  the options are as follows, by order of decreasing desirability:

  1) Reuse OBSOLETE_TOKEN_XXX instead of consuming new token values

  2) Consider if you really need to create a new token,
  instead of re using an existing one.

  Keep in mind that syntax sugar in the parser still adds
  to complexity, by making the parser tables bigger,
  so adding tokens all the time is not a good practice.

  3) Expand boundary values for
     - range_for_sql_hints
     - range_for_digests
  and record again all the MTR tests that print a DIGEST,
  because DIGEST values have now changed.

  While at it, because digests have changed anyway,
  please seriously consider to clean up and reorder:
  - all the tokens in sql/sql_yacc.yy in one nice list,
  ordered alphabetically, removing obsolete values if any.
  - likewise for sql/sql_hints.yy
*/

<<<<<<< HEAD
int start_token_range_for_sql_hints = 1000;
int start_token_range_for_digests = 1100;
int start_token_range_for_sql_percona = 1300;
int percona_tokens = 0;

/*
  This is a tool used during build only,
  so MY_MAX_TOKEN does not need to be exact,
  only big enough to hold:
  - 256 character terminal tokens
  - YYNTOKENS named terminal tokens from bison (sql_yacc.yy).
  - padding (see start_token_range_for_sql_hints)
  - YYNTOKENS named terminal tokens from bison (sql_hints.yy).
  - padding (see start_token_range_for_digests)
  - DIGEST special tokens.
  See also YYMAXUTOK.
*/
#define MY_MAX_TOKEN 1400
||||||| ea7d2e2d16a
int start_token_range_for_sql_hints = 1000;
int start_token_range_for_digests = 1100;
/*
  This is a tool used during build only,
  so MY_MAX_TOKEN does not need to be exact,
  only big enough to hold:
  - 256 character terminal tokens
  - YYNTOKENS named terminal tokens from bison (sql_yacc.yy).
  - padding (see start_token_range_for_sql_hints)
  - YYNTOKENS named terminal tokens from bison (sql_hints.yy).
  - padding (see start_token_range_for_digests)
  - DIGEST special tokens.
  See also YYMAXUTOK.
*/
#define MY_MAX_TOKEN 1200
=======
>>>>>>> mysql-8.0.20
/** Generated token. */
struct gen_lex_token_string {
  gen_lex_token_string(const char *token_string, int token_length,
                       bool append_space, bool start_expr)
      : m_token_string{token_string},
        m_token_length{token_length},
        m_append_space{append_space},
        m_start_expr{start_expr} {}

  gen_lex_token_string()
      : m_token_string{},
        m_token_length{},
        m_append_space{true},
        m_start_expr{false} {}

  /**
    Real lexeme string or user-specified text to output with a normalized
    query string.
  */
  const char *m_token_string;

  /**
    Byte length of m_token_string.
  */
  int m_token_length;

  /**
    If true, output ' ' after this token to a normalized query string.
    See digest_add_token().
  */
  bool m_append_space;

  /**
    See digest_add_token().
  */
  bool m_start_expr;
<<<<<<< HEAD
  bool m_percona_token;
||||||| ea7d2e2d16a
=======

  /**
    The structure is uninitialized if false.
  */
  bool m_initialized{false};
>>>>>>> mysql-8.0.20
};

/*
  This is a tool used during build only,
  so MY_MAX_TOKEN does not need to be exact,
  only big enough to hold:
  - 256 of single-character lexeme tokens
  - up to 1000 named tokens from bison (sql_yacc.yy).
  - padding
  - tokens from bison (sql_hints.yy).
  - padding
  - DIGEST special tokens.
  - padding
  - mode named tokens from bison (sql_yacc.yy).
  See also YYMAXUTOK.
*/
const int MY_MAX_TOKEN = 1200;

gen_lex_token_string compiled_token_array[MY_MAX_TOKEN];
<<<<<<< HEAD
int max_token_seen = 0;
int max_token_seen_in_sql_yacc = 0;
int max_token_seen_in_sql_hints = 0;
int max_token_seen_in_special_tokens = 0;
||||||| ea7d2e2d16a
int max_token_seen = 0;
int max_token_seen_in_sql_yacc = 0;
int max_token_seen_in_sql_hints = 0;
=======
>>>>>>> mysql-8.0.20

struct range {
  range(const char *title, int start, int end)
      : title{title}, start{start}, end{end}, max_seen{0} {}

  void set_token(int tok, const char *str, int line) {
    if (tok <= 0) {
      fprintf(stderr, "%s:%d: Bad token found\n", __FILE__, line);
      exit(1);
    }

    if (tok > end) {
      fprintf(stderr,
              "%s:%d: Token reserve for %s exhausted: %d (should be <= %d).\n"
              "Please see MAINTAINER instructions in sql/gen_lex_token.cc\n",
              __FILE__, line, title, tok, end);
      exit(1);
    }

    if (tok >= MY_MAX_TOKEN) {
      fprintf(stderr,
              "%s:%d: Added that many new keywords ? Increase MY_MAX_TOKEN\n",
              __FILE__, line);
      exit(1);
    }

    if (tok > max_seen) {
      max_seen = tok;
    }

    compiled_token_array[tok].m_initialized = true;
    compiled_token_array[tok].m_token_string = str;
    compiled_token_array[tok].m_token_length = strlen(str);
    compiled_token_array[tok].m_append_space = true;
    compiled_token_array[tok].m_start_expr = false;
  }

  int add_token(const char *str, int line) {
    set_token(max_seen ? max_seen + 1 : start, str, line);
    return max_seen;
  }

  void print(const char *header1, const char *header2 = nullptr) const {
    puts(header1);
    for (int tok = start; tok <= max_seen; tok++) {
      print_token(tok);
    }

    if (header2 == nullptr) {
      return;
    }

    puts(header2);
    for (int tok = max_seen + 1; tok <= end; tok++) {
      printf("/* reserved %03d for %s */  { \"\", 0, false, false},\n", tok,
             title);
    }
  }

 private:
  void print_token(int tok) const {
    const gen_lex_token_string *x = &compiled_token_array[tok];
    if (tok < 256) {
      printf("/* %03d */  { \"\\x%02x\", 1, %s, %s},\n", tok, tok,
             x->m_append_space ? "true" : "false",
             x->m_start_expr ? "true" : "false");
      return;
    }

    if (!x->m_initialized) {
      static const gen_lex_token_string dummy{"(unknown)", 9, true, false};
      x = &dummy;
    }
    printf("/* %03d */  { \"%s\", %d, %s, %s},\n", tok, x->m_token_string,
           x->m_token_length, x->m_append_space ? "true" : "false",
           x->m_start_expr ? "true" : "false");
  }

 private:
  const char *const title;

 public:
  const int start;

 private:
  const int end;

  int max_seen;
};

static_assert(NOT_A_TOKEN_SYM == 1150,
              "NOT_A_TOKEN_SYM should be equal to 1150");
range range_for_sql_yacc2{"sql/sql_yacc.yy (before NOT_A_TOKEN_SYM)",
                          NOT_A_TOKEN_SYM, MY_MAX_TOKEN};

range range_for_digests{"digest specials", 1100, range_for_sql_yacc2.start - 1};

static_assert(MAX_EXECUTION_TIME_HINT == 1000,
              "MAX_EXECUTION_TIME_HINT should be equal to 1000");
range range_for_sql_hints{"sql/sql_hints.yy", MAX_EXECUTION_TIME_HINT,
                          range_for_digests.start - 1};

range range_for_sql_yacc1{"sql/sql_yacc.yy (after NOT_A_TOKEN_SYM)", 256,
                          range_for_sql_hints.start - 1};

int tok_generic_value = 0;
int tok_generic_value_list = 0;
int tok_row_single_value = 0;
int tok_row_single_value_list = 0;
int tok_row_multiple_value = 0;
int tok_row_multiple_value_list = 0;
int tok_in_generic_value_expression = 0;
int tok_ident = 0;
int tok_ident_at = 0;  ///< Fake token for the left part of table\@query_block.
int tok_hint_comment_open =
    0;  ///< Fake token value for "/*+" of hint comments.
int tok_hint_comment_close =
    0;  ///< Fake token value for "*/" of hint comments.
int tok_unused = 0;

<<<<<<< HEAD
/**
  Adjustment value to translate hint parser's internal token values to generally
  visible token values. This adjustment is necessary, since keyword token values
  of separate parsers may interfere.
*/
int tok_hint_adjust = 0;
int tok_percona_adjust = 0;

static void set_token(int tok, const char *str, bool percona_token = false) {
  if (tok <= 0) {
    fprintf(stderr, "Bad token found\n");
    exit(1);
  }

  if (tok > max_token_seen) {
    max_token_seen = tok;
  }

  if (max_token_seen >= MY_MAX_TOKEN) {
    fprintf(stderr, "Added that many new keywords ? Increase MY_MAX_TOKEN\n");
    exit(1);
  }

  compiled_token_array[tok].m_token_string = str;
  compiled_token_array[tok].m_token_length = strlen(str);
  compiled_token_array[tok].m_append_space = true;
  compiled_token_array[tok].m_start_expr = false;
  compiled_token_array[tok].m_percona_token = percona_token;
}

||||||| ea7d2e2d16a
/**
  Adjustment value to translate hint parser's internal token values to generally
  visible token values. This adjustment is necessary, since keyword token values
  of separate parsers may interfere.
*/
int tok_hint_adjust = 0;

static void set_token(int tok, const char *str) {
  if (tok <= 0) {
    fprintf(stderr, "Bad token found\n");
    exit(1);
  }

  if (tok > max_token_seen) {
    max_token_seen = tok;
  }

  if (max_token_seen >= MY_MAX_TOKEN) {
    fprintf(stderr, "Added that many new keywords ? Increase MY_MAX_TOKEN\n");
    exit(1);
  }

  compiled_token_array[tok].m_token_string = str;
  compiled_token_array[tok].m_token_length = strlen(str);
  compiled_token_array[tok].m_append_space = true;
  compiled_token_array[tok].m_start_expr = false;
}

=======
>>>>>>> mysql-8.0.20
static void set_start_expr_token(int tok) {
  compiled_token_array[tok].m_start_expr = true;
}

static void add_percona_tokens() {
  int tok_percona_min = INT_MAX;
  for (unsigned int i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
    if (!(symbols[i].percona_symbol)) continue;
    if (!(symbols[i].group & SG_MAIN_PARSER)) continue;

    if (static_cast<int>(symbols[i].tok) < tok_percona_min)
      tok_percona_min =
          symbols[i].tok;  // Calculate the minimal Percona token value.
  }

  tok_percona_adjust = start_token_range_for_sql_percona - tok_percona_min;

  for (unsigned int i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
    if (!(symbols[i].percona_symbol)) continue;
    if (!(symbols[i].group & SG_MAIN_PARSER)) continue;
    set_token(symbols[i].tok + tok_percona_adjust, symbols[i].name, true);
    percona_tokens++;
  }
}

static void compute_tokens() {
<<<<<<< HEAD
  int tok;
  char *str;

  /*
    Default value.
  */
  for (tok = 0; tok < MY_MAX_TOKEN; tok++) {
    compiled_token_array[tok].m_token_string = "(unknown)";
    compiled_token_array[tok].m_token_length = 9;
    compiled_token_array[tok].m_append_space = true;
    compiled_token_array[tok].m_start_expr = false;
    compiled_token_array[tok].m_percona_token = false;
  }

||||||| ea7d2e2d16a
  int tok;
  char *str;

  /*
    Default value.
  */
  for (tok = 0; tok < MY_MAX_TOKEN; tok++) {
    compiled_token_array[tok].m_token_string = "(unknown)";
    compiled_token_array[tok].m_token_length = 9;
    compiled_token_array[tok].m_append_space = true;
    compiled_token_array[tok].m_start_expr = false;
  }

=======
>>>>>>> mysql-8.0.20
  /*
    Tokens made of just one terminal character
  */

  // Do nothing -- see range::print() for token numbers in [0 .. 255]

  /*
    Tokens hard coded in sql_lex.cc
  */

  range_for_sql_yacc1.set_token(WITH_ROLLUP_SYM, "WITH ROLLUP", __LINE__);
  range_for_sql_yacc1.set_token(NOT2_SYM, "!", __LINE__);
  range_for_sql_yacc1.set_token(OR2_SYM, "||", __LINE__);
  range_for_sql_yacc1.set_token(PARAM_MARKER, "?", __LINE__);
  range_for_sql_yacc1.set_token(SET_VAR, ":=", __LINE__);
  range_for_sql_yacc1.set_token(UNDERSCORE_CHARSET, "(_charset)", __LINE__);
  range_for_sql_yacc1.set_token(END_OF_INPUT, "", __LINE__);
  range_for_sql_yacc1.set_token(JSON_SEPARATOR_SYM, "->", __LINE__);
  range_for_sql_yacc1.set_token(JSON_UNQUOTED_SEPARATOR_SYM, "->>", __LINE__);

  /*
    Values.
    These tokens are all normalized later,
    so this strings will never be displayed.
  */
  range_for_sql_yacc1.set_token(BIN_NUM, "(bin)", __LINE__);
  range_for_sql_yacc1.set_token(DECIMAL_NUM, "(decimal)", __LINE__);
  range_for_sql_yacc1.set_token(FLOAT_NUM, "(float)", __LINE__);
  range_for_sql_yacc1.set_token(HEX_NUM, "(hex)", __LINE__);
  range_for_sql_yacc1.set_token(LEX_HOSTNAME, "(hostname)", __LINE__);
  range_for_sql_yacc1.set_token(LONG_NUM, "(long)", __LINE__);
  range_for_sql_yacc1.set_token(NUM, "(num)", __LINE__);
  range_for_sql_yacc1.set_token(TEXT_STRING, "(text)", __LINE__);
  range_for_sql_yacc1.set_token(NCHAR_STRING, "(nchar)", __LINE__);
  range_for_sql_yacc1.set_token(ULONGLONG_NUM, "(ulonglong)", __LINE__);

  /*
    Identifiers.
  */
  range_for_sql_yacc1.set_token(IDENT, "(id)", __LINE__);
  range_for_sql_yacc1.set_token(IDENT_QUOTED, "(id_quoted)", __LINE__);

  /*
    See symbols[] in sql/lex.h
  */
  for (const SYMBOL &sym : symbols) {
<<<<<<< HEAD
    if (sym.percona_symbol) continue;
    if (!(sym.group & SG_MAIN_PARSER)) continue;
    set_token(sym.tok, sym.name);
  }

  max_token_seen_in_sql_yacc = max_token_seen;

  if (max_token_seen_in_sql_yacc >= start_token_range_for_sql_hints) {
    fprintf(stderr,
            "sql/sql_yacc.yy token reserve exhausted.\n"
            "Please see MAINTAINER instructions in sql/gen_lex_token.cc\n");
    exit(1);
||||||| ea7d2e2d16a
    if (!(sym.group & SG_MAIN_PARSER)) continue;
    set_token(sym.tok, sym.name);
  }

  max_token_seen_in_sql_yacc = max_token_seen;

  if (max_token_seen_in_sql_yacc >= start_token_range_for_sql_hints) {
    fprintf(stderr,
            "sql/sql_yacc.yy token reserve exhausted.\n"
            "Please see MAINTAINER instructions in sql/gen_lex_token.cc\n");
    exit(1);
=======
    if ((sym.group & SG_MAIN_PARSER) != 0) {
      if (sym.tok < NOT_A_TOKEN_SYM)
        range_for_sql_yacc1.set_token(sym.tok, sym.name, __LINE__);
      else
        range_for_sql_yacc2.set_token(sym.tok, sym.name, __LINE__);
    } else if ((sym.group & SG_HINTS) != 0) {
      range_for_sql_hints.set_token(sym.tok, sym.name, __LINE__);
    } else {
      fprintf(stderr, "%s:%d: Unknown symbol group flag: %x\n", __FILE__,
              __LINE__, sym.group & ~(SG_MAIN_PARSER | SG_HINTS));
      exit(1);
    }
>>>>>>> mysql-8.0.20
  }

  /*
    Additional FAKE tokens,
    used internally to normalize a digest text.
  */

  /* Digest tokens in 5.7 */

  tok_generic_value = range_for_digests.add_token("?", __LINE__);
  tok_generic_value_list = range_for_digests.add_token("?, ...", __LINE__);
  tok_row_single_value = range_for_digests.add_token("(?)", __LINE__);
  tok_row_single_value_list =
      range_for_digests.add_token("(?) /* , ... */", __LINE__);
  tok_row_multiple_value = range_for_digests.add_token("(...)", __LINE__);
  tok_row_multiple_value_list =
      range_for_digests.add_token("(...) /* , ... */", __LINE__);
  tok_ident = range_for_digests.add_token("(tok_id)", __LINE__);
  tok_ident_at = range_for_digests.add_token("(tok_id_at)", __LINE__);
  tok_hint_comment_open =
      range_for_digests.add_token(HINT_COMMENT_STARTER, __LINE__);
  tok_hint_comment_close =
      range_for_digests.add_token(HINT_COMMENT_TERMINATOR, __LINE__);

  /* New in 8.0 */

  tok_in_generic_value_expression =
      range_for_digests.add_token("IN (...)", __LINE__);

  max_token_seen_in_special_tokens = max_token_seen;

  /* Percona tokens */
  add_percona_tokens();

  /* Add new digest tokens here */

<<<<<<< HEAD
  tok_unused = start_token_range_for_sql_percona + percona_tokens;
  set_token(tok_unused, "UNUSED");
||||||| ea7d2e2d16a
  tok_unused = max_token_seen++;
  set_token(tok_unused, "UNUSED");
=======
  tok_unused = range_for_digests.add_token("UNUSED", __LINE__);
>>>>>>> mysql-8.0.20

  /*
    Fix whitespace for some special tokens.
  */

  /*
    The lexer parses "@@variable" as '@', '@', 'variable',
    returning a token for '@' alone.

    This is incorrect, '@' is not really a token,
    because the syntax "@ @ variable" (with spaces) is not accepted:
    The lexer keeps some internal state after the '@' fake token.

    To work around this, digest text are printed as "@@variable".
  */
  compiled_token_array[(int)'@'].m_append_space = false;

  /*
    Define additional properties for tokens.

    List all the token that are followed by an expression.
    This is needed to differentiate unary from binary
    '+' and '-' operators, because we want to:
    - reduce <unary +> <NUM> to <?>,
    - preserve <...> <binary +> <NUM> as is.
  */
  set_start_expr_token('(');
  set_start_expr_token(',');
  set_start_expr_token(EVERY_SYM);
  set_start_expr_token(AT_SYM);
  set_start_expr_token(STARTS_SYM);
  set_start_expr_token(ENDS_SYM);
  set_start_expr_token(DEFAULT_SYM);
  set_start_expr_token(RETURN_SYM);
  set_start_expr_token(IF);
  set_start_expr_token(ELSEIF_SYM);
  set_start_expr_token(CASE_SYM);
  set_start_expr_token(WHEN_SYM);
  set_start_expr_token(WHILE_SYM);
  set_start_expr_token(UNTIL_SYM);
  set_start_expr_token(SELECT_SYM);

  set_start_expr_token(OR_SYM);
  set_start_expr_token(OR2_SYM);
  set_start_expr_token(XOR);
  set_start_expr_token(AND_SYM);
  set_start_expr_token(AND_AND_SYM);
  set_start_expr_token(NOT_SYM);
  set_start_expr_token(BETWEEN_SYM);
  set_start_expr_token(LIKE);
  set_start_expr_token(REGEXP);

  set_start_expr_token('|');
  set_start_expr_token('&');
  set_start_expr_token(SHIFT_LEFT);
  set_start_expr_token(SHIFT_RIGHT);
  set_start_expr_token('+');
  set_start_expr_token('-');
  set_start_expr_token(INTERVAL_SYM);
  set_start_expr_token('*');
  set_start_expr_token('/');
  set_start_expr_token('%');
  set_start_expr_token(DIV_SYM);
  set_start_expr_token(MOD_SYM);
  set_start_expr_token('^');
}

static void print_tokens() {
  int tok;

  printf("#ifdef LEX_TOKEN_WITH_DEFINITION\n");
  printf("static lex_token_string lex_token_array[]=\n");
  printf("{\n");
  printf("/* PART 1: character tokens. */\n");

  for (tok = 0; tok < 256; tok++) {
    printf("/* %03d */  { \"\\x%02x\", 1, %s, %s, %d},\n", tok, tok,
           compiled_token_array[tok].m_append_space ? "true" : "false",
           compiled_token_array[tok].m_start_expr ? "true" : "false",
           compiled_token_array[tok].m_percona_token);
  }

  range_for_sql_yacc1.print(
      "/* PART 2: named tokens from sql/sql_yacc.yy (chunk 1). */",
      "/* PART 3: padding reserved for sql/sql_yacc.yy extensions. */");

<<<<<<< HEAD
  for (tok = 256; tok <= max_token_seen_in_sql_yacc; tok++) {
    printf("/* %03d */  { \"%s\", %d, %s, %s, %d},\n", tok,
           compiled_token_array[tok].m_token_string,
           compiled_token_array[tok].m_token_length,
           compiled_token_array[tok].m_append_space ? "true" : "false",
           compiled_token_array[tok].m_start_expr ? "true" : "false",
           compiled_token_array[tok].m_percona_token);
  }
||||||| ea7d2e2d16a
  for (tok = 256; tok <= max_token_seen_in_sql_yacc; tok++) {
    printf("/* %03d */  { \"%s\", %d, %s, %s},\n", tok,
           compiled_token_array[tok].m_token_string,
           compiled_token_array[tok].m_token_length,
           compiled_token_array[tok].m_append_space ? "true" : "false",
           compiled_token_array[tok].m_start_expr ? "true" : "false");
  }
=======
  range_for_sql_hints.print(
      "/* PART 4: named tokens from sql/sql_hints.yy. */",
      "/* PART 5: padding reserved for sql/sql_hints.yy extensions. */");
>>>>>>> mysql-8.0.20

<<<<<<< HEAD
  printf("/* PART 3: padding reserved for sql/sql_yacc.yy extensions. */\n");

  for (tok = max_token_seen_in_sql_yacc + 1;
       tok < start_token_range_for_sql_hints; tok++) {
    printf(
        "/* reserved %03d for sql/sql_yacc.yy */  { \"\", 0, false, false, "
        "%d},\n",
        tok, compiled_token_array[tok].m_percona_token);
  }
||||||| ea7d2e2d16a
  printf("/* PART 3: padding reserved for sql/sql_yacc.yy extensions. */\n");

  for (tok = max_token_seen_in_sql_yacc + 1;
       tok < start_token_range_for_sql_hints; tok++) {
    printf(
        "/* reserved %03d for sql/sql_yacc.yy */  { \"\", 0, false, false},\n",
        tok);
  }
=======
  range_for_digests.print(
      "/* PART 6: Digest special tokens. */",
      "/* PART 7: padding reserved for digest special tokens. */");
>>>>>>> mysql-8.0.20

  range_for_sql_yacc2.print(
      "/* PART 8: named tokens from sql/sql_yacc.yy (chunk 2). */");

<<<<<<< HEAD
  for (tok = start_token_range_for_sql_hints;
       tok <= max_token_seen_in_sql_hints; tok++) {
    printf("/* %03d */  { \"%s\", %d, %s, %s, %d},\n", tok,
           compiled_token_array[tok].m_token_string,
           compiled_token_array[tok].m_token_length,
           compiled_token_array[tok].m_append_space ? "true" : "false",
           compiled_token_array[tok].m_start_expr ? "true" : "false",
           compiled_token_array[tok].m_percona_token);
  }

  printf("/* PART 5: padding reserved for sql/sql_hints.yy extensions. */\n");

  for (tok = max_token_seen_in_sql_hints + 1;
       tok < start_token_range_for_digests; tok++) {
    printf(
        "/* reserved %03d for sql/sql_hints.yy */  { \"\", 0, false, false, "
        "%d},\n",
        tok, compiled_token_array[tok].m_percona_token);
  }

  printf("/* PART 6: Digest special tokens. */\n");

  for (tok = start_token_range_for_digests;
       tok < max_token_seen_in_special_tokens; tok++) {
    printf("/* %03d */  { \"%s\", %d, %s, %s, %d},\n", tok,
           compiled_token_array[tok].m_token_string,
           compiled_token_array[tok].m_token_length,
           compiled_token_array[tok].m_append_space ? "true" : "false",
           compiled_token_array[tok].m_start_expr ? "true" : "false",
           compiled_token_array[tok].m_percona_token);
  }

  printf("/* PART 7: padding reserved for digest special tokens. */\n");

  for (tok = max_token_seen_in_special_tokens + 1;
       tok < start_token_range_for_sql_percona; tok++) {
    printf(
        "/* reserved %03d for digest special tokens */  { \"\", 0, false, "
        "false, %d},\n",
        tok, compiled_token_array[tok].m_percona_token);
  }

  printf("/* PART 8: Percona tokens. */\n");

  for (tok = start_token_range_for_sql_percona;
       tok < start_token_range_for_sql_percona + percona_tokens; tok++) {
    printf("/* %03d */  { \"%s\", %d, %s, %s, %d},\n", tok,
           compiled_token_array[tok].m_token_string,
           compiled_token_array[tok].m_token_length,
           compiled_token_array[tok].m_append_space ? "true" : "false",
           compiled_token_array[tok].m_start_expr ? "true" : "false",
           compiled_token_array[tok].m_percona_token);
  }

  printf("/* PART 9: UNUSED token. */\n");
  printf("/* %03d */  { \"%s\", %d, %s, %s, %d},\n", tok_unused,
         compiled_token_array[tok_unused].m_token_string,
         compiled_token_array[tok_unused].m_token_length,
         compiled_token_array[tok_unused].m_append_space ? "true" : "false",
         compiled_token_array[tok_unused].m_start_expr ? "true" : "false",
         compiled_token_array[tok_unused].m_percona_token);

  printf("/* PART 10: End of token list. */\n");
||||||| ea7d2e2d16a
  for (tok = start_token_range_for_sql_hints;
       tok <= max_token_seen_in_sql_hints; tok++) {
    printf("/* %03d */  { \"%s\", %d, %s, %s},\n", tok,
           compiled_token_array[tok].m_token_string,
           compiled_token_array[tok].m_token_length,
           compiled_token_array[tok].m_append_space ? "true" : "false",
           compiled_token_array[tok].m_start_expr ? "true" : "false");
  }

  printf("/* PART 5: padding reserved for sql/sql_hints.yy extensions. */\n");

  for (tok = max_token_seen_in_sql_hints + 1;
       tok < start_token_range_for_digests; tok++) {
    printf(
        "/* reserved %03d for sql/sql_hints.yy */  { \"\", 0, false, false},\n",
        tok);
  }

  printf("/* PART 6: Digest special tokens. */\n");

  for (tok = start_token_range_for_digests; tok < max_token_seen; tok++) {
    printf("/* %03d */  { \"%s\", %d, %s, %s},\n", tok,
           compiled_token_array[tok].m_token_string,
           compiled_token_array[tok].m_token_length,
           compiled_token_array[tok].m_append_space ? "true" : "false",
           compiled_token_array[tok].m_start_expr ? "true" : "false");
  }

  printf("/* PART 7: End of token list. */\n");
=======
  printf("/* PART 9: End of token list. */\n");
>>>>>>> mysql-8.0.20

  printf("/* DUMMY */ { \"\", 0, false, false, %d}\n", 0);
  printf("};\n");
  printf("#endif /* LEX_TOKEN_WITH_DEFINITION */\n");

  printf("/* DIGEST specific tokens. */\n");
  printf("#define TOK_GENERIC_VALUE %d\n", tok_generic_value);
  printf("#define TOK_GENERIC_VALUE_LIST %d\n", tok_generic_value_list);
  printf("#define TOK_ROW_SINGLE_VALUE %d\n", tok_row_single_value);
  printf("#define TOK_ROW_SINGLE_VALUE_LIST %d\n", tok_row_single_value_list);
  printf("#define TOK_ROW_MULTIPLE_VALUE %d\n", tok_row_multiple_value);
  printf("#define TOK_ROW_MULTIPLE_VALUE_LIST %d\n",
         tok_row_multiple_value_list);
  printf("#define TOK_IDENT %d\n", tok_ident);
  printf("#define TOK_IDENT_AT %d\n", tok_ident_at);
  printf("#define TOK_HINT_COMMENT_OPEN %d\n", tok_hint_comment_open);
  printf("#define TOK_HINT_COMMENT_CLOSE %d\n", tok_hint_comment_close);
  printf("#define TOK_IN_GENERIC_VALUE_EXPRESSION %d\n",
         tok_in_generic_value_expression);
<<<<<<< HEAD
  printf("#define TOK_HINT_ADJUST(x) ((x) + %d)\n", tok_hint_adjust);
  printf("#define TOK_PERCONA_ADJUST(x) ((x) + %d)\n", tok_percona_adjust);
||||||| ea7d2e2d16a
  printf("#define TOK_HINT_ADJUST(x) ((x) + %d)\n", tok_hint_adjust);
=======
>>>>>>> mysql-8.0.20
  printf("#define TOK_UNUSED %d\n", tok_unused);
}

/*
  ZEROFILL_SYM is the last token in the MySQL 5.7 token list,
  see sql/sql_yacc.yy
  The token value is frozen and should not change,
  to avoid changing query digest values.
*/
static const int zerofill_expected_value = 906;

static_assert(!(ZEROFILL_SYM < zerofill_expected_value),
              "Token deleted. "
              "Please read MAINTAINER instructions in sql/sql_yacc.yy");
static_assert(!(ZEROFILL_SYM > zerofill_expected_value),
              "Token added in the wrong place. "
              "Please read MAINTAINER instructions in sql/sql_yacc.yy");

int main(int, char **) {
  puts(ORACLE_GPL_COPYRIGHT_NOTICE("2016"));

  printf("/*\n");
  printf("  This file is generated, do not edit.\n");
  printf("  See file sql/gen_lex_token.cc.\n");
  printf("*/\n");
  printf("struct lex_token_string\n");
  printf("{\n");
  printf("  const char *m_token_string;\n");
  printf("  int m_token_length;\n");
  printf("  bool m_append_space;\n");
  printf("  bool m_start_expr;\n");
  printf("  bool m_percona_token;\n");
  printf("};\n");
  printf("typedef struct lex_token_string lex_token_string;\n");

  compute_tokens();
  print_tokens();

  return 0;
}
