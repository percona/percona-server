.. rn:: 5.7.27-30

================================================================================
|Percona Server| 5.7.27-30
================================================================================

Percona is glad to announce the release of |Percona Server| 5.7.27-30 on August 22, 2019. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.27-30/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.27 <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-27.html>`_ and includes all the bug fixes in it. |Percona Server| 5.7.27-30 is now the current GA
(Generally Available) release in the 5.7 series.

All software developed by Percona is open-source and free.

.. note::

   If you're currently using |Percona Server| 5.7, Percona recommends upgrading to this version of 5.7 prior to upgrading to |Percona Server| 8.0.

Bugs Fixed
================================================================================

- Parallel doublewrite buffer writes must crash the server on an I/O error occurs. Bug fixed :psbug:`5678`.

- On a server with two million or more tables using foreign keys and AUTOINC columns, the shutdown may take a measurable length of time. Bug fixed :psbug:`5639`. (Upstream :mysqlbug:`95895`)

- If large pages are enabled on the MySQL side, the maximum size for :variable:`innodb_buffer_pool_chunk_size` is effectively limited to 4GB. Bug fixed :psbug:`5517`. (Upstream :mysqlbug:`94747`)

- The TokuDB hot backup library continually dumps TRACE information to the Server error log. The user cannot enable or disable the dump of this information. Bug fixed :psbug:`4850`.

- The TokuDBBackupPlugin is optional at cmake time. Bug fixed :psbug:`5748`.

- A multi-table ``DELETE`` with a foreign key breaks replication. Bug fixed :psbug:`3845`.


- A ``TRUNCATE`` with any table and interfacing with Adaptive Hash Index (AHI) can cause server stalls due to the interaction with AHI, whether the AHI is enabled or not. Bug fixed :psbug:`5576`. (Upstream :mysqlbug:`94610`)


- In specific configurations and with :variable:`log_slow_verbosity` set to log InnoDB statistics, memory usage increases while running a stored procedure.  Bug fixed :psbug:`5581`.

- :ref:`threadpool` functionality to track network I/O was disabled.  Bug fixed :psbug:`5723`.

- When Adaptive Hash Index (AHI) is enabled or disabled, there is an AHI overhead during DDL operations. Bug fixed :psbug:`5747`.

- An instance started with the default values but setting the `redo-log` to encrypt without specifying the keyring plugin parameters does not fail or throw an error. Bug fixed :psbug:`5476`.

- Setting the encryption to ``ON`` for the system tablespace generates the encryption key and encrypts system temporary tablespace pages. Resetting encryption to ``OFF`` , all subsequent pages are written to the temporary tablespace without encryption. To allow any encrypted tables to be decrypted, the generated keys are not erased. Modifying the :variable:`innodb_temp_tablespace_encrypt` does not affect file-per-table temporary tables. This type of table is encrypted if ``ENCRYPTION`` ='Y' is set during the table creation. Bug fixed :psbug:`5736`.

- After resetting the :variable:`innodb_temp_tablespace_encrypt` to ``OFF`` during runtime, the subsequent file-per-table temporary tables continue to be encrypted. Bug fixed :psbug:`5734`.

Other bugs fixed:
:psbug:`5752`,
:psbug:`5749`,
:psbug:`5746`,
:psbug:`5744`,
:psbug:`5743`,
:psbug:`5742`,
:psbug:`5740`,
:psbug:`5695`,
:psbug:`5681`,
:psbug:`5669`,
:psbug:`5645`,
:psbug:`5638`,
:psbug:`5593`,
:psbug:`5532`,
:psbug:`5790`,
:psbug:`5812`,
:psbug:`3970`,
:psbug:`5696`,
:psbug:`5689`,
:psbug:`5146`,
:psbug:`5715`,
:psbug:`5791`,
:psbug:`5662`,
:psbug:`5420`,
:psbug:`5149`,
:psbug:`5686`,
:psbug:`5688`,
:psbug:`5697`,
:psbug:`5716`,
:psbug:`5725`,
:psbug:`5773`,
:psbug:`5775`,
:psbug:`5820`, and
:psbug:`5839`.

.. August 22, 2019 replace:: August 22, 2019
.. 5.7.27-30 replace:: 5.7.27-30
