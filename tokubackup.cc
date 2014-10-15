#include <mysql_version.h>
#include <mysql/plugin.h>
#include <my_global.h>
#include <my_dbug.h>
#include <sql_class.h>
#include <log.h>
#include "backup/backup.h"

static char *tokubackup_version = (char *) tokubackup_version_string;

static MYSQL_SYSVAR_STR(version, tokubackup_version, PLUGIN_VAR_NOCMDARG | PLUGIN_VAR_READONLY, "version",
                        NULL, NULL, NULL);

static MYSQL_THDVAR_ULONG(debug, PLUGIN_VAR_THDLOCAL, "debug", 
                          NULL, NULL, 0 /*default*/, 0 /*min*/, 1 /*max*/, 1 /*blocksize*/);

static MYSQL_THDVAR_ULONG(last_error, PLUGIN_VAR_THDLOCAL, "last error",
                          NULL, NULL, 0 /*default*/, 0 /*min*/, ~0ULL /*max*/, 1 /*blocksize*/);

static MYSQL_THDVAR_STR(last_error_string, PLUGIN_VAR_THDLOCAL | PLUGIN_VAR_MEMALLOC, "last error string", NULL, NULL, NULL);

struct tokubackup_progress_extra {
    THD *_thd;
    char *_the_string;
};

static int tokubackup_progress_fun(float progress, const char *progress_string, void *extra) {
    tokubackup_progress_extra *be = static_cast<tokubackup_progress_extra *>(extra);

    // print to error log
    if (THDVAR(be->_thd, debug))
        sql_print_information("tokubackup progress %f %s", progress, progress_string);

    // set thd proc info
    thd_proc_info(be->_thd, "");
    size_t len = 100 + strlen(progress_string);
    be->_the_string = (char *) my_realloc(be->_the_string, len, MYF(MY_FAE+MY_ALLOW_ZERO_PTR));
    float percentage = progress * 100;
    int r = snprintf(be->_the_string, len, "Tokubackup about %.0f%% done: %s", percentage, progress_string);
    assert(0 < r && (size_t)r <= len);
    thd_proc_info(be->_thd, be->_the_string);

    if (thd_killed(be->_thd)) {
        return ER_ABORTING_CONNECTION;
    }

    return 0;
}

static void tokubackup_set_error(THD *thd, int last_error, const char *last_error_string) {
    THDVAR(thd, last_error) = last_error;
    char *old_error_string = THDVAR(thd, last_error_string);
    THDVAR(thd, last_error_string) = last_error_string ? my_strdup(last_error_string, MYF(MY_FAE)) : NULL;
    if (old_error_string)
        my_free(old_error_string);
}

struct tokubackup_error_extra {
    THD *_thd;
};

static void tokubackup_error_fun(int error_number, const char *error_string, void *extra) {
    tokubackup_error_extra *be = static_cast<tokubackup_error_extra *>(extra);

    // print to error log
    if (THDVAR(be->_thd, debug))
        sql_print_information("tokubackup error %d %s", error_number, error_string);

    // set last_error and last_error_string
    tokubackup_set_error(be->_thd, error_number, error_string);
}

static void tokubackup_update_dir(THD *thd, struct st_mysql_sys_var *var, void *var_ptr, const void *save) {
    // reset error variables
    tokubackup_set_error(thd, 0, NULL);
    // do the backup
    tokubackup_progress_extra progress_extra = { thd, NULL };
    tokubackup_error_extra error_extra = { thd };
    const char *source_dirs[1] = { mysql_real_data_home };
    const char *dest_dirs[1] = { *(const char **) save };
    if (THDVAR(thd, debug))
        sql_print_information("%s backup %s %s", __FUNCTION__, source_dirs[0], dest_dirs[0]);
    int error = tokubackup_create_backup(source_dirs, dest_dirs, 1, tokubackup_progress_fun, &progress_extra, tokubackup_error_fun, &error_extra);
    if (THDVAR(thd, debug))
        sql_print_information("%s backup error %d", __FUNCTION__, error);
    THDVAR(thd, last_error) = error;
    thd_proc_info(thd, "tokubackup done"); // must be a static string
    my_free(progress_extra._the_string);
}

static MYSQL_THDVAR_STR(dir, PLUGIN_VAR_THDLOCAL, "backup dir", NULL, tokubackup_update_dir, NULL);

static void tokubackup_update_throttle(THD *thd, struct st_mysql_sys_var *var, void *var_ptr, const void *save) {
    my_ulonglong *val = (my_ulonglong *) var_ptr;
    *val = *(my_ulonglong*) save;
    unsigned long nb = *val;
    if (THDVAR(thd, debug))
        sql_print_information("%s %lu", __FUNCTION__, nb);
    tokubackup_throttle_backup(nb);
}

static MYSQL_THDVAR_ULONGLONG(throttle, PLUGIN_VAR_THDLOCAL, "backup throttle",
                              NULL, tokubackup_update_throttle, 0 /*default*/, 0 /*min*/, ~0ULL /*max*/, 1 /*blocksize*/);

static struct st_mysql_sys_var *tokubackup_system_variables[] = {
    MYSQL_SYSVAR(version),
    MYSQL_SYSVAR(debug),
    MYSQL_SYSVAR(dir),
    MYSQL_SYSVAR(throttle),
    MYSQL_SYSVAR(last_error),
    MYSQL_SYSVAR(last_error_string),
    NULL,
};

static int tokubackup_plugin_init(void *p) {
    DBUG_ENTER(__FUNCTION__);
    DBUG_RETURN(0);
}

static int tokubackup_plugin_deinit(void *p) {
    DBUG_ENTER(__FUNCTION__);
    DBUG_RETURN(0);
}

struct st_mysql_daemon tokubackup_plugin = { 
    MYSQL_DAEMON_INTERFACE_VERSION 
};

mysql_declare_plugin(tokubackup) {
    MYSQL_DAEMON_PLUGIN,
    &tokubackup_plugin,
    "tokubackup",
    "Tokutek",
    "Tokutek hot backup",
    PLUGIN_LICENSE_PROPRIETARY,
    tokubackup_plugin_init,      // Plugin Init
    tokubackup_plugin_deinit,    // Plugin Deinit
    0x0100, // 1.0
    NULL,                        // status variables
    tokubackup_system_variables, // system variables
    NULL,                        // config options
    0,                           // flags
}
mysql_declare_plugin_end;
