.. _apt_repo:

====================================================
Installing |Percona Server| on *Debian* and *Ubuntu*
====================================================

Ready-to-use packages are available from the |Percona Server| software repositories and the `download page <http://www.percona.com/downloads/Percona-Server-5.7/>`_.

Supported Releases:

* Debian:

 * 8.0 (jessie)
 * 9.0 (stretch)
 * 10.0 (buster)

* Ubuntu:

 * 16.04LTS (xenial)
 * 18.04 (bionic)

Supported Platforms:

 * x86
 * x86_64 (also known as ``amd64``)

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

1. Fetch the repository packages from Percona web:

   .. code-block:: bash

      $ wget https://repo.percona.com/apt/percona-release_latest.$(lsb_release -sc)_all.deb

2. Install the downloaded package with :program:`dpkg`. To do that, run the following commands as root or with :program:`sudo`:

   .. code-block:: bash

      $ sudo dpkg -i percona-release_latest.$(lsb_release -sc)_all.deb

   Once you install this package, the Percona repositories should be added. You can check the repository setup in the :file:`/etc/apt/sources.list.d/percona-original-release.list` file.

3. Remember to update the local cache:

   .. code-block:: bash

      $ sudo apt-get update

4. After that you can install the server package:

   .. code-block:: bash

     $ sudo apt-get install percona-server-server-5.7

.. note::

  |Percona Server| 5.7 comes with the :ref:`TokuDB storage engine <tokudb_intro>`. You can find more information on how to install and enable the |TokuDB| storage in the :ref:`tokudb_installation` guide.

Percona ``apt`` Testing repository
----------------------------------

Percona offers pre-release builds from the testing repository. To enable it add the just uncomment the testing repository lines in the Percona repository definition in your repository file (default :file:`/etc/apt/sources.list.d/percona-release.list`). It should looks like this (in this example ``VERSION`` is the name of your distribution): ::

  # Testing & pre-release packages
  #
  deb http://repo.percona.com/apt VERSION testing
  deb-src http://repo.percona.com/apt VERSION testing

Apt-Pinning the packages
------------------------

In some cases, you might need to `pin <https://wiki.debian.org/AptConfiguration?action=show&redirect=AptPinning>`_ the selected packages to avoid upgrades from the distribution repositories. Create a new file :file:`/etc/apt/preferences.d/00percona.pref` and add the following lines: ::

  Package: *
  Pin: release o=Percona Development Team
  Pin-Priority: 1001


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

.. rubric:: AppArmor settings

AppArmor is a kernel-integrated system which controls how applications access the file system by creating application profiles. If the installation of MySQL adds an AppArmor profile, you can find the profile in the following locations:

* /etc/apparmor.d/usr.sbin.mysqld
* /etc/apparmor.d/local/usr.sbin.mysqld

The ``local`` version contains only comments. Add any changes specific for the server to the ``local`` file. 

The ``usr.sbin.mysqld`` file has the following settings:

.. sourcecode:: text

    #include <tunables/global>

    /usr/sbin/mysqld {
      ...
      # Allow data dir access
      /var/lib/mysql/ r,
      /var/lib/mysql/** rwk,

      # Allow data files dir access
        /var/lib/mysql-files/ r,
        /var/lib/mysql-files/** rwk,

      # Allow keyring dir access
        /var/lib/mysql-keyring/ r,
        /var/lib/mysql-keyring/** rwk,

      # Allow log file access
        /var/log/mysql/ r,
        /var/log/mysql/** rw,
      ...
    }

The settings govern how the files are accessed. For example, the data file directory access gives read (r) access to a directory and read, write, and lock access (rwk) to all directories and files underneath ``/mysql/``.

You should download the `apparmor-utils` package when you are working with existing AppArmor profiles. The utilities allow you to edit a profile without stopping AppArmor or removing the profile.

Before you edit a profile, change the profile to ``complain`` mode:

.. sourcecode:: bash

      # aa-complain /usr/sbin/mysqld
      setting /usr/sbin/mysqld to complain mode

In complain mode, you can edit the profile to add settings because you have relocated the data directory: ``/<volume>/dev/percona/data``:

   .. code-block:: text

        /<volume>/percona/data/ r,
        /<volume>/percona/data/** rwk,
        

    You may need to reload AppArmor or reload the specific AppArmor profile to apply the changes. 

You can also modify the ``/etc/apparmor.d/tunables/alias`` file as follows:

    .. code-block:: text
    
        alias /var/lib/mysql -> /volume/percona/data/

To reload one profile, run the following command:

.. sourcecode:: bash

    $ sudo apparmor_parser -r /etc/apparmor.d/usr.sbin.mysqld
        
Restart AppArmor with the following command:

..  code-block:: bash

    $ sudo systemctl restart apparmor
    
You can also disable AppArmor, but this action is not recommended. For earlier Ubuntu systems, prior to 16.04, use the following command:

.. code-block:: bash

    $ sudo systemctl stop apparmor
    $ sudo update-rc.d -f apparmor remove

For later Ubuntu systems, use the following:

.. sourcecode:: bash

    $ sudo sudo systemctl stop apparmor
    $ sudo systemctl disable apparmor



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

  *Debian* 8.0 (jessie) and *Ubuntu* 15.04 (vivid) come with `systemd <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and service manager so you can invoke all the above commands with ``sytemctl`` instead of ``service``. Currently both are supported.

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
