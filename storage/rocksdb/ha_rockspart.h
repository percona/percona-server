/* Copyright (C) 2018 Percona

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
   USA */

#pragma once

#include "partitioning/partition_base.h"

/* This class must contain engine-specific functions for partitioning */
class ha_rockspart : public native_part::Partition_base {
 public:
  ha_rockspart(handlerton *hton, TABLE_SHARE *table_arg)
      : native_part::Partition_base(hton, table_arg){};

  ha_rockspart(handlerton *hton, TABLE_SHARE *share,
               partition_info *part_info_arg,
               native_part::Partition_base *clone_arg,
               MEM_ROOT *clone_mem_root_arg)
      : native_part::Partition_base(hton, share, part_info_arg, clone_arg,
                                    clone_mem_root_arg) {}

  ~ha_rockspart() {}

  int open(const char *name, int mode, uint test_if_locked) override;
  int create(const char *name, TABLE *form,
             HA_CREATE_INFO *create_info) override;

 private:
  virtual handler *get_file_handler(TABLE_SHARE *share, MEM_ROOT *alloc);
  virtual handler *clone(const char *name, MEM_ROOT *mem_root);
  virtual ulong index_flags(uint inx, uint part, bool all_parts) const;
  virtual const char **bas_ext() const;

  void set_pk_can_be_decoded_for_each_partition();
  mutable bool m_pk_can_be_decoded = false;
};
