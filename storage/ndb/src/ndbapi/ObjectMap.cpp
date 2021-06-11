/*
   Copyright (c) 2007, 2021, Oracle and/or its affiliates.
    All rights reserved. Use is subject to license terms.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "ObjectMap.hpp"

NdbObjectIdMap::NdbObjectIdMap(Uint32 sz, Uint32 eSz):
  m_expandSize(eSz),
  m_size(0),
  m_firstFree(InvalidId),
  m_lastFree(InvalidId),
  m_map(0)
{
  expand(sz);
#ifdef DEBUG_OBJECTMAP
  ndbout_c("NdbObjectIdMap:::NdbObjectIdMap(%u)", sz);
#endif
}

NdbObjectIdMap::~NdbObjectIdMap()
{
  assert(checkConsistency());
  free(m_map);
  m_map = NULL;
}

int NdbObjectIdMap::expand(Uint32 incSize)
{
  assert(checkConsistency());
  Uint32 newSize = m_size + incSize;
  MapEntry * tmp = (MapEntry*)realloc(m_map, newSize * sizeof(MapEntry));

  if (likely(tmp != 0))
  {
    m_map = tmp;
    
    for(Uint32 i = m_size; i < newSize-1; i++)
    {
      m_map[i].setNext(i+1);
    }
    m_firstFree = m_size;
    m_lastFree = newSize - 1;
    m_map[newSize-1].setNext(InvalidId);
    m_size = newSize;
    assert(checkConsistency());
  }
  else
  {
    g_eventLogger->error("NdbObjectIdMap::expand: realloc(%u*%lu) failed",
                         newSize, sizeof(MapEntry));
    return -1;
  }
  return 0;
}

bool NdbObjectIdMap::checkConsistency()
{
  if (m_firstFree == InvalidId)
  {
    for (Uint32 i = 0; i<m_size; i++)
    {
      if (m_map[i].isFree())
      {
        assert(false);
        return false;
      }
    }
    return true;
  }

  Uint32 i = m_firstFree;
  while (m_map[i].getNext() != InvalidId)
  {
    i = m_map[i].getNext();
  }
  assert(i == m_lastFree);
  return i == m_lastFree;
}
