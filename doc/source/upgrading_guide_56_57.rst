.. _upgrading_guide:

==========================================================
 Percona Server In-Place Upgrading Guide: From 5.6 to 5.7
==========================================================

In-place upgrades are those which are done using the existing data in the server. Generally speaking, this is stopping the server, installing the new server and starting it with the same data files. While they may not be suitable for high-complexity environments, they may be adequate for many scenarios.

The following is a summary of the more relevant changes in the 5.7 series. It's strongly recommended to that you read the following guides as they contain the list of incompatible changes that could cause automatic upgrade to fail: 

  * :ref:`changed_in_57`

  * `Upgrading MySQL <http://dev.mysql.com/doc/refman/5.7/en/upgrading.html>`_

  * `Upgrading from MySQL 5.6 to 5.7 <http://dev.mysql.com/doc/refman/5.7/en/upgrading-from-previous-series.html>`_

.. warning:: 

 Upgrade from 5.6 to 5.7 on a crashed instance is not recommended. If the server instance has crashed, crash recovery should be run before proceeding with the upgrade. 

Upgrading using the Percona repositories
========================================

The easiest and recommended way of installing - where possible - is by using the Percona repositories.

Instructions for enabling the repositories in a system can be found in:

  * :doc:`Percona APT Repository <installation/apt_repo>`

  * :doc:`Percona YUM Repository <installation/yum_repo>`

``DEB``-based distributions
---------------------------

.. note::

  Following commands will need to be run either as a root user or with :program:`sudo`.

Having done the full backup (or dump if possible), stop the server: 

.. code-block:: bash

  $ service mysql stop

and proceed to do the modifications needed in your configuration file, as explained at the beginning of this guide.

.. note:: 

  If you're running *Debian*/*Ubuntu* system with `systemd <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and service manager you can invoke the above command with :program:`systemctl` instead of :program:`service`. Currently both are supported.

Then install the new server with: 

.. code-block:: bash

  $ apt install percona-server-server-5.7

If you're using |Percona Server| 5.6 with TokuDB you'll need to specify the TokuDB package as well:

.. code-block:: bash

  $ apt install percona-server-server-5.7 percona-server-tokudb-5.7

The installation script will *NOT* run automatically :command:`mysql_upgrade` as it was the case in previous versions. You'll need to run the command manually and restart the service after it's finished.

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

Note that this procedure is the same for upgrading from MySQL 5.6 or 5.7 to |Percona Server| 5.7.

``RPM``-based distributions
---------------------------

.. note::

  Following commands will need to be run either as a root user or with :program:`sudo`.

Having done the full backup (and dump if possible), stop the server: 

.. code-block:: bash

  $ service mysql stop

.. note::

  If you're running *RHEL*/*CentOS* system with `systemd <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and service manager you can invoke the above command with :program:`systemctl` instead of :program:`service`. Currently both are supported.

and check your installed packages with: 

.. code-block:: bash

  $ rpm -qa | grep Percona-Server
  Percona-Server-shared-56-5.6.28-rel76.1.el7.x86_64
  Percona-Server-server-56-5.6.28-rel76.1.el7.x86_64
  Percona-Server-devel-56-5.6.28-rel76.1.el7.x86_64
  Percona-Server-client-56-5.6.28-rel76.1.el7.x86_64
  Percona-Server-test-56-5.6.28-rel76.1.el7.x86_64
  Percona-Server-56-debuginfo-5.6.28-rel76.1.el7.x86_64

After checking, proceed to remove them without dependencies: 

.. code-block:: bash

  $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important that you remove it without dependencies as many packages may depend on these (as they replace ``mysql``) and will be removed if omitted.

Note that this procedure is the same for upgrading from MySQL 5.6 or 5.7 to |Percona Server| 5.7: just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove them.

You will have to install the following package:

  * ``Percona-Server-server-57``

.. code-block:: bash

  $ yum install Percona-Server-server-57 

If you're using |Percona Server| 5.6 with TokuDB you'll need to specify the TokuDB package as well when doing the upgrade: 

.. code-block:: bash

  $ yum install Percona-Server-server-57 Percona-Server-tokudb-57

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and reinstall the plugins if necessary. 

.. note:: If you're using TokuDB storage engine you'll need to comment out all the TokuDB specific variables in your configuration file(s) before starting the server, otherwise server won't be able to start. *RHEL*/*CentOS* 7 automatically backs up the previous configuration file to :file:`/etc/my.cnf.rpmsave` and installs the default :file:`my.cnf`. After upgrade/install process completes you can move the old configuration file back (after you remove all the unsupported system variables).

You can now start the ``mysql`` service:

.. code-block:: bash

  $ service mysql start

and use ``mysql_upgrade`` to migrate to the new grant tables, it will rebuild the indexes needed and do the modifications needed: 

.. note:: If you're using TokuDB storage engine you'll need re-enable the storage engine plugin by running the: ``ps-admin --enable-tokudb`` before running ``mysql_upgrade`` otherwise you'll get errors.

.. code-block:: bash

  $ mysql_upgrade
  Checking if update is needed.
  Checking server version.
  Running queries to upgrade MySQL server.
  Checking system database.
  mysql.columns_priv                                 OK
  mysql.db                                           OK
  ...
  pgrade process completed successfully.
  Checking if update is needed.

Once this is done, just restart the server as usual: 

.. code-block:: bash

  $ service mysql restart

After the service has been successfully restarted you can use the new |Percona Server| 5.7.

Upgrading using Standalone Packages
===================================

DEB-based distributions
-----------------------

Having done the full backup (and dump if possible), stop the server: ::

  $ sudo /etc/init.d/mysql stop

and remove the installed packages with their dependencies: ::

  $ sudo apt autoremove percona-server-server-5.6 percona-server-client-5.6

Once removed, proceed to do the modifications needed in your configuration file, as explained at the beginning of this guide.

Then, download the following packages for your architecture:

  * ``percona-server-server-5.7``

  * ``percona-server-client-5.7``

  * ``percona-server-common-5.7``

  * ``libperconaserverclient20``

Following example will download |Percona Server| :rn:`5.7.10-3` release packages for *Debian* 8.0:

.. code-block:: bash

  $ wget https://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.10-3/binary/debian/jessie/x86_64/Percona-Server-5.7.10-3-r63dafaf-jessie-x86_64-bundle.tar

You should then unpack the bundle to get the packages:

.. code-block:: bash

  $ tar xvf Percona-Server-5.7.10-3-r63dafaf-jessie-x86_64-bundle.tar

After you unpack the bundle you should see the following packages:

.. code-block:: bash

  $ ls *.deb
  libperconaserverclient20-dev_5.7.10-3-1.jessie_amd64.deb
  libperconaserverclient20_5.7.10-3-1.jessie_amd64.deb
  percona-server-5.7-dbg_5.7.10-3-1.jessie_amd64.deb
  percona-server-client-5.7_5.7.10-3-1.jessie_amd64.deb
  percona-server-common-5.7_5.7.10-3-1.jessie_amd64.deb
  percona-server-server-5.7_5.7.10-3-1.jessie_amd64.deb
  percona-server-source-5.7_5.7.10-3-1.jessie_amd64.deb
  percona-server-test-5.7_5.7.10-3-1.jessie_amd64.deb
  percona-server-tokudb-5.7_5.7.10-3-1.jessie_amd64.deb

Now you can install |Percona Server| by running:

.. code-block:: bash

  $ sudo dpkg -i *.deb

This will install all the packages from the bundle. Another option is to download/specify only the packages you need for running |Percona Server| installation (``libperconaserverclient20_5.7.10-3-1.jessie_amd64.deb``, ``percona-server-client-5.7_5.7.10-3-1.jessie_amd64.deb``, ``percona-server-common-5.7_5.7.10-3-1.jessie_amd64.deb``, and ``percona-server-server-5.7_5.7.10-3-1.jessie_amd64.deb``. Optionally you can install ``percona-server-tokudb-5.7_5.7.10-3-1.jessie_amd64.deb`` if you want TokuDB storage engine).

.. note::

  |Percona Server| 5.7 comes with the :ref:`TokuDB storage engine <tokudb_intro>`. You can find more information on how to install and enable the TokuDB storage in the :ref:`tokudb_installation` guide.

.. warning::

  When installing packages manually like this, you'll need to make sure to resolve all the dependencies and install missing packages yourself. At least the following packages should be installed before installing |Percona Server| 5.7: ``libmecab2``, ``libjemalloc1``, ``zlib1g-dev``, and ``libaio1``.

The installation script will not run automatically :command:`mysql_upgrade`, so you'll need to run it yourself and restart the service afterwards.

RPM-based distributions
-----------------------

Having done the full backup (and dump if possible), stop the server: 

.. code-block:: bash

  $ service mysql stop

and check your installed packages:

.. code-block:: bash

  $ rpm -qa | grep Percona-Server
  
  Percona-Server-shared-56-5.6.28-rel76.1.el6.x86_64
  Percona-Server-server-56-5.6.28-rel76.1.el6.x86_64
  Percona-Server-client-56-5.6.28-rel76.1.el6.x86_64
  Percona-Server-tokudb-56-5.6.28-rel76.1.el6.x86_64

You may have a fourth, ``shared-compat``, which is for compatibility purposes.

After checked that, proceed to remove them without dependencies: ::

  $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important that you remove it without dependencies as many packages may depend on these (as they replace ``mysql``) and will be removed if ommited.

Note that this procedure is the same for upgrading from MySQL 5.6 to |Percona Server| 5.7, just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove them.

Download the packages of the desired series for your architecture from the `download page <http://www.percona.com/downloads/Percona-Server-5.7/>`_. The easiest way is to download bundle which contains all the packages. Following example will download |Percona Server| 5.7.10-3 release packages for *CentOS* 7:

.. code-block:: bash

  $ wget https://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.10-3/binary/redhat/7/x86_64/Percona-Server-5.7.10-3-r63dafaf-el7-x86_64-bundle.tar

You should then unpack the bundle to get the packages:

.. code-block:: bash

   $ tar xvf Percona-Server-5.7.10-3-r63dafaf-el7-x86_64-bundle.tar

After you unpack the bundle you should see the following packages:

.. code-block:: bash

  $ ls *.rpm
  Percona-Server-57-debuginfo-5.7.10-3.1.el7.x86_64.rpm
  Percona-Server-client-57-5.7.10-3.1.el7.x86_64.rpm
  Percona-Server-devel-57-5.7.10-3.1.el7.x86_64.rpm
  Percona-Server-server-57-5.7.10-3.1.el7.x86_64.rpm
  Percona-Server-shared-57-5.7.10-3.1.el7.x86_64.rpm
  Percona-Server-shared-compat-57-5.7.10-3.1.el7.x86_64.rpm
  Percona-Server-test-57-5.7.10-3.1.el7.x86_64.rpm
  Percona-Server-tokudb-57-5.7.10-3.1.el7.x86_64.rpm

Now you can install |Percona Server| 5.7 by running:

.. code-block:: bash

  rpm -ivh Percona-Server-server-57-5.7.10-3.1.el7.x86_64.rpm \
  Percona-Server-client-57-5.7.10-3.1.el7.x86_64.rpm \
  Percona-Server-shared-57-5.7.10-3.1.el7.x86_64.rpm

This will install only packages required to run the |Percona Server| 5.7. Optionally you can install :ref:`TokuDB <tokudb_intro>` storage engine by adding the ``Percona-Server-tokudb-57-5.7.10-3.1.el7.x86_64.rpm`` to the command above. You can find more information on how to install and enable the TokuDB storage in the :ref:`tokudb_installation` guide.

To install all the packages (for debugging, testing, etc.) you should run:

.. code-block:: bash

  $ rpm -ivh *.rpm

.. note::

  When installing packages manually like this, you'll need to make sure to resolve all the dependencies and install missing packages yourself.

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and install the plugins if necessary. If you're using TokuDB storage engine you'll need to comment out all the TokuDB specific variables in your configuration file(s) before starting the server, otherwise server won't be able to start. *RHEL*/*CentOS* 7 automatically backs up the previous configuration file to :file:`/etc/my.cnf.rpmsave` and installs the default :file:`my.cnf`. After upgrade/install process completes you can move the old configuration file back (after you remove all the unsupported system variables). 

As the schema of the grant table has changed, the server must be started without reading them: 

.. code-block:: bash

  $ service mysql start

and use :file:`mysql_upgrade` to migrate to the new grant tables, it will rebuild the indexes needed and do the modifications needed: 

.. note:: If you're using TokuDB storage engine you'll need re-enable the storage engine plugin by running the: ``ps-admin --enable-tokudb`` before running ``mysql_upgrade`` otherwise you'll get errors.

.. code-block:: bash

  $ mysql_upgrade

After this is done, just restart the server as usual: 

.. code-block:: bash

  $ service mysql restart

Performing a Distribution upgrade in-place on a System with installed Percona packages
--------------------------------------------------------------------------------------------

The recommended process for performing a distribution upgrade on a system with
the Percona packages installed is the following:

    1. Record the installed Percona packages
    2. Backup the data and configurations
    3. Uninstall the Percona packages without removing the configurations or
       data
    4. Perform the upgrade by following the distribution upgrade instructions
    5. Reboot the system
    6. Install the Percona packages intended for the upgraded version of the
       distribution
