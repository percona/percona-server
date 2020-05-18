.. _yum_repo:

====================================================================
 Installing |Percona Server| on Red Hat Enterprise Linux and CentOS
====================================================================

Ready-to-use packages are available from the |Percona Server| software repositories and the `download page <http://www.percona.com/downloads/Percona-Server-5.6/>`_. The |Percona| :program:`yum` repository supports popular *RPM*-based operating systems, including the *Amazon Linux AMI*.

The easiest way to install the *Percona Yum* repository is to install an *RPM* that configures :program:`yum` and installs the `Percona GPG key <https://www.percona.com/downloads/RPM-GPG-KEY-percona>`_.

Specific information on the supported platforms, products, and versions are described in `Percona Software and Platform Lifecycle <https://www.percona.com/services/policies/percona-software-platform-lifecycle#mysql>`_.

What's in each RPM package?
===========================

Each of the |Percona Server| RPM packages have a particular purpose.

The ``Percona-Server-server-56`` package contains the server itself (the ``mysqld`` binary).

The ``Percona-Server-56-debuginfo`` package contains debug symbols for the server.

The ``Percona-Server-client-56`` package contains the command line client.

The ``Percona-Server-devel-56`` package contains the header files needed to compile software using the client library.

The ``Percona-Server-shared-56`` package includes the client shared library.

The ``Percona-Server-shared-compat`` package includes shared libraries for software compiled against old versions of the client library. Following libraries are included in this package: ``libmysqlclient.so.12``, ``libmysqlclient.so.14``, ``libmysqlclient.so.15``, and ``libmysqlclient.so.16``.

The ``Percona-Server-test-56`` package includes the test suite for |Percona Server|.

Installing |Percona Server| from Percona ``yum`` repository
===========================================================

1. Install the Percona repository

   You can install Percona yum repository by running the following command as a ``root`` user or with sudo:

   .. code-block:: bash

      $ yum install https://repo.percona.com/yum/percona-release-latest.noarch.rpm


   .. admonition:: Output example


      .. code-block:: guess


	 Retrieving https://repo.percona.com/yum/percona-release-latest.noarch.rpm
	 Preparing...                ########################################### [100%]
         1:percona-release        ########################################### [100%]


   To install |Percona Server| with SELinux policies, you also need the :program:`Percona-Server-selinux-*.noarch.rpm` package:

   .. code-block:: bash

      $ yum install http://repo.percona.com/centos/7/RPMS/x86_64/Percona-Server-selinux-56-5.6.42-rel84.2.el7.noarch.rpm


2. Testing the repository

   Make sure packages are now available from the repository, by executing the following command:

   .. code-block:: bash

     yum list | grep percona

   You should see output similar to the following:

   .. code-block:: bash

     ...
     Percona-Server-56-debuginfo.x86_64          5.6.25-rel73.1.el6           @percona-release-x86_64
     Percona-Server-client-56.x86_64             5.6.25-rel73.1.el6           @percona-release-x86_64
     Percona-Server-devel-56.x86_64              5.6.25-rel73.1.el6           @percona-release-x86_64
     Percona-Server-server-56.x86_64             5.6.25-rel73.1.el6           @percona-release-x86_64
     Percona-Server-shared-56.x86_64             5.6.25-rel73.1.el6           @percona-release-x86_64
     Percona-Server-test-56.x86_64               5.6.25-rel73.1.el6           @percona-release-x86_64
     Percona-Server-shared-compat.x86_64         5.1.68-rel14.6.551.rhel6     percona-release-x86_64
     ...

3. Install the packages

   You can now install |Percona Server| by running:

   .. code-block:: bash

     yum install Percona-Server-server-56

Percona `yum` Testing repository
--------------------------------

Percona offers pre-release builds from our testing repository. To subscribe to the testing repository, you'll need to enable the testing repository in :file:`/etc/yum.repos.d/percona-release.repo`. To do so, set both ``percona-testing-$basearch`` and ``percona-testing-noarch`` to ``enabled = 1`` (Note that there are 3 sections in this file: release, testing and experimental - in this case it is the second section that requires updating). **NOTE:** You'll need to install the Percona repository first (ref above) if this hasn't been done already.


.. _standalone_rpm:

Installing |Percona Server| using downloaded rpm packages
=========================================================

1. Download the packages of the desired series for your architecture from the `download page <http://www.percona.com/downloads/Percona-Server-5.6/>`_. The easiest way is to download bundle which contains all the packages. Following example will download |Percona Server| 5.6.25-73.1 release packages for *CentOS* 6:

   .. code-block:: bash

     wget https://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.25-73.1/binary/redhat/6/x86_64/Percona-Server-5.6.25-73.1-r07b797f-el6-x86_64-bundle.tar

2. You should then unpack the bundle to get the packages:

   .. code-block:: bash

     tar xvf Percona-Server-5.6.25-73.1-r07b797f-el6-x86_64-bundle.tar

   After you unpack the bundle you should see the following packages:

   .. code-block:: bash

     $ ls *.rpm

     Percona-Server-56-debuginfo-5.6.25-rel73.1.el6.x86_64.rpm
     Percona-Server-client-56-5.6.25-rel73.1.el6.x86_64.rpm
     Percona-Server-devel-56-5.6.25-rel73.1.el6.x86_64.rpm
     Percona-Server-server-56-5.6.25-rel73.1.el6.x86_64.rpm
     Percona-Server-shared-56-5.6.25-rel73.1.el6.x86_64.rpm
     Percona-Server-test-56-5.6.25-rel73.1.el6.x86_64.rpm


3. Now you can install |Percona Server| by running:

   .. code-block:: bash

     rpm -ivh Percona-Server-server-56-5.6.25-rel73.1.el6.x86_64.rpm \
     Percona-Server-client-56-5.6.25-rel73.1.el6.x86_64.rpm \
     Percona-Server-shared-56-5.6.25-rel73.1.el6.x86_64.rpm

This will install only packages required to run the |Percona Server|. To install all the packages (for debugging, testing, etc.) you should run:

.. code-block:: bash

   $ rpm -ivh *.rpm

.. note::

   When installing packages manually like this, you'll need to make sure to resolve all the dependencies and install missing packages yourself.

The following table lists the default locations for files:

.. list-table::
    :widths: 30 30
    :header-rows: 1

    * - Files
      - Location
    * - mysqld server
      - :file:`/usr/bin`
    * - Configuration
      - :file:`/etc/my.cnf`
    * - Data directory
      - :file:`/var/lib/mysql`
    * - Logs
      - :file:`/var/log/mysqld.log`

You can use the following command to locate the Data directory:

.. code-block:: bash

    grep datadir /etc/my.cnf

    datadir=/var/lib/mysql


Running |Percona Server|
========================

1. Starting the service

   |Percona Server| does not start automatically on *RHEL* and *CentOS* after
   the installation. You should start the server by running:

   .. code-block:: bash

     service mysql start

2. Confirming that service is running

   You can check the service status by running:

   .. code-block:: bash

     service mysql status

3. Stopping the service

   You can stop the service by running:

   .. code-block:: bash

     service mysql stop

4. Restarting the service

   You can restart the service by running:

   .. code-block:: bash

     service mysql restart

.. note::

  *RHEL* 7 and *CentOS* 7 come with `systemd <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and service manager so you can invoke all the above commands with ``sytemctl`` instead of ``service``. Currently both are supported.

Uninstalling |Percona Server|
=============================

To completely uninstall |Percona Server| you'll need to remove all the installed packages and data files.

1.  Stop the |Percona Server| service

    .. code-block:: bash

     service mysql stop

2. Remove the packages

   .. code-block:: bash

    yum remove Percona-Server*

3. Remove the data and configuration files

   .. code-block:: bash

     rm -rf /var/lib/mysql
     rm -f /etc/my.cnf

.. warning::

  This will remove all the packages and delete all the data files (databases, tables, logs, etc.), you might want to take a backup before doing this in case you need the data.


