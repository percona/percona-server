.. _apt_repo:

====================================================
Installing |Percona Server| on *Debian* and *Ubuntu*
====================================================

Ready-to-use packages are available from the |Percona Server| software repositories and the `download page <http://www.percona.com/downloads/Percona-Server-5.7/>`_.

Specific information on the supported platforms, products, and versions is described in `Percona Software and Platform Lifecycle <https://www.percona.com/services/policies/percona-software-platform-lifecycle#mysql>`_.

What's in each DEB package?
===========================

The ``percona-server-server-5.7`` package contains the database server itself, the ``mysqld`` binary and associated files.

The ``percona-server-common-5.7`` package contains files common to the server and client.

The ``percona-server-client-5.7`` package contains the command line client.

The ``percona-server-5.7-dbg`` package contains debug symbols for the server.

The ``percona-server-test-5.7`` package contains the database test suite.

The ``percona-server-source-5.7`` package contains the server source.

The ``libperconaserverclient20-dev`` package contains header files needed to compile software to use the client library.

The ``libperconaserverclient20`` package contains the client shared library. The ``18.1`` is a reference to the version of the shared library. The version is incremented when there is a ABI change that requires software using the client library to be recompiled or its source code modified.

Installing |Percona Server| from Percona ``apt`` repository
===========================================================

1. Install ``GnuPG``, the GNU Privacy Guard:

   .. code-block:: bash

      $ sudo apt-get install gnupg2

2. Fetch the repository packages from Percona web:

   .. code-block:: bash

      $ wget https://repo.percona.com/apt/percona-release_latest.$(lsb_release -sc)_all.deb

3. Install the downloaded package with :program:`dpkg`. To do that, run the following commands as root or with :program:`sudo`:

   .. code-block:: bash

      $ sudo dpkg -i percona-release_latest.$(lsb_release -sc)_all.deb

  
4. Remember to update the local cache:

   .. code-block:: bash

      $ sudo apt-get update
      
   Once you install this package the Percona repositories should be added. You can check the repository setup in the :file:`/etc/apt/sources.list.d/percona-release.list` file.

5. After that you can install the server package:

   .. code-block:: bash

     $ sudo apt-get install percona-server-server-5.7

.. note::

  |Percona Server| 5.7 comes with the :ref:`TokuDB storage engine <tokudb_intro>` and :ref:`MyRocks storage engine<myrocks_intro>`. These storage engines are installed as plugin.
  
  For information on how to install and configure |TokuDB|, refer to the :ref:`tokudb_installation` guide.
  
  For information on how to install and configure |MyRocks|, refer to the :ref:`myrocks_install` guide.

  
The |Percona Server| distribution contains several useful User Defined Functions (UDF) from Percona Toolkit. After the installation completes, run the following commands to create these functions:

.. code-block:: bash

    mysql -e "CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME 'libfnv1a_udf.so'"
    mysql -e "CREATE FUNCTION fnv_64 RETURNS INTEGER SONAME 'libfnv_udf.so'"
    mysql -e "CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME 'libmurmur_udf.so'"
    
For more details on the UDFs, see `Percona Toolkit UDFS <https://www.percona.com/doc/percona-server/5.7/management/udf_percona_toolkit.html>`_.

Percona ``apt`` Testing repository
----------------------------------

Percona offers pre-release builds from the testing repository. To enable it, run
|percona-release| with the ``testing`` argument. |tip.run-this.root|.

.. code-block:: bash

    $ sudo percona-release enable original testing

Apt-Pinning the packages
------------------------

In some cases you might need to "pin" the selected packages to avoid the upgrades from the distribution repositories. You'll need to make a new file :file:`/etc/apt/preferences.d/00percona.pref` and add the following lines in it: ::

  Package: *
  Pin: release o=Percona Development Team
  Pin-Priority: 1001

For more information about the pinning you can check the official `debian wiki <http://wiki.debian.org/AptPreferences>`_.

.. _standalone_deb:

Installing |Percona Server| using downloaded deb packages
=========================================================

Download the packages of the desired series for your architecture from the `download page <http://www.percona.com/downloads/Percona-Server-5.7/>`_. The easiest way is to download bundle which contains all the packages. Following example will download |Percona Server| :rn:`5.7.10-3` release packages for *Debian* 8.0:

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

This will install all the packages from the bundle. Another option is to download/specify only the packages you need for running |Percona Server| installation (``libperconaserverclient20_5.7.10-3-1.jessie_amd64.deb``, ``percona-server-client-5.7_5.7.10-3-1.jessie_amd64.deb``, ``percona-server-common-5.7_5.7.10-3-1.jessie_amd64.deb``, and ``percona-server-server-5.7_5.7.10-3-1.jessie_amd64.deb``. Optionally you can install ``percona-server-tokudb-5.7_5.7.10-3-1.jessie_amd64.deb`` if you want |TokuDB| storage engine).

.. note::

  |Percona Server| 5.7 comes with the :ref:`TokuDB storage engine <tokudb_intro>`. You can find more information on how to install and enable the |TokuDB| storage in the :ref:`tokudb_installation` guide.

.. warning::

  When installing packages manually like this, you'll need to make sure to resolve all the dependencies and install missing packages yourself. Following packages will need to be installed before you can manually install Percona Server: ``mysql-common``, ``libjemalloc1``, ``libaio1`` and ``libmecab2``

The following table lists the default locations for files:

.. list-table::
    :widths: 30 30
    :header-rows: 1

    * - Files
      - Location
    * - `mysqld` server
      - :file:`/usr/sbin`
    * - Configuration
      - :file:`/etc/mysql/my.cnf`
    * - Data directory
      - :file:`/var/lib/mysql`
    * - Logs
      - :file:`/var/log/mysql`

.. note::

  *Debian* and *Ubuntu* installation does not automatically create a special
  ``debian-sys-maint`` user which can be used by the control scripts to control
  the |Percona Server| ``mysqld`` and ``mysqld_safe`` services like it was the
  case with previous |Percona Server| versions. If you still require this user you must create the user manually.

Running |Percona Server|
========================

The following procedure runs the |Percona Server|:

1. Starting the service

   |Percona Server| starts automatically after installation unless the server
   encounters errors during the installation process. You can also manually
   start it by running the following command:

   .. code-block:: bash

     $ sudo service mysql start

2. Confirming the service is running

   You can verify the service status by running the following command:

   .. code-block:: bash

     $ service mysql status

3. Stopping the service

   You can stop the service by running the following command:

   .. code-block:: bash

     $ sudo service mysql stop

4. Restarting the service

   You can restart the service by running the following command:

   .. code-block:: bash

     $ sudo service mysql restart

.. note::

  *Debian* 8.0 (jessie) and *Ubuntu* 16.04(Xenial) come with `systemd <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and service manager so you can invoke all the above commands with ``sytemctl`` instead of ``service``. Currently, both are supported.

Uninstalling |Percona Server|
=============================

To uninstall |Percona Server|, you must remove all of the installed packages. 

You have the following options:

* Removing packages with :command:`apt-get remove` leaves the configuration and data files. 
* Removing the packages with :command:`apt-get purge` removes all the packages with configuration files and data files (all the databases). 

Depending on your needs, you can choose which command better suits you. 

.. seealso:: 

    `apt-get <http://manpages.ubuntu.com/manpages/bionic/man8/apt-get.8.html>`_

1. Stop the |Percona Server| service

   .. code-block:: bash

     $ sudo service mysql stop

2. Remove the packages

   a) Remove the packages. This option does not delete the configuration or data files. If you do not require these files, you must delete each file manually. 

   .. code-block:: bash

     $ sudo apt-get remove 'percona-server*'

   b) Purge the packages. This option deletes packages, configuration, and data files. The option does not delete any configuration or data files stored in your home directory. You may need to delete some files manually.

   .. code-block:: bash

     $ sudo apt-get purge 'percona-server*'
     $ sudo apt-get autoremove -y
     $ sudo apt-get autoclean
     $ sudo rm -rf /etc/mysql

.. note::

    In a regular expression, the ``*`` (asterisk) matches zero or more of the preceding item. The single quotes prevent the shell from misinterpreting the asterisk as a shell command.    

  If you do not plan to upgrade, run the following commands to remove the data directory location:

  .. code-block:: bash

      rm -rf /var/lib/mysql
      rm -rf /var/log/mysql

     $ sudo apt-get purge percona-server*

.. include:: ../.res/replace.txt
.. include:: ../.res/replace.program.txt

