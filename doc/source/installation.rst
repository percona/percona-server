.. _installation:

===============================
Installing |Percona Server| 5.5
===============================

This page provides the information on how to you can install |Percona Server|. Following options are available: 

* :ref:`installing_from_binaries` (recommended)
* Installing |Percona Server| from Downloaded `:ref:`rpm <standalone_rpm>` or :ref:`apt <standalone_deb>` Packages
* :ref:`installing_from_binary_tarball`
* :ref:`installing_from_source_tarball`
* :ref:`source_from_git`
* :ref:`compile_from_source`

Before installing, you might want to read the :doc:`release-notes/release-notes_index`.

.. _installing_from_binaries:

Installing |Percona Server| from Repositories
=============================================

|Percona| provides repositories for :program:`yum` (``RPM`` packages for *Red Hat*, *CentOS* and *Amazon Linux AMI*) and :program:`apt` (:file:`.deb` packages for *Ubuntu* and *Debian*) for software such as |Percona Server|, |Percona XtraBackup|, and *Percona Toolkit*. This makes it easy to install and update your software and its dependencies through your operating system's package manager. This is the recommend way of installing where possible.

Following guides describe the installation process for using the official Percona repositories for :file:`.deb` and :file:`.rpm` packages.

.. toctree::
   :maxdepth: 1
   :titlesonly:

   installation/apt_repo 
   installation/yum_repo 

.. _installing_from_binary_tarball:

Installing |Percona Server| from a Binary Tarball
===================================================

In |Percona Server| :rn:`5.5.43-37.2` and newer, the single binary tarball was replaced with multiple tarballs depending on the *OpenSSL* library available in the distribution:

 * ssl100 - for all *Debian/Ubuntu* versions except *Squeeze* (``libssl.so.1.0.0 => /usr/lib/x86_64-linux-gnu/libssl.so.1.0.0 (0x00007f2e389a5000)``);
 * ssl098 - only for *Debian Squeeze* (``libssl.so.0.9.8 => /usr/lib/libssl.so.0.9.8 (0x00007f9b30db6000)``);
 * ssl101 - for *CentOS* 6 and *CentOS* 7 (``libssl.so.10 => /usr/lib64/libssl.so.10 (0x00007facbe8c4000)``);
 * ssl098e - to be used only for *CentOS* 5 (``libssl.so.6 => /lib64/libssl.so.6 (0x00002aed5b64d000)``).

You can download the binary tarballs from the ``Linux - Generic`` `section <http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.43-37.2/binary/tarball/>`_ on the download page.

Fetch and extract the correct binary tarball. For example for *Debian Wheezy*: 

.. code-block:: bash

  $ wget http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.43-37.2/binary/tarball/Percona-Server-5.5.43-rel37.2-Linux.x86_64.ssl100.tar.gz


.. _installing_from_source_tarball:

Installing |Percona Server| from a Source Tarball
=================================================

Fetch and extract the source tarball. For example: ::

  $ wget http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.15-21.0/source/Percona-Server-5.5.15-rel21.0.tar.gz
  $ tar xfz Percona-Server-5.5.15-rel21.0.tar.gz

Next, follow the instructions in :ref:`compile_from_source` below.

.. _source_from_git:

Installing |Percona Server| from the Git Source Tree
====================================================

Percona uses the `Github <http://github.com/>`_ revision
control system for development. To build the latest |Percona Server|
from the source tree you will need ``git`` installed on your system.

You can now fetch the latest |Percona Server| 5.5 sources. 

.. code-block:: bash

  $ git clone https://github.com/percona/percona-server.git
  $ cd percona-server
  $ git checkout 5.5

If you are going to be making changes to |Percona Server| 5.5 and wanting
to distribute the resulting work, you can generate a new source tarball
(exactly the same way as we do for release): ::

  $ cmake .
  $ make dist

Next, follow the instructions in :ref:`compile_from_source` below.

.. _compile_from_source:

Compiling |Percona Server| from Source
======================================

After either fetching the source repository or extracting a source tarball
(from Percona or one you generated yourself), you will now need to
configure and build Percona Server.

First, run cmake to configure the build. Here you can specify all the normal
build options as you do for a normal |MySQL| build. Depending on what
options you wish to compile Percona Server with, you may need other
libraries installed on your system. Here is an example using a
configure line similar to the options that Percona uses to produce
binaries: ::

  $ cmake . -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_CONFIG=mysql_release -DFEATURE_SET=community -DWITH_EMBEDDED_SERVER=OFF

Now, compile using make ::

  $ make

Install: ::

  $ make install

Percona Server 5.5 will now be installed on your system.

Building |Percona Server| Debian/Ubuntu packages
================================================

If you wish to build your own Percona Server Debian/Ubuntu (dpkg) packages,
you first need to start with a source tarball, either from the Percona
website or by generating your own by following the instructions above(
:ref:`source_from_git`).

Extract the source tarball: ::

  $ tar xfz percona-server-5.5.34-32.0.tar.gz
  $ cd percona-server-5.5.34-32.0

Put the debian packaging in the directory that Debian expects it to be in: ::

  $ cp -ap build-ps/debian debian

Update the changelog for your distribution (here we update for the unstable
distribution - sid), setting the version number appropriately. The trailing one
in the version number is the revision of the Debian packaging. ::

  $ dch -D unstable --force-distribution -v "5.5.34-32.0-1" "Update to 5.5.34-32.0"

Build the Debian source package: ::

  $ dpkg-buildpackage -S

Use sbuild to build the binary package in a chroot: ::

  $ sbuild -d sid percona-server-5.5_5.5.34_32.0-1.dsc

You can give different distribution options to dch and sbuild to build binary
packages for all Debian and Ubuntu releases.


.. note::

  :ref:`pam_plugin` has been merged into |Percona Server| in :rn:`5.5.24-26.0` but it is not built with the server by default. In order to build the Percona Server with PAM plugin, additional option :option:`-DWITH_PAM=ON` should be used.
