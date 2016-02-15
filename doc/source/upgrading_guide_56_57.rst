.. _upgrading_guide:

==========================================================
 Percona Server In-Place Upgrading Guide: From 5.6 to 5.7
==========================================================

In-place upgrades are those which are done using the existing data in the server. Generally speaking, this is stopping the server, installing the new server and starting it with the same data files. While they may not be suitable for high-complexity environments, they may be adequate for many scenarios.

The following is a summary of the more relevant changes in the 5.7 series. For more details, see

  * :ref:`Percona Server documentation <dochome>`

  * :ref:`changed_in_57`

  * `Upgrading MySQL <http://dev.mysql.com/doc/refman/5.7/en/upgrading.html>`_

  * `Upgrading from MySQL 5.6 to 5.7 <http://dev.mysql.com/doc/refman/5.7/en/upgrading-from-previous-series.html>`_

.. warning:: 

 Upgrade from 5.6 to 5.7 on a crashed instance is not recommended. If the server instance has crashed, crash recovery should be run before proceeding with the upgrade. 

Upgrading using the Percona repositories
========================================

The easiest and recommended way of installing - where possible - is by using the |Percona| repositories.

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

  $ apt-get install percona-server-server-5.7

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

Note that this procedure is the same for upgrading from |MySQL| 5.6 or 5.7 to |Percona Server| 5.7.

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

Note that this procedure is the same for upgrading from |MySQL| 5.6 or 5.7 to |Percona Server| 5.7: just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove them.

You will have to install the following packages:

  * ``Percona-Server-server-57``

  * ``Percona-Server-client-57``

.. code-block:: bash

  $ yum install Percona-Server-server-57 Percona-Server-client-57

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and reinstall the plugins if necessary, as explained at the beginning of this guide.

You can now start the ``mysql`` service:

.. code-block:: bash

  $ service mysql start

and use ``mysql_upgrade`` to migrate to the new grant tables, it will rebuild the indexes needed and do the modifications needed: 

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

  $ sudo /etc/init.d/mysqld stop

and remove the the installed packages with their dependencies: ::

  $ sudo apt-get autoremove percona-server-server-56 percona-server-client-56

Once removed, proceed to do the modifications needed in your configuration file, as explained at the beginning of this guide.

Then, download the following packages for your architecture:

  * ``percona-server-server-5.7``

  * ``percona-server-client-5.7``

  * ``percona-server-common-5.7``

  * ``libperconaserverclient20``

At the moment of writing this guide, for *Debian* 8.0 (*jessie*) on ``x86_64``, a way of doing this is: ::

  $ wget -r -l 1 -nd -A deb -R "*dev*" \
  http://www.percona.com/downloads/Percona-Server-5.7/LATEST/binary/debian/jessie/x86_64/

Install them in one command: ::

  $ sudo dpkg -i *.deb

The installation won't succeed as there will be missing dependencies. To handle this, use: ::

  $ apt-get -f install

and all dependencies will be handled by :command:`apt`.

The installation script will not run automatically :command:`mysql_upgrade`, so you'll need to run it yourself and restart the service afterwards.

RPM-based distributions
-----------------------

Having done the full backup (and dump if possible), stop the server: ::

  $ /sbin/service mysql stop

and check your installed packages: ::

  $ rpm -qa | grep Percona-Server
  
  Percona-Server-client-55-5.5.29-rel29.4.401.rhel6.x86_64.rpm
  Percona-Server-server-55-5.5.29-rel29.4.401.rhel6.x86_64.rpm
  Percona-Server-shared-55-5.5.29-rel29.4.401.rhel6.x86_64.rpm


You may have a forth, ``shared-compat``, which is for compatibility purposes.

After checked that, proceed to remove them without dependencies: ::

  $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important that you remove it without dependencies as many packages may depend on these (as they replace ``mysql``) and will be removed if ommited.

Note that this procedure is the same for upgrading from |MySQL| 5.6 to |Percona Server| 5.7, just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove them.

Download the following packages for your architecture:

  * ``Percona-Server-server-57``

  * ``Percona-Server-client-57``

  * ``Percona-Server-shared-57``

At the moment of writing this guide, a way of doing this is: ::

  $ wget -r -l 1 -nd -A rpm -R "*devel*,*debuginfo*" \
  http://www.percona.com/downloads/Percona-Server-5.7/LATEST/binary/redhat/6/x86_64/

Install them in one command: ::

  $ rpm -ivh Percona-Server-shared-57-5.7.10-3.1.el6.x86_64.rpm \
  Percona-Server-client-57-5.7.10-3.1.el6.x86_64.rpm \
  Percona-Server-server-57-5.7.10-3.1.el6.x86_64.rpm

If you don't install all "at the same time", you will need to do it in a specific order - ``shared``, ``client``, ``server``: ::

  $ rpm -ivh Percona-Server-shared-57-5.7.10-3.1.el6.x86_64.rpm
  $ rpm -ivh Percona-Server-client-57-5.7.10-3.1.el6.x86_64.rpm
  $ rpm -ivh Percona-Server-server-57-5.7.10-3.1.el6.x86_64.rpm

Otherwise, the dependencies won't be met and the installation will fail.

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and recompile the plugins if necessary, as explained at the beginning of this guide.

As the schema of the grant table has changed, the server must be started without reading them: ::

  $ /usr/sbin/mysqld --skip-grant-tables --user=mysql &

and use :file:`mysql_upgrade` to migrate to the new grant tables, it will rebuild the indexes needed and do the modifications needed: ::

  $ mysql_upgrade

After this is done, just restart the server as usual: ::

  $ /sbin/service mysql restart

If it can't find the pid file, kill the server and start it normally: ::

  $ killall /usr/sbin/mysqld
  $ /sbin/service mysql start
