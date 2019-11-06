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

Supported Releases:

- *CentOS* 6 and *RHEL* 6 (Current Stable)
- *CentOS* 7 and *RHEL* 7
- *RHEL* 8
- *Amazon Linux AMI* (works the same as *CentOS* 6)
- *Amazon Linux* 2

.. important::

   "Current Stable": We support only the current stable RHEL6/CentOS6 release,
   because there is no official (i.e. RedHat provided) method to support or
   download the latest OpenSSL on RHEL/CentOS versions prior to 6.5. Similarly,
   and also as a result thereof, there is no official Percona way to support the
   latest builds of |Percona Server| on RHEL/CentOS versions prior to
   6.5. Additionally, many users will need to upgrade to OpenSSL 1.0.1g or later
   (due to the `Heartbleed vulnerability
   <http://www.percona.com/resources/ceo-customer-advisory-heartbleed>`_), and
   this OpenSSL version is not available for download from any official
   RHEL/Centos repository for versions 6.4 and prior. For any officially
   unsupported system, src.rpm packages may be used to rebuild |Percona Server|
   for any environment. Please contact our `support service
   <http://www.percona.com/products/mysql-support>`_ if you require further
   information on this.

The *CentOS* repositories should work well with *Red Hat Enterprise
Linux* too, provided that :program:`yum` is installed on the server.

.. important::

     *CentOS* 6 offers an outdated version of the ``curl`` library required by the
     :ref:`keyring Vault plugin <keyring_vault_plugin>` of |Percona Server|. The
     version of the ``curl`` library in *CentOS* 6, which depends on the ``nss``
     library, is known to create memory corruption issues. This bug is `registered in
     Red Hat Bugzilla <https://bugzilla.redhat.com/show_bug.cgi?id=1057388>`_. Its
     current status is `CLOSED WONTFIX`.

     If you intend to use the keyring Vault plugin of |Percona Server|
     make sure that you use the latest version of the ``curl`` library.
     We recommend that you `build it from source
     <https://curl.haxx.se/docs/install.html>`_ configuring with
     ``ssl`` but without ``nss``:

     .. code-block:: bash

        $ ./configuration --with-ssl --without-nss --prefix=<INSTALATION DIRECTORY>

     As soon as you install ``curl``, make sure that |Percona Server| will use
     this version.

    .. seealso::

       How to install curl and libcurl
          https://curl.haxx.se/docs/install.html

Supported Platforms:

 * x86_64 (also known as ``amd64``)

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

Please add sudo to percona-release setup and yum install commands


|tip.run-all.root|

1. Install the Percona repository

   You can install Percona yum repository by running the following command as a ``root`` user or with sudo:

   .. code-block:: bash

      $ sudo yum install https://repo.percona.com/yum/percona-release-latest.noarch.rpm

   You should see some output such as the following:

   .. code-block:: bash

      Retrieving http://www.percona.com/downloads/percona-release/redhat/0.1-6/percona-release-latest.noarch.rpm
      Preparing...                ########################################### [100%]
      1:percona-release        ########################################### [100%]

#. Enable the repository:

   .. code-block:: bash

      $ sudo percona-release setup ps80

#. Install the packages

   You can now install |Percona Server| by running:

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
   with ``sytemctl`` instead of ``service``. Currently both are
   supported.

Uninstalling |Percona Server|
=============================

To completely uninstall |Percona Server| you'll need to remove all the installed packages and data files.

1.  Stop the |Percona Server| service: |service.mysql.stop|
#. Remove the packages:

   .. code-block:: bash

      $ sudo yum remove percona-server*

#. Remove the data and configuration files

   .. code-block:: bash

      rm -rf /var/lib/mysql
      rm -f /etc/my.cnf

.. warning::

   This will remove all the packages and delete all the data files (databases,
   tables, logs, etc.), you might want to take a backup before doing this in
   case you need the data.

.. rubric:: Footnotes

.. [#f1]

.. include:: ../.res/replace.program.txt
.. include:: ../.res/replace.txt
