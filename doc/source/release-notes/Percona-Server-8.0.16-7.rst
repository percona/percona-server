.. _8.0.16-7:

===============================================================================
*Percona Server for MySQL* 8.0.16-7
===============================================================================

|Percona| announces the release of |Percona Server| |release| on |date|
(downloads are available `here
<https://www.percona.com/downloads/Percona-Server-8.0/>`__ and from the `Percona
Software Repositories
<https://www.percona.com/doc/percona-server/8.0/installation.html#installing-from-binaries>`__).
This release includes fixes to bugs found in previous releases of |Percona
Server| 8.0.
|Percona Server| |release| is now the current GA release in the 8.0
series. All of |Percona|’s software is open-source and free.

Percona Server for MySQL 8.0 includes all the `features and bug fixes available in MySQL 8.0.16
Community Edition
<https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-16.html>`__ in addition to
enterprise-grade features developed by Percona.

Encryption Features General Availability (GA)
===============================================================================

- :ref:`encrypting-temporary-files`
- :ref:`undo-tablespace-encryption`
- :ref:`encrypting-system-tablespace`
- :ref:`default_table_encryption` =OFF/ON
- :refref:`table_encryption_privilege_check` =OFF/ON
- :ref:`encrypting-redo-log` for master key encryption only
- :ref:`merge-sort-encryption`
- :ref:`encrypting-doublewrite-buffers`

Bugs Fixed
===============================================================================

- Parallel doublewrite buffer writes must crash the server on an I/O error occurs. Bug fixed :psbug:`5678`.

- After resetting the :ref:`innodb_temp_tablespace_encrypt` to ``OFF`` during runtime the subsequent file-per-table temporary tables continue to be encrypted. Bug fixed :psbug:`5734`.

- Setting the encryption to ``ON`` for the system tablespace generates an encryption key and encrypts system temporary tablespace pages. Resetting the encryption to ``OFF``, all subsequent pages are written to the temporary tablespace without encryption. To allow any encrypted tables to be decrypted, the generated keys are not erased. Modifying the :ref:`innodb_temp_tablespace_encrypt` does not affect file-per-table temporary tables. This type of table is encrypted if ``ENCRYPTION`` ='Y' is set during table creation. Bug fixed :psbug:`5736`.

- An instance started with the default values but setting the redo-log without specifying the keyring plugin parameters does not fail or throw an error. Bug fixed :psbug:`5476`.

- The :ref:`rocksdb_large_prefix` allows index key prefixes up to 3072 bytes. The default value is changed to ``TRUE`` to match the behavior of the :ref:`innodb_large_prefix`. :psbug:`5655`.

- On a server with a large number of tables, a shutdown may take a measurable length of time. Bug fixed :psbug:`5639`.

- The changed page tracking uses the LOG flag during read operations. The redo log encryption may attempt to decrypt pages with a specific bit set and fail. This failure generates error messages. A NO_ENCRYPTION flag lets the read process safely disable decryption errors in this case. Bug fixed :psbug:`5541`.

- If large pages are enabled on MySQL side, the maximum size for :ref:`innodb_buffer_pool_chunk_size` is effectively limited to 4GB. Bug fixed :psbug:`5517`. (Upstream `94747 <https://bugs.mysql.com/bug.php?id=94747>`__)

- The TokuDB hot backup library continually dumps TRACE information to the server error log. The user cannot enable or disable the dump of this information. Bug fixed :psbug:`4850`.



Other bugs fixed:
:psbug:`5688`,
:psbug:`5723`,
:psbug:`5695`,
:psbug:`5749`,
:psbug:`5752`,
:psbug:`5610`,
:psbug:`5689`,
:psbug:`5645`,
:psbug:`5734`,
:psbug:`5772`,
:psbug:`5753`,
:psbug:`5129`,
:psbug:`5102`,
:psbug:`5681`,
:psbug:`5686`,
:psbug:`5681`,
:psbug:`5310`,
:psbug:`5713`,
:psbug:`5007`,
:psbug:`5102`,
:psbug:`5129`,
:psbug:`5130`,
:psbug:`5149`,
:psbug:`5696`,
:psbug:`3845`,
:psbug:`5149`,
:psbug:`5581`,
:psbug:`5652`,
:psbug:`5662`,
:psbug:`5697`,
:psbug:`5775`,
:psbug:`5668`,
:psbug:`5752`,
:psbug:`5782`,
:psbug:`5767`,
:psbug:`5669`,
:psbug:`5753`,
:psbug:`5696`,
:psbug:`5803`,
:psbug:`5804`,
:psbug:`5820`,
:psbug:`5827`,
:psbug:`5835`,
:psbug:`5724`,
:psbug:`5767`,
:psbug:`5782`,
:psbug:`5794`,
:psbug:`5796`,
:psbug:`5746` and,
:psbug:`5748`.

Known Issues
==============================================================================

- :psbug:`5865`: |Percona Server| |release| does not support encryption for the MyRocks storage engine. An attempt to move any table from InnoDB to `MyRocks <https://www.percona.com/doc/percona-server/LATEST/myrocks/limitations.html>`__ fails as MyRocks currently sees all InnoDB tables as being encrypted.

.. |release| replace:: 8.0.16-7
.. |date| replace:: August 15, 2019
