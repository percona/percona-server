/*
 Copyright (c) 2013, 2021, Oracle and/or its affiliates. All rights
 reserved.
 
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
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 02110-1301  USA
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <my_global.h>
#include <mysql.h>
#include <my_sys.h>

#include <NdbApi.hpp> 

#include "EncoderCharset.h"

/* C++ initializes this to zeros:
*/
EncoderCharset * csinfo_table[MY_CS_CTYPE_TABLE_SIZE];


inline bool colIsUtf16le(const NdbDictionary::Column *col) {
  return (strncmp("utf16le", col->getCharset()->csname, 7) == 0);
}

inline bool colIsUtf8(const NdbDictionary::Column *col) {
  return (strncmp("utf8", col->getCharset()->csname, 4) == 0);
}

inline bool colIsLatin1(const NdbDictionary::Column *col) {
  return (strncmp("latin1", col->getCharset()->csname, 6) == 0);
}

inline bool colIsAscii(const NdbDictionary::Column *col) {
  return (strncmp("ascii", col->getCharset()->csname, 5) == 0);
}

inline bool colIsMultibyte(const NdbDictionary::Column *col) { 
  return (col->getCharset()->mbminlen > 1);
}


EncoderCharset * createEncoderCharset(const NdbDictionary::Column *col) {
  EncoderCharset * csinfo = new EncoderCharset;
  
  csinfo->name = col->getCharset()->csname;
  csinfo->minlen = col->getCharset()->mbminlen;
  csinfo->maxlen = col->getCharset()->mbmaxlen;
  csinfo->isMultibyte = colIsMultibyte(col);
  csinfo->isAscii = colIsAscii(col);
  csinfo->isUtf8 = colIsUtf8(col);
  csinfo->isUtf16le = colIsUtf16le(col);

  return csinfo;
}


const EncoderCharset * getEncoderCharsetForColumn(const NdbDictionary::Column *col) {
  int csnum = col->getCharsetNumber();
  EncoderCharset *csinfo = csinfo_table[csnum];
  if(csinfo == 0) {
    csinfo = createEncoderCharset(col);
    csinfo_table[csnum] = csinfo;
  }
  return csinfo;
}
