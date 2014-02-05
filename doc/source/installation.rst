.. _installation:

===============================================
 Installing |Percona Server| 5.5 from Binaries
===============================================

Before installing, you might want to read the :doc:`release-notes/release-notes_index`.

Ready-to-use binaries are available from the |Percona Server| `download page <http://www.percona.com/downloads/Percona-Server-5.5/>`_, including:

 * ``RPM`` packages for *RHEL* 5 and *RHEL* 6

 * *Debian* packages for *Debian* and *Ubuntu*

.. * *Microsoft Windows* binaries

 * Generic ``.tar.gz`` packages

Using Percona Software Repositories
===================================

.. toctree::
   :maxdepth: 1

   installation/apt_repo
   installation/yum_repo

|Percona| provides repositories for :program:`yum` (``RPM`` packages for *Red Hat*, *CentOS* and *Amazon Linux AMI*) and :program:`apt` (:file:`.deb` packages for *Ubuntu* and *Debian*) for software such as |Percona Server|, |XtraDB|, |XtraBackup|, and *Percona Toolkit*. This makes it easy to install and update your software and its dependencies through your operating system's package manager.

This is the recommend way of installing where possible.

``YUM``-Based Systems
---------------------

Once the repository is set up, use the following commands: ::

      $ yum install Percona-Server-client-55 Percona-Server-server-55

``DEB``-Based Systems
---------------------

Once the repository is set up, use the following commands: ::

      $ sudo apt-get install percona-server-server-5.5

Using Standalone Packages
=========================

``RPM``-Based Systems
---------------------

Download the packages of the desired series for your architecture from `here <http://www.percona.com/downloads/Percona-Server-5.5/>`_.

For example, at the moment of writing, a way of doing this is: ::

  $ wget -r -l 1 -nd -A rpm -R "*devel*,*debuginfo*"  \ 
  http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.14-20.5/RPM/rhel5/i686/

Install them in one command: ::

  $ rpm -ivh Percona-Server-server-55-5.5.14-rel20.5.149.rhel5.i686.rpm \
  Percona-Server-client-55-5.5.14-rel20.5.149.rhel5.i686.rpm \
  Percona-Server-shared-55-5.5.14-rel20.5.149.rhel5.i686.rpm

If you don’t install all “at the same time”, you will need to do it in a specific order - ``shared``, ``client``, ``server``: ::

  $ rpm -ivh Percona-Server-shared-55-5.5.14-rel20.5.149.rhel5.i686.rpm
  $ rpm -ivh Percona-Server-client-55-5.5.14-rel20.5.149.rhel5.i686.rpm
  $ rpm -ivh Percona-Server-server-55-5.5.14-rel20.5.149.rhel5.i686.rpm

Otherwise, the dependencies won’t be met and the installation will fail.

What's in each RPM?
~~~~~~~~~~~~~~~~~~~

Each of the |Percona Server| RPM packages have a particular purpose.

The ``Percona-Server-server`` package contains the server itself (the
``mysqld`` binary).

The ``Percona-Server-55-debuginfo`` package contains debug symbols for
use debugging the database server.

The ``Percona-Server-client`` package contains the command line
client.

The ``Percona-Server-devel`` package contains the header files needed
to compile software using the client library.

The ``Percona-Server-shared`` package includes the client shared
library.

The ``Percona-Server-shared-compat`` package includes shared libraries
for software compiled against old versions of the client library.

The ``Percona-Server-test`` package includes the test suite for
|Percona Server|.

``DEB``-Based Systems
---------------------

Download the packages of the desired series for your architecture from `here <http://www.percona.com/downloads/Percona-Server-5.5/>`_.

For example, at the moment of writing, for *Ubuntu* Maverick on ``i686``, a way of doing this is: ::

  $ wget -r -l 1 -nd -A deb -R "*dev*" \
  http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.14-20.5/deb/maverick/x86_64/

Install them in one command: ::

  $ sudo dpkg -i *.deb

The installation won’t succeed as there will be missing dependencies. To handle this, use: ::

  $ apt-get -f install

and all dependencies will be installed and the Percona Server
installation will be finished by :command:`apt`.

What's in each DEB package?
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``percona-server-server`` package contains the database server
itself, the ``mysqld`` binary and associated files.

The ``percona-server-common`` package contains files common to the
server and client.

The ``percona-server-client`` package contains the command line
client.

The ``percona-server-dfsg`` package contains....

The ``libperconaserverclient-dev`` package contains header files needed to
compile software to use the client library.

The ``libperconaserverclient18`` package contains the client shared
library. The ``18`` is a reference to the version of the shared
library. The version is incremented when there is a ABI change that
requires software using the client library to be recompiled or their
source code modified.

===================================================
 Installing |Percona Server| from a Source Tarball
===================================================

Fetch and extract the source tarball. For example: ::

  $ wget http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.15-21.0/source/Percona-Server-5.5.15-rel21.0.tar.gz
  $ tar xfz Percona-Server-5.5.15-rel21.0.tar.gz

Next, follow the instructions in :ref:`compile_from_source` below.

.. _source-from-bzr
=========================================================
 Installing |Percona Server| from the Bazaar Source Tree
=========================================================

Percona uses the `Bazaar <http://bazaar.canonical.com/en/>`_ revision
control system for development. To build the latest Percona Server
from the source tree you will need Bazaar installed on your system.

Good practice is to use a shared repository, create one like this: ::

  $ bzr init-repo ~/percona-server

You can now fetch the latest Percona Server 5.5 sources. In the
future, we will provide instructions for fetching each specific
Percona Server version and building it, but currently we will just
talk about building the latest Percona Server 5.5 development tree. ::

  $ cd ~/percona-server
  $ bzr branch lp:percona-server/5.5

Fetching all the history of Percona Server 5.5 may take a long time,
up to 20 or 30 minutes is not uncommon.

If you are going to be making changes to Percona Server 5.5 and wanting
to distribute the resulting work, you can generate a new source tarball
(exactly the same way as we do for release): ::

  $ cmake .
  $ make dist

Next, follow the instructions in :ref:`compile_from_source` below.

.. _compile_from_source:

=======================================
 Compiling |Percona Server| from Source
=======================================

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

=================================================
 Building |Percona Server| Debian/Ubuntu packages
=================================================

If you wish to build your own Percona Server Debian/Ubuntu (dpkg) packages,
you first need to start with a source tarball, either from the Percona
website or by generating your own by following the instructions above(
:ref:`source-from-bzr`).

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
