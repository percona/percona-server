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
 ProcFS plugin is intended to provide access to Linux performance counters
 by running SQL queries. This is useful for Cloud and MySQL-as-service
 installations.
 The plugin reads files from /proc and /sys filesystems and returns
 contents as is. All files not specified in datadir/procfs.cnf are ignored.
 You can write patterns in glob(7) style line by line:
 /sys/block/sd[a-z]/stat
 /proc/version*

 Clients could fetch individual files:
 SELECT * FROM INFORMATION_SCHEMA.PROCFS WHERE FILE = '/proc/version';

 To install this plugin, copy the .so file to the plugin directory and do
 INSTALL PLUGIN procfs SONAME 'procfs.so';
*/

#include <m_ctype.h> /* my_charset_bin */
#include <mysql_version.h>
#if MYSQL_VERSION_ID >= 80002
#include <sql/sql_class.h> /* THD, Security context */
#include <sql/item_cmpfunc.h>
#include <mf_wcomp.h>
bool schema_table_store_record(THD *thd, TABLE *table);
#else
#include <sql_class.h> /* THD, Security context */
#include <item_cmpfunc.h>
#include <sql_show.h> /* schema_table_store_record */
#endif
#include <mysql/psi/mysql_thread.h>

#include <stdlib.h>
#include <dirent.h>
#include <fnmatch.h>
#include <time.h>
#include <ctype.h>
#include <mysql/plugin.h>

#include <fstream>
#define IS_PROCFS_CONTENTS_SIZE 60000
#define IS_PROCFS_REFRESH_SEC 60
#define IS_PROCFS_FILES_PER_PATTERN 10000
#define IS_PROCFS_PATTERN_DEPTH 10

namespace procfs_plugin {
std::vector<std::string> files;
time_t                   files_last_updated_at;
}  // namespace procfs_plugin

static struct st_mysql_information_schema procfs_view= {
    MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION};
MYSQL_PLUGIN procfs_plugin_info= 0;

static ST_FIELD_INFO procfs_view_fields[]= {
    {"FILE", 1024, MYSQL_TYPE_STRING, 0, MY_I_S_UNSIGNED, 0, 0},
    {"CONTENTS", IS_PROCFS_CONTENTS_SIZE, MYSQL_TYPE_STRING, 0,
     MY_I_S_UNSIGNED, 0, 0},
    {0, 0, MYSQL_TYPE_NULL, 0, 0, 0, 0}};

static mysql_rwlock_t LOCK_procfs_files;

#ifdef HAVE_PSI_INTERFACE
static PSI_rwlock_key key_rwlock_LOCK_procfs_files;

static PSI_rwlock_info all_procfs_rwlocks[]= {{&key_rwlock_LOCK_procfs_files,
                                               "LOCK_plugin_procfs", 0
#if MYSQL_VERSION_ID >= 80002
                                               ,
                                               0, PSI_DOCUMENT_ME
#endif
}};

static void init_procfs_psi_keys()
{
  const char *category= "PROCFS";
  int         count;

  count= array_elements(all_procfs_rwlocks);
  mysql_rwlock_register(category, all_procfs_rwlocks, count);
}
#endif

static bool get_equal_condition_argument(Item *cond, std::string *eq_arg,
                                         const std::string &field_name)
{
  if (cond != 0 && cond->type() == Item::FUNC_ITEM)
  {
    Item_func *func= static_cast<Item_func *>(cond);
    if (func != NULL && func->functype() == Item_func::EQ_FUNC)
    {
      Item_func_eq *eq_func= static_cast<Item_func_eq *>(func);
      if (eq_func->arguments()[0]->type() == Item::FIELD_ITEM &&
          my_strcasecmp(system_charset_info,
                        eq_func->arguments()[0]->full_name(),
                        field_name.c_str()) == 0)
      {
        char    buff[1024];
        String *res;
        String  filter(buff, sizeof(buff), system_charset_info);
        if (eq_func->arguments()[1] != NULL &&
            (res= eq_func->arguments()[1]->val_str(&filter)))
        {
          eq_arg->append(res->c_ptr_safe(), res->length());
          return false;
        }
      }
    }
  }
  return true;
}

static bool get_like_condition_argument(Item *cond, std::string *like_arg,
                                        const std::string &field_name)
{
  if (cond != 0 && cond->type() == Item::FUNC_ITEM)
  {
    Item_func *func= static_cast<Item_func *>(cond);
    if (func != NULL && func->functype() == Item_func::LIKE_FUNC)
    {
      Item_func_like *like_func= static_cast<Item_func_like *>(func);
      if (like_func->arguments()[0]->type() == Item::FIELD_ITEM &&
          my_strcasecmp(system_charset_info,
                        like_func->arguments()[0]->full_name(),
                        field_name.c_str()) == 0)
      {
        char    buff[1024];
        String *res;
        String  filter(buff, sizeof(buff), system_charset_info);
        if (like_func->arguments()[1] != NULL &&
            (res= like_func->arguments()[1]->val_str(&filter)))
        {
          like_arg->append(res->c_ptr_safe(), res->length());
          return false;
        }
      }
    }
  }
  return true;
}


static bool get_in_condition_argument(Item *                       cond,
                                      std::map<std::string, bool> &in_args,
                                      const std::string &          field_name)
{
  if (cond != 0 && cond->type() == Item::FUNC_ITEM)
  {
    Item_func *func= static_cast<Item_func *>(cond);
    if (func != NULL && func->functype() == Item_func::IN_FUNC)
    {
      Item_func_in *in_func= static_cast<Item_func_in *>(func);
      if (in_func->arguments()[0]->type() == Item::FIELD_ITEM &&
          my_strcasecmp(system_charset_info,
                        in_func->arguments()[0]->full_name(),
                        field_name.c_str()) == 0)
      {
        char    buff[1024];
        String *res;
        String  filter(buff, sizeof(buff), system_charset_info);
        for (uint i= 1; i < in_func->arg_count; ++i)
        {
          if (in_func->arguments()[i] != NULL &&
              (res= in_func->arguments()[i]->val_str(&filter)) &&
              res->length() > 0)
          {
            in_args[std::string(res->c_ptr_safe(), res->length())]= true;
          }
        }
        return false;
      }
    }
  }
  return true;
}

static void fill_procfs_view_row(THD *thd, TABLE *table, const char *fname,
                                 char *buf, size_t sz)
{
  if (sz == 0)
    return;

  table->field[0]->store(fname, strlen(fname), system_charset_info);
  table->field[1]->store(buf, sz, system_charset_info);
  schema_table_store_record(thd, table);
}

static std::vector<std::string> limited_glob_files(std::string path,
                                                   std::string pattern,
                                                   int         max_results)
{
  std::vector<std::string> files_found;

  if (max_results <= 0)
    return files_found;

  DIR *          dir= opendir(path.c_str());
  struct dirent *dir_entry;

  if (!dir)
    return files_found;

  std::vector<char> real_file_path(PATH_MAX);

  while ((dir_entry= readdir(dir)) != 0)
  {
    if (!fnmatch(pattern.c_str(), dir_entry->d_name,
                 FNM_FILE_NAME | FNM_CASEFOLD | FNM_NOESCAPE | FNM_PERIOD))
    {
      if (dir_entry->d_type != DT_DIR)
      {
        std::string fname=
            std::string(path).append("/").append(dir_entry->d_name);

        if (realpath(fname.c_str(), real_file_path.data()))
        {
          if (strncmp("/sys/", real_file_path.data(), strlen("/sys/")) == 0 ||
              strncmp("/proc/", real_file_path.data(), strlen("/proc/")) == 0)
            files_found.push_back(fname);
        }
        if (files_found.size() >= static_cast<size_t>(max_results))
          break;
      }
    }
  }

  closedir(dir);
  return files_found;
}

static std::vector<std::string> limited_glob(std::string query,
                                             int max_results, int max_depth)
{
  std::vector<std::string> files_found;

  if (max_results <= 0 || max_depth <= 0)
    return files_found;

  std::string path;
  std::string pattern;
  std::string rest;

  std::string::size_type first_star= query.find_first_of("*?[");
  std::string::size_type last_sep_before_star=
      query.find_last_of("/", first_star);
  std::string::size_type first_sep_after_star=
      query.find_first_of("/", first_star);

  if (last_sep_before_star == std::string::npos)
  {
    path= std::string(".");
    pattern= query;
  }
  else if (first_sep_after_star == std::string::npos)
  {
    path= std::string(query.begin(), query.begin() + last_sep_before_star);
    pattern=
        std::string(query.begin() + last_sep_before_star + 1, query.end());
  }
  else
  {
    path= std::string(query.begin(), query.begin() + last_sep_before_star);
    pattern= std::string(query.begin() + last_sep_before_star + 1,
                         query.begin() + first_sep_after_star);
    rest= std::string(query.begin() + first_sep_after_star + 1, query.end());
  }

  if (rest.size() == 0)
  {
    return limited_glob_files(path, pattern, max_results);
  }

  DIR *          dir= opendir(path.c_str());
  struct dirent *dir_entry;

  if (!dir)
    return files_found;

  while ((dir_entry= readdir(dir)) != 0)
  {
    if (!fnmatch(pattern.c_str(), dir_entry->d_name,
                 FNM_CASEFOLD | FNM_NOESCAPE | FNM_PERIOD))
    {
      if (dir_entry->d_type == DT_DIR || dir_entry->d_type == DT_LNK ||
          dir_entry->d_type == DT_UNKNOWN)
      {
        std::string new_pattern= std::string(path)
                                     .append("/")
                                     .append(dir_entry->d_name)
                                     .append("/")
                                     .append(rest);
        std::vector<std::string> v= limited_glob(
            new_pattern, max_results - files_found.size(), max_depth - 1);
        files_found.insert(files_found.end(), v.begin(), v.end());
      }
    }
  }

  closedir(dir);
  return files_found;
}

static void fill_files_list()
{
  time_t ts= time(NULL);


  mysql_rwlock_rdlock(&LOCK_procfs_files);
  if (ts < procfs_plugin::files_last_updated_at + IS_PROCFS_REFRESH_SEC)
  {
    mysql_rwlock_unlock(&LOCK_procfs_files);
    return;
  }
  mysql_rwlock_unlock(&LOCK_procfs_files);

  mysql_rwlock_wrlock(&LOCK_procfs_files);
  if (ts < procfs_plugin::files_last_updated_at + IS_PROCFS_REFRESH_SEC)
  {
    mysql_rwlock_unlock(&LOCK_procfs_files);
    return;
  }

  procfs_plugin::files_last_updated_at= ts;

  std::ifstream procfs_cnf("procfs.cnf");

  procfs_plugin::files.clear();
  while (procfs_cnf)
  {
    std::string path;
    std::getline(procfs_cnf, path);
    if (path.rfind("/proc", 0) != 0 && path.rfind("/sys", 0) != 0)
      continue;

    if (path.find_first_of("*?[") == std::string::npos)
    {
      procfs_plugin::files.push_back(path);
      continue;
    }

    std::vector<std::string> v1= limited_glob(
        path, IS_PROCFS_FILES_PER_PATTERN, IS_PROCFS_PATTERN_DEPTH);
    procfs_plugin::files.insert(procfs_plugin::files.end(), v1.begin(),
                                v1.end());
  }
  mysql_rwlock_unlock(&LOCK_procfs_files);

  procfs_cnf.close();
}

static int fill_procfs_view(THD *thd, TABLE_LIST *tables, Item *cond)
{
  TABLE *table= tables->table;
  char * buf= static_cast<char *>(
      my_malloc(PSI_NOT_INSTRUMENTED, IS_PROCFS_CONTENTS_SIZE, MY_ZEROFILL));

  std::string I_S_PROCFS_FILE("INFORMATION_SCHEMA.PROCFS.FILE");
  std::string like_arg;

  std::map<std::string, bool> in_args;

  fill_files_list();

  if (cond != 0)
  {
    std::string eq_arg;
    if (!get_equal_condition_argument(cond, &eq_arg, I_S_PROCFS_FILE) &&
        !eq_arg.empty())
    {
      in_args[eq_arg]= true;
    }
    else if (!get_like_condition_argument(cond, &like_arg, I_S_PROCFS_FILE) &&
             !like_arg.empty())
    {
    }
    else
    {
      get_in_condition_argument(cond, in_args, I_S_PROCFS_FILE);
    }
  }

  mysql_rwlock_rdlock(&LOCK_procfs_files);
  for (std::vector<std::string>::const_iterator fname=
           procfs_plugin::files.begin();
       fname != procfs_plugin::files.end(); ++fname)
  {
    if (cond != 0 && in_args.size() > 0 &&
        in_args.find(*fname) == in_args.end())
      continue;
#if MYSQL_VERSION_ID >= 80012
    if (cond != 0 && like_arg.size() > 0 &&
        wild_compare(fname->c_str(), fname->size(), like_arg.c_str(),
                     like_arg.size(), 0))
      continue;
#else
    if (cond != 0 && like_arg.size() > 0 &&
        wild_compare(fname->c_str(), like_arg.c_str(), 0))
      continue;
#endif

    std::ifstream f(fname->c_str());
    if (!f || !f.is_open())
      continue;

    f.read(buf, IS_PROCFS_CONTENTS_SIZE);
    fill_procfs_view_row(thd, table, fname->c_str(), buf, f.gcount());
    f.close();
  }
  mysql_rwlock_unlock(&LOCK_procfs_files);

  my_free(buf);

  return false;
}

static int procfs_view_init(void *ptr)
{
#ifdef HAVE_PSI_INTERFACE
  init_procfs_psi_keys();
#endif
  mysql_rwlock_init(key_rwlock_LOCK_procfs_files, &LOCK_procfs_files);

  ST_SCHEMA_TABLE *schema_table= (ST_SCHEMA_TABLE *)ptr;

  schema_table->fields_info= procfs_view_fields;
  schema_table->fill_table= fill_procfs_view;
  schema_table->idx_field1= 0;
  schema_table->idx_field2= 1;

  procfs_plugin::files_last_updated_at= 0;

  return 0;
}

static int procfs_view_deinit(void *)
{
  procfs_plugin::files.clear();
  procfs_plugin::files_last_updated_at= 0;
  mysql_rwlock_destroy(&LOCK_procfs_files);
  return 0;
}

mysql_declare_plugin(procfs)
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN, /* type                            */
      &procfs_view,                /* descriptor                      */
      "PROCFS",                    /* name                            */
      "Percona Inc",               /* author                          */
      "I_S table providing a view /proc/ statistics", /* description                     */
      PLUGIN_LICENSE_GPL, /* plugin license                  */
      procfs_view_init,   /* init function (when loaded)     */
#if MYSQL_VERSION_ID >= 80002
      NULL, /* check uninstall function        */
#endif
      procfs_view_deinit, /* deinit function (when unloaded) */
      0x0100,             /* version                         */
      NULL,               /* status variables                */
      NULL,               /* system variables                */
      NULL, 0
}
mysql_declare_plugin_end;
