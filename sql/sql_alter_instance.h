/* Copyright (c) 2016, 2021, Oracle and/or its affiliates.

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

#ifndef SQL_ALTER_INSTANCE_INCLUDED
#define SQL_ALTER_INSTANCE_INCLUDED

#include <my_inttypes.h>

#include "system_key.h"

class THD;
/*
  Base class for execution control for ALTER INSTANCE ... statement
*/
class Alter_instance {
 protected:
  THD *m_thd;

 public:
  explicit Alter_instance(THD *thd) : m_thd(thd) {}
  virtual bool execute() = 0;
  bool log_to_binlog();
  virtual ~Alter_instance() = default;
};

class Rotate_innodb_key : public Alter_instance {
 public:
  explicit Rotate_innodb_key(THD *thd) : Alter_instance(thd) {}

  // virtual bool execute() = 0 override;
  ~Rotate_innodb_key() override {}

 protected:
  bool check_security_context();
  bool acquire_backup_locks();
};

class Rotate_percona_system_key final {
 public:
  explicit Rotate_percona_system_key(const char *key_name, unsigned int key_id)
      : system_key_name(key_name),
        system_key_id(key_id),
        using_system_key_id(true) {}

  explicit Rotate_percona_system_key(const char *key_name)
      : system_key_name(key_name), using_system_key_id(false) {}

  bool rotate();

 private:
  const char *system_key_name;
  unsigned int system_key_id;
  bool using_system_key_id;
};

class Rotate_innodb_master_key final : public Rotate_innodb_key {
 public:
  explicit Rotate_innodb_master_key(THD *thd) : Rotate_innodb_key(thd) {}

  bool execute() override;
  ~Rotate_innodb_master_key() override = default;
};

class Rotate_innodb_system_key final : public Rotate_innodb_key {
 public:
  explicit Rotate_innodb_system_key(THD *thd, unsigned int system_key_id_arg)
      : Rotate_innodb_key(thd),
        rotate_percona_system_key(PERCONA_INNODB_KEY_NAME, system_key_id_arg) {}

  bool execute() override;
  ~Rotate_innodb_system_key() override {}

 private:
  Rotate_percona_system_key rotate_percona_system_key;
};

class Rotate_binlog_master_key : public Alter_instance {
 public:
  explicit Rotate_binlog_master_key(THD *thd) : Alter_instance(thd) {}

  /**
    Executes master key rotation by calling Rpl_encryption api.

    @retval False on success
    @retval True on error
  */
  bool execute() override;
  ~Rotate_binlog_master_key() override = default;
};

class Rotate_redo_system_key final : public Alter_instance {
 public:
  explicit Rotate_redo_system_key(THD *thd)
      : Alter_instance(thd), rotate_percona_system_key(PERCONA_REDO_KEY_NAME) {}

  bool execute() override;
  ~Rotate_redo_system_key() override {}

 private:
  Rotate_percona_system_key rotate_percona_system_key;
};

/** Alter Innodb redo log properties. */
class Innodb_redo_log : public Alter_instance {
 public:
  /**
    @param[in]  thd     server THD
    @param[in]  enable  enable or disable redo logging
  */
  Innodb_redo_log(THD *thd, bool enable)
      : Alter_instance(thd), m_enable(enable) {}

  bool execute() override;

 private:
  /** Enable or disable redo logging. */
  bool m_enable;
};

class Reload_keyring : public Alter_instance {
 public:
  explicit Reload_keyring(THD *thd) : Alter_instance(thd) {}

  /**
    Execute keyring reload operation by calling required APIs

    @returns status of the operation
      @retval false Success
      @retval true  Error
  */
  bool execute() override;
  virtual ~Reload_keyring() override = default;

 private:
  const static size_t s_error_message_length;
};

#endif /* SQL_ALTER_INSTANCE_INCLUDED */
