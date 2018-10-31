.. _yum_repo:

====================================================================
 Installing |Percona Server| on Red Hat Enterprise Linux and CentOS
====================================================================

Ready-to-use packages are available from the |Percona Server| software repositories and the `download page <http://www.percona.com/downloads/Percona-Server-5.7/>`_. The |Percona| :program:`yum` repository supports popular *RPM*-based operating systems, including the *Amazon Linux AMI*.

The easiest way to install the *Percona Yum* repository is to install an *RPM* that configures :program:`yum` and installs the `Percona GPG key <https://www.percona.com/downloads/RPM-GPG-KEY-percona>`_.

Supported Releases:

 * *CentOS* 6 and *RHEL* 6 (Current Stable) [#f1]_

 * *CentOS* 7 and *RHEL* 7

 * *Amazon Linux AMI* (works the same as *CentOS* 6)

 * Amazon Linux 2

The *CentOS* repositories should work well with *Red Hat Enterprise Linux* too, provided that :program:`yum` is installed on the server.

Supported Platforms:

 * x86_64 (also known as ``amd64``)

What's in each RPM package?
===========================

Each of the |Percona Server| RPM packages have a particular purpose.

The ``percona-server-server`` package contains the server itself (the ``mysqld`` binary).

The ``percona-server-debuginfo`` package contains debug symbols for the server.

The ``percona-server-client`` package contains the command line client.

The ``percona-server-devel`` package contains the header files needed to compile software using the client library.

The ``percona-server-shared`` package includes the client shared library.

The ``percona-server-shared-compat`` package includes shared libraries for software compiled against old versions of the client library. Following libraries are included in this package: ``libmysqlclient.so.12``, ``libmysqlclient.so.14``, ``libmysqlclient.so.15``, ``libmysqlclient.so.16``, and ``libmysqlclient.so.18``.

The ``percona-server-test`` package includes the test suite for |Percona Server|.

Installing |Percona Server| from Percona ``yum`` repository
===========================================================

1. Install the Percona repository 
   
   You can install Percona yum repository by running the following command as a ``root`` user or with sudo:

   .. code-block:: bash

     $ sudo yum install https://repo.percona.com/centos/7/RPMS/noarch/percona-release-0.1-8.noarch.rpm

   You should see some output such as the following: 

   .. code-block:: bash

     Retrieving http://www.percona.com/downloads/percona-release/redhat/0.1-8/percona-release-0.1-8.noarch.rpm
     Preparing...                ########################################### [100%]
        1:percona-release        ########################################### [100%]

#. Enable the repository:

   .. code-block:: bash

      $ sudo percona-release disable all
      $ sudo percona-release enable ps-80 testing

#. Install the packages

   You can now install |Percona Server| by running:

   .. code-block:: bash

     sudo yum install \
       percona-server-server-8.0.12-1.2.rc1.el7 \
       percona-server-client-8.0.12-1.2.rc1.el7 \
       percona-server-shared-8.0.12-1.2.rc1.el7 \
       percona-server-shared-compat-8.0.12-1.2.rc1.el7

.. note::

  |Percona Server| 8.0 comes with the :ref:`TokuDB storage engine <tokudb_intro>`. You can find more information on how to install and enable the |TokuDB| storage in the :ref:`tokudb_installation` guide.

Running |Percona Server|
========================

|Percona Server| stores the data files in :file:`/var/lib/mysql/` by default. You can find the configuration file that is used to manage |Percona Server| in :file:`/etc/my.cnf`. 

1. Starting the service

   |Percona Server| isn't started automatically on *RHEL* and *CentOS* after it gets installed. You should start it by running:

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

.. rubric:: Footnotes

.. [#f1] "Current Stable": We support only the current stable RHEL6/CentOS6 release, because there is no official (i.e. RedHat provided) method to support or download the latest OpenSSL on RHEL/CentOS versions prior to 6.5. Similarly, and also as a result thereof, there is no official Percona way to support the latest Percona Server builds on RHEL/CentOS versions prior to 6.5. Additionally, many users will need to upgrade to OpenSSL 1.0.1g or later (due to the `Heartbleed vulnerability <http://www.percona.com/resources/ceo-customer-advisory-heartbleed>`_), and this OpenSSL version is not available for download from any official RHEL/Centos repository for versions 6.4 and prior. For any officially unsupported system, src.rpm packages may be used to rebuild Percona Server for any environment. Please contact our `support service <http://www.percona.com/products/mysql-support>`_ if you require further information on this.
