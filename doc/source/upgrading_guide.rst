.. _upgrading_guide:

======================================================================
*Percona Server for MySQL* In-Place Upgrading Guide: From 5.7 to 8.0
======================================================================

An in-place upgrade is performed by using existing data on the server and involves the following actions:

* Stopping the MySQL 5.7 server
* Replacing the old binaries with MySQL 8.0 binaries
* Starting the MySQL 8.0 server with the same data files.

While an in-place upgrade may not be suitable for all environments, especially those environments with many variables to consider, the upgrade should work in most cases.

The following list summarizes a number of the changes in the 8.0 series and has useful guides that can help you perform a smooth upgrade. We strongly recommend reading this information:

.. warning::

   Do not upgrade from 5.7 to 8.0 on a crashed instance. If the server instance
   has crashed, crash recovery should be run before proceeding with the upgrade.

   Note that in |Percona Server| 8.0, the ``ROW FORMAT`` clause is not supported
   in ``CREATE TABLE`` and ``ALTER TABLE`` statements. Instead, use the
   :variable:`tokudb_row_format` variable to set the default compression
   algorithm.

   With partitioned tables that use the TokuDB or MyRocks storage
   engine, the upgrade only works with native partitioning.

   As of **Percona Server8.0.28-19** the `TokuDB` storage engine has been removed. Before you upgrade to this version or later, verify if your database has any TokuDB tables. If you do, then convert them to another storage engine before the upgrade. If you continue the upgrade with TokuDB tables, the data in those tables is lost. For more information, see :ref:`TokuDB <tokudb_intro>`.

-------------------------------------------------------------------------------

.. contents::
   :local:
   :depth: 1

Upgrading using the Percona repositories
===============================================================================

The easiest and recommended way of installing - where possible - is by using the
|Percona| repositories.

Instructions for enabling the repositories in a system can be found in:

* :doc:`Percona APT Repository <installation/apt_repo>`
* :doc:`Percona YUM Repository <installation/yum_repo>`

DEB-based distributions
-------------------------------------------------------------------------------

|tip.run-all.root|

Having done the full backup (or dump if possible), stop the server running
and proceed to do the modifications needed in your
configuration file, as explained at the beginning of this guide.

.. note::

   If you are running *Debian*/*Ubuntu* system with `systemd
   <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and
   service manager you can invoke the above command with :program:`systemctl`
   instead of :program:`service`. Currently both are supported.

Then install the new server with:

Enable the repository:

.. code-block:: bash

   $ percona-release enable ps-80 release
   $ apt-get update

.. code-block:: bash

   $ apt-get install percona-server-server

If you used or |TokuDB| or |MyRocks| storage engines

The |TokuDB| and |MyRocks| storage engines are installed separately. The ``percona-server-tokudb`` package installs both of them.

.. code-block:: bash

   $ apt-get install percona-server-tokudb

If you only used the |MyRocks| storage engine in |Percona Server| |version.prev|, install the ``percona-server-rocksdb`` package.

.. code-block:: bash

   $ apt-get install percona-server-rocksdb

.. deprecated:: 8.0.16-7

The :command:`mysql_upgrade` was deprecated as of |Percona Server|
8.0.16-7. The functionality was moved to the `mysqld` binary which automatically
runs the upgrade process, if needed. If you attempt to run `mysql_upgrade`,
no operation happens and a message stating, "The mysql_upgrade client is now
deprecated. The actions executed by the upgrade client are now done by the
server."

.. seealso::

    `MySQL Upgrade Process Upgrades
    <https://dev.mysql.com/doc/refman/8.0/en/upgrading-what-is-upgraded.html>`__

If you are upgrading to a |Percona Server| version before 8.0.16-7, the
installation script will *NOT* run automatically :command:`mysql_upgrade` as
it was the case in previous versions. You'll need to run the command manually
and restart the service after it's finished.

.. code-block:: bash

   $ mysql_upgrade

   Checking if update is needed.
   Checking server version.
   Running queries to upgrade MySQL server.
   Checking system database.
   mysql.columns_priv                                 OK
   mysql.db                                           OK
   mysql.engine_cost                                  OK
   ...
   Upgrade process completed successfully.
   Checking if update is needed.

   $ service mysql restart

RPM-based distributions
---------------------------

|tip.run-all.root|

Having done the full backup (and dump if possible), stop the server:
:bash:`service mysql stop` and check your installed packages with :bash:`rpm -qa | grep Percona-Server`
* `Upgrading MySQL <http://dev.mysql.com/doc/refman/8.0/en/upgrading.html>`_
* `Before You Begin <https://dev.mysql.com/doc/refman/8.0/en/upgrade-before-you-begin.html>`_
* `Upgrade Paths <https://dev.mysql.com/doc/refman/8.0/en/upgrade-paths.html>`_
* `Changes in MySQL 8.0 <https://dev.mysql.com/doc/refman/8.0/en/upgrading-from-previous-series.html>`_ 
* `Preparing your Installation for Upgrade <https://dev.mysql.com/doc/refman/8.0/en/upgrade-prerequisites.html>`_
* `MySQL 8 Minor Version Upgrades Are ONE-WAY Only <https://www.percona.com/blog/2020/01/10/mysql-8-minor-version-upgrades-are-one-way-only/>`_
* `Percona Utilities That Make Major MySQL Version Upgrades Easier <https://www.percona.com/blog/percona-utilities-that-make-major-mysql-version-upgrades-easier/>`_
* `release-notes_index`   
* `Upgrade Troubleshooting <https://dev.mysql.com/doc/refman/8.0/en/upgrade-troubleshooting.html>`_
* `Rebuilding or Repairing Tables or Indexes <https://dev.mysql.com/doc/refman/8.0/en/rebuilding-tables.html>`_

.. note::

   Review other `Percona blogs <https://www.percona.com/blog/>`__ that contain upgrade information.

Implemented in release :rn:`8.0.15-5`, *Percona Server for MySQL* uses the upstream
implementation of binary log file encryption and relay log file encryption.

The :variable:`encrypt-binlog` variable is
removed, and the related command-line option `--encrypt-binlog` is not
supported. It is important to remove the `encrypt-binlog` variable from your
configuration file before you attempt to upgrade either from another release
in the *Percona Server for MySQL* 8.0 series or from *Percona Server for MySQL* 5.7.
Otherwise, a server boot error is generated, and reports an unknown
variable.

The implemented binary log file encryption is compatible with the older
format. The encrypted binary log file used in a previous version of MySQL 8.0
series or Percona Server for MySQL series is supported.

.. seealso::

   *MySQL* Documentation
      - `Encrypting Binary Log Files and Relay Log Files
        <https://dev.mysql.com/doc/refman/8.0/en/replication-binlog-encryption.html>`_
      - `binlog_encryption variable
        <https://dev.mysql.com/doc/refman/8.0/en/replication-options-binary-log.html#sysvar_binlog_encryption>`_

Before you start the upgrade process, it is recommended to make a full backup of your database. 
Copy the database configuration file, for example, ``my.cnf``, to another directory to save it.

.. warning::

   Do not upgrade from 5.7 to 8.0 on a crashed instance. If the server instance
   has crashed, run the crash recovery before proceeding with the upgrade.

You can select one of the following ways to upgrade *Percona Server for MySQL* from 5.7 to 8.0:

* `upgrading_using_percona_repos`
* `upgrading_tokudb_myrocks`
* `upgrading_using_standalone_packages`

