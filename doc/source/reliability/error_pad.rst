.. _error_pad:

==========================
 Error Code Compatibility
==========================

|Percona Server| with |XtraDB| has error code incompatibilities with |MySQL| 5.5. It is important to maintain compatibility in the error codes used by the servers. For example, scripts that may be run on both servers could contain references to error codes.

The reasons for the current incompatibilities are:

  * |Percona Server| with |XtraDB| contains features that have been backported from MyQL 5.5. Some of the |MySQL| 5.5 features added new error codes.

  * Some |Percona Server| with |XtraDB| features have added new error codes.

The solution to the first problem is to preserve |MySQL| 5.5 error codes in the |Percona Server|. An example of where this has been done is |Percona Server| feature Query Cache Enhancements. This feature adds error ``ER_QUERY_CACHE_DISABLED`` to the |Percona Server|, which is defined as error code 1651 in |MySQL| 5.5.

After migrating |Percona Server| / |XtraDB| to |MySQL| 5.5, users might experience troubles because of this.

The solution to the second problem is to insure that unique error codes are chosen, when adding new ones to |Percona Server|, that will never be duplicated during |MySQL| development.

For example, |MySQL| has a tool ``comp_err`` that generates:

  - :file:`errmsg.sys` files

  - header file :file:`include/mysqld_error.h`

  - header file :file:`include/mysqld_ername.h`

from the file :file:`errmsg.txt`.

To keep error numbers consistent, we should add some fictive errors to :file:`errmsg.txt`, because ``comp_err`` assigns error code numbers sequentially, without gaps.

I propose patch to ``comp_err``.

This patch allows usage of a new syntax, with prefix ``PADD``, for example: ::

  PADD_QUERY_CACHE_DISABLED 1651
    eng "ER_QUERY_CACHE_DISABLED padding to 1651 error"
  ER_QUERY_CACHE_DISABLED
    eng "Query cache is disabled; restart the server with query_cache_type=1 to enable it"

comp_err with my patch padds empty intervals (from last error code number to 1651) by error message ``ER_QUERY_CACHE_DISABLED padding to 1651 error``, i.e. and ``ER_QUERY_CACHE_DISABLED`` now has error code 1651 (as desired). I propose to use this patch for Percona errors, for example: ::

  PADD_PERCONA_NEW_ERROR_CODE 4000
    end "Padd empty space to error code number 4000 (Percona error codes)"
  ...some percona error codes...

Patch only adds prefix ``PADD_`` and padds error in sys files. All other |MySQL| code (load*.sys files, my_error, etc) works as old one.


Version-Specific Information
============================

  * 5.1.49-12.0:
    Full functionality available.
