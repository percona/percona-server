.. _upgrading_guide:

================================================================================
|Percona Server| In-Place Upgrading Guide: From 5.7 to 8.0
================================================================================

In-place upgrades are those which are done using the existing data in the
server. Generally speaking, this is stopping the server, installing the new
server and starting it with the same data files. While they may not be suitable
for high-complexity environments, they may be adequate for many scenarios.

The following is a summary of the more relevant changes in the 8.0 series. It is
strongly recommended that you read the following guides as they contain the list
of incompatible changes that could cause automatic upgrade to fail:

- :ref:`changed_in_8.0`
- `Upgrading MySQL <http://dev.mysql.com/doc/refman/8.0/en/upgrading.html>`_
- `Upgrading from MySQL 5.7 to 8.0 <http://dev.mysql.com/doc/refman/8.0/en/upgrading-from-previous-series.html>`_
- `Upgrade Paths <https://dev.mysql.com/doc/refman/8.0/en/upgrade-paths.html>`_
- `Preparing your Installation for Upgrade <https://dev.mysql.com/doc/refman/8.0/en/upgrade-prerequisites.html>`_

.. include:: ../.res/text/encrypt_binlog.removing.txt

.. warning:: 

   Do not upgrade from 5.7 to 8.0 on a crashed instance. If the server instance
   has crashed, crash recovery should be run before proceeding with the upgrade.

   Note that in |Percona Server| 8.0, the ``ROW FORMAT`` clause is not supported
   in ``CREATE TABLE`` and ``ALTER TABLE`` statements. Instead, use the
   :variable:`tokudb_row_format` variable to set the default compression
   algorithm.

   With partitioned tables that use the TokuDB or MyRocks storage
   engine, the upgrade only works with native partitioning.

--------------------------------------------------------------------------------

.. contents::
   :local:
   :depth: 1

Upgrading using the Percona repositories
================================================================================

The easiest and recommended way of installing - where possible - is by using the
|Percona| repositories.

Instructions for enabling the repositories in a system can be found in:

* :doc:`Percona APT Repository <installation/apt_repo>`
* :doc:`Percona YUM Repository <installation/yum_repo>`

DEB-based distributions
--------------------------------------------------------------------------------

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

If you only used the |MyRocks| storage engine in |Percona Server| |version.prev|, install the
``percona-server-rocksdb`` package.

.. code-block:: bash

   $ apt-get install percona-server-rocksdb


The installation script will *NOT* run automatically :command:`mysql_upgrade` as
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

.. note::

   If you're running *RHEL*/*CentOS* system with `systemd
   <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and
   service manager you can invoke the above command with :program:`systemctl`
   instead of :program:`service`. Currently both are supported.

.. admonition:: Output of :bash:`rpm -qa | grep Percona-Server`

   .. code-block:: guess

      Percona-Server-57-debuginfo-5.7.10-3.1.el7.x86_64
      Percona-Server-client-57-5.7.10-3.1.el7.x86_64
      Percona-Server-devel-57-5.7.10-3.1.el7.x86_64
      Percona-Server-server-57-5.7.10-3.1.el7.x86_64
      Percona-Server-shared-57-5.7.10-3.1.el7.x86_64
      Percona-Server-shared-compat-57-5.7.10-3.1.el7.x86_64
      Percona-Server-test-57-5.7.10-3.1.el7.x86_64
      Percona-Server-tokudb-57-5.7.10-3.1.el7.x86_64

After checking, proceed to remove them without dependencies: 

.. code-block:: bash

   $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important that you remove them without dependencies as many packages may
depend on these (as they replace ``mysql``) and will be removed if omitted.

.. important:: |etc.my-cnf| Backed Up in |centos| 7

   In |centos| 7, the |etc.my-cnf| configuration file is backed up when you
   uninstall the |Percona Server| packages with the |rpm.e.nodeps| command.

   The backup file is stored in the same directory with the `_backup` suffix
   followed by a timestamp: |etc.my-cnf-backup|.

Substitute :bash:`grep '^mysql-'` for :bash:`grep 'Percona-Server'` in the previous command and
remove the listed packages.

You will have to install the ``percona-server-server`` package:

.. code-block:: bash

   $ yum install percona-server-server

The |TokuDB| and |MyRocks| storage engines are installed separately.

If you used |TokuDB| in |Percona Server| |version.prev|, install the
``percona-server-tokudb`` package when doing the upgrade. This command installs
both

.. code-block:: bash

   $ yum install percona-server-tokudb

If you used the |MyRocks| storage engine in |Percona Server| |version.prev|, install the
``percona-server-rocksdb`` package:

.. code-block:: bash

   $ yum install percona-server-rocksdb

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and
reinstall the plugins if necessary.

.. note::

   If you are using |TokuDB| storage engine you'll need to comment out all the
   |TokuDB| specific variables in your configuration file(s) before starting the
   server, otherwise the server won't be able to start. *RHEL*/*CentOS* 7
   automatically backs up the previous configuration file to
   :file:`/etc/my.cnf.rpmsave` and installs the default :file:`my.cnf`. After
   upgrade/install process completes you can move the old configuration file
   back (after you remove all the unsupported system variables).

You can now start the ``mysql`` service using :bash:`service mysql start` and
using ``mysql_upgrade`` to migrate to the new grant tables, it will rebuild the
indexes needed and do the modifications needed:

.. note::

   If you're using |TokuDB| storage engine you'll need re-enable the storage
   engine plugin by running the: |ps-admin.enable-tokudb| before running
   ``mysql_upgrade`` otherwise you'll get errors.

.. code-block:: bash

   $ mysql_upgrade

.. admonition:: Output

   .. code-block:: guess

      Checking if update is needed.
      Checking server version.
      Running queries to upgrade MySQL server.
      Checking system database.
      mysql.columns_priv                                 OK
      mysql.db                                           OK
      ...
      pgrade process completed successfully.
      Checking if update is needed.

Once this is done, just restart the server as usual: |service.mysql.restart| 

After the service has been successfully restarted you can use the new |Percona
Server| 8.0.

Upgrading using Standalone Packages
================================================================================

DEB-based distributions
--------------------------------------------------------------------------------

Having done the full backup (and dump if possible), stop the
server. |tip.run-this.root|: :bash:`/etc/init.d/mysql stop` and remove the
installed packages with their dependencies: :bash:`apt-get autoremove percona-server percona-client`

Once removed, proceed to do the modifications needed in your configuration file,
as explained at the beginning of this guide.

Then, download the following packages for your architecture:

- ``percona-server-server``
- ``percona-server-client``
- ``percona-server-common``
- ``libperconaserverclient21``

The following example will download |Percona Server| :rn:`8.0.13-3` release
packages for *Debian* 9.0:

.. code-block:: bash

   $ wget https://www.percona.com/downloads/Percona-Server-8.9/Percona-Server-8.0.13-3/binary/debian/stretch/x86_64/percona-server-8.0.13-3-r63dafaf-stretch-x86_64-bundle.tar

You should then unpack the bundle to get the packages: :bash:`tar xvf Percona-Server-8.0.13-3-r63dafaf-stretch-x86_64-bundle.tar`

After you unpack the bundle you should see the following packages:

.. code-block:: bash

   $ ls *.deb

   libperconaserverclient21-dev_8.0.13-3-1.stretch_amd64.deb
   libperconaserverclient21_8.0.13-3-1.stretch_amd64.deb
   percona-server-dbg_8.0.13-3-1.stretch_amd64.deb
   percona-server-client_8.0.13-3-1.stretch_amd64.deb
   percona-server-common_8.0.13-3-1.stretch_amd64.deb
   percona-server-server_8.0.13-3-1.stretch_amd64.deb
   percona-server-source_8.0.13-3-1.stretch_amd64.deb
   percona-server-test_8.0.13-3-1.stretch_amd64.deb
   percona-server-tokudb_8.0.13-3-1.stretch_amd64.deb

Now you can install |Percona Server| by running:

.. code-block:: bash

   $ sudo dpkg -i *.deb

This will install all the packages from the bundle. Another option is to
download/specify only the packages you need for running |Percona Server|
installation (``libperconaserverclient21_8.0.13-3.stretch_amd64.deb``,
``percona-server-client-8.0.13-3.stretch_amd64.deb``,
``percona-server-common-8.0.13-3.stretch_amd64.deb``, and
``percona-server-server-8.0.13-3.stretch_amd64.deb``. Optionally you can
install ``percona-server-tokudb-8.0.13-3.stretch_amd64.deb`` if you want
|TokuDB| storage engine).

.. note::

   |Percona Server| 8.0 comes with the :ref:`TokuDB storage engine
   <tokudb_intro>`. You can find more information on how to install and enable
   the |TokuDB| storage in the :ref:`tokudb_installation` guide.
   
.. warning::

   When installing packages manually like this, you'll need to make sure to
   resolve all the dependencies and install missing packages yourself. At least
   the following packages should be installed before installing |Percona Server|
   8.0: ``libmecab2``, ``libjemalloc1``, ``zlib1g-dev``, and ``libaio1``.

The installation script will not run automatically :command:`mysql_upgrade`, so
you'll need to run it yourself and restart the service afterwards.

RPM-based distributions
-----------------------

Having done the full backup (and dump if possible), stop the server (command:
:bash:`service mysql stop`) and check your installed packages:

.. code-block:: bash

   $ rpm -qa | grep Percona-Server
   
   Percona-Server-57-debuginfo-5.7.10-3.1.el7.x86_64
   Percona-Server-client-57-5.7.10-3.1.el7.x86_64
   Percona-Server-devel-57-5.7.10-3.1.el7.x86_64
   Percona-Server-server-57-5.7.10-3.1.el7.x86_64
   Percona-Server-shared-57-5.7.10-3.1.el7.x86_64
   Percona-Server-shared-compat-57-5.7.10-3.1.el7.x86_64
   Percona-Server-test-57-5.7.10-3.1.el7.x86_64
   Percona-Server-tokudb-57-5.7.10-3.1.el7.x86_64

You may have the ``shared-compat`` package, which is for compatibility purposes.

After checked that, proceed to remove them without dependencies: :bash:`rpm -qa | grep percona-server | xargs rpm -e --nodeps`

It is important that you remove it without dependencies as many packages may
depend on these (as they replace ``mysql``) and will be removed if ommited.

Note that this procedure is the same for upgrading from |MySQL| 5.7 to |Percona
Server| 8.0, just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove
them.

Download the packages of the desired series for your architecture from the
`download page <http://www.percona.com/downloads/Percona-Server-8.0/>`_. The
easiest way is to download bundle which contains all the packages. The following
example will download |Percona Server| 8.0.13-3 release packages for *CentOS* 7:

.. code-block:: bash

   $ wget https://www.percona.com/downloads/Percona-Server-8.0/Percona-Server-8.0.13-3/binary/redhat/7/x86_64/Percona-Server-8.0.13-3-r63dafaf-el7-x86_64-bundle.tar

You should then unpack the bundle to get the packages: :bash:`tar xvf Percona-Server-8.0.13-3-r63dafaf-el7-x86_64-bundle.tar`

After you unpack the bundle you should see the following packages: :bash:`ls *.rpm`

.. admonition:: Output

   .. code-block:: guess

      percona-server-debuginfo-8.0.13-3.1.el7.x86_64.rpm
      percona-server-client-8.0.13-3.1.el7.x86_64.rpm
      percona-server-devel-8.0.13-3.1.el7.x86_64.rpm
      percona-server-server-8.0.13-3.1.el7.x86_64.rpm
      percona-server-shared-8.0.13-3.1.el7.x86_64.rpm
      percona-server-shared-compat-8.0.13-3.1.el7.x86_64.rpm
      percona-server-test-8.0.13-3.1.el7.x86_64.rpm
      percona-server-tokudb-8.0.13-3.1.el7.x86_64.rpm


Now, you can install |Percona Server| 8.0 by running:

.. code-block:: bash

   rpm -ivh percona-server-server_8.0.13-3.el7.x86_64.rpm \
   percona-server-client_8.0.13-3.el7.x86_64.rpm \
   percona-server-shared_8.0.13-3.el7.x86_64.rpm

This will install only packages required to run the |Percona Server|
8.0. Optionally you can install :ref:`TokuDB <tokudb_intro>` storage engine by
adding the ``percona-server-tokudb-8.0.13-3.el7.x86_64.rpm`` to the command
above. You can find more information on how to install and enable the |TokuDB|
storage in the :ref:`tokudb_installation` guide.

To install all the packages (for debugging, testing, etc.) you should run:
:bash:`rpm -ivh *.rpm`

.. note::

   When installing packages manually like this, you'll need to make sure to
   resolve all the dependencies and install missing packages yourself.

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and
install the plugins if necessary. If you're using |TokuDB| storage engine you'll
need to comment out all the |TokuDB| specific variables in your configuration
file(s) before starting the server, otherwise server won't be able to
start. *RHEL*/*CentOS* 7 automatically backs up the previous configuration file
to :file:`/etc/my.cnf.rpmsave` and installs the default :file:`my.cnf`. After
upgrade/install process completes you can move the old configuration file back
(after you remove all the unsupported system variables).

As the schema of the grant table has changed, the server must be started without
reading them: :bash:`service mysql start`

Then, use :file:`mysql_upgrade` to migrate to the new grant tables. It will
rebuild the indexes needed and do the modifications needed:
:bash:`mysql_upgrade`

.. note::

   If you're using |TokuDB| storage engine you'll need re-enable the storage
   engine plugin by running the: |ps-admin.enable-tokudb| before running
   ``mysql_upgrade`` otherwise you'll get errors.

After this is done, just restart the server as usual: |service.mysql.restart|

Upgrading from Systems that Use the |TokuDB| or |MyRocks| Storage Engine and Partitioned Tables
====================================================================================================

Due to the limitation imposed by |MySQL|, it is the storage engine that must
provide support for partitioning.  |MySQL| 8.0 only provides support for
partitioned table for the |InnoDB| storage engine.

If you use partitioned tables with the |TokuDB| or |MyRocks| storage engine, the
upgrade may fail if the native partitioning (i.e. provided by the storage engine
itself) is not enabled.

Before you attempt the upgrade, check whether or not you have any tables that do not use the native partitioning.

.. code-block:: bash

   $ mysqlcheck -u root --all-databases --check-upgrade

If such tables are found |mysqlcheck| issues a warning:

.. admonition:: Output of |mysqlcheck| detecting a table that do not use the native partitioning

   .. code-block:: guess

      | comp_test.t1_RocksDB_lz4     OK
      | warning  : The partition engine, used by table 'comp_test.t1_RocksDB_lz4',
      | is deprecated and will be removed in a future release. Please use native partitioning instead.

In this case ``comp_test.t1_RocksDB_lz4`` is not using native partitions. To
switch, enable either :variable:`rocksdb_enable_native_partition` or
:variable:`tokudb_enable_native_partition` variable depending on the storage
engine that you are using. Then restart the server. Your next step is to alter
the tables that are not using the native partitioning with the
|sql.upgrade-partitioning| clause:

.. code-block:: mysql

   ALTER TABLE comp_test.t1_RocksDB_lz4 UPGRADE PARTITIONING

In this example, the table ``comp_test.t1_RocksDB_lz4`` to native
partitioning. Unless you complete these steps for each table that |mysqlcheck|
complained about, the upgrade to |MySQL| |version| will fail and your error log
will contain messages like:

.. code-block:: guess

   2018-12-17T18:34:14.152660Z 2 [ERROR] [MY-013140] [Server] The 'partitioning' feature is not available; you need to remove '--skip-partition' or use MySQL built with '-DWITH_PARTITION_STORAGE_ENGINE=1'
   2018-12-17T18:34:14.152679Z 2 [ERROR] [MY-013140] [Server] Can't find file: './comp_test/t1_RocksDB_lz4.frm' (errno: 0 - Success)
   2018-12-17T18:34:14.152691Z 2 [ERROR] [MY-013137] [Server] Can't find file: './comp_test/t1_RocksDB_lz4.frm' (OS errno: 0 - Success)

.. seealso::

   |MySQL| Documentation: Partitioning Limitations Relating to Storage Engines
      https://dev.mysql.com/doc/refman/8.0/en/partitioning-limitations-storage-engines.html

.. include:: .res/replace.txt
.. include:: .res/replace.program.txt
.. include:: .res/replace.concept.txt
.. include:: .res/replace.file.txt
