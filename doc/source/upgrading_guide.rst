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
- `Preparing your Installation for Upgrade <https://dev.mysql.com/doc/refman/8.0/en/upgrade-prerequisites.html>`_
- `Partitioning Limitations Relating to Storage Engines
  <https://dev.mysql.com/doc/refman/8.0/en/partitioning-limitations-storage-engines.html>`_

.. warning:: 

   Do not upgrade from 5.7 to 8.0 on a crashed instance. If the server instance
   has crashed, crash recovery should be run before proceeding with the upgrade.

   With partitioned tables that use the |TokuDB| or |MyRocks| storage engine, the
   upgrade only works with native partitioning.

   Note that in |Percona Server| 8.0, the ``ROW FORMAT`` clause is not supported
   in ``CREATE TABLE`` and ``ALTER TABLE`` statements. Instead, use the
   :variable:`tokudb_row_format` variable to set the default compression
   algorithm.


Upgrading using the Percona repositories
================================================================================

The easiest and recommended way of installing - where possible - is by using the
|Percona| repositories.

Instructions for enabling the repositories in a system can be found in:

* :doc:`Percona APT Repository <installation/apt_repo>`
* :doc:`Percona YUM Repository <installation/yum_repo>`

``DEB``-based distributions
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

   $ percona-release enable ps-80 testing
   $ apt-get update

.. code-block:: bash

   $ apt-get install percona-server

If you're using |Percona Server| 8.0 with |TokuDB| you'll need to specify the |TokuDB| package as well:

.. code-block:: bash

   $ apt-get install percona-server percona-server-tokudb-8.0

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

``RPM``-based distributions
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

      Percona-Server-shared-80-8.0.12-rel76.1.el7.x86_64
      Percona-Server-server-80-8.0.12-rel76.1.el7.x86_64
      Percona-Server-devel-80-8.0.12-rel76.1.el7.x86_64
      Percona-Server-client-80-8.0.12-rel76.1.el7.x86_64
      Percona-Server-test-80-8.0.12-rel76.1.el7.x86_64
      Percona-Server-80-debuginfo-8.0.12-rel76.1.el7.x86_64

After checking, proceed to remove them without dependencies: 

.. code-block:: bash

   $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important that you remove it without dependencies as many packages may
depend on these (as they replace ``mysql``) and will be removed if omitted.

Substitute :bash:`grep '^mysql-'` for :bash:`grep 'Percona-Server'` in the previous command and
remove the listed packages.

You will have to install the ``percona-server`` package: :bash:`yum install percona-server`

If you're using |Percona Server| 8.0 with |TokuDB| you'll need to specify the
|TokuDB| package as well when doing the upgrade:

.. code-block:: bash

   $ yum install percona-server Percona-Server-tokudb-80

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
   engine plugin by running the: ``ps_tokudb_admin --enable`` before running
   ``mysql_upgrade`` otherwise you'll get errors.

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

- ``percona-server-server-8.0``
- ``percona-server-client-8.0``
- ``percona-server-common-8.0``
- ``libperconaserverclient20``

The following example will download |Percona Server| :rn:`8.0.13-3` release
packages for *Debian* 8.0:

.. code-block:: bash

   $ wget https://www.percona.com/downloads/Percona-Server-8.9/Percona-Server-8.0.13-3/binary/debian/jessie/x86_64/Percona-Server-8.0.13-3-r63dafaf-jessie-x86_64-bundle.tar

You should then unpack the bundle to get the packages: :bash:`tar xvf Percona-Server-8.0.13-3-r63dafaf-jessie-x86_64-bundle.tar`

After you unpack the bundle you should see the following packages:

.. code-block:: bash

   $ ls *.deb
   libperconaserverclient20-dev_8.0.10-3-1.jessie_amd64.deb
   libperconaserverclient20_8.0.10-3-1.jessie_amd64.deb
   percona-server-8.0-dbg_8.0.10-3-1.jessie_amd64.deb
   percona-server-client-8.0_8.0.13-3.jessie_amd64.deb
   percona-server-common-8.0_8.0.13-3.jessie_amd64.deb
   percona-server-server-8.0_8.0.13-3.jessie_amd64.deb
   percona-server-source-8.0_8.0.13-3.jessie_amd64.deb
   percona-server-test-8.0_8.0.13-3.jessie_amd64.deb
   percona-server-tokudb-8.0_8.0.13-3.jessie_amd64.deb

Now you can install |Percona Server| by running:

.. code-block:: bash

   $ sudo dpkg -i *.deb

This will install all the packages from the bundle. Another option is to
download/specify only the packages you need for running |Percona Server|
installation (``libperconaserverclient20_8.0.13-3.jessie_amd64.deb``,
``percona-server-client-8.0_8.0.13-3.jessie_amd64.deb``,
``percona-server-common-8.0_8.0.13-3.jessie_amd64.deb``, and
``percona-server-server-8.0_8.0.13-3.jessie_amd64.deb``. Optionally you can
install ``percona-server-tokudb-8.0_8.0.13-3.jessie_amd64.deb`` if you want
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
   
   Percona-Server-shared-80-8.0.13-3.el6.x86_64
   Percona-Server-server-80-8.0.13-3.el6.x86_64
   Percona-Server-client-80-8.0.13-3.el6.x86_64
   Percona-Server-tokudb-80-8.0.13-3.el6.x86_64

You may have a fourth, ``shared-compat``, which is for compatibility purposes.

After checked that, proceed to remove them without dependencies: ::

  $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

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

      percona-server-80-debuginfo-8.0.13-3.el7.x86_64.rpm
      percona-server-client-80-8.0.13-3.el7.x86_64.rpm
      percona-server-devel-80-8.0.13-3.el7.x86_64.rpm
      percona-server-server-80-8.0.13-3.el7.x86_64.rpm
      percona-server-shared-80-8.0.13-3.el7.x86_64.rpm
      percona-server-shared-compat-80-8.0.13-3.el7.x86_64.rpm
      percona-server-test-80-8.0.13-3.el7.x86_64.rpm
      percona-server-tokudb-80-8.0.13-3.el7.x86_64.rpm

Now you can install |Percona Server| 8.0 by running:

.. code-block:: bash

   rpm -ivh percona-Server-server-8.0.13-3.el7.x86_64.rpm \
   percona-perver-client-8.0.13-3.el7.x86_64.rpm \
   percona-server-shared-8.0.13-3.el7.x86_64.rpm

This will install only packages required to run the |Percona Server|
8.0. Optionally you can install :ref:`TokuDB <tokudb_intro>` storage engine by
adding the ``Percona-Server-tokudb-80-8.0.13-3.el7.x86_64.rpm`` to the command
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
   engine plugin by running the: ``ps-admin --enable`` before running
   ``mysql_upgrade`` otherwise you'll get errors.

After this is done, just restart the server as usual: :bash:`service mysql restart`

.. include:: .res/replace.txt
.. include:: .res/replace.program.txt
