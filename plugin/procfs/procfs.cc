/*
(C) 2019 Percona LLC and/or its affiliates
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

/**
 @file
  The ProcFS plugin is intended to provide access to Linux performance counters
  by running SQL queries against a Percona Server 8.0 (or other MySQL 8.0
  compatible variant). This is useful for Cloud and MySQL-as-service
  installations.

  The plugin reads files from /proc and /sys filesystems and returns the files
  name and content as is within individual rows of a new view
  INFORMATION_SCHEMA.PROCFS.  The schema definition of this view is:

  CREATE TEMPORARY TABLE `PROCFS` (
   `FILE` varchar(1024) NOT NULL DEFAULT '',
   `CONTENTS` longtext NOT NULL
  ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
 

  Access to /proc and /sys dfiles and directories is limited by the system
  variable procfs_files_spec.  This variable is a global, read only variable,
  meaning that it may only be set either via the mysqld command line or within
  my.cnf and is not changeable at run time.  This is for security purposes to
  prevent a compromised account from giving itself greater access to the /proc
  or /sys filesystems.  Any files that are not included within the
  procfs_files_spec are ignored and considered to not exist.  You can write file
  access patterns in glob(7) style:  /sys/block/sd[a-z]/stat;/proc/version*

  The default value for procfs_files_spec is : /proc/cpuinfo;/proc/irq//;
  /proc/loadavg/proc/net/dev;/proc/net/sockstat;/proc/net/sockstat_rhe4;
  /proc/net/tcpstat;/proc/self/net/netstat;/proc/self/stat;/proc/self/io;
  /proc/self/numa_maps/proc/softirqs;/proc/spl/kstat/zfs/arcstats;/proc/stat;
  /proc/sys/fs/file-nr;/proc/version;/proc/vmstat

  To install this plugin, copy the procfs.so file to the mysql plugin
  installation directory and execute "INSTALL PLUGIN procfs SONAME 'procfs.so';"

  Access to the INFORMATION_SCHEMA.PROCFS is limited to users with the
  ACCESS_PROCFS dynamic privilege.  Upon plugin startup, this new dynamic
  privilege is registered with the server.  After the plugin has been installed,
  grant a user access to the INFORMATION_SCHEMA.PROCFS view by executing:
  "GRANT ACCESS_PROCFS ON *. TO 'user'@'host';".

  Authorized users can now obtain information from individual files by
  specifying the exact file name within a WHERE clause. Example:
  "SELECT * FROM INFORMATION_SCHEMA.PROCFS WHERE FILE = '/proc/version';".
  Specifying specific files is critical to limiting the impact of the plugin to
  the servers performance.  Limited file globbing is supported by the LIKE
  expression.  Failure to limit the set of files scanned via a WHERE clause
  through equality, LIKE or IN qualifiers can lead to lengthy query response
  times and high load and memory usage on the server as all files that match the
  procfs_files_spec must be opened, read, held in memory, and finally returned
  to the client.

  Some basic metrics are provided by status variables:

  Name                        Description
  -----------------------------------------------------------------------------
  procfs_access_violations    number of attempted queries by users without the
                              ACCESS_PROCFS privilege
  procfs_queries              number of queries made against the procfs view
  procfs_files_read           number of files read to provide content
  procfs_bytes_read           number of bytes read to provide content


 Credit goes to Nickolay Ihalainen (nickolay.ihalainen@percona.com) for the
 original implementation on which this work is based.  Thank you Nickolay!
*/

#define LOG_COMPONENT_TAG "procfs"

#include "mf_wcomp.h"
#include "my_sys.h"
#include "mysql/components/my_service.h"
#include "mysql/components/services/component_sys_var_service.h"
#include "mysql/components/services/dynamic_privilege.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/plugin.h"
#include "mysql/service_security_context.h"
#include "mysql/strings/m_ctype.h"
#include "mysql_version.h"
#include "sql/item_cmpfunc.h"
#include "sql/sql_class.h" /* THD, Security context */
#include "sql/sql_show.h"

#include <ctype.h>
#include <dirent.h>
#include <fnmatch.h>
#include <stdlib.h>
#include <time.h>

#include <fstream>

// MySQL 8.0 logger service interface
static SERVICE_TYPE(registry) *reg_srv = nullptr;
SERVICE_TYPE(log_builtins) *log_bi = nullptr;
SERVICE_TYPE(log_builtins_string) *log_bs = nullptr;

namespace procfs {

static const char *IS_TABLE_NAME = "PROCFS";
static const char *PRIVILEGE_NAME = "ACCESS_PROCFS";
static const constexpr ulong MAX_CONTENTS_SIZE = 60000;
static const constexpr ulong MAX_FILES_PER_PATTERN = 10000;
static const constexpr ulong MAX_PATTERN_DEPTH = 10;
static const char *DEFAULT_FILES_SPEC =
    "/proc/cpuinfo;"
    "/proc/irq/*/*;"
    "/proc/loadavg/proc/net/dev;"
    "/proc/net/sockstat;"
    "/proc/net/sockstat_rhe4;"
    "/proc/net/tcpstat;"
    "/proc/self/net/netstat;"
    "/proc/self/stat;"
    "/proc/self/io;"
    "/proc/self/numa_maps/proc/softirqs;"
    "/proc/spl/kstat/zfs/arcstats;"
    "/proc/stat;"
    "/proc/sys/fs/file-nr;"
    "/proc/version;"
    "/proc/vmstat";

static char *files_spec = nullptr;
static char *buffer = nullptr;

static struct st_mysql_information_schema view = {
    MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION};

MYSQL_PLUGIN plugin_info = 0;

static ST_FIELD_INFO view_fields[] = {
    {"FILE", 1024, MYSQL_TYPE_STRING, 0, MY_I_S_UNSIGNED, 0, 0},
    {"CONTENTS", MAX_CONTENTS_SIZE, MYSQL_TYPE_STRING, 0, MY_I_S_UNSIGNED, 0,
     0},
    {0, 0, MYSQL_TYPE_NULL, 0, 0, 0, 0}};

static MYSQL_SYSVAR_STR(files_spec, files_spec,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY |
                            PLUGIN_VAR_NOPERSIST,
                        "List of files and directories accessable to users "
                        "with ACCESS_PROCFS privilege.",
                        nullptr, nullptr, DEFAULT_FILES_SPEC);

static SYS_VAR *system_variables[] = {MYSQL_SYSVAR(files_spec), nullptr};

static std::atomic<uint64_t> access_violations(0);
static std::atomic<uint64_t> queries(0);
static std::atomic<uint64_t> files_read(0);
static std::atomic<uint64_t> bytes_read(0);

static SHOW_VAR status_variables[] = {
    {"procfs_access_violations", (char *)&queries, SHOW_LONGLONG,
     SHOW_SCOPE_GLOBAL},
    {"procfs_queries", (char *)&queries, SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
    {"procfs_files_read", (char *)&files_read, SHOW_LONGLONG,
     SHOW_SCOPE_GLOBAL},
    {"procfs_bytes_read", (char *)&bytes_read, SHOW_LONGLONG,
     SHOW_SCOPE_GLOBAL},
    {NullS, NullS, SHOW_LONG, SHOW_SCOPE_GLOBAL}};

static bool get_equal_condition_argument(Item *cond, std::string *eq_arg,
                                         const std::string &field_name) {
  if (cond != 0 && cond->type() == Item::FUNC_ITEM) {
    Item_func *func = static_cast<Item_func *>(cond);
    if (func != nullptr && func->functype() == Item_func::EQ_FUNC) {
      Item_func_eq *eq_func = static_cast<Item_func_eq *>(func);
      if (eq_func->arguments()[0]->type() == Item::FIELD_ITEM &&
          my_strcasecmp(system_charset_info,
                        eq_func->arguments()[0]->full_name(),
                        field_name.c_str()) == 0) {
        char buff[1024];
        String *res;
        String filter(buff, sizeof(buff), system_charset_info);
        if (eq_func->arguments()[1] != nullptr &&
            (res = eq_func->arguments()[1]->val_str(&filter))) {
          eq_arg->append(res->c_ptr_safe(), res->length());
          return false;
        }
      }
    }
  }
  return true;
}

static bool get_like_condition_argument(Item *cond, std::string *like_arg,
                                        const std::string &field_name) {
  if (cond != 0 && cond->type() == Item::FUNC_ITEM) {
    Item_func *func = static_cast<Item_func *>(cond);
    if (func != nullptr && func->functype() == Item_func::LIKE_FUNC) {
      Item_func_like *like_func = static_cast<Item_func_like *>(func);
      if (like_func->arguments()[0]->type() == Item::FIELD_ITEM &&
          my_strcasecmp(system_charset_info,
                        like_func->arguments()[0]->full_name(),
                        field_name.c_str()) == 0) {
        char buff[1024];
        String *res;
        String filter(buff, sizeof(buff), system_charset_info);
        if (like_func->arguments()[1] != nullptr &&
            (res = like_func->arguments()[1]->val_str(&filter))) {
          like_arg->append(res->c_ptr_safe(), res->length());
          return false;
        }
      }
    }
  }
  return true;
}

static bool get_in_condition_argument(Item *cond,
                                      std::map<std::string, bool> &in_args,
                                      const std::string &field_name) {
  if (cond != 0 && cond->type() == Item::FUNC_ITEM) {
    Item_func *func = static_cast<Item_func *>(cond);
    if (func != nullptr && func->functype() == Item_func::IN_FUNC) {
      Item_func_in *in_func = static_cast<Item_func_in *>(func);
      if (in_func->arguments()[0]->type() == Item::FIELD_ITEM &&
          my_strcasecmp(system_charset_info,
                        in_func->arguments()[0]->full_name(),
                        field_name.c_str()) == 0) {
        char buff[1024];
        String *res;
        String filter(buff, sizeof(buff), system_charset_info);
        for (uint i = 1; i < in_func->arg_count; ++i) {
          if (in_func->arguments()[i] != nullptr &&
              (res = in_func->arguments()[i]->val_str(&filter)) &&
              res->length() > 0) {
            in_args[std::string(res->c_ptr_safe(), res->length())] = true;
          }
        }
        return false;
      }
    }
  }
  return true;
}

static void limited_glob_files(const std::string &path,
                               const std::string &pattern, int max_results,
                               std::vector<std::string> &files_found) {
  static const constexpr char SYS_NAME[] = "/sys/";
  static const constexpr int SYS_NAME_LEN = sizeof(SYS_NAME);
  static const constexpr char PROC_NAME[] = "/proc/";
  static const constexpr int PROC_NAME_LEN = sizeof(PROC_NAME);

  if (max_results <= 0) return;

  DIR *dir = opendir(path.c_str());
  struct dirent *dir_entry;

  if (!dir) return;

  std::vector<char> real_file_path(PATH_MAX);

  while ((dir_entry = readdir(dir)) != 0) {
    if (!fnmatch(pattern.c_str(), dir_entry->d_name,
                 FNM_FILE_NAME | FNM_CASEFOLD | FNM_NOESCAPE | FNM_PERIOD)) {
      if (dir_entry->d_type != DT_DIR) {
        std::string fname =
            std::string(path).append("/").append(dir_entry->d_name);

        if (realpath(fname.c_str(), real_file_path.data())) {
          if (strncmp(SYS_NAME, real_file_path.data(), SYS_NAME_LEN) == 0 ||
              strncmp(PROC_NAME, real_file_path.data(), PROC_NAME_LEN) == 0)
            files_found.push_back(fname);
        }
        if (files_found.size() >= static_cast<size_t>(max_results)) break;
      }
    }
  }

  closedir(dir);
  return;
}

static void limited_glob(const std::string &query, int max_results,
                         int max_depth, std::vector<std::string> &files_found) {
  if (max_results <= 0 || max_depth <= 0) return;

  std::string path;
  std::string pattern;
  std::string rest;

  std::string::size_type first_star = query.find_first_of("*?[");
  std::string::size_type last_sep_before_star =
      query.find_last_of("/", first_star);
  std::string::size_type first_sep_after_star =
      query.find_first_of("/", first_star);

  if (last_sep_before_star == std::string::npos) {
    path = std::string(".");
    pattern = query;
  } else if (first_sep_after_star == std::string::npos) {
    path = std::string(query.begin(), query.begin() + last_sep_before_star);
    pattern =
        std::string(query.begin() + last_sep_before_star + 1, query.end());
  } else {
    path = std::string(query.begin(), query.begin() + last_sep_before_star);
    pattern = std::string(query.begin() + last_sep_before_star + 1,
                          query.begin() + first_sep_after_star);
    rest = std::string(query.begin() + first_sep_after_star + 1, query.end());
  }

  if (rest.size() == 0) {
    limited_glob_files(path, pattern, max_results, files_found);
    return;
  }

  DIR *dir = opendir(path.c_str());
  struct dirent *dir_entry;

  if (!dir) return;

  while ((dir_entry = readdir(dir)) != 0) {
    if (!fnmatch(pattern.c_str(), dir_entry->d_name,
                 FNM_CASEFOLD | FNM_NOESCAPE | FNM_PERIOD)) {
      if (dir_entry->d_type == DT_DIR || dir_entry->d_type == DT_LNK ||
          dir_entry->d_type == DT_UNKNOWN) {
        std::string new_pattern = std::string(path)
                                      .append("/")
                                      .append(dir_entry->d_name)
                                      .append("/")
                                      .append(rest);
        limited_glob(new_pattern, max_results - files_found.size(),
                     max_depth - 1, files_found);
      }
    }
  }

  closedir(dir);
  return;
}

static void fill_files_list(std::vector<std::string> &files) {
  std::istringstream list(files_spec);

  files.clear();
  while (list) {
    std::string path;
    std::getline(list, path, ';');
    if (path.rfind("/proc", 0) != 0 && path.rfind("/sys", 0) != 0) continue;

    if (path.find_first_of("*?[") == std::string::npos) {
      files.push_back(path);
      continue;
    }

    limited_glob(path, MAX_FILES_PER_PATTERN, MAX_PATTERN_DEPTH, files);
  }
}

static void fill_view_row(THD *thd, TABLE *table, const char *fname, char *buf,
                          size_t sz) {
  if (sz == 0) return;

  table->field[0]->store(fname, strlen(fname), system_charset_info);
  table->field[1]->store(buf, sz, system_charset_info);
  schema_table_store_record(thd, table);

  files_read++;
  bytes_read += sz;
}

static int fill_view(THD *thd, Table_ref *tables, Item *cond) {
  if (!thd->security_context()
           ->has_global_grant(PRIVILEGE_NAME, strlen(PRIVILEGE_NAME))
           .first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), PRIVILEGE_NAME);
    access_violations++;
    return 1;
  }

  TABLE *table = tables->table;

  static const std::string I_S_FILE("INFORMATION_SCHEMA.PROCFS.FILE");
  std::string like_arg;

  std::map<std::string, bool> in_args;

  if (cond != 0) {
    std::string eq_arg;
    if (!get_equal_condition_argument(cond, &eq_arg, I_S_FILE) &&
        !eq_arg.empty()) {
      in_args[eq_arg] = true;
    } else if (!get_like_condition_argument(cond, &like_arg, I_S_FILE) &&
               !like_arg.empty()) {
    } else {
      get_in_condition_argument(cond, in_args, I_S_FILE);
    }
  }

  std::vector<std::string> files;
  fill_files_list(files);
  for (std::vector<std::string>::const_iterator fname = files.begin();
       fname != files.end(); ++fname) {
    if (cond != 0 && in_args.size() > 0 &&
        in_args.find(*fname) == in_args.end())
      continue;
    if (cond != 0 && like_arg.size() > 0 &&
        wild_compare(fname->c_str(), fname->size(), like_arg.c_str(),
                     like_arg.size(), 0))
      continue;

    std::ifstream f(fname->c_str());
    if (!f || !f.is_open()) continue;

    f.read(buffer, MAX_CONTENTS_SIZE);
    fill_view_row(thd, table, fname->c_str(), buffer, f.gcount());
    f.close();
  }

  queries++;

  return false;
}

static int view_init(void *ptr) {
  if (init_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs)) {
    return 1;
  }

  LogErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, "Plugin procfs initializing...");

  if (strcmp(files_spec, DEFAULT_FILES_SPEC) != 0) {
    LogPluginErrMsg(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "default procfs_files_spec has been overridden with "
                    "\"%s\", sensitive system information may be exposed via "
                    "the information_schema.procfs table",
                    files_spec);
  }

  if (reg_srv == nullptr) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "reg_srv is NULL in init");
    return 1;
  }
  my_service<SERVICE_TYPE(dynamic_privilege_register)> reg_priv(
      "dynamic_privilege_register", reg_srv);
  if (reg_priv.is_valid()) {
    if (reg_priv->register_privilege(PRIVILEGE_NAME, strlen(PRIVILEGE_NAME))) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "cannot register privilege \"%s\"", PRIVILEGE_NAME);
      deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);
      return 1;
    }
  }

  ST_SCHEMA_TABLE *schema_table = (ST_SCHEMA_TABLE *)ptr;

  schema_table->table_name = IS_TABLE_NAME;
  schema_table->fields_info = view_fields;
  schema_table->fill_table = fill_view;
  schema_table->old_format = nullptr;
  schema_table->process_table = nullptr;

  buffer = static_cast<char *>(
      my_malloc(PSI_NOT_INSTRUMENTED, MAX_CONTENTS_SIZE, MY_ZEROFILL));

  return 0;
}

static int view_deinit(void *) {
  LogPluginErrMsg(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                  "Plugin procfs de-initializing...");

  if (reg_srv == nullptr) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "reg_srv is NULL in deinit");
  } else {
    my_service<SERVICE_TYPE(dynamic_privilege_register)> reg_priv(
        "dynamic_privilege_register", reg_srv);

    if (reg_priv.is_valid()) {
      if (reg_priv->unregister_privilege(PRIVILEGE_NAME,
                                         strlen(PRIVILEGE_NAME))) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "cannot unregister privilege \"%s\"", PRIVILEGE_NAME);
      }
    }
  }

  my_free(buffer);

  deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);

  return 0;
}

}  // namespace procfs

mysql_declare_plugin(procfs){
    MYSQL_INFORMATION_SCHEMA_PLUGIN, /* type                            */
    &procfs::view,                   /* descriptor                      */
    "PROCFS",                        /* name                            */
    "Percona Inc",                   /* author                          */
    "I_S table providing a view /proc/ statistics", /* description */
    PLUGIN_LICENSE_GPL,       /* plugin license                  */
    procfs::view_init,        /* init function (when loaded)     */
    nullptr,                  /* check uninstall function        */
    procfs::view_deinit,      /* deinit function (when unloaded) */
    0x0100,                   /* version                         */
    procfs::status_variables, /* status variables                */
    procfs::system_variables, /* system variables                */
    nullptr,
    0} mysql_declare_plugin_end;
