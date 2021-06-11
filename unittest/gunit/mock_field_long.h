#ifndef MOCK_FIELD_LONG_INCLUDED
#define MOCK_FIELD_LONG_INCLUDED
/* Copyright (c) 2013, 2021, Oracle and/or its affiliates.

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
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include "m_string.h"
#include "field.h"

/*
  Base class for creating mock Field objects.

  To do: Make all other tests #include this file instead of using
  their own copy-pasted variants.
*/
class Mock_field_long : public Field_long
{
  uchar buffer[PACK_LENGTH];
  uchar null_byte;
  char m_name[1024];

  void initialize(const char *name)
  {
    ptr= buffer;
    memset(buffer, 0, PACK_LENGTH);
    static const char *table_name_buf= "table_name";
    table_name= &table_name_buf;
    if (name)
    {
      my_snprintf(m_name, sizeof(m_name), "%.1023s", name);
      field_name= m_name;
    }
  }
public:

  Mock_field_long(const char *name, bool is_nullable)
    : Field_long(0,                               // ptr_arg
                 8,                               // len_arg
                 is_nullable ? &null_byte : NULL, // null_ptr_arg
                 is_nullable ? 1 : 0,             // null_bit_arg
                 Field::NONE,                     // unireg_check_arg
                 "field_name",                    // field_name_arg
                 false,                           // zero_arg
                 false),                          // unsigned_arg
      null_byte('\0')
  {
    initialize(name);
    if (is_nullable)
      set_null_ptr(&null_byte, 1);
  }

  /// Creates a non NULLable column with an optional name.
  Mock_field_long(const char *name)
    : Field_long(0,                           // ptr_arg
                 8,                           // len_arg
                 NULL,                        // null_ptr_arg
                 0,                           // null_bit_arg
                 Field::NONE,                 // unireg_check_arg
                 "field_name",                // field_name_arg
                 false,                       // zero_arg
                 false)                       // unsigned_arg
  {
    initialize(name);
  }

  void make_writable() { bitmap_set_bit(table->write_set, field_index); }
  void make_readable() { bitmap_set_bit(table->read_set, field_index); }
};


#endif // MOCK_FIELD_LONG_INCLUDED
