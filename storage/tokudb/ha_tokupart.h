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

#ifndef _HA_TOKUPART_H
#define _HA_TOKUPART_H

#include "partitioning/partition_base.h"

/* This class must contain engine-specific functions for partitioning */
class ha_tokupart : public native_part::Partition_base {
   public:
    ha_tokupart(handlerton *hton, TABLE_SHARE *table_arg)
        : native_part::Partition_base(hton, table_arg){};

    ha_tokupart(handlerton *hton,
                TABLE_SHARE *share,
                partition_info *part_info_arg,
                native_part::Partition_base *clone_arg,
                MEM_ROOT *clone_mem_root_arg)
        : native_part::Partition_base(hton,
                                      share,
                                      part_info_arg,
                                      clone_arg,
                                      clone_mem_root_arg) {}

    ~ha_tokupart() override {}

   private:
    handler *get_file_handler(TABLE_SHARE *share, MEM_ROOT *alloc) override;
    handler *clone(const char *name, MEM_ROOT *mem_root) override;
    ulong index_flags(uint inx, uint part, bool all_parts) const override;
    const char **bas_ext() const override;
};

#endif  // _HA_TOKUPART_H
