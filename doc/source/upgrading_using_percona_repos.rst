.. _upgrading_using_percona_repos:

==========================================
Upgrading using the Percona repositories
==========================================

Upgrading using the Percona repositories is the easiest and recommended way.

Find the instructions on how to enable the repositories in the following documents:

* :doc:`Percona APT Repository <installation/apt_repo>`
* :doc:`Percona YUM Repository <installation/yum_repo>`

DEB-based distributions
-------------------------

Run the following commands as root or by using the :program:`sudo` command.

1. Make a full backup (or dump if possible) of you database. Move the database configuration file, ``my.cnf``, to another direction to save it.
2. Stop the server with :bash:`/etc/init.d/mysql stop`.

.. note::

   If you are running *Debian*/*Ubuntu* system with `systemd
   <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and
   service manager, you can invoke the above command with :program:`systemctl`
   instead of :program:`service`. Currently both are supported.
   
3. Do the required modifications in the database configuration file ``my.cnf``.

4. Install *Percona Server for MySQL*: 

.. code-block:: bash

   $ sudo dpkg -i *.deb

5. Enable the repository:

.. code-block:: bash

   $ percona-release enable ps-80 release
   $ apt-get update

6. Install the server package:

.. code-block:: bash

   $ apt-get install percona-server-server

7. Install the storage engin packages. 

*TokuDB* is deprecated. For more information, see :ref:`tokudb_intro`. If you used *TokuDB* storage engine in *Percona Server for MySQL* 5.7, install the ``percona-server-tokudb`` package:

.. code-block:: bash

   $ apt install percona-server-tokudb

If you used the *MyRocks* storage engine in *Percona Server for MySQL* 5.7, install the ``percona-server-rocksdb`` package:

.. code-block:: bash

   $ apt install percona-server-rocksdb

8. Running the upgrade:

Starting with *Percona Server for MySQL* 8.0.16-7, the :command:`mysql_upgrade` is deprecated. The functionality was moved to the `mysqld` binary which automatically runs the upgrade process, if needed. If you attempt to run `mysql_upgrade`, no operation happens and the following message appears: "The mysql_upgrade client is now deprecated. The actions executed by the upgrade client are now done by the server." To find more information, see `MySQL Upgrade Process Upgrades <https://dev.mysql.com/doc/refman/8.0/en/upgrading-what-is-upgraded.html>`__
 
If you are upgrading to a *Percona Server for MySQL* version before 8.0.16-7, the installation script will *NOT* run automatically :command:`mysql_upgrade`. You must run the :command:`mysql_upgrade` manually.

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

 9. Restart the service with :bash:`service mysql restart`.
     
After the service has been successfully restarted you can use the new *Percona Server for MySQL* 8.0.

RPM-based distributions
---------------------------

Run the following commands as root or by using the :program:`sudo` command.

1. Make a full backup (or dump if possible) of you database. Copy the database configuration file, for example, ``my.cnf``, to another directory to save it.
2. Stop the server with :bash:`/etc/init.d/mysql stop`.
   
.. note::

   If you are running *RHEL*/*CentOS* system with `systemd
   <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and
   service manager you can invoke the above command with :program:`systemctl`
   instead of :program:`service`. Currently both are supported.

4. Check your installed packages with :bash:`rpm -qa | grep Percona-Server`.

.. admonition:: Output of :bash:`rpm -qa | grep Percona-Server`

   .. code-block:: bash

      Percona-Server-57-debuginfo-5.7.10-3.1.el7.x86_64
      Percona-Server-client-57-5.7.10-3.1.el7.x86_64
      Percona-Server-devel-57-5.7.10-3.1.el7.x86_64
      Percona-Server-server-57-5.7.10-3.1.el7.x86_64
      Percona-Server-shared-57-5.7.10-3.1.el7.x86_64
      Percona-Server-shared-compat-57-5.7.10-3.1.el7.x86_64
      Percona-Server-test-57-5.7.10-3.1.el7.x86_64
      Percona-Server-tokudb-57-5.7.10-3.1.el7.x86_64

5. Remove the packages without dependencies. This command only removes the specified packages and leaves any dependent packages. The command does not prompt for confirmation:

.. code-block:: bash

   $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important to remove the packages without dependencies as many packages may
depend on these (as they replace ``mysql``) and will be removed if omitted.

Substitute :bash:`grep '^mysql-'` for :bash:`grep 'Percona-Server'` in the previous command and
remove the listed packages.

.. important::

   In CentOS 7, the :file:`/etc/my.cnf` configuration file is backed up when you
   uninstall the *Percona Server for MySQL* packages with the :bash:`rpm -e --nodeps` command.

   The backup file is stored in the same directory with the `_backup` suffix
   followed by a timestamp: :file:`etc/my.cnf_backup-20181201-1802`.

6. Install the ``percona-server-server`` package:

.. code-block:: bash

   $ yum install percona-server-server

7. Install the storage engine packages. 

*TokuDB* is deprecated. For more information, see :ref:`tokudb_intro`. If you used *TokuDB* storage engine in *Percona Server for MySQL* 5.7, install the ``percona-server-tokudb`` package:

.. code-block:: bash

   $ yum install percona-server-tokudb
 
If you used the *MyRocks* storage engine in *Percona Server for MySQL* 5.7, install the ``percona-server-rocksdb`` package:

.. code-block:: bash

   $ apt-get install percona-server-rocksdb

8. Modify your configuration file, :file:`my.cnf`, and reinstall the plugins if necessary.

.. note::

   If you are using *TokuDB* storage engine you need to comment out all the
   *TokuDB* specific variables in your configuration file(s) before starting the
   server, otherwise the server is not able to start. *RHEL*/*CentOS* 7
   automatically backs up the previous configuration file to
   :file:`/etc/my.cnf.rpmsave` and installs the default :file:`my.cnf`. After
   upgrade/install process completes you can move the old configuration file
   back (after you remove all the unsupported system variables).

9. Running the upgrade

Starting with Percona Server 8.0.16-7, the :command:`mysql_upgrade` is deprecated. The functionality was moved to the `mysqld` binary which automatically runs the upgrade process, if needed. If you attempt to run `mysql_upgrade`, no operation happens and the following message appears: "The mysql_upgrade client is now deprecated. The actions executed by the upgrade client are now done by the server." To find more information, see `MySQL Upgrade Process Upgrades <https://dev.mysql.com/doc/refman/8.0/en/upgrading-what-is-upgraded.html>`__

If you are upgrading to a *Percona Server for MySQL* version before 8.0.16-7, you can start the mysql service using :command:`service mysql start`. Use :command:`mysql_upgrade` to migrate to the new grant tables. The :command:`mysql_upgrade` rebuilds the required indexes and does the required modifications:

.. code-block:: bash

   $ mysql_upgrade

.. admonition:: Output

   .. code-block:: bash

      Checking if update is needed.
      Checking server version.
      Running queries to upgrade MySQL server.
      Checking system database.
      mysql.columns_priv                                 OK
      mysql.db                                           OK
      ...
      pgrade process completed successfully.
      Checking if update is needed.

10. Restart the service with :bash:`service mysql restart`.
     
After the service has been successfully restarted you can use the new *Percona Server for MySQL* 8.0.