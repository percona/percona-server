.. _encrypting-doublewrite-buffers:

=======================================================================
Encrypting Doublewrite Buffers
=======================================================================

A summary of Doublewrite buffer and Doublewrite buffer encryption changes:

.. tabularcolumns:: |p{5cm}|p{11cm}|

.. list-table::
   :header-rows: 1

   * - |Percona Server| Versions
     - Doublewrite Buffer and Doublewrite Buffer Encryption Implementation
   * - Percona-Server-8.0.12-1.alpha to Percona-Server-8.0.19-10 inclusive
     - |Percona Server| had its own implementation of the parallel doublewrite buffer which was enabled by setting the :variable:`innodb_parallel_doublewrite_path` variable.

       Enabling the :variable:`innodb_parallel_dblwr_encrypt` controlled whether the parallel
       doublewrite pages were encrypted or not. In case the parallel doublewrite buffer was disabled (:variable:`innodb_parallel_doublewrite_path` was set to empty string),the doublewrite buffer pages were located in the system tablespace (ibdata1). The system tablespace itself could be encrypted by setting :variable:`innodb_sys_tablespace_encrypt`, which also encrypted the  doublewrite buffer pages.
   * - Percona Server from Percona-Server-8.0.20-11 to Percona-Server-8.0.22-13 inclusive
     - |MySQL| 8.0.20 implemented its own `parallel doublewrite buffer <https://dev.mysql.com/doc/refman/8.0/en/innodb-doublewrite-buffer.html>`__, which is stored in external files (#ib_16384_xxx.dblwr) and not stored in the system tablespace. Percona's implementation was reverted. As a result, :variable:`innodb_parallel_doublewrite_path` was deprecated.

       However, |MySQL| did not implement parallel doublewrite buffer
       encryption at this time, so Percona reimplemented parallel doublewrite
       buffer encryption on top of the |MySQL| parallel doublewrite buffer
       implementation. Percona preserved the meaning and
       functionality of the :variable:`innodb_parallel_dblwr_encrypt` variable.
   * - Percona Server from Percona-Server-8.0.23-14
     - |MySQL| 8.0.23 implemented its own version of `parallel doublewrite encryption <https://dev.mysql.com/doc/refman/8.0/en/innodb-data-encryption.html#innodb-doublewrite-file-encryption>`__.
       Pages that belong to encrypted tablespaces are also written into the
       doublewrite buffer in an encrypted form. Percona's
       implementation was reverted and :variable:`innodb_parallel_dblwr_encrypt` is deprecated.

For |Percona Server| versions below |Percona Server| version 8.0.23-14, |Percona| encrypts the ``doublewrite buffer`` using :variable:`innodb_parallel_dblwr_encrypt`.


 .. variable:: innodb_parallel_dblwr_encrypt

    :deprecated: Percona-Server 8.0.23-14
    :cli: ``--innodb-parallel-dblwr-encrypt``
    :dyn: Yes
    :scope: Global
    :vartype: Boolean
    :default: ``OFF``

This variable controls whether the parallel doublewrite buffer pages were encrypted or not. The encryption used the key of the tablespace to which the page belong. 

Starting from |Percona Server| 8.0.23-14, regardless of the value of this variable, pages from the encrypted tablespaces are always written to the doublewrite buffer as encrypted, and pages from unencrypted tablespaces are always written unencrypted.

The :variable:`innodb_parallel_dblwr_encrypt` is accepted but has no effect. An explicit attempt to change the value generates the following warning in the error log file:

    **Setting Percona-specific INNODB_PARALLEL_DBLWR_ENCRYPT is deprecated and has no effect.**
