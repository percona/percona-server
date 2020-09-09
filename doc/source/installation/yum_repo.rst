.. _yum_repo:

====================================================================
 Installing |Percona Server| on Red Hat Enterprise Linux and CentOS
====================================================================

.. package name: percona-server-server-8.0.13-3.1.el7.x86_64.rpm

Ready-to-use packages are available from the |Percona Server| software
repositories and the `download page
<http://www.percona.com/downloads/Percona-Server-8.0/>`_. The
|Percona| :program:`yum` repository supports popular *RPM*-based
operating systems, including the *Amazon Linux AMI*.

The easiest way to install the *Percona Yum* repository is to install an *RPM*
that configures :program:`yum` and installs the `Percona GPG key
<https://www.percona.com/downloads/RPM-GPG-KEY-percona>`_.

Specific information on the supported platforms, products, and versions are described in `Percona Software and Platform Lifecycle <https://www.percona.com/services/policies/percona-software-platform-lifecycle#mysql>`_.

What's in each RPM package?
===========================

Each of the |Percona Server| RPM packages have a particular purpose.

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Package
     - Contains
   * - percona-server-server
     - The server itself (the ``mysqld`` binary)
   * - percona-server-debuginfo
     - Debug symbols for the server
   * - percona-server-client
     - The command line client
   * - percona-server-devel
     - the header files needed to compile software using the client library.
   * - percona-server-shared
     - The client shared library.
   * - percona-server-shared-compat
     - Shared libraries for software compiled against old versions of
       the client library. The following libraries are included in
       this package: ``libmysqlclient.so.12``,
       ``libmysqlclient.so.14``, ``libmysqlclient.so.15``,
       ``libmysqlclient.so.16``, and ``libmysqlclient.so.18``.
   * - percona-server-test
     - package includes the test suite for |Percona Server|.

Installing |Percona Server| from Percona ``yum`` repository
===========================================================

You can install Percona yum repository by running the following commands as a ``root`` user or with sudo.

1. Install the Percona repository

   .. code-block:: bash

      $ sudo yum install https://repo.percona.com/yum/percona-release-latest.noarch.rpm

   You should see an output that the files are being downloaded, like the following:

   .. code-block:: bash

      Retrieving http://www.percona.com/downloads/percona-release/redhat/0.1-6/percona-release-latest.noarch.rpm
      Preparing...                ########################################### [100%]
      1:percona-release        ########################################### [100%]

#. Enable the repository:

   .. code-block:: bash

      $ sudo percona-release setup ps80

#. Install the packages

      .. code-block:: bash

      $ sudo yum install percona-server-server

.. note::

   |Percona Server| 8.0 comes with the :ref:`TokuDB storage engine
   <tokudb_intro>` and :ref:`MyRocks <myrocks_intro>` storage engines. These
   storage engines are installed as plugins. You can find more information on how
   to install and enable the |TokuDB| storage in the :ref:`tokudb_installation`
   guide. More information about how to install |MyRocks| can be found in the
   section :ref:`myrocks_install`.

Percona `yum` Testing repository
--------------------------------------------------------------------------------

Percona offers pre-release builds from our testing repository. To
subscribe to the testing repository, you'll need to enable the testing
repository in :file:`/etc/yum.repos.d/percona-release.repo`. To do so,
set both ``percona-testing-$basearch`` and ``percona-testing-noarch``
to ``enabled = 1`` (Note that there are 3 sections in this file:
release, testing and experimental - in this case it is the second
section that requires updating). **NOTE:** You'll need to install the
Percona repository first (ref above) if this hasn't been done already.


.. _standalone_rpm:

Installing |Percona Server| using downloaded rpm packages
================================================================================

1. Download the packages of the desired series for your architecture from the
   `download page <http://www.percona.com/downloads/Percona-Server-8.0/>`_. The
   easiest way is to download bundle which contains all the packages. Following
   example will download |Percona Server| 8.0.13-3 release packages for *CentOS*
   7:

   .. code-block:: bash

      $ wget https://www.percona.com/downloads/Percona-Server-8.0/Percona-Server-8.0.13-3/binary/redhat/7/x86_64/Percona-Server-8.0.13-3-r63dafaf-el7-x86_64-bundle.tar

2. You should then unpack the bundle to get the packages: :bash:`tar xvf Percona-Server-8.0.13-3-r63dafaf-el7-x86_64-bundle.tar`

   After you unpack the bundle you should see the following packages when running :bash:`ls *.rpm`:

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

  .. note::

    For an RHEL 8 package installation, Percona Server requires the mysql module to be disabled.

    .. code-block:: bash

        $ sudo yum module disable mysql

3. Now you can install |Percona Server| 8.0 by running:

   .. code-block:: bash

      $ sudo rpm -ivh percona-server-server-80-8.0.13-3.el7.x86_64.rpm \
      percona-server-client-80-8.0.13-3.el7.x86_64.rpm \
      percona-server-shared-80-8.0.13-3.el7.x86_64.rpm

This will install only packages required to run the |Percona Server|
8.0. Optionally you can install :ref:`TokuDB <tokudb_intro>` storage engine by
adding the ``percona-server-tokudb-80-8.0.13-3.el7.x86_64.rpm`` to the command
above. You can find more information on how to install and enable the |TokuDB|
storage in the :ref:`tokudb_installation` guide.

To install all the packages (for debugging, testing, etc.) you should run:

   .. code-block:: bash

      $ sudo rpm -ivh *.rpm

.. note::

   When installing packages manually like this, you'll need to make sure to
   resolve all the dependencies and install missing packages yourself.

Running |Percona Server|
========================

|Percona Server| stores the data files in :file:`/var/lib/mysql/` by
default. You can find the configuration file that is used to manage |Percona
Server| in :file:`/etc/my.cnf`.

1. Starting the service

   |Percona Server| is not started automatically on *RHEL* and *CentOS* after it
   gets installed. You should start it by running:

   .. code-block:: bash

      $ sudo service mysql start

2. Confirming that service is running

   You can check the service status by running:

   .. code-block:: bash

      $ sudo service mysql status

3. Stopping the service

   You can stop the service by running:

   .. code-block:: bash

      $ sudo service mysql stop

4. Restarting the service

   You can restart the service by running:

   .. code-block:: bash

      $ sudo service mysql restart

.. note::

   *RHEL* 7 and *CentOS* 7 come with `systemd
   <http://freedesktop.org/wiki/Software/systemd/>`_ as the default
   system and service manager so you can invoke all the above commands
   with ``sytemctl`` instead of ``service``. Currently, both are
   supported.

Uninstalling |Percona Server|
=============================

To completely uninstall |Percona Server| you'll need to remove all the installed packages and data files.

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

      rm -rf /var/lib/mysql
      rm -f /etc/my.cnf



