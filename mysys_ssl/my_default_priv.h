/* Copyright (c) 2012, 2021, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  Without limiting anything contained in the foregoing, this file,
  which is part of C Driver for MySQL (Connector/C), is also subject to the
  Universal FOSS Exception, version 1.0, a copy of which can be found at
  http://oss.oracle.com/licenses/universal-foss-exception.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#ifndef MY_DEFAULT_PRIV_INCLUDED
#define MY_DEFAULT_PRIV_INCLUDED

#include "my_global.h"                          /* C_MODE_START, C_MODE_END */

/*
  Number of byte used to store the length of
  cipher that follows.
*/
#define MAX_CIPHER_STORE_LEN 4U
#define LOGIN_KEY_LEN 20U

C_MODE_START

/**
  Place the login file name in the specified buffer.

  @param file_name     [out]  Buffer to hold login file name
  @param file_name_size [in]  Length of the buffer

  @return 1 - Success
          0 - Failure
*/
int my_default_get_login_file(char *file_name, size_t file_name_size);

C_MODE_END

#endif /* MY_DEFAULT_PRIV_INCLUDED */
