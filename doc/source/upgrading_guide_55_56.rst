.. _upgrading_guide:

==========================================================
 Percona Server In-Place Upgrading Guide: From 5.5 to 5.6
==========================================================

In-place upgrades are those which are done using the existing data in the server. Generally speaking, this is stopping the server, installing the new server and starting it with the same data files. While they may not be suitable for high-complexity environments, they may be adequate for many scenarios.

The following is a summary of the more relevant changes in the 5.6 series. For more details, see

  * :ref:`Percona Server documentation <dochome>`

  * :ref:`changed_in_56`

  * `Upgrading from MySQL 5.5 to 5.6 <http://dev.mysql.com/doc/refman/5.6/en/upgrading-from-previous-series.html>`_

.. warning:: 

 Upgrade from 5.5 to 5.6 on a crashed instance is not recommended. If the server instance has crashed, crash recovery should be run before proceeding with the upgrade. 

.. note::

 Upgrading the from older |Percona Server| version that doesn't have default (16k) |InnoDB| page size is not recommended. This could happen if the variable `innodb_page_size <http://www.percona.com/doc/percona-server/5.5/flexibility/innodb_files_extend.html>`_ was set to non-default value.

Upgrading using the Percona repositories
========================================

The easiest and recommended way of installing - where possible - is by using the |Percona| repositories.

Instructions for enabling the repositories in a system can be found in:

  * :doc:`Percona APT Repository <installation/apt_repo>`

  * :doc:`Percona YUM Repository <installation/yum_repo>`

``DEB``-based distributions
---------------------------

Having done the full backup (or dump if possible), stop the server: ::

  $ sudo /etc/init.d/mysqld stop

and proceed to do the modifications needed in your configuration file, as explained at the beginning of this guide.

Then install the new server with: ::

  $ sudo apt-get install percona-server-server-5.6

The installation script will run automatically :command:`mysql_upgrade` to migrate to the new grant tables, rebuild the indexes where needed and then start the server.

Note that this procedure is the same for upgrading from |MySQL| 5.5 or 5.6 to |Percona Server| 5.6.

``RPM``-based distributions
---------------------------

Having done the full backup (and dump if possible), stop the server: ::

  $ /sbin/service mysql stop

and check your installed packages with: ::

  $ rpm -qa | grep Percona-Server
  Percona-Server-client-55-5.5.29-rel29.4.401.rhel6.x86_64.rpm
  Percona-Server-server-55-5.5.29-rel29.4.401.rhel6.x86_64.rpm
  Percona-Server-shared-55-5.5.29-rel29.4.401.rhel6.x86_64.rpm

You may have a forth, ``shared-compat``, which is for compatibility purposes.

After checking, proceed to remove them without dependencies: ::

  $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important that you remove it without dependencies as many packages may depend on these (as they replace ``mysql``) and will be removed if omitted.

Note that this procedure is the same for upgrading from |MySQL| 5.5 or 5.6 to |Percona Server| 5.6: just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove them.

You will have to install the following packages:

  * ``Percona-Server-server-56``

  * ``Percona-Server-client-56``

::

  $ yum install Percona-Server-server-56 Percona-Server-client-56

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and recompile the plugins if necessary, as explained at the beginning of this guide.

As the schema of the grant table has changed, the server must be started without reading them: ::

  $  /usr/sbin/mysqld --skip-grant-tables --user=mysql &

and use ``mysql_upgrade`` to migrate to the new grant tables, it will rebuild the indexes needed and do the modifications needed: ::

  $ mysql_upgrade
  ...
  OK

Once this is done, just restart the server as usual: ::

  $ /sbin/service mysql restart

If it can't find the PID file, kill the server and start it normally: ::

  $ killall /usr/sbin/mysqld
  $ /sbin/service mysql start

Upgrading using Standalone Packages
===================================

DEB-based distributions
-----------------------

Having done the full backup (and dump if possible), stop the server: ::

  $ sudo /etc/init.d/mysqld stop

and remove the the installed packages with their dependencies: ::

  $ sudo apt-get autoremove percona-server-server-55 percona-server-client-55

Once removed, proceed to do the modifications needed in your configuration file, as explained at the beginning of this guide.

Then, download the following packages for your architecture:

  * ``percona-server-server-5.6``

  * ``percona-server-client-5.6``

  * ``percona-server-common-5.6``

  * ``libperconaserverclient18``

At the moment of writing this guide, for *Ubuntu* 12.04LTS on ``x86_64``, a way of doing this is: ::

  $ wget -r -l 1 -nd -A deb -R "*dev*" \
  http://www.percona.com/downloads/Percona-Server-5.6/LATEST/deb/precise/x86_64/

Install them in one command: ::

  $ sudo dpkg -i *.deb

The installation won't succeed as there will be missing dependencies. To handle this, use: ::

  $ apt-get -f install

and all dependencies will be handled by :command:`apt`.

The installation script will run automatically :command:`mysql_upgrade` to migrate to the new grant tables and rebuild the indexes where needed.

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

Note that this procedure is the same for upgrading from |MySQL| 5.5 to |Percona Server| 5.6, just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove them.

Download the following packages for your architecture:

  * ``Percona-Server-server-56``

  * ``Percona-Server-client-56``

  * ``Percona-Server-shared-56``

At the moment of writing this guide, a way of doing this is: ::

  $ wget -r -l 1 -nd -A rpm -R "*devel*,*debuginfo*" \
  http://www.percona.com/downloads/Percona-Server-5.6/LATEST/RPM/rhel6/x86_64/

Install them in one command: ::

  $ rpm -ivh Percona-Server-shared-56-5.6.6-alpha60.1.285.rhel6.x86_64.rpm \ 
  Percona-Server-client-56-5.6.6-alpha60.1.285.rhel6.x86_64.rpm \
  Percona-Server-server-56-5.6.6-alpha60.1.285.rhel6.x86_64.rpm

If you don't install all "at the same time", you will need to do it in a specific order - ``shared``, ``client``, ``server``: ::

  $ rpm -ivh Percona-Server-shared-56-5.6.6-alpha60.1.285.rhel6.x86_64.rpm
  $ rpm -ivh Percona-Server-client-56-5.6.6-alpha60.1.285.rhel6.x86_64.rpm
  $ rpm -ivh Percona-Server-server-56-5.6.6-alpha60.1.285.rhel6.x86_64.rpm

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
