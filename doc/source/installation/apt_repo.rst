.. _apt_repo:

================================================================================
Installing |Percona Server| on *Debian* and *Ubuntu*
================================================================================

Ready-to-use packages are available from the |Percona Server| software
repositories and the `Percona downloads`_ page.

Specific information on the supported platforms, products, and versions is described in `Percona Software and Platform Lifecycle <https://www.percona.com/services/policies/percona-software-platform-lifecycle#mysql>`_.

What's in each DEB package?
===========================

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Package
     - Contains
   * - percona-server-server
     - The database server itself, the ``mysqld`` binary and associated files.
   * - percona-server-common
     - The files common to the server and client.
   * - percona-server-client
     - The command line client.
   * - percona-server-dbg
     - Debug symbols for the server.
   * - percona-server-test
     - The database test suite.
   * - percona-server-source
     - The server source.
   * - libperconaserverclient21-dev
     - Header files needed to compile software to use the client library.
   * - libperconaserverclient21
     - The client shared library. The version is incremented when there is an
       ABI change that requires software using the client library to be
       recompiled or its source code modified.

.. _apt-install:

Installing |Percona Server| from Percona ``apt`` repository
===========================================================

1. Install ``GnuPG``, the GNU Privacy Guard:

   .. code-block:: bash

      $ sudo apt install gnupg2 curl

2. Fetch the repository packages from Percona web:

   .. code-block:: bash

      $ wget https://repo.percona.com/apt/percona-release_latest.$(lsb_release -sc)_all.deb

#. Install the downloaded package with :program:`dpkg`. To do that, run the following commands as root or with :program:`sudo`:

   .. code-block:: bash

      $ sudo dpkg -i percona-release_latest.$(lsb_release -sc)_all.deb

#. Once you install this package the Percona repositories should be added. You
   can check the repository setup in the
   :file:`/etc/apt/sources.list.d/percona-release.list` file.

#. Enable the repository:

   .. code-block:: bash

     $ sudo percona-release setup ps80

#. After that you can install the server package:

   .. code-block:: bash

      $ sudo apt install percona-server-server

.. note::

   |ps-last| comes with the :ref:`TokuDB storage engine <tokudb_intro>` and :ref:`MyRocks storage engine<myrocks_intro>`. These storage engines are installed as plugin.

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds. For more information, see :ref:`tokudb_intro`.

   For information on how to install and configure |TokuDB|, refer to the :ref:`tokudb_installation` guide.

   For information on how to install and configure |MyRocks|, refer to the :ref:`myrocks_install` guide.

The |Percona Server| distribution contains several useful User Defined Functions (UDF) from Percona Toolkit. After the installation completes, run the following commands to create these functions:

.. code-block:: bash

    mysql -e "CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME 'libfnv1a_udf.so'"
    mysql -e "CREATE FUNCTION fnv_64 RETURNS INTEGER SONAME 'libfnv_udf.so'"
    mysql -e "CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME 'libmurmur_udf.so'"

For more details on the UDFs, see `Percona Toolkit UDFS <https://www.percona.com/doc/percona-server/8.0/management/udf_percona_toolkit.html>`_.

Percona ``apt`` Testing repository
--------------------------------------------------------------------------------

Percona offers pre-release builds from the testing repository. To enable it, run
|percona-release| with the ``testing`` argument. |tip.run-this.root|.

.. code-block:: bash

   $ sudo percona-release enable ps80 testing

Apt-Pinning the packages
--------------------------------------------------------------------------------

In some cases you might need to "pin" the selected packages to avoid the
upgrades from the distribution repositories. You'll need to make a new file
:file:`/etc/apt/preferences.d/00percona.pref` and add the following lines in it:
::

  Package: *
  Pin: release o=Percona Development Team
  Pin-Priority: 1001

For more information about the pinning you can check the official `debian wiki
<http://wiki.debian.org/AptPreferences>`_.

.. _standalone_deb:

Installing |Percona Server| using downloaded deb packages
================================================================================

Download the packages of the desired series for your architecture from the
`Percona downloads`_ page. The easiest way is to download bundle which contains
all the packages. The following example will download |Percona Server|
:rn:`8.0.13-3` release packages for |debian-last|:

.. code-block:: bash

   $ wget https://www.percona.com/downloads/Percona-Server-8.0/Percona-Server-8.0.13-3/binary/debian/stretch/x86_64/percona-server-8.0.13-3-r63dafaf-stretch-x86_64-bundle.tar

You should then unpack the bundle to get the packages:

 .. code-block:: bash

    $ tar xvf percona-server-8.0.13-3-r63dafaf-stretch-x86_64-bundle.tar

After you unpack the bundle you should see the following packages:

  .. code-block:: bash

     $ ls *.deb

  .. admonition:: Output

     .. code-block:: bash

        libperconaserverclient21-dev_8.0.13-3-1.stretch_amd64.deb
        libperconaserverclient21_8.0.13-3-1.stretch_amd64.deb
        percona-server-dbg_8.0.13-3-1.stretch_amd64.deb
        percona-server-client_8.0.13-3-1.stretch_amd64.deb
        percona-server-common_8.0.13-3-1.stretch_amd64.deb
        percona-server-server_8.0.13-3-1.stretch_amd64.deb
        percona-server-source_8.0.13-3-1.stretch_amd64.deb
        percona-server-test_8.0.13-3-1.stretch_amd64.deb
        percona-server-tokudb_8.0.13-3-1.stretch_amd64.deb



Now, you can install |Percona Server| using |dpkg|. |tip.run-this.root|

  .. code-block:: bash

    $ sudo dpkg -i *.deb

This will install all the packages from the bundle. Another option is to
download/specify only the packages you need for running |Percona Server|
installation (``libperconaserverclient21_8.0.13-3-1.stretch_amd64.deb``,
``percona-server-client_8.0.13-3-1.stretch_amd64.deb``,
``percona-server-common_8.0.13-3-1.stretch_amd64.deb``, and
``percona-server-server_8.0.13-3-1.stretch_amd64.deb``. Optionally, you can install
``percona-server-tokudb_8.0.13-3-1.stretch_amd64.deb`` if you want the |TokuDB|
storage engine).

.. note::

   |Percona Server| |version| comes with the :ref:`TokuDB storage engine
   <tokudb_intro>`. You can find more information on how to install and enable
   the |TokuDB| storage in the :ref:`tokudb_installation` guide.

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds. For more information, see :ref:`tokudb_intro`.

.. warning::

   When installing packages manually like this, you'll need to make sure to
   resolve all the dependencies and install missing packages yourself. Following
   packages will need to be installed before you can manually install Percona
   Server: ``mysql-common``, ``libjemalloc1``, ``libaio1`` and ``libmecab2``


Running |Percona Server|
========================

|Percona Server| stores the data files in :file:`/var/lib/mysql/` by
default. You can find the configuration file that is used to manage |Percona
Server| in :file:`/etc/mysql/my.cnf`.

.. note::

   *Debian* and *Ubuntu* installation doesn't automatically create a special
    ``debian-sys-maint`` user which can be used by the control scripts to
    control the |Percona Server| ``mysqld`` and ``mysqld_safe`` services like it
    was the case with previous |Percona Server| versions. If you still require
    this user you'll need to create it manually.

|tip.run-all.root|

1. Starting the service

   |Percona Server| is started automatically after it gets installed unless it
   encounters errors during the installation process. You can also manually
   start it by running: :bash:`service mysql start`

#. Confirming that service is running. You can check the service status by
   running: :bash:`service mysql status`

#. Stopping the service

   You can stop the service by running: :bash:`service mysql stop`

#. Restarting the service. :bash:`service mysql restart`

.. note::

   |debian-last| and |ubuntu-lts| come with `systemd
   <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and
   service manager. You can invoke all the above commands with ``systemctl``
   instead of ``service``. Currently both are supported.

Working with AppArmor
======================

For information on AppArmor, see :ref:`enable-apparmor`.

Uninstalling |Percona Server|
=============================

To uninstall |Percona Server| you'll need to remove all the installed
packages. Removing packages with `apt remove` does not remove the
configuration and data files. Removing the packages with `apt purge` does remove the packages with configuration files and data files (all
the databases). Depending on your needs you can choose which command better
suits you.

1. Stop the |Percona Server| service: :bash:`service mysql stop`
2. Remove the packages

   a) Remove the packages. This will leave the data files (databases, tables, logs, configuration, etc.) behind. In case you don't need them you'll need to remove them manually: :bash:`apt remove percona-server*`

   b) Purge the packages. **NOTE**: This command removes all the packages and delete all the data files (databases, tables, logs, and so on.): :bash:`apt purge percona-server*`


.. include:: ../.res/replace.txt
.. include:: ../.res/replace.program.txt
.. include:: ../.res/url.txt
