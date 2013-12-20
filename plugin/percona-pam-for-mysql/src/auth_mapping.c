/*
(C) 2012, 2013 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "auth_mapping.h"
#include "groups.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

/** Token representation:
    token type, string repr, length of token */
struct token
{
  enum { tok_id, tok_comma, tok_eq, tok_eof } token_type;
  const char *token;
  size_t token_len;
};

/** Iterator in key-value mapping:
    position and length of key,
    position and length of value,
    current position in string */
struct mapping_iter {
  const char *key;
  size_t key_len;
  const char *value;
  size_t value_len;
  const char *ptr;
};

/** Get next token from buf. Returns new buf position. */
static const char *get_token(struct token *token,
                             const char *buf)
{
  const char *ptr= buf;

  while (*ptr && isspace(*ptr))
    ++ptr;

  token->token= ptr;
  switch (*ptr)
  {
  case '\0':
    token->token_type= tok_eof;
    break;
  case ',':
    token->token_len= 1;
    token->token_type = tok_comma;
    ++ptr;
    break;
  case '=':
    token->token_len= 1;
    token->token_type = tok_eq;
    ++ptr;
    break;
  default:
    token->token_len= 0;
    while (*ptr && !isspace(*ptr) && *ptr != ',' && *ptr != '=')
    {
      ++token->token_len;
      ++ptr;
    }
    token->token_type= tok_id;
  }

  return ptr;
}

/** Create iterator through mapping string.
    Initially iterator set to position before first
    key-value pair. On success non-NULL pointer returned, otherwise NULL */
struct mapping_iter *mapping_iter_new(const char *mapping_string)
{
  struct mapping_iter *it= malloc(sizeof(struct mapping_iter));
  struct token token;
  if (it != NULL)
  {
    it->key= NULL;
    it->value= NULL;
    /* eat up service name and move to (, key = value)* part */
    it->ptr= get_token(&token, mapping_string);
  }
  return it;
}

/** Move iterator to next key-value pair.
    On success pointer to key position in string returned,
    otherwise NULL */
const char *mapping_iter_next(struct mapping_iter *it)
{
  struct token token[4]= {{0, 0, 0}};

  /* read next 4 tokens */
  it->ptr= get_token(token + 3,
           get_token(token + 2,
           get_token(token + 1,
           get_token(token, it->ptr))));

  /* was it ", id = id"? */
  if (!((token[0].token_type == tok_comma) &&
        (token[1].token_type == tok_id) &&
        (token[2].token_type == tok_eq) &&
        (token[3].token_type == tok_id)))
  {
    /* we got something inconsistent */
    return NULL;
  }

  /* set key */
  it->key= token[1].token;
  it->key_len= token[1].token_len;

  /* set value */
  it->value= token[3].token;
  it->value_len= token[3].token_len;

  return it->key;
}

/** Finish iteration and release iterator */
void mapping_iter_free(struct mapping_iter *it)
{
  free(it);
}

/** Get mapped value for given user name.
    Value is looked up by using all user groups as a key.
    Auth string is iterated only once, while groups are iterated
    for every key-value pair. This is mean than auth string order
    is dominant.

    Example:

    given:
      user "foo" is the member of "wheel", "staff" and "bar".
      auth string is "mysql, root=user1, bar=user2, staff=user3"

    result is "user2".

    On success value_buf returned, otherwise NULL */
char *mapping_lookup_user(const char *user_name,
                          char *value_buf, size_t value_buf_len,
                          const char *mapping_string)
{
  /* Iterate through the key-value list stored in auth_string and
  find key (which is interpreted as group name) in the list of groups
  for specified user. If match is found, store appropriate value in
  the authenticated_as field. */
  struct groups_iter *group_it;
  struct mapping_iter *keyval_it;
  const char *key;
  const char *group;

  keyval_it= mapping_iter_new(mapping_string);
  if (keyval_it == NULL)
    return NULL;

  group_it= groups_iter_new(user_name);
  if (group_it == NULL)
  {
    mapping_iter_free(keyval_it);
    return NULL;
  }

  while ((key= mapping_iter_next(keyval_it)) != NULL) {
    while ((group= groups_iter_next(group_it)) != NULL) {
      if (keyval_it->key_len == strlen(group) &&
          strncmp(key, group, keyval_it->key_len) == 0) {
        /* match is found */
        memcpy(value_buf, keyval_it->value,
               MIN(value_buf_len, keyval_it->value_len));
        value_buf[MIN(value_buf_len, keyval_it->value_len)]= '\0';
        groups_iter_free(group_it);
        mapping_iter_free(keyval_it);
        return value_buf;
      }
    }
    groups_iter_reset(group_it);
  }

  groups_iter_free(group_it);
  mapping_iter_free(keyval_it);

  return NULL;
}

/** Get key in current iterator pos. On success buf returned,
    otherwise NULL */
char *mapping_iter_get_key(struct mapping_iter *it, char *buf, size_t buf_len)
{
  if (it->key == NULL)
    return NULL;
  memcpy(buf, it->key, MIN(buf_len, it->key_len));
  buf[MIN(buf_len, it->key_len)]= '\0';
  return buf;
}

/** Get value in current iterator pos. On success buf returned,
    otherwise NULL */
char *mapping_iter_get_value(struct mapping_iter *it, char *buf, size_t buf_len)
{
  if (it->value == NULL)
    return NULL;
  memcpy(buf, it->value, MIN(buf_len, it->value_len));
  buf[MIN(buf_len, it->value_len)]= '\0';
  return buf;
}

/** Get value by key. On success pointer to service_name
    returned, otherwise NULL */
char *mapping_get_service_name(char *buf, size_t buf_len,
                               const char *mapping_string)
{
  struct token token;

  get_token(&token, mapping_string);
  if (token.token_type == tok_id)
  {
    memcpy(buf, token.token, MIN(buf_len, token.token_len));
    buf[MIN(buf_len, token.token_len)]= '\0';
    return buf;
  }

  return NULL;
}
