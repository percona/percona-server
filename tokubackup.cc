#include <mysql_version.h>
#include <mysql/plugin.h>
#include <my_global.h>
#include <my_dbug.h>
#include <log.h>
#include <dlfcn.h>
#include "backup/backup.h"

static MYSQL_THDVAR_ULONG(last_error, PLUGIN_VAR_THDLOCAL, "last error",
                          NULL, NULL, 0 /*default*/, 0 /*min*/, ~0ULL /*max*/, 1 /*blocksize*/);

static int tokubackup_poll_fun(float progress, const char *progress_string, void *extra) {
    // print to error log
    sql_print_information("tokubackup progress %f %s", progress, progress_string);
    // TODO set thd info
    return 0;
}

static void tokubackup_error_fun(int error_number, const char *error_string, void *extra) {
    // print to error log
    sql_print_information("tokubackup error %d %s", error_number, error_string);
    // TODO set thd info
    // TODO set last_error_string
}

static void tokubackup_update_dir(THD *thd, struct st_mysql_sys_var *var, void *var_ptr, const void *save) {
    const char *source_dirs[1] = { mysql_real_data_home };
    const char *dest_dirs[1] = { *(const char **) save };
    sql_print_information("%s backup %s %s", __FUNCTION__, source_dirs[0], dest_dirs[0]);
    int error = tokubackup_create_backup(source_dirs, dest_dirs, 1, tokubackup_poll_fun, NULL, tokubackup_error_fun, NULL);
    sql_print_information("%s backup error %d", __FUNCTION__, error);
    THDVAR(thd, last_error) = error;
}

static MYSQL_THDVAR_STR(dir, PLUGIN_VAR_THDLOCAL, "backup dir", NULL, tokubackup_update_dir, "?");

static void tokubackup_update_throttle(THD *thd, struct st_mysql_sys_var *var, void *var_ptr, const void *save) {
    my_ulonglong *val = (my_ulonglong *) var_ptr;
    *val = *(my_ulonglong*) save;
    unsigned long nb = *val;
    sql_print_information("%s %lu", __FUNCTION__, nb);
    tokubackup_throttle_backup(nb);
}

static MYSQL_THDVAR_ULONGLONG(throttle, PLUGIN_VAR_THDLOCAL, "backup throttle",
                              NULL, tokubackup_update_throttle, 0 /*default*/, 0 /*min*/, ~0ULL /*max*/, 1 /*blocksize*/);

static struct st_mysql_sys_var *tokubackup_system_variables[] = {
    MYSQL_SYSVAR(dir),
    MYSQL_SYSVAR(throttle),
    MYSQL_SYSVAR(last_error),
    NULL,
};

static int tokubackup_plugin_init(void *p) {
    DBUG_ENTER(__FUNCTION__);
    int error = 0;
    sql_print_information("tokubackup %s", tokubackup_version_string);
    DBUG_RETURN(error);
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
