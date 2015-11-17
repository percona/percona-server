/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:  
#ident "Copyright (c) 2014 Tokutek Inc.  All rights reserved."

#define MYSQL_SERVER
#include <my_config.h>
#include <mysql_version.h>
#include <mysql/plugin.h>
#include <my_global.h>
#include <my_dbug.h>
#include <log.h>
#include <sql_class.h>
#if MYSQL_VERSION_ID <= 50599 || defined(MARIADB_BASE_VERSION)
#include <log.h>       // normalize_binlog_name
#else
#include <binlog.h>    // normalize_binlog_name
#endif
#include <sql_acl.h>   // SUPER_ACL
#include <sql_parse.h> // check_global_access
#include "backup/backup.h"
#include <regex.h>

#ifdef TOKUDB_BACKUP_PLUGIN_VERSION
#define stringify2(x) #x
#define stringify(x) stringify2(x)
#define TOKUDB_BACKUP_PLUGIN_VERSION_STRING stringify(TOKUDB_BACKUP_PLUGIN_VERSION)
#else
#define TOKUDB_BACKUP_PLUGIN_VERSION_STRING NULL
#endif

static char *tokudb_backup_plugin_version;

static MYSQL_SYSVAR_STR(plugin_version, tokudb_backup_plugin_version,
    PLUGIN_VAR_NOCMDARG | PLUGIN_VAR_READONLY,
    "version of the tokudb backup plugin",
    NULL, NULL, TOKUDB_BACKUP_PLUGIN_VERSION_STRING);

static char *tokudb_backup_version = (char *) tokubackup_version_string;

static MYSQL_SYSVAR_STR(version, tokudb_backup_version,
    PLUGIN_VAR_NOCMDARG | PLUGIN_VAR_READONLY,
    "version of the tokutek backup library",
    NULL, NULL, NULL);

static MYSQL_THDVAR_ULONG(last_error,
    PLUGIN_VAR_THDLOCAL,
    "error from the last backup. 0 is success",
    NULL, NULL, 0, 0, ~0ULL, 1);

static void tokudb_backup_update_last_error_str(THD* thd,
                                                struct st_mysql_sys_var* var,
                                                void* var_ptr, const void* save);

static MYSQL_THDVAR_STR(last_error_string,
    PLUGIN_VAR_THDLOCAL,
    "error string from the last backup",
    NULL, tokudb_backup_update_last_error_str, NULL);

static MYSQL_THDVAR_STR(exclude,
    PLUGIN_VAR_THDLOCAL + PLUGIN_VAR_MEMALLOC,
    "exclude source file regular expression",
    NULL, NULL, NULL);

static int tokudb_backup_check_dir(THD* thd, struct st_mysql_sys_var* var,
                                   void* save, struct st_mysql_value* value);

static void tokudb_backup_update_dir(THD* thd, struct st_mysql_sys_var* var,
                                     void* var_ptr, const void* save);

static MYSQL_THDVAR_STR(dir,
    PLUGIN_VAR_THDLOCAL + PLUGIN_VAR_MEMALLOC,
    "name of the directory where the backup is stored",
    tokudb_backup_check_dir, tokudb_backup_update_dir, NULL);

static int tokudb_backup_check_throttle(THD* thd, struct st_mysql_sys_var* var,
                                        void* save, struct st_mysql_value* value);

static void tokudb_backup_update_throttle(THD* thd, struct st_mysql_sys_var* var,
                                          void* var_ptr, const void* save);

static MYSQL_THDVAR_ULONGLONG(throttle,
    PLUGIN_VAR_THDLOCAL,
    "backup throttle on write rate in bytes per second",
    tokudb_backup_check_throttle, tokudb_backup_update_throttle,
    ~0ULL, 0, ~0ULL, 1);

static char *tokudb_backup_allowed_prefix;

static MYSQL_SYSVAR_STR(allowed_prefix, tokudb_backup_allowed_prefix,
    PLUGIN_VAR_READONLY,
    "allowed prefix of the destination directory",
    NULL, NULL, NULL);

static struct st_mysql_sys_var *tokudb_backup_system_variables[] = {
    MYSQL_SYSVAR(plugin_version),
    MYSQL_SYSVAR(version),
    MYSQL_SYSVAR(allowed_prefix),
    MYSQL_SYSVAR(throttle),
    MYSQL_SYSVAR(dir),
    MYSQL_SYSVAR(last_error),
    MYSQL_SYSVAR(last_error_string),
    MYSQL_SYSVAR(exclude),
    NULL,
};

struct tokudb_backup_exclude_copy_extra {
    THD *_thd;
    char *exclude_string;
    regex_t *re;
};

static int tokudb_backup_exclude_copy_fun(const char *source_file, void *extra) {
    tokudb_backup_exclude_copy_extra *exclude_extra = static_cast<tokudb_backup_exclude_copy_extra *>(extra);
    int r = 0;
    if (0) fprintf(stderr, "%s %s\n", __FUNCTION__, source_file);
    if (exclude_extra->exclude_string) {
        int regr = regexec(exclude_extra->re, source_file, 0, NULL, 0);
        if (regr == 0) {
            if (1) fprintf(stderr, "tokudb backup exclude %s\n", source_file);
            r = 1;
        }
    }
    return r;
}

struct tokudb_backup_progress_extra {
    THD *_thd;
    char *_the_string;
};

static int tokudb_backup_progress_fun(float progress, const char *progress_string, void *extra) {
    tokudb_backup_progress_extra *be = static_cast<tokudb_backup_progress_extra *>(extra);

    // set thd proc info
    thd_proc_info(be->_thd, "");
    size_t len = 100 + strlen(progress_string);
    be->_the_string = (char *) my_realloc(be->_the_string, len, MYF(MY_FAE+MY_ALLOW_ZERO_PTR));
    float percentage = progress * 100;
    int r = snprintf(be->_the_string, len, "tokudb backup about %.0f%% done: %s", percentage, progress_string);
    assert(0 < r && (size_t)r <= len);
    thd_proc_info(be->_thd, be->_the_string);

    if (thd_killed(be->_thd)) {
        return ER_ABORTING_CONNECTION;
    }
    return 0;
}

static void tokudb_backup_set_error(THD *thd, int error, const char *error_string) {
    THDVAR(thd, last_error) = error;
    char *old_error_string = THDVAR(thd, last_error_string);
    if (error_string)
        THDVAR(thd, last_error_string) = my_strdup(error_string, MYF(MY_FAE));
    else
        THDVAR(thd, last_error_string) = NULL;
    if (old_error_string)
        my_free(old_error_string);
}

static void tokudb_backup_set_error_string(THD *thd, int error, const char *error_fmt, const char *s1, const char *s2, const char *s3) {
    size_t n = strlen(error_fmt) + (s1 ? strlen(s1) : 0) + (s2 ? strlen(s2) : 0) + (s3 ? strlen(s3) : 0);
    char *error_string = static_cast<char *>(my_malloc(n+1, MYF(MY_FAE)));
    int r = snprintf(error_string, n+1, error_fmt, s1, s2, s3);
    assert(0 < r && (size_t)r <= n);
    tokudb_backup_set_error(thd, error, error_string);
    my_free(error_string);
}

static void tokudb_backup_update_last_error_str(THD* thd,
                                                struct st_mysql_sys_var* var,
                                                void* var_ptr, const void* save) {
    tokudb_backup_set_error(thd, THDVAR(thd, last_error), ((LEX_STRING*)save)->str);
    *((char**)var_ptr) = THDVAR(thd, last_error_string);
}

struct tokudb_backup_error_extra {
    THD *_thd;
};

static void tokudb_backup_error_fun(int error_number, const char *error_string, void *extra) {
    tokudb_backup_error_extra *be = static_cast<tokudb_backup_error_extra *>(extra);
    char *last_error_string = THDVAR(be->_thd, last_error_string);
    if (last_error_string == NULL) {
        tokudb_backup_set_error(be->_thd, error_number, error_string);
    } else {
        // append the new error string to the last error string
        tokudb_backup_set_error_string(be->_thd, error_number, "%s; %s", last_error_string, error_string, NULL);
    }
}

static char *tokudb_backup_realpath_with_slash(const char *a) {
    char *result = NULL;
    char *apath = realpath(a, NULL);
    if (apath) {
        result = apath;
        size_t apath_len = strlen(apath);
        if (apath[apath_len] != '/') {
            char *apath_with_slash = (char *) malloc(apath_len+2);
            assert(apath_with_slash);
            sprintf(apath_with_slash, "%s/", apath);
            free(apath);
            result = apath_with_slash;
        }
    }
    return result;
}

static bool tokudb_backup_is_child_of(const char *a, const char *b) {
    bool result = false;
    char *apath = tokudb_backup_realpath_with_slash(a);
    char *bpath = tokudb_backup_realpath_with_slash(b);
    if (apath && bpath) {
        result = strncmp(apath, bpath, strlen(bpath)) == 0;
    }
    if (apath) 
        free(apath);
    if (bpath) 
        free(bpath);
    return result;
}

const int MYSQL_MAX_DIR_COUNT = 4;

class source_dirs {
    int m_count;
    const char *m_dirs[MYSQL_MAX_DIR_COUNT];
    char *m_mysql_data_dir;
    const char *m_tokudb_data_dir;
    const char *m_tokudb_log_dir;
    const char *m_log_bin_dir;

public:
    bool log_bin_set;
    bool tokudb_data_set;
    bool tokudb_log_set;

public:
    source_dirs() : m_count(0),
                    m_mysql_data_dir(NULL),
                    m_tokudb_data_dir(NULL),
                    m_tokudb_log_dir(NULL),
                    m_log_bin_dir(NULL),
                    log_bin_set(false),
                    tokudb_data_set(false),
                    tokudb_log_set(false) {
        for (int i = 0; i < MYSQL_MAX_DIR_COUNT; ++i) {
            m_dirs[i] = NULL;
        }
    }

    ~source_dirs() {
        my_free((void*)m_mysql_data_dir);
        my_free((void*)m_tokudb_data_dir);
        my_free((void*)m_tokudb_log_dir);
        my_free((void*)m_log_bin_dir);
    }

    void find_and_allocate_dirs(THD *thd) {
        // Sanitize the trailing slash of the MySQL Data Dir.
        m_mysql_data_dir = my_strdup(mysql_real_data_home, MYF(MY_FAE));
#if 0
        // These APIs do not exist on MySQL 5.5 or MariaDB.  We only need this code if the tokudb storage
        // engine is NOT installed.  
        // To avoid crashes due to my_error being called prematurely by find_plug_in_sys_var, we make sure
        // that the tokudb system variables exist which is the case if the tokudb plugin is loaded.
        const char *tokudb = "TokuDB";
        LEX_STRING tokudb_string = { (char *) tokudb, strlen(tokudb) };
        lock_plugin_data();
        bool tokudb_found = plugin_find_by_type(&tokudb_string, MYSQL_ANY_PLUGIN) != NULL;
        unlock_plugin_data();

        // Note: These all allocate new strings or return NULL.
        if (tokudb_found) {
#endif
            m_tokudb_data_dir = this->find_plug_in_sys_var("tokudb_data_dir", thd);
            m_tokudb_log_dir = this->find_plug_in_sys_var("tokudb_log_dir", thd);
#if 0
        }
#endif
        m_log_bin_dir = this->find_log_bin_dir(thd);
    }

    bool check_dirs_layout(THD *thd) {
        // Ignore directories that are children of the MySQL data dir.
        if (m_tokudb_data_dir != NULL &&
            this->dir_is_child_of_dir(m_tokudb_data_dir, m_mysql_data_dir) == false) {
            tokudb_data_set = true;
        }

        if (m_tokudb_log_dir != NULL &&
            this->dir_is_child_of_dir(m_tokudb_log_dir, m_mysql_data_dir) == false) {
            tokudb_log_set = true;
        }

        if (m_log_bin_dir != NULL &&
            this->dir_is_child_of_dir(m_log_bin_dir, m_mysql_data_dir) == false) {
            log_bin_set = true;
        }

        // Check if TokuDB log dir is a child of TokuDB data dir.  If it is, we want to ignore it.
        if (tokudb_log_set && tokudb_data_set) {
            if (this->dir_is_child_of_dir(m_tokudb_log_dir, m_tokudb_data_dir)) {
                tokudb_log_set = false;
            }
        }

        // Check if log bin dir is a child of either TokuDB data dir.
        if (log_bin_set && tokudb_data_set) {
            if (this->dir_is_child_of_dir(m_log_bin_dir, m_tokudb_data_dir)) {
                log_bin_set = false;
            }
        }

        // Check if log bin dir is a child of either TokuDB log dir.
        if (log_bin_set && tokudb_log_set) {
            if (this->dir_is_child_of_dir(m_log_bin_dir, m_tokudb_log_dir)) {
                log_bin_set = false;
            }
        }

        // Check if any of the three non-mysql dirs is a strict parent
        // of the mysql data dir.  This is an error.  NOTE: They can
        // be the same.
        int error = EINVAL;
        const char *error_fmt = "%s directory %s can't be a parent of mysql data dir %s when backing up";
        if (tokudb_data_set &&
            this->dir_is_child_of_dir(m_mysql_data_dir, m_tokudb_data_dir) == true &&
            this->dirs_are_the_same(m_tokudb_data_dir, m_mysql_data_dir) == false) {
            tokudb_backup_set_error_string(thd, error, error_fmt, "tokudb-data-dir", m_tokudb_data_dir, m_mysql_data_dir);
            return false;
        }

        if (tokudb_log_set &&
            this->dir_is_child_of_dir(m_mysql_data_dir, m_tokudb_log_dir) == true &&
            this->dirs_are_the_same(m_tokudb_log_dir, m_mysql_data_dir) == false) {
            tokudb_backup_set_error_string(thd, error, error_fmt, "tokudb-log-dir", m_tokudb_log_dir, m_mysql_data_dir);
            return false;
        }

        if (log_bin_set &&
            this->dir_is_child_of_dir(m_mysql_data_dir, m_log_bin_dir) == true &&
            this->dirs_are_the_same(m_log_bin_dir, m_mysql_data_dir) == false) {
            tokudb_backup_set_error_string(thd, error, error_fmt, "mysql log-bin", m_log_bin_dir, m_mysql_data_dir);
            return false;
        }

        return true;
    }

    void set_dirs(void) {
        // Set the directories in the output array.
        m_count = 0;
        m_dirs[m_count++] = m_mysql_data_dir;
        if (tokudb_data_set) {
            m_dirs[m_count++] = m_tokudb_data_dir;
        }
        if (tokudb_log_set) {
            m_dirs[m_count++] = m_tokudb_log_dir;
        }
        if (log_bin_set) {
            m_dirs[m_count++] = m_log_bin_dir;
        }
    }

    int set_valid_dirs_and_get_count(const char *array[], const int size) {
        int count = 0;
        if (size > MYSQL_MAX_DIR_COUNT) {
            return count;
        }
        for (int i = 0; i < MYSQL_MAX_DIR_COUNT; ++i) {
            if (m_dirs[i] != NULL) {
                count++;
            }
            array[i] = m_dirs[i];
        }
        return count;
    }

    bool is_child_of_any(const char *dest_dir, THD * thd) {
        bool result = false;
        for (int i = 0; i < m_count; i++) {
            if (tokudb_backup_is_child_of(dest_dir, m_dirs[i])) { 
                tokudb_backup_set_error_string(thd, EINVAL, "%s is a child of %s", dest_dir, m_dirs[i], NULL);
                result = true;
            }
        }
        return result;
    }

private:

    const char * find_log_bin_dir(THD *thd) {
        if (opt_bin_logname == NULL) {
            return NULL;
        }

        // If this has been set to just a filename, and not a path to
        // a regular file, we don't want to back this up to its own
        // directory, just skip it.
        if (opt_bin_logname[0] != '/') {
            return NULL;
        }

        int length = strlen(opt_bin_logname);
        char *buf = (char *)my_malloc(length + 1, 0);
        if (buf == NULL) {
            return NULL;
        }

        bool r = normalize_binlog_name(buf, opt_bin_logname, false);
        if (r) {
            my_free((void*)buf);
            return NULL;
        }

        // Add end of string char.
        buf[length] = 0;

        // NOTE: We have to extract the directory of this field.
        this->truncate_and_set_file_name(buf, length);
        return buf;
    }

    const char * find_plug_in_sys_var(const char *name, THD *thd) {
        const char * result = NULL;
        String null_string;
        String name_to_find(name, &my_charset_bin);
        // If get_system_var fails, it calls my_error
        Item *item = get_system_var(thd,
                                    OPT_GLOBAL,
                                    name_to_find.lex_string(),
                                    null_string.lex_string());
        if (item) {
            String scratch;
            String * str = item->val_str(&scratch);
            if (str) {
                result = my_strdup(str->ptr(), MYF(MY_FAE));
            }
        }

        // delete item; // auto deleted when the query ends

        return result;
    }

    // is directory "a" a child of directory "b"
    bool dir_is_child_of_dir(const char *a, const char *b) {
        return tokudb_backup_is_child_of(a, b);
    }

    // is directory "a" the same as directory "b"
    bool dirs_are_the_same(const char *a, const char *b) {
        bool result = false;
        char *apath = tokudb_backup_realpath_with_slash(a);
        char *bpath = tokudb_backup_realpath_with_slash(b);
        if (apath && bpath) {
            result = strcmp(apath, bpath) == 0;
        }
        if (apath)
            free(apath);
        if (bpath)
            free(bpath);
        return result;
    }

    // Removes the trailing bin log file from the system variable.
    void truncate_and_set_file_name(char *str, int length) {
        const char slash = '/';
        int position_of_last_slash = 0;

        // NOTE: We don't care about the leading slash, so it's ok to
        // only scan backwards to the 2nd character.
        for (int i = length; i > 0; --i) {
            if (str[i] == slash) {
                position_of_last_slash = i;
                break;
            }
        }

        // NOTE: MySQL should not allow this to happen.  The user
        // needs to specify a file, not the root dir (/).  This
        // shouldn't happen, but it might, so let's pretend it's ok.
        if (position_of_last_slash != 0) {
            // NOTE: We are sanitizing the path by removing the last slash.
            str[position_of_last_slash] = 0;
        }
    }
};

struct destination_dirs {
    const char * m_backup_dir;
    int m_backup_dir_len;
    const char * m_dirs[MYSQL_MAX_DIR_COUNT];

    destination_dirs(const char *backup_dir) : m_backup_dir(backup_dir) {
        m_backup_dir_len = strlen(m_backup_dir);
        m_dirs[0] = m_backup_dir;
        for (int i = 1; i < MYSQL_MAX_DIR_COUNT; ++i) {
            m_dirs[i] = NULL;
        }
    };

    ~destination_dirs() {
        for (int i = 0; i < MYSQL_MAX_DIR_COUNT; ++i) {
            my_free((void*)m_dirs[i]);
        }
    }

    bool set_backup_subdir(const char *postfix, const int index) {
        bool result = false;
        if (index < 0 || index >= MYSQL_MAX_DIR_COUNT) {
            return false;
        }
        const int len = strlen(postfix);
        const int total_len = len + m_backup_dir_len + 1;
        char *str = (char *)my_malloc(sizeof(char) * total_len, MYF(0));
        if (str) {
            strcpy(str, m_backup_dir);
            strcat(str, postfix);
            m_dirs[index] = str;
            result = true;
        }
        return result;
    };

    int create_dirs(void) {
        int result = 0;
        for (int i = 0; i < MYSQL_MAX_DIR_COUNT; ++i) {
            if (m_dirs[i]) {
                result = my_mkdir(m_dirs[i], 0777, MYF(0));
                if (result != 0) {
                    result = errno;
                    break;
                }
            }
        }
        return result;
    };

private:
    destination_dirs() {};
};

static void tokudb_backup_run(THD *thd, const char *dest_dir) {
    int error = 0;

    // check that the dest dir is a child of the tokudb_backup_allowed_prefix
    if (tokudb_backup_allowed_prefix) {
        if (!tokudb_backup_is_child_of(dest_dir, tokudb_backup_allowed_prefix)) {
            error = EINVAL;
            tokudb_backup_set_error_string(thd, error, "%s is not a child of %s", dest_dir, tokudb_backup_allowed_prefix, NULL);
            return;
        }
    }

    // check if the dest dir exists
    char *dest_dir_path = tokudb_backup_realpath_with_slash(dest_dir);
    if (dest_dir_path == NULL) {
        error = errno;
        tokudb_backup_set_error_string(thd, error, "Could not get real path for %s", dest_dir, NULL, NULL);
        return;
    }
    free(dest_dir_path);

    source_dirs sources;
    sources.find_and_allocate_dirs(thd);

    if (sources.check_dirs_layout(thd) == false) {
        return;
    }

    sources.set_dirs();

    if (sources.is_child_of_any(dest_dir, thd)) {
        return;
    }

    struct destination_dirs destinations(dest_dir);
    int index = 0;
    destinations.set_backup_subdir("/mysql_data_dir", index);
    if (sources.tokudb_data_set) {
       destinations.set_backup_subdir("/tokudb_data_dir", ++index);
    }

    if (sources.tokudb_log_set) {
       destinations.set_backup_subdir("/tokudb_log_dir", ++index);
    }

    if (sources.log_bin_set) {
        destinations.set_backup_subdir("/mysql_log_bin", ++index);
    }

    error = destinations.create_dirs();
    if (error) {
        tokudb_backup_set_error(thd, error, "tokudb backup couldn't create needed directories.");
        return;
    }

    char *exclude_string = THDVAR(thd, exclude);
    regex_t exclude_re;
    if (exclude_string) {
        int regr = regcomp(&exclude_re, exclude_string, REG_EXTENDED);
        if (regr) {
            error = EINVAL;
            char reg_error[100+strlen(exclude_string)];
            snprintf(reg_error, sizeof reg_error, "tokudb backup exclude %s regcomp %d", exclude_string, regr);
            tokudb_backup_set_error(thd, error, reg_error);
            return;
        }
    }

    const char *source_dirs[MYSQL_MAX_DIR_COUNT] = {};
    const char *dest_dirs[MYSQL_MAX_DIR_COUNT] = {};
    int count = sources.set_valid_dirs_and_get_count(source_dirs, MYSQL_MAX_DIR_COUNT);
    for (int i = 0; i < count; ++i) {
        dest_dirs[i] = destinations.m_dirs[i];
    }

    // set the throttle
    tokubackup_throttle_backup(THDVAR(thd, throttle));

    // do the backup
    tokudb_backup_progress_extra progress_extra = { thd, NULL };
    tokudb_backup_error_extra error_extra = { thd };
    tokudb_backup_exclude_copy_extra exclude_copy_extra = { thd, exclude_string, &exclude_re };
    error = tokubackup_create_backup(source_dirs, dest_dirs, count,
                                     tokudb_backup_progress_fun, &progress_extra,
                                     tokudb_backup_error_fun, &error_extra,
                                     tokudb_backup_exclude_copy_fun, &exclude_copy_extra);

    if (exclude_string)
        regfree(&exclude_re);

    // cleanup
    thd_proc_info(thd, "tokudb backup done"); // must be a static string
    my_free(progress_extra._the_string);
    
    THDVAR(thd, last_error) = error;
}

static int tokudb_backup_check_dir(THD *thd, struct st_mysql_sys_var *var, void *save, struct st_mysql_value *value) {
    // check for set global and its synomyms

    // reset error variables
    int error = 0;
    tokudb_backup_set_error(thd, error, NULL);

    // check access
    if (check_global_access(thd, SUPER_ACL)) {
        return 1;
    }

    // check_func_str
    char buff[STRING_BUFFER_USUAL_SIZE];
    int length = sizeof(buff);
    const char *str = value->val_str(value, buff, &length);
    if (str) {
        str = thd->strmake(str, length);
        *(const char**)save = str;
    }

    if (str) {
        // run backup
        tokudb_backup_run(thd, str);

        // get the last backup error
        error = THDVAR(thd, last_error);
    } else {
        error = EINVAL;
    }

    return error;
}

static void tokudb_backup_update_dir(THD *thd, struct st_mysql_sys_var *var, void *var_ptr, const void *save) {
    // nothing to do, backup is run in the check dir function
}

static int tokudb_backup_check_throttle(THD *thd, struct st_mysql_sys_var *var, void *save, struct st_mysql_value *value) {
    // check access
    if (check_global_access(thd, SUPER_ACL)) {
        return 1;
    }

    // save throttle
    longlong n;
    value->val_int(value, &n);
    *(longlong *) save = n;
    return 0;
}

static void tokudb_backup_update_throttle(THD *thd, struct st_mysql_sys_var *var, void *var_ptr, const void *save) {
    my_ulonglong *val = (my_ulonglong *) var_ptr;
    *val = *(my_ulonglong*) save;
    unsigned long nb = *val;
    tokubackup_throttle_backup(nb);
}

static int tokudb_backup_plugin_init(void *p) {
    DBUG_ENTER(__FUNCTION__);
    DBUG_RETURN(0);
}

static int tokudb_backup_plugin_deinit(void *p) {
    DBUG_ENTER(__FUNCTION__);
    DBUG_RETURN(0);
}

struct st_mysql_daemon tokudb_backup_plugin = {
    MYSQL_DAEMON_INTERFACE_VERSION
};

#ifndef TOKUDB_BACKUP_PLUGIN_VERSION_MAJOR
#define TOKUDB_BACKUP_PLUGIN_VERSION_MAJOR 0
#endif
#ifndef TOKUDB_BACKUP_PLUGIN_VERSION_MINOR
#define TOKUDB_BACKUP_PLUGIN_VERSION_MINOR 0
#endif

mysql_declare_plugin(tokudb_backup) {
    MYSQL_DAEMON_PLUGIN,
    &tokudb_backup_plugin,
    "tokudb_backup",
    "Tokutek",
    "Tokutek hot backup",
    PLUGIN_LICENSE_GPL,
    tokudb_backup_plugin_init,      // Plugin Init
    tokudb_backup_plugin_deinit,    // Plugin Deinit
    (TOKUDB_BACKUP_PLUGIN_VERSION_MAJOR << 8) + TOKUDB_BACKUP_PLUGIN_VERSION_MINOR,
    NULL,                        // status variables
    tokudb_backup_system_variables, // system variables
    NULL,                        // config options
    0,                           // flags
}
mysql_declare_plugin_end;
