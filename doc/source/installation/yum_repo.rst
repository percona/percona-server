.. _yum_repo:

====================================================================
 Installing |Percona Server| on Red Hat Enterprise Linux and CentOS
====================================================================

.. package name: percona-server-server-8.0.13-3.1.el7.x86_64.rpm

Ready-to-use packages are available from the |Percona Server| software
repositories and the `download page
<http://www.percona.com/downloads/Percona-Server-8.0/>`_. The
|Percona| :program:`yum` repository supports popular *RPM*-based
operating systems. The easiest way to install the *Percona Yum* repository is to install an *RPM*
that configures :program:`yum` and installs the `Percona GPG key
<https://www.percona.com/downloads/RPM-GPG-KEY-percona>`_.

Specific information on the supported platforms, products, and versions are described in `Percona Software and Platform Lifecycle <https://www.percona.com/services/policies/percona-software-platform-lifecycle#mysql>`_.

|Percona Server| is certified for Red Hat Enterprise Linux 8. This certification is based on common and secure best practices, and successful interoperability with the operating system. Percona Server is listed in the `Red Hat Ecosystem Catalog <https://catalog.redhat.com/software/applications/detail/5869161>`_. 


.. note:: 

    The RPM packages for Red Hat Enterprise Linux 7 (and compatible derivatives)  do not support TLSv1.3, as it requires OpenSSL 1.1.1, which is currently not available on this platform. 
  

What's in each RPM package?
===========================

Each of the |Percona Server| RPM packages have a particular purpose.

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Package
     - Contains
   * - percona-server-server
     - Server itself (the ``mysqld`` binary)
   * - percona-server-debuginfo
     - Debug symbols for the server
   * - percona-server-client
     - Command line client
   * - percona-server-devel
     - Header files needed to compile software using the client library.
   * - percona-server-shared
     - Client shared library.
   * - percona-server-shared-compat
     - Shared libraries for software compiled against old versions of
       the client library. The following libraries are included in
       this package: ``libmysqlclient.so.12``,
       ``libmysqlclient.so.14``, ``libmysqlclient.so.15``,
       ``libmysqlclient.so.16``, and ``libmysqlclient.so.18``.
   * - percona-server-test
     - Includes the test suite for |Percona Server|.

Installing |Percona Server| from Percona ``yum`` repository
===========================================================

You can install Percona yum repository by running the following commands as a ``root`` user or with sudo.

1. Install the Percona repository

   .. code-block:: bash

      $ sudo yum install https://repo.percona.com/yum/percona-release-latest.noarch.rpm

   You should see an output that the files are being downloaded, like the following:

   .. code-block:: bash

      percona-release-latest.noarch-rpm               36 kB/s | 19 kb 00:00
      =====================================================================
        Package         Architecture      Version    Repository    Size
      =====================================================================
      Installing:
         percona release    noarch         1.0-25     @commandline  19k
      ...

#. Enable the repository:

   .. code-block:: bash

      $ sudo percona-release setup ps80
      On RedHat 8 systems it is needed to disable dnf mysql module to install Percona-Server
      Do you want to disable it? [y/N] y
      ...

#. Install the packages

   .. code-block:: bash

      $ sudo yum install percona-server-server

.. note::

   |Percona Server| 8.0 also provides the :ref:`TokuDB storage engine
   <tokudb_intro>` and :ref:`MyRocks <myrocks_intro>` storage engines which can
   be installed as plugins. 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds. For more information, see :ref:`tokudb_intro`.

   For more information on how to install and enable the |TokuDB| storage review the :ref:`tokudb_installation` document. 
   For information on how to install and enable |MyRocks| review the
   section :ref:`myrocks_install`.

Percona `yum` Testing repository
--------------------------------------------------------------------------------

Percona offers pre-release builds from our testing repository. To
subscribe to the testing repository, you enable the testing
repository in :file:`/etc/yum.repos.d/percona-release.repo`. To do so,
set both ``percona-testing-$basearch`` and ``percona-testing-noarch``
to ``enabled = 1`` (Note that there are three sections in this file:
release, testing and experimental - in this case it is the second
section that requires updating). 

.. note:: 
   
   You must install the Percona repository first if the installation has not been done already.


.. _standalone_rpm:

Installing |Percona Server| using downloaded rpm packages
================================================================================

1. Download the packages of the desired series for your architecture from the
   `download page <http://www.percona.com/downloads/Percona-Server-8.0/>`_. The
   easiest way is to download bundle which contains all the packages. Following
   example will download |Percona Server| 8.0.21-12 release packages for *RHEL* 8. 

   .. code-block:: bash

      $ wget https://www.percona.com/downloads/Percona-Server-8.0/Percona-Server-8.0.21-12/binary/redhat/8/x86_64/Percona-Server-8.0.21-12-r7ddfdfe-el8-x86_64-bundle.tar

2. Unpack the bundle to get the packages: :bash:`tar xvf Percona-Server-8.0.21-12-r7ddfdfe-el8-x86_64-bundle.tar`

3. To view a list of packages, run the following command:

   .. code-block:: bash

      $ ls *.rpm

      percona-mysql-router-8.0.21-12.2.el8.x86_64.rpm
      percona-mysql-router-debuginfo-8.0.21-12.2.el8.x86_64.rpm
      percona-server-client-8.0.21-12.2.el8.x86_64.rpm
      percona-server-client-debuginfo-8.0.21-12.2.el8.x86_64.rpm
      percona-server-debuginfo-8.0.21-12.2.el8.x86_64.rpm
      percona-server-debugsource-8.0.21-12.2.el8.x86_64.rpm
      percona-server-devel-8.0.21-12.2.el8.x86_64.rpm
      percona-server-rocksdb-8.0.21-12.2.el8.x86_64.rpm
      percona-server-rocksdb-debuginfo-8.0.21-12.2.el8.x86_64.rpm
      percona-server-server-8.0.21-12.2.el8.x86_64.rpm
      percona-server-server-debuginfo-8.0.21-12.2.el8.x86_64.rpm
      percona- server-shared-8.0.21-12.2.el8.x86_64.rpm
      percona-server-shared-compat-8.0.21-12.2.el8.x86_64.rpm
      percona-server-shared-debuginfo-8.0.21-12.2.el8.x86_64.rpm
      percona-server-test-8.0.21-12.2.el8.x86_64.rpm
      percona-server-test-debuginfo-8.0.21-12.2.el8.x86_64.rpm
      percona-server-tokudb-8.0.21-12.2.el8.x86_64.rpm

4. Install ``jemalloc`` with the following command, if needed:

   .. code-block:: bash

       wget https://repo.percona.com/yum/release/8/RPMS/x86_64/jemalloc-3.6.0-1.el8.x86_64.rpm

5.  For a *RHEL* distribution and derivatives package installation, |Percona Server| requires the mysql module to be disabled before installing the packages: 

    .. code-block:: bash

        sudo yum module disable mysql 

6. Install all the packages (for debugging, testing, etc.) with the following command:

   .. code-block:: bash

      $ sudo rpm -ivh *.rpm

.. note::

   When installing packages manually, you must make sure to
   resolve all dependencies and install any missing packages yourself.

Running |Percona Server|
========================

|Percona Server| stores the data files in :file:`/var/lib/mysql/` by
default. The configuration file used to manage |Percona
Server| is the :file:`/etc/my.cnf`.

The following commands start, provide the server status, stop the server, and restart the server.

.. note::

   The *RHEL* distributions and derivatives come with `systemd
   <http://freedesktop.org/wiki/Software/systemd/>`_ as the default
   system and service manager so you can invoke all of the commands
   with ``sytemctl`` instead of ``service``. Currently, both options are
   supported.

* |Percona Server| is not started automatically on the *RHEL* distributions and derivatives after installation. Start the server with the following command:

   .. code-block:: bash

      $ sudo service mysql start

* Review the service status with the following command:

   .. code-block:: bash

      $ sudo service mysql status

* Stop the service with the following command:

   .. code-block:: bash

      $ sudo service mysql stop

* Restart the service with the following command:

   .. code-block:: bash

      $ sudo service mysql restart



SELinux and security considerations
===============================================

For information on working with SELinux, see :ref:`selinux`.

The *RHEL* 8 distributions and derivatives have added `system-wide cryptographic policies component <https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/security_hardening/using-the-system-wide-cryptographic-policies_security-hardening>`__. This component allows the configuration of cryptographic subsystems. 

Uninstalling |Percona Server|
=============================

To completely uninstall |Percona Server|, remove all the installed packages and data files.

1.  Stop the |Percona Server| service:

    .. code-block:: bash

        $ sudo service mysql stop
        
#. Remove the packages:

   .. code-block:: bash

      $ sudo yum remove percona-server*

#. Remove the data and configuration files:

.. warning::

    This step removes all the packages and deletes all the data files (databases,
    tables, logs, etc.). Take a backup before doing this in case you need the data.


   .. code-block:: bash

      $ rm -rf /var/lib/mysql
      $ rm -f /etc/my.cnf



