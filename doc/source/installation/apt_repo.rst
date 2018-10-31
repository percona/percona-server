.. _apt_repo:

====================================================
Installing |Percona Server| on *Debian* and *Ubuntu*
====================================================

Ready-to-use packages are available from the |Percona Server| software repositories and the `download page <http://www.percona.com/downloads/Percona-Server-5.7/>`_.

Supported Releases:

* Debian:

 * 9.0 (stretch)

* Ubuntu:

 * 16.04LTS (xenial) 
 * 17.04 (zesty)
 * 18.04 (bionic)

Supported Platforms:

 * x86_64 (also known as ``amd64``)

What's in each DEB package?
===========================

The ``percona-server-server`` package contains the database server itself, the ``mysqld`` binary and associated files.

The ``percona-server-common`` package contains files common to the server and client.

The ``percona-server-client`` package contains the command line client.

The ``percona-server-dbg`` package contains debug symbols for the server.

The ``percona-server-test`` package contains the database test suite.

The ``percona-server-source`` package contains the server source.

The ``libperconaserverclient20-dev`` package contains header files needed to compile software to use the client library.

The ``libperconaserverclient20`` package contains the client shared library. The ``18.1`` is a reference to the version of the shared library. The version is incremented when there is a ABI change that requires software using the client library to be recompiled or its source code modified.
                   
Installing |Percona Server| from Percona ``apt`` repository
===========================================================

1. Fetch the repository packages from Percona web: 

   .. code-block:: bash

     wget https://repo.percona.com/apt/percona-release_0.1-8.$(lsb_release -sc)_all.deb

#. Install the downloaded package with :program:`dpkg`. To do that, run the following commands as root or with :program:`sudo`: 

   .. code-block:: bash

     dpkg -i percona-release_0.1-8.$(lsb_release -sc)_all.deb

   Once you install this package the Percona repositories should be added. You can check the repository setup in the :file:`/etc/apt/sources.list.d/percona-release.list` file.

#. Enable the repository:

   .. code-block:: bash

     $ sudo percona-release enable ps-80 testing

#. Remember to update the local cache:

   .. code-block:: bash

     $ sudo apt-get update

#. After that you can install the server package:

   .. code-block:: bash

     $ sudo apt-get install percona-server

.. note:: 

  |Percona Server| 8.0 comes with the :ref:`TokuDB storage engine <tokudb_intro>`. You can find more information on how to install and enable the |TokuDB| storage in the :ref:`tokudb_installation` guide.

Apt-Pinning the packages
------------------------

In some cases you might need to "pin" the selected packages to avoid the upgrades from the distribution repositories. You'll need to make a new file :file:`/etc/apt/preferences.d/00percona.pref` and add the following lines in it: :: 

  Package: *
  Pin: release o=Percona Development Team
  Pin-Priority: 1001

For more information about the pinning you can check the official `debian wiki <http://wiki.debian.org/AptPreferences>`_.

Running |Percona Server|
========================

|Percona Server| stores the data files in :file:`/var/lib/mysql/` by default. You can find the configuration file that is used to manage |Percona Server| in :file:`/etc/mysql/my.cnf`. 

.. note:: 

  *Debian* and *Ubuntu* installation doesn't automatically create a special ``debian-sys-maint`` user which can be used by the control scripts to control the |Percona Server| ``mysqld`` and ``mysqld_safe`` services like it was the case with previous |Percona Server| versions. If you still require this user you'll need to create it manually.

1. Starting the service

   |Percona Server| is started automatically after it gets installed unless it encounters errors during the installation process. You can also manually start it by running: 

   .. code-block:: bash

     $ sudo service mysql start

2. Confirming that service is running 

   You can check the service status by running:  

   .. code-block:: bash

     $ service mysql status

3. Stopping the service

   You can stop the service by running:

   .. code-block:: bash

     $ sudo service mysql stop

4. Restarting the service 

   You can restart the service by running: 

   .. code-block:: bash

     $ sudo service mysql restart

.. note:: 

  *Debian* 9.0 (stretch) and *Ubuntu* 16.04 (xenial) come with `systemd <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and service manager so you can invoke all the above commands with ``sytemctl`` instead of ``service``. Currently both are supported.
     
Uninstalling |Percona Server|
=============================

To uninstall |Percona Server| you'll need to remove all the installed packages. Removing packages with :command:`apt-get remove` will leave the configuration and data files. Removing the packages with :command:`apt-get purge` will remove all the packages with configuration files and data files (all the databases). Depending on your needs you can choose which command better suits you.

1. Stop the |Percona Server| service

   .. code-block:: bash

     $ sudo service mysql stop 

2. Remove the packages
   
   a) Remove the packages. This will leave the data files (databases, tables, logs, configuration, etc.) behind. In case you don't need them you'll need to remove them manually.

   .. code-block:: bash

     $ sudo apt-get remove percona-server*

   b) Purge the packages. **NOTE**: This will remove all the packages and delete all the data files (databases, tables, logs, etc.)

   .. code-block:: bash

     $ sudo apt-get purge percona-server*


