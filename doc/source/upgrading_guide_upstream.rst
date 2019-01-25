.. _upgrading_guide_upstream:

================================================================================
|Percona Server| In-Place Upgrading Guide from |MySQL|
================================================================================

In-place upgrades keep the existing data on the server. Generally speaking,
in-place upgrades involve stopping the server, removing the installed version of
|MySQL|, installing the new server, and starting it with the same data files.

While in-place upgrades may not be suitable for high-complexity environments,
they may be appropriate for many simpler scenarios.

It is strongly recommended that you read the following guides to be aware of the
incompatible changes that could cause the in-place upgrade to fail:

* `Upgrading MySQL <http://dev.mysql.com/doc/refman/5.7/en/upgrading.html>`_
* `Upgrading from MySQL 5.6 to 5.7 <http://dev.mysql.com/doc/refman/5.7/en/upgrading-from-previous-series.html>`_

Use one of the following approaches to make the in-place upgrade to |Percona
Server| 5.7 from |MySQL|.

.. contents::
   :local:
   :depth: 1

.. _percona-server.in-place-upgrade.repository.upstream:

In-place upgrade using |Percona| repositories
================================================================================

The recommended way of installing |Percona Server| is by using the |Percona|
repositories.

Instructions for your system about how to enable |Percona| repositories can be
found in the section :ref:`installing_from_binaries`.

The in-place upgrade using |Percona| repositories requires the following steps:

.. contents::
   :local:
   :depth: 1

.. _percona-server.in-place-upgrade.preliminary.upstream:

Preliminary steps common to all distributions
--------------------------------------------------------------------------------

.. include:: .res/text/system-service-manager.support.txt

|tip.run-all.root|.

#. Do the full backup (or dump, if possible) a then stop the server. 

   .. code-block:: bash

      $ service mysql stop

The following steps are different for DEB based and RPM based
distributions. Depending on your operating system, follow the instructions in
either :ref:`percona-server.mysql.in-place-upgrade.upstream.deb` or in
:ref:`percona-server.in-place-upgrade.upstream.rpm`.  Then, :ref:`run mysql_upgrade
<percona-server.mysql-upgrade.run-manually.upstream>`.

.. _percona-server.in-place-upgrade.upstream.deb:

Upgrading from |MySQL| 5.7 for DEB based Linux distributions
--------------------------------------------------------------------------------

The following steps are relevant to upgrading |MySQL| 5.7 installed on |debian|
or |ubuntu|.

#. Check which |MySQL| packages are installed:

   .. code-block:: bash

      $ dpkg-query -l 'mysql-*'

#. Remove |MySQL| server leaving data files intact. In the following
   example, **MYSQL-PACKAGE-NAME** is a placeholder which represents
   an installed |MySQL| package that you need to remove. For example,
   if you have |MySQL| Server Community Edition you need to uninstall
   `mysql-server` at least.

   .. code-block:: bash

      $ apt remove MYSQL-PACKAGE-NAME

#. Install the new server

   .. code-block:: bash

      $ apt install percona-server-server-5.7

   If you intend to use the |TokuDB| storage engine, specify the |TokuDB|
   package as well:

   .. code-block:: bash

      $ apt install percona-server-server-5.7 percona-server-tokudb-5.7

.. _percona-server.mysql.in-place-upgrade.rpm:

Upgrading from |MySQL| 5.7 for RPM based Linux distributions
--------------------------------------------------------------------------------

The following steps are relevant to upgrading from |MySQL|
5.7. |tip.run-all.root|.


.. rubric:: Removing packages

#. Check which |MySQL| related packages are installed:

   .. code-block:: bash

      $ rpm -qa | grep '^mysql-'

#. If any |MySQL| packages are found, remove them without dependencies.

   .. code-block:: bash

      $ rpm -qa | grep '^mysql-' | xargs rpm -e --nodeps
   
   .. include:: .res/text/important.package.removing-without-dependency.txt

.. rubric:: Installing new packages

#. Install the ``percona-server-server-57`` package

   .. code-block:: bash

      $ yum install Percona-Server-server-57 

   If you intend to use |Percona Server| 5.7 with |TokuDB|, run :program:`yum
   install` specifying the |TokuDB| package along with
   ``percona-server-server-57``.

   .. code-block:: bash

      $ yum install percona-server-server-57 percona-server-tokudb-57

#. Modify your :file:`my.cnf` configuration file and reinstall the plugins if necessary.

.. _percona-server.in-place-upgrade.starting.upstream:

Starting the server
--------------------------------------------------------------------------------

With |Percona Server| installed, start the :program:`mysql` service:

.. code-block:: bash

   $ service mysql start

|red-hat-enterprise| and |centos| 7 automatically back up the previous
configuration file to :file:`/etc/my.cnf.rpmsave` and install the default
:file:`my.cnf`. After the upgrading or installation process completes, you can
move the old configuration file back with all unsupported system removed or
commented out.

.. important::

   If you use the |TokuDB| storage engine, comment out all variables specific to
   |TokuDB| in your configuration files before starting the server. Otherwise,
   the server will not be able to start.

.. _percona-server.mysql-upgrade.run-manually:

Running ``mysql_upgrade`` manually
--------------------------------------------------------------------------------

When the :program:`mysql` service is started, use :program:`mysql_upgrade` to
migrate to the new grant tables. It will rebuild the indexes and do the
necessary modifications.

The installation script does *NOT* run :command:`mysql_upgrade` automatically as
it was the case in previous versions. You need to run this command manually and
then restart the ``mysql`` service.
   
|tip.run-all.root|

#. Run :program:`mysql_upgrade`

   .. code-block:: bash

      $ mysql_upgrade

   .. admonition:: Output Example

      .. code-block:: guess

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

#. Restart the :program:`mysql` service.

   .. code-block:: bash

      $ service mysql restart

After the service successfully restarts, you can use the new |Percona Server|
5.7.

.. important::

   If you are using the |TokuDB| storage engine, re-enable the storage engine
   plugin before running :program:`mysql_upgrade`. Otherwise, you'll get errors.

   .. code-block:: bash

      $ ps_tokudb_admin --enable

.. _percona-server.inplace-upgrade.standalone:

In-place upgrade using standalone packages
================================================================================

.. include:: .res/text/system-service-manager.support.txt

|tip.run-all.root|

#. Do the full backup (or dump, if possible) a then stop the server. 

   .. code-block:: bash

      $ service mysql stop

#. Modify :ref:`your configuration file as needed <changed_in_57>`.

The following steps are different for DEB based and RPM based
distributions. Depending on your operating system, complete either the
:ref:`percona-server.in-place-upgrade.standalone.deb` or
:ref:`percona-server.in-place-upgrade.standalone.rpm` procedure.

.. contents::
   :local:
      
.. _percona-server.in-place-upgrade.standalone.deb:

In-place upgrade using standalone packages in DEB based distributions 
--------------------------------------------------------------------------------

When installing packages manually, make sure to resolve all the dependencies
and install missing packages yourself. At least, the following packages should
be installed before installing |Percona Server| 5.7:

- ``libmecab2``
- ``libjemalloc1``
- ``zlib1g-dev``
- ``libaio1``

|tip.run-all.root|

#. Remove the installed packages and their dependencies.

   .. code-block:: bash

      $ apt autoremove percona-server-server-5.6 percona-server-client-5.6

#. Download the tarball that contains the packages for your architecture:

   - ``percona-server-server-5.7``
   - ``percona-server-client-5.7``
   - ``percona-server-common-5.7``
   - ``libperconaserverclient20``

   .. code-block:: bash

      $ wget https://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.10-3/binary/debian/jessie/x86_64/Percona-Server-5.7.10-3-r63dafaf-jessie-x86_64-bundle.tar

   Another option is to download only the packages you need for running |Percona
   Server| installation:

   - ``libperconaserverclient20_5.7.10-3-1.jessie_amd64.deb``
   - ``percona-server-client-5.7_5.7.10-3-1.jessie_amd64.deb``
   - ``percona-server-common-5.7_5.7.10-3-1.jessie_amd64.deb``
   - ``percona-server-server-5.7_5.7.10-3-1.jessie_amd64.deb``

.. .. rubric:: An example of installing |Percona Server| :rn:`5.7.10-3` release packages for *Debian* 8.0

#. Unpack the bundle.

   .. code-block:: bash

      $ tar xvf Percona-Server-5.7.10-3-r63dafaf-jessie-x86_64-bundle.tar

   .. admonition:: Extracted DEB files

      .. code-block:: guess

	 libperconaserverclient20-dev_5.7.10-3-1.jessie_amd64.deb
	 libperconaserverclient20_5.7.10-3-1.jessie_amd64.deb
	 percona-server-5.7-dbg_5.7.10-3-1.jessie_amd64.deb
	 percona-server-client-5.7_5.7.10-3-1.jessie_amd64.deb
	 percona-server-common-5.7_5.7.10-3-1.jessie_amd64.deb
	 percona-server-server-5.7_5.7.10-3-1.jessie_amd64.deb
	 percona-server-source-5.7_5.7.10-3-1.jessie_amd64.deb
	 percona-server-test-5.7_5.7.10-3-1.jessie_amd64.deb
	 percona-server-tokudb-5.7_5.7.10-3-1.jessie_amd64.deb

#. Install |Percona Server| (assuming that the current working directory only
   contains the DEB files extracted from the
   :file:`Percona-Server-5.7.10-3-r63dafaf-jessie-x86_64-bundle.tar`)

   .. code-block:: bash

      $ dpkg -i *.deb

You need to install ``percona-server-tokudb-5.7_5.7.10-3-1.jessie_amd64.deb`` if
you intend to use the |TokuDB| storage engine.

.. admonition:: Cross-Reference

  |Percona Server| 5.7 comes with the :ref:`TokuDB storage engine
  <tokudb_intro>`. You can find more information on how to install and enable
  the |TokuDB| storage engine in the :ref:`tokudb_installation` guide.

.. _percona-server.mysql.in-place-upgrade.standalone.rpm:

Upgrading from |MySQL| 5.7 using standalone packages RPM based distributions
----------------------------------------------------------------------------------------------------

#. Check the installed packages:

   .. code-block:: bash

      $ rpm -qa | grep '^mysql-'
  
#. Remove these packages without dependencies:

   .. code-block:: bash

      $ rpm -qa | grep '^mysql-' | xargs rpm -e --nodeps

   .. include:: .res/text/important.package.removing-without-dependency.txt

#. Download the packages of the desired series for your architecture from the
   `Percona Server download page
   <http://www.percona.com/downloads/Percona-Server-5.7/>`_. The easiest way is
   to download a bundle which contains all the packages.

   .. admonition:: An example to download |Percona Server| 5.7.10-3 release packages for |centos| 7

      .. code-block:: bash

	 $ wget https://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.10-3/binary/redhat/7/x86_64/Percona-Server-5.7.10-3-r63dafaf-el7-x86_64-bundle.tar

#. Extract the packages from the bundle:

   .. code-block:: bash

      $ tar xvf Percona-Server-5.7.10-3-r63dafaf-el7-x86_64-bundle.tar

   .. admonition:: Extracted RPM files

      .. code-block:: guess

	 Percona-Server-57-debuginfo-5.7.10-3.1.el7.x86_64.rpm
	 Percona-Server-client-57-5.7.10-3.1.el7.x86_64.rpm
	 Percona-Server-devel-57-5.7.10-3.1.el7.x86_64.rpm
	 Percona-Server-server-57-5.7.10-3.1.el7.x86_64.rpm
	 Percona-Server-shared-57-5.7.10-3.1.el7.x86_64.rpm
	 Percona-Server-shared-compat-57-5.7.10-3.1.el7.x86_64.rpm
	 Percona-Server-test-57-5.7.10-3.1.el7.x86_64.rpm
	 Percona-Server-tokudb-57-5.7.10-3.1.el7.x86_64.rpm

#. Install |Percona Server| 5.7.

   .. code-block:: bash

      $ rpm -ivh Percona-Server-server-57-5.7.10-3.1.el7.x86_64.rpm \
      Percona-Server-client-57-5.7.10-3.1.el7.x86_64.rpm \
      Percona-Server-shared-57-5.7.10-3.1.el7.x86_64.rpm

   To install the :ref:`TokuDB <tokudb_intro>` storage engine, add the
   ``Percona-Server-tokudb-57-5.7.10-3.1.el7.x86_64.rpm`` to the command
   above.

   .. admonition:: Cross-Reference

      You can find more information on how to install and enable the
      |TokuDB| storage in the :ref:`tokudb_installation` guide.

   To install all the packages from the bundle (for debugging, testing, etc.),
   run :program:`rpm` as follows (assuming that the current working directory
   only contains the RPM files extracted from the
   :file:`Percona-Server-5.7.10-3-r63dafaf-el7-x86_64-bundle.tar`)

   .. code-block:: bash

      $ rpm -ivh *.rpm


.. |tip.run-all.root|  replace:: Run the following commands as root or by using the :program:`sudo` command
.. |red-hat-enterprise| replace:: Red Hat Enterprise
.. |centos| replace:: CentOS
.. |debian| replace:: Debian
.. |ubuntu| replace:: Ubuntu
.. |mysql| replace:: MySQL
