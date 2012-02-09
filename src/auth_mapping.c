/*
(C) 2012 Percona Inc.

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
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

/** The maximum length of buffer for storing NSS record. NSS will store in
    buffer the whole result of lookup request including user name,
    gecos, etc. */
enum { max_nss_name_len = 10240 };

/** Token representation:
    token type, string repr, length of token */
struct token
{
  enum { tok_id, tok_comma, tok_eq, tok_eof } token_type;
  const char *token;
  int token_len;
};

/** Iterator in key-value mapping:
    position and length of key,
    position and length of value,
    current position in string */
struct mapping_iter {
  const char *key;
  int key_len;
  const char *value;
  int value_len;
  const char *ptr;
};


/** Lookup NSS database for group name by specified user name.
    On sucess user_group returned, otherwise NULL */
char *lookup_user_group (const char *user_name,
                         char *user_group, int user_group_len)
{
  struct passwd pwd, *pwd_result;
  struct group grp, *grp_result;
  char *buf;
  int error;

  buf= malloc(max_nss_name_len);
  if (buf == NULL)
    return NULL;

  error= getpwnam_r(user_name, &pwd, buf, max_nss_name_len, &pwd_result);
  if (error != 0 || pwd_result == NULL)
  {
    free(buf);
    return NULL;
  }

  error= getgrgid_r(pwd_result->pw_gid,
                    &grp, buf, max_nss_name_len, &grp_result);
  if (error != 0 || grp_result == NULL)
  {
    free(buf);
    return NULL;
  }

  strncpy(user_group, grp_result->gr_name, user_group_len);
  user_group[user_group_len]= '\0';
  free(buf);

  return user_group;
}

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
  struct token token[4];

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

/** Get value by given key. On success value_buf returned,
    otherwise NULL */
char *mapping_get_value(const char *key, char *value_buf, int value_buf_len,
                        const char *mapping_string)
{
  struct mapping_iter *it= mapping_iter_new(mapping_string);
  int key_len= strlen(key);

  if (it == NULL)
    return NULL;

  while (mapping_iter_next(it))
  {
    if (key_len == it->key_len && strncmp(key, it->key, key_len) == 0)
    {
      memcpy(value_buf, it->value, MIN(value_buf_len, it->value_len));
      value_buf[MIN(value_buf_len, it->value_len)]= '\0';
      mapping_iter_free(it);
      return value_buf;
    }
  }
  mapping_iter_free(it);

  return NULL;
}

/** Get key in current iterator pos. On success buf returned,
    otherwise NULL */
char *mapping_iter_get_key(struct mapping_iter *it, char *buf, int buf_len)
{
  if (it->key == NULL)
    return NULL;
  memcpy(buf, it->key, MIN(buf_len, it->key_len));
  buf[MIN(buf_len, it->key_len)]= '\0';
  return buf;
}

/** Get value in current iterator pos. On success buf returned,
    otherwise NULL */
char *mapping_iter_get_value(struct mapping_iter *it, char *buf, int buf_len)
{
  if (it->value == NULL)
    return NULL;
  memcpy(buf, it->value, MIN(buf_len, it->value_len));
  buf[MIN(buf_len, it->value_len)]= '\0';
  return buf;
}

/** Get value by key. On success pointer to service_name
    returned, otherwise NULL */
char *mapping_get_service_name(char *buf, int buf_len,
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
