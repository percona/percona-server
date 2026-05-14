/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:

#ifndef BACKUP_H
#define BACKUP_H

#ident "Copyright (c) 2012-2013 Tokutek Inc.  All rights reserved."
#ident "$Id: 4edcb140608da110c9a225a606dc13e77301d274 $"

extern "C" {

// These public API's should be in C.

typedef int (*backup_poll_fun_t)(float progress, const char *progress_string, void *poll_extra);

typedef void (*backup_error_fun_t)(int error_number, const char *error_string, void *error_extra);

// The exclude_copy callback if called for every file that will be copied.
// When it returns 0, the file is copied.  Otherwise, the file copy is skipped.
typedef int (*backup_exclude_copy_fun_t)(const char *source_file,void *extra);

typedef void (*backup_before_stop_capt_fun_t)(void *extra);
typedef void (*backup_after_stop_capt_fun_t)(void *extra);


int tokubackup_create_backup(const char *source_dirs[],
                             const char *dest_dirs[],
                             int dir_count,
                             backup_poll_fun_t poll_fun,
                             void *poll_extra,
                             backup_error_fun_t error_fun,
                             void *error_extra,
                             backup_exclude_copy_fun_t check_fun,
                             void *exclude_copy_extra,
                             backup_before_stop_capt_fun_t bsc_fun,
                             void *bsc_extra,
                             backup_after_stop_capt_fun_t asc_fun,
                             void *asc_extra)
    throw() __attribute__((visibility("default")));
// Effect: Backup the directories in source_dirs into correspnding dest_dirs.
// Periodically call poll_fun.
//  If poll_fun returns 0, then the backup continues.
//  If poll_fun returns nonzero, then the backup halts, and the result
//      returned by poll_fun is returned by tokubackup_create_backup().
//  We pass to the poll_fun
//    * a progress number (a float that ranges from 0.0 (no progress so
//      far) to 1.0 (done), and 0.5 meaning we think we are half done).
//    * a string which the backup software can specify.  The string is
//      human readable information about the backup progress.  The string may
//      be deallocated after the poll_fun returns, so if you want to save the
//      data, copy it somewhere.
//    * the poll_extra which was passed to tokubackup_create_backup().
//  If an error occurs and error_fun is non-NULL then we call
//  error_fun with the error number and a string which is descriptive.
//  The string may be deallocated soon, so copy it if you want it.
// Arguments:
//   source_dirs: an array of strings which name the source directories.
//   dest_dirs: an array of strings naming the destinations.
//   dir_count: how many source dirs  and dest_dirs are there?
//   poll_fun: a function to call periodically during backup.
//   poll_extra: a value passed to poll_fun.
//   error_fun: a function to call if an error happens
//   error_extra: a value passed to error_fun.
//   exclude_copy_fun: a function to call for every file being copied
//   exclude_copy_extra: a valuue passed to the exclude_copy_fun
// Return value:  0 if the backup succeeded.  Nonzero if the backup
//   was stopped early (returning the result from the poll_fun) or the
//   error code.
// Rationale: The poll_fun gives us a way to stop the backup, and also
//     to inform the user of progress.
//   The error_fun provides a way to give a description (e.g., to put
//     into the log).  There are thus two ways for the caller to find out the
//     error: the error_fun is called and the error number is returned.
//   This single function provides almost everything we need to do a
//     backup.  For example the mysql code can abort backup when the user types ctrl-C 
//     by returning a nonzero value from poll_fun.

void tokubackup_throttle_backup(unsigned long bytes_per_second) throw() __attribute__((visibility("default")));
// Effect: Throttle the rate at which copying happens.
//   This function can be called by any thread at any time, and will throttle
//   future backups as well as any currently running backup.
//  If you pass zero, then the backup will stop consuming any read
//   bandwidth on the source directory (but will continue to capture writes
//   and put them in the destination).  You might want to do that, for example,
//   if you have a critical query running and want to absolutely minimize the
//   impact of backup, without actually aborting the backup.
//  Pass in ULONG_MAX completely unthrottle the backup.  (This system may
//   actually throttle the backup to that rate, but 
//   ULONG_MAX comes out to 16 Yobibytes/second, which is effectively
//   infinite).
//  The system throttles the reads out of the source directory, not writes into
//   the destination directory.  If the underlying data directory is being modified
//   at a high rate, then the destination directory will receive those modifications
//   at the same rate, plus receive the throttled read data from the source.

const extern char *tokubackup_version_string  __attribute__((visibility("default")));

const int BACKUP_SUCCESS = 0;
}

#endif // end of header guardian.
