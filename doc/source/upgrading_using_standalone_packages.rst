.. _upgrading_using_standalone_packages:

=====================================
Upgrading using Standalone Packages
=====================================

DEB-based distributions
-------------------------

1. Make a full backup (or dump if possible) of you database. Move the database configuration file, ``my.cnf``, to another direction to save it.
2. Stop the server with :bash:`/etc/init.d/mysql stop`.
3. Remove the installed packages with their dependencies: :bash:`apt-get autoremove percona-server percona-client`
4. Do the required modifications in the database configuration file ``my.cnf``.
5. Download the following packages for your architecture:

* ``percona-server-server``
* ``percona-server-client``
* ``percona-server-common``
* ``libperconaserverclient21``

The following example will download :ref:`8.0.13-3` release
packages for *Debian* 9.0:

.. code-block:: bash

   $ wget https://www.percona.com/downloads/Percona-Server-8.9/Percona-Server-8.0.13-3/binary/debian/stretch/x86_64/percona-server-8.0.13-3-r63dafaf-stretch-x86_64-bundle.tar

6. Unpack the bundle to get the packages: :bash:`tar xvf Percona-Server-8.0.13-3-r63dafaf-stretch-x86_64-bundle.tar`

After you unpack the bundle, you should see the following packages:

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

7. Install *Percona Server for MySQL*:

.. code-block:: bash

   $ sudo dpkg -i *.deb

This will install all the packages from the bundle. Another option is to
download/specify only the packages you need for running *Percona Server for MySQL*
installation (``libperconaserverclient21_8.0.13-3.stretch_amd64.deb``,
``percona-server-client-8.0.13-3.stretch_amd64.deb``,
``percona-server-common-8.0.13-3.stretch_amd64.deb``, and
``percona-server-server-8.0.13-3.stretch_amd64.deb``. Optionally you can
install ``percona-server-tokudb-8.0.13-3.stretch_amd64.deb`` if you want
*TokuDB* storage engine).


.. Important:: 

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.
    
   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.
   
   We recommend :ref:`migrate-myrocks`.
   
   Starting with Percona 8.0.26, **the TokuDB storage engine is no longer supported and is removed from the installation packages and not enabled in our binary builds**.

.. warning::

   When installing packages manually, you must resolve all the dependencies and install missing packages yourself. At least
   the following packages should be installed before installing *Percona Server for MySQL* 8.0: 
   * ``libmecab2``, 
   * ``libjemalloc1``, 
   * ``zlib1g-dev``, 
   * ``libaio1``.

8. Running the upgrade:
   
Starting with Percona Server 8.0.16-7, the :command:`mysql_upgrade` is deprecated. The functionality was moved to the `mysqld` binary which automatically runs the upgrade process, if needed. If you attempt to run `mysql_upgrade`, no operation happens and the following message appears: "The mysql_upgrade client is now deprecated. The actions executed by the upgrade client are now done by the server." To find more information, see `MySQL Upgrade Process Upgrades <https://dev.mysql.com/doc/refman/8.0/en/upgrading-what-is-upgraded.html>`__

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
-----------------------

1. Make a full backup (or dump if possible) of you database. Move the database configuration file, ``my.cnf``, to another direction to save it.
2. Stop the server with :bash:`/etc/init.d/mysql stop`. 
3. Check the installed packages:
   
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

You may have the ``shared-compat`` package, which is required for compatibility.

5. Remove the packages without dependencies with :bash:`rpm -qa | grep percona-server | xargs rpm -e --nodeps`.
   
It is important that you remove the packages without dependencies as many packages may
depend on these (as they replace ``mysql``) and will be removed if ommited.

Substitute :bash:`grep '^mysql-'` for :bash:`grep 'Percona-Server'` in the previous command and
remove the listed packages.

7. Download the packages of the desired series for your architecture from the
`download page <http://www.percona.com/downloads/Percona-Server-8.0/>`_. The
easiest way is to download bundle which contains all the packages. The following
example will download *Percona Server for MySQL* 8.0.13-3 release packages for *CentOS* 7:

.. code-block:: bash

   $ wget https://www.percona.com/downloads/Percona-Server-8.0/Percona-Server-8.0.13-3/binary/redhat/7/x86_64/Percona-Server-8.0.13-3-r63dafaf-el7-x86_64-bundle.tar

8. Unpack the bundle to get the packages with :bash:`tar xvf Percona-Server-8.0.13-3-r63dafaf-el7-x86_64-bundle.tar`.

After you unpack the bundle, you should see the following packages: :bash:`ls *.rpm`

.. admonition:: Output

   .. code-block:: bash

      percona-server-debuginfo-8.0.13-3.1.el7.x86_64.rpm
      percona-server-client-8.0.13-3.1.el7.x86_64.rpm
      percona-server-devel-8.0.13-3.1.el7.x86_64.rpm
      percona-server-server-8.0.13-3.1.el7.x86_64.rpm
      percona-server-shared-8.0.13-3.1.el7.x86_64.rpm
      percona-server-shared-compat-8.0.13-3.1.el7.x86_64.rpm
      percona-server-test-8.0.13-3.1.el7.x86_64.rpm
      percona-server-tokudb-8.0.13-3.1.el7.x86_64.rpm

9. Install *Percona Server for MySQL*:

.. code-block:: bash

   rpm -ivh percona-server-server_8.0.13-3.el7.x86_64.rpm \
   percona-server-client_8.0.13-3.el7.x86_64.rpm \
   percona-server-shared_8.0.13-3.el7.x86_64.rpm

This command will install only packages required to run the *Percona Server for MySQL*
8.0. Optionally you can install :ref:`TokuDB <tokudb_intro>` storage engine by
adding the ``percona-server-tokudb-8.0.13-3.el7.x86_64.rpm`` to the command
above. You can find more information on how to install and enable the *TokuDB*
storage in the :ref:`tokudb_installation` guide.

.. Important:: 

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.
    
   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   We recommend :ref:`migrate-myrocks`.

   Starting with Percona 8.0.26, **the TokuDB storage engine is no longer supported and is removed from the installation packages and not enabled in our binary builds**.

10. You can install all the packages (for debugging, testing, etc.) with :bash:`rpm -ivh *.rpm`.

.. note::

   When installing packages manually, you must
   resolve all the dependencies and install missing packages.

11. Modify your configuration file, :file:`my.cnf`, and install the plugins if necessary. If you are using *TokuDB* storage engine you must comment out all the *TokuDB* specific variables in your configuration file(s) before starting the server, otherwise server will not start. *RHEL*/*CentOS* 7 automatically backs up the previous configuration file to :file:`/etc/my.cnf.rpmsave` and installs the default :file:`my.cnf`. After upgrade/install process completes you can move the old configuration file back (after you remove all the unsupported system variables).

12. As the schema of the grant table has changed, the server must be started without reading them with :bash:`service mysql start`.

13. Running the upgrade:

Starting with Percona Server 8.0.16-7, the :command:`mysql_upgrade` is deprecated. The functionality was moved to the `mysqld` binary which automatically runs the upgrade process, if needed. If you attempt to run `mysql_upgrade`, no operation happens and the following message appears: "The mysql_upgrade client is now deprecated. The actions executed by the upgrade client are now done by the server." To find more information, see `MySQL Upgrade Process Upgrades <https://dev.mysql.com/doc/refman/8.0/en/upgrading-what-is-upgraded.html>`__ 

If you are upgrading to a *Percona Server for MySQL* version before 8.0.16-7, run
:command:`mysql_upgrade` to migrate to the new grant tables. :command:`mysql_upgrade` will
rebuild the required indexes and do the required modifications.

14. Restart the server with :bash:`service mysql restart`.

After the service has been successfully restarted you can use the new *Percona Server for MySQL* 8.0.
