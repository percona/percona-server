/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:  
#ident "Copyright (c) 2014 Tokutek Inc.  All rights reserved."

#define MYSQL_SERVER
#define HAVE_REPLICATION
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
#include <rpl_mi.h>
#include <rpl_slave.h>
#include <rpl_rli.h>
#include <sql_parse.h>
#include <mysqld.h>
#include <debug_sync.h>

#include <inttypes.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

template <typename T>
class BasicLockableClassWrapper
{
    T &m_lockable;
    void (T::*m_lock)(void);
    void (T::*m_unlock)(void);
public:
    BasicLockableClassWrapper(T &a_lockable,
                              void (T::*a_lock)(void),
                              void (T::*a_unlock)(void)) :
        m_lockable(a_lockable),
        m_lock(a_lock),
        m_unlock(a_unlock) {}

    void lock() {
        ((m_lockable).*(m_lock))();
    }

    void unlock() {
        ((m_lockable).*(m_unlock))();
    }
};

class BasicLockableMysqlMutextT {
    mysql_mutex_t &m_mutex;
    public:
        BasicLockableMysqlMutextT(mysql_mutex_t &mutex) : m_mutex(mutex) {}
        void lock() { mysql_mutex_lock(&m_mutex); }
        void unlock() {mysql_mutex_unlock(&m_mutex); }
};

template <typename BasicLockableWrapper>
class scoped_lock_wrapper
{
    BasicLockableWrapper m_lockable;
public:
    scoped_lock_wrapper(const BasicLockableWrapper &lockable) :
        m_lockable(lockable) {
        m_lockable.lock();
    }
    ~scoped_lock_wrapper() {
        m_lockable.unlock();
    }
private:
    scoped_lock_wrapper(
        const scoped_lock_wrapper<BasicLockableWrapper> &);
    scoped_lock_wrapper& operator=(
        scoped_lock_wrapper<BasicLockableWrapper> &);
};

typedef BasicLockableClassWrapper<Checkable_rwlock> Checkable_rwlock_lockable;

struct tokudb_backup_master_info {
    std::string host;
    std::string user;
    uint32_t port;
    std::string master_log_file;
    std::string relay_log_file;
    uint64_t exec_master_log_pos;
    std::string executed_gtid_set;
    std::string channel_name;
};

struct tokudb_backup_master_state {
    std::string file_name;
    my_off_t position;
    std::string executed_gtid_set;
    ulong gtid_mode;
};

#ifdef TOKUDB_BACKUP_PLUGIN_VERSION
#define stringify2(x) #x
#define stringify(x) stringify2(x)
#define TOKUDB_BACKUP_PLUGIN_VERSION_STRING stringify(TOKUDB_BACKUP_PLUGIN_VERSION)
#else
#define TOKUDB_BACKUP_PLUGIN_VERSION_STRING NULL
#endif

/* innodb_use_native_aio option */
extern my_bool srv_use_native_aio;

static char *tokudb_backup_plugin_version;

static const char* master_info_file_name = "tokubackup_slave_info";
static const char* master_state_file_name = "tokubackup_binlog_info";

static char tokudb_backup_safe_slave = FALSE;
static ulonglong tokudb_backup_safe_slave_timeout = 0;
static bool sql_thread_started = false;

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

static MYSQL_SYSVAR_BOOL(safe_slave, tokudb_backup_safe_slave,
       PLUGIN_VAR_OPCMDARG, "Wait until there is no temporary slave tables.",
       NULL,
       NULL,
       0);

static MYSQL_SYSVAR_ULONGLONG(safe_slave_timeout,
    tokudb_backup_safe_slave_timeout,
    PLUGIN_VAR_OPCMDARG,
    "The maximum amount of seconds to wait for slave temp tables disappear "
    "0 - don't wait",
    NULL, NULL, 0, 0, ULONGLONG_MAX, 0);

static struct st_mysql_sys_var *tokudb_backup_system_variables[] = {
    MYSQL_SYSVAR(plugin_version),
    MYSQL_SYSVAR(version),
    MYSQL_SYSVAR(allowed_prefix),
    MYSQL_SYSVAR(safe_slave),
    MYSQL_SYSVAR(safe_slave_timeout),
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

static bool tokudb_backup_check_slave_sql_thread_running(THD *thd) {
     scoped_lock_wrapper<BasicLockableMysqlMutextT>
        with_LOCK_active_mi_locked(
        BasicLockableMysqlMutextT(&LOCK_active_mi));

    Master_info *mi = active_mi;

    if (!mi || !mi->inited || !mi->host[0])
        return false;

    scoped_lock_wrapper<BasicLockableMysqlMutextT>
        with_mi_data_locked_1(BasicLockableMysqlMutextT(
            mi->data_lock));
    scoped_lock_wrapper<BasicLockableMysqlMutextT>
        with_mi_data_locked_2(BasicLockableMysqlMutextT(
            mi->rli->data_lock));
    scoped_lock_wrapper<BasicLockableMysqlMutextT>
        with_mi_data_locked_3(BasicLockableMysqlMutextT(
            mi->err_lock));
    scoped_lock_wrapper<BasicLockableMysqlMutextT>
        with_mi_data_locked_4(BasicLockableMysqlMutextT(
            mi->rli->err_lock));

    return mi->rli->slave_running;
}

static bool tokudb_backup_stop_slave_sql_thread(THD *thd) {
    bool stop_slave_result;
    bool result;

    if (!active_mi)
        return true;

    thd->lex->slave_thd_opt = SLAVE_SQL;

    {
        scoped_lock_wrapper<BasicLockableMysqlMutextT>
            with_LOCK_active_mi_locked(
            BasicLockableMysqlMutextT(&LOCK_active_mi));

        if (!active_mi || !active_mi->inited || !active_mi->host[0])
            return true;

        stop_slave_result = stop_slave(thd, active_mi, 0);
    }

    result = !stop_slave_result &&
             !tokudb_backup_check_slave_sql_thread_running(thd);

    if (!result)
        sql_print_error("TokuDB Hotbackup: Can't stop slave sql thread\n");

    return result;
}

static bool tokudb_backup_start_slave_sql_thread(THD *thd) {
    bool start_slave_result;
    bool result;

    if (!active_mi)
        return true;

    thd->lex->slave_thd_opt = SLAVE_SQL;

    {
        scoped_lock_wrapper<BasicLockableMysqlMutextT>
            with_LOCK_active_mi_locked(
            BasicLockableMysqlMutextT(&LOCK_active_mi));

        if (!active_mi || !active_mi->inited || !active_mi->host[0])
            return true;

        start_slave_result = start_slave(thd, active_mi, 0);
    }

    result = !start_slave_result &&
             tokudb_backup_check_slave_sql_thread_running(thd);

    if (!result)
        sql_print_error("TokuDB Hotbackup: can't start slave sql thread");

    return result;
}

static bool tokudb_backup_wait_for_safe_slave(THD *thd, uint timeout) {
    static const uint sleep_time = 3000000;
    size_t n_attemts = tokudb_backup_safe_slave_timeout ?
        (1000000 * tokudb_backup_safe_slave_timeout / sleep_time) : 1;

    DEBUG_SYNC(thd, "tokudb_backup_wait_for_safe_slave_entered");
    if (!active_mi) {
        sql_thread_started = false;
        return false;
    }

    sql_thread_started = tokudb_backup_check_slave_sql_thread_running(thd);

    if (sql_thread_started && !tokudb_backup_stop_slave_sql_thread(thd))
        return false;

    while(slave_open_temp_tables && n_attemts--) {
        DEBUG_SYNC(thd, "tokudb_backup_wait_for_temp_tables_loop_begin");
        if (!tokudb_backup_start_slave_sql_thread(thd))
            return false;
        DEBUG_SYNC(thd, "tokudb_backup_wait_for_temp_tables_loop_slave_started");
        my_sleep(sleep_time);
        if (!tokudb_backup_stop_slave_sql_thread(thd))
            return false;
        DEBUG_SYNC(thd, "tokudb_backup_wait_for_temp_tables_loop_end");
    }

    if (!n_attemts &&
        slave_open_temp_tables) {

        (sql_thread_started &&
        !tokudb_backup_check_slave_sql_thread_running(thd) &&
        !tokudb_backup_start_slave_sql_thread(thd));

        return false;
    }

    return true;
}

static my_bool
tokudb_backup_flush_log_plugin_callback(THD *,
                                        plugin_ref plugin,
                                        void *) {

    const char *name = plugin_name(plugin)->str;
    handlerton *hton= plugin_data(plugin, handlerton *);

    if (!strcmp(name, "TokuDB") &&
        hton->state == SHOW_OPTION_YES && hton->flush_logs &&
        !hton->flush_logs(hton))
        return TRUE;

    return FALSE;
}

static void tokudb_backup_before_stop_capt_fun(void *arg) {
    THD *thd = static_cast<THD *>(arg);
    if (tokudb_backup_safe_slave) {
        if (!tokudb_backup_wait_for_safe_slave(
                thd,tokudb_backup_safe_slave_timeout)) {
            sql_print_error("TokuDB Hotbackup: safe slave option error");
        }
    }
    else
        (void)lock_binlog_for_backup(thd);
    if (!plugin_foreach(NULL,
                        tokudb_backup_flush_log_plugin_callback,
                        MYSQL_STORAGE_ENGINE_PLUGIN,
                        0))
        tokudb_backup_set_error_string(thd, EINVAL, "Can't flush TokuDB log",
            NULL, NULL, NULL);
}

std::string tokudb_backup_get_executed_gtids_set() {
    char* sql_gtid_set_buffer = NULL;
    std::string result;
    {
        scoped_lock_wrapper<Checkable_rwlock_lockable>
            with_global_sid_lock_wrlock(
                Checkable_rwlock_lockable(*global_sid_lock,
                                     &Checkable_rwlock::wrlock,
                                     &Checkable_rwlock::unlock));

        const Gtid_set *sql_gtid_set= gtid_state->get_logged_gtids();
        (void)sql_gtid_set->to_string(&sql_gtid_set_buffer);
    }
    result.assign(sql_gtid_set_buffer);
    result.erase(std::remove(result.begin(), result.end(),'\n'), result.end());
    return result;
};

static void tokudb_backup_get_master_infos(
    THD *thd,
    std::vector<tokudb_backup_master_info> *master_info_channels) {

    Master_info *mi = active_mi;
    tokudb_backup_master_info tbmi;

    {
        scoped_lock_wrapper<BasicLockableMysqlMutextT>
            with_LOCK_active_mi_locked(
            BasicLockableMysqlMutextT(&LOCK_active_mi));

        if (!active_mi)
            return;

        std::string executed_gtid_set = tokudb_backup_get_executed_gtids_set();

        {
            scoped_lock_wrapper<BasicLockableMysqlMutextT>
                with_mi_data_locked_1(BasicLockableMysqlMutextT(
                    mi->data_lock));
            scoped_lock_wrapper<BasicLockableMysqlMutextT>
                with_mi_data_locked_2(BasicLockableMysqlMutextT(
                    mi->rli->data_lock));
            scoped_lock_wrapper<BasicLockableMysqlMutextT>
                with_mi_data_locked_3(BasicLockableMysqlMutextT(
                    mi->err_lock));
            scoped_lock_wrapper<BasicLockableMysqlMutextT>
                with_mi_data_locked_4(BasicLockableMysqlMutextT(
                    mi->rli->err_lock));

            tbmi.host.assign(mi->host);
            tbmi.user.assign(mi->get_user());
            tbmi.port = mi->port;
            tbmi.master_log_file.assign(mi->get_master_log_name());
            tbmi.relay_log_file.assign(mi->rli->get_group_relay_log_name() +
                dirname_length(mi->rli->get_group_relay_log_name()));
            tbmi.exec_master_log_pos = mi->rli->get_group_master_log_pos();
            tbmi.executed_gtid_set.assign(executed_gtid_set);
        }
    }

    master_info_channels->push_back(tbmi);
}

void tokudb_backup_get_master_state(
    tokudb_backup_master_state *master_state) {

    if (!mysql_bin_log.is_open())
        return;

    LOG_INFO li;
    mysql_bin_log.get_current_log(&li);

    master_state->file_name =
        (li.log_file_name + dirname_length(li.log_file_name));
    master_state->position = li.pos;
    master_state->executed_gtid_set = tokudb_backup_get_executed_gtids_set();
    master_state->gtid_mode = gtid_mode;

    return;
}

struct tokudb_backup_after_stop_capt_extra {
    THD *thd;
    std::vector<tokudb_backup_master_info> *master_info_channels;
    tokudb_backup_master_state *master_state;
};

static void tokudb_backup_after_stop_capt_fun(void *arg) {
    tokudb_backup_after_stop_capt_extra *extra =
        static_cast<tokudb_backup_after_stop_capt_extra *>(arg);
    THD *thd = extra->thd;
    std::vector<tokudb_backup_master_info> *master_info_channels =
        extra->master_info_channels;
    tokudb_backup_master_state *master_state = extra->master_state;
    if (tokudb_backup_safe_slave &&
        sql_thread_started &&
        tokudb_backup_check_slave_sql_thread_running(thd)) {

        tokudb_backup_set_error_string(thd,
                                       EINVAL,
                                       "Slave sql stread is not stopped",
                                       NULL, NULL, NULL);
        sql_print_error("TokuDB Hotbackup: "
                        "master and slave info can't be saved "
                        "because slave sql thread can't be stopped\n");

        return;
    }

    tokudb_backup_get_master_infos(thd, master_info_channels);
    tokudb_backup_get_master_state(master_state);

    if (tokudb_backup_safe_slave && sql_thread_started) {
        if (!tokudb_backup_start_slave_sql_thread(thd)) {
            tokudb_backup_set_error_string(thd,
                                       EINVAL,
                                       "Slave sql stread is not started",
                                       NULL, NULL, NULL);
            sql_print_error("TokuDB Hotbackup: "
                            "slave sql thread can't be started\n");
            return;
        }
    }
    else if (thd->backup_binlog_lock.is_acquired()) {
        thd->backup_binlog_lock.release(thd);
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

int tokudb_backup_save_master_infos(
    THD *thd,
    const char *dest_dir,
    const std::vector<tokudb_backup_master_info> &master_info_channels) {

    int error = 0;
    std::string mi_full_file_name(dest_dir);
    mi_full_file_name.append("/");
    mi_full_file_name.append(master_info_file_name);

    int fd = open(mi_full_file_name.c_str(),
                  O_WRONLY|O_CREAT,
                  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    if (fd < 0) {
        error = errno;
        tokudb_backup_set_error_string(
            thd, error, "Can't open master info file %s\n",
            mi_full_file_name.c_str(), NULL, NULL);
        return error;
    }

    for (std::vector<tokudb_backup_master_info>::const_iterator i =
            master_info_channels.begin(), end = master_info_channels.end();
        i != end;
        ++i) {

        std::stringstream out;
        out << "host: " << i->host << ", "
            << "user: " << i->user << ", "
            << "port: " << i->port << ", "
            << "master log file: " << i->master_log_file << ", "
            << "relay log file: " << i->relay_log_file << ", "
            << "exec master log pos: " << i->exec_master_log_pos << ", "
            << "executed gtid set: " << i->executed_gtid_set << ", "
            << "channel name: " << i->channel_name << std::endl;
        const std::string &out_str = out.str();
        if (write(fd, out_str.c_str(), out_str.length()) <
            (int)out_str.length())
        {
            error = EINVAL;
            tokudb_backup_set_error_string(
                thd, error, "Master info was not written fully",
                NULL, NULL, NULL);
            break;
        }
    }

    if (close(fd) < 0) {
        error = errno;
        tokudb_backup_set_error_string(
            thd, error, "Can't close master info file %s\n",
            mi_full_file_name.c_str(), NULL, NULL);
    }

    return error;
}

int tokudb_backup_save_master_state(
    THD *thd,
    const char *dest_dir,
    const tokudb_backup_master_state &master_state) {

    int error = 0;
    std::string ms_full_file_name(dest_dir);
    ms_full_file_name.append("/");
    ms_full_file_name.append(master_state_file_name);

    int fd = open(ms_full_file_name.c_str(),
                  O_WRONLY|O_CREAT,
                  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    if (fd < 0) {
        error = errno;
        tokudb_backup_set_error_string(
            thd, error, "Can't open master state file %s\n",
            ms_full_file_name.c_str(), NULL, NULL);
        return error;
    }

    std::stringstream out;
    out << "filename: " << master_state.file_name << ", "
        << "position: " << master_state.position << ", "
        << "gtid_mode: "
        << gtid_mode_names[master_state.gtid_mode] << ", "
        << "GTID of last change: " << master_state.executed_gtid_set
        << std::endl;

    const std::string &out_str = out.str();
    if (write(fd, out_str.c_str(), out_str.length()) <
        (int)out_str.length())
    {
        error = EINVAL;
        tokudb_backup_set_error_string(
            thd, error, "Master state was not written fully",
            NULL, NULL, NULL);
    }

    if (close(fd) < 0) {
        error = errno;
        tokudb_backup_set_error_string(
            thd, error, "Can't close master state file %s\n",
            ms_full_file_name.c_str(), NULL, NULL);
    }

    return error;

}

static void tokudb_backup_run(THD *thd, const char *dest_dir) {
    int error = 0;

    if (srv_use_native_aio) {
        error = EINVAL;
        tokudb_backup_set_error_string(thd,
                                       error,
                                       "tokudb hot backup is disabled when "
                                       "innodb_use_native_aio is enabled",
                                       NULL,
                                       NULL,
                                       NULL);
        return;
    }
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

    std::vector<tokudb_backup_master_info> master_info_channels;
    tokudb_backup_master_state master_state;

    // do the backup
    tokudb_backup_progress_extra progress_extra = { thd, NULL };
    tokudb_backup_error_extra error_extra = { thd };
    tokudb_backup_exclude_copy_extra exclude_copy_extra = { thd, exclude_string, &exclude_re };
    tokudb_backup_after_stop_capt_extra asce = {thd,
                                                &master_info_channels,
                                                &master_state};
    error = tokubackup_create_backup(source_dirs,
                                     dest_dirs,
                                     count,
                                     tokudb_backup_progress_fun,
                                     &progress_extra,
                                     tokudb_backup_error_fun,
                                     &error_extra,
                                     tokudb_backup_exclude_copy_fun,
                                     &exclude_copy_extra,
                                     tokudb_backup_before_stop_capt_fun,
                                     thd,
                                     tokudb_backup_after_stop_capt_fun,
                                     &asce);

    if (exclude_string)
        regfree(&exclude_re);

    if (!master_info_channels.empty() &&
        (error = tokudb_backup_save_master_infos(thd,
                                                 dest_dir,
                                                 master_info_channels)))
        goto exit;

    if (!master_state.file_name.empty() &&
        (error = tokudb_backup_save_master_state(thd,
                                                 dest_dir,
                                                 master_state)))
        goto exit;

exit:
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
