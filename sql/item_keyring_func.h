/* Copyright (c) 2018 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "item_cmpfunc.h"

class Item_func_rotate_system_key final : public Item_bool_func {
 public:
  Item_func_rotate_system_key(const POS &pos, Item *system_key_id)
      : Item_bool_func(pos, system_key_id) {
    null_value = false;
  }

 public:
  longlong val_int() override;
  const char *func_name() const noexcept override { return "rotate_system_key"; }
  bool itemize(Parse_context *pc, Item **res) override;
  bool fix_fields(THD *, Item **) override;

 protected:
  virtual bool calc_value(const String *arg);
};
