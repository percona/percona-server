.. _installation:

==============================================
 Installing |Percona Server| 5.1 from Binaries
==============================================

Before installing, you might want to read the :doc:`release-notes/release-notes_index`.

Ready-to-use binaries are available from the |Percona Server| `download page <http://www.percona.com/downloads/Percona-Server-5.1/>`_, including:

 * ``RPM`` packages for *RHEL* 5 and *RHEL* 6

 * *Debian* packages

 * *FreeBSD* ``.tbz`` packages

.. * *Microsoft Windows* binaries

 * Generic ``.tar.gz`` packages

Using Percona Software Repositories
===================================

.. toctree::
   :maxdepth: 1

   installation/apt_repo
   installation/yum_repo

|Percona| provides repositories for :program:`yum` (``RPM`` packages for *Red Hat*, *CentOS*, *Amazon Linux AMI*, and *Fedora*) and :program:`apt` (:file:`.deb` packages for *Ubuntu* and *Debian*) for software such as |Percona Server|, |XtraDB|, |XtraBackup|, and *Maatkit*. This makes it easy to install and update your software and its dependencies through your operating system's package manager.

This is the recommend way of installing where possible.

``YUM``-Based Systems
---------------------

Once the repository is set up, use the following commands: ::

     $ yum install Percona-Server-client-51 Percona-Server-server-51

``DEB``-Based Systems
---------------------

Once the repository is set up, use the following commands: ::

     $ sudo apt-get install percona-server-server-5.1

Using Standalone Packages
=========================

``RPM``-Based Systems
---------------------

Download the packages of the desired series for your architecture from `here <http://www.percona.com/downloads/Percona-Server-5.1/>`_.

For example, at the moment of writing, a way of doing this is: ::

  $ wget -r -l 1 -nd -A rpm -R "*devel*,*debuginfo*"  \ 
  http://www.percona.com/redir/downloads/Percona-Server-5.1/Percona-Server-5.1.58-12.9/RPM/rhel6/x86_64/

Install them in one command: ::

  $ rpm -ivh Percona-Server-server-51-5.1.58-rel12.9.271.rhel6.x86_64.rpm \
  Percona-Server-client-51-5.1.58-rel12.9.271.rhel6.x86_64.rpm \
  Percona-Server-shared-51-5.1.58-rel12.9.271.rhel6.x86_64.rpm

If you don’t install all “at the same time”, you will need to do it in a specific order - ``shared``, ``client``, ``server``: ::

  $ rpm -ivh Percona-Server-shared-51-5.1.58-rel12.9.271.rhel6.x86_64.rpm
  $ rpm -ivh Percona-Server-client-51-5.1.58-rel12.9.271.rhel6.x86_64.rpm
  $ rpm -ivh Percona-Server-server-51-5.1.58-rel12.9.271.rhel6.x86_64.rpm

Otherwise, the dependencies won’t be met and the installation will fail.

What's in each RPM?
~~~~~~~~~~~~~~~~~~~

Each of the |Percona Server| RPM packages have a particular purpose.

The ``Percona-Server-server`` package contains the server itself (the
``mysqld`` binary).

The ``Percona-Server-51-debuginfo`` package contains debug symbols for
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

Download the packages of the desired series for your architecture from `here <http://www.percona.com/downloads/Percona-Server-5.1/>`_.

For example, at the moment of writing, for *Ubuntu* Maverick on ``i686``, a way of doing this is: ::

  $ wget -r -l 1 -nd -A deb -R "*dev*" \
  http://www.percona.com/redir/downloads/Percona-Server-5.1/Percona-Server-5.1.58-12.9/deb/maverick/x86_64/

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

The ``libmysqlclient-dev`` package contains header files needed to
compile software to use the client library.

The ``libmysqlclient16`` package contains the client shared
library. The ``16`` is a reference to the version of the shared
library. The version is incremented when there is a ABI change that
requires software using the client library to be recompiled or their
source code modified.

==================================================
 Installing |Percona Server| from a Source Tarball
==================================================

Fetch and extract the source tarball. For example: ::

  $ wget http://www.percona.com/redir/downloads/Percona-Server-5.1/Percona-Server-5.1.58-12.9/source/Percona-Server-5.1.58.tar.gz
  $ tar xfz Percona-Server-5.1.58.tar.gz

Next, run the configure script. Here you can specify all the normal
build options as you do for a normal |MySQL| build. Depending on what
options you wish to compile Percona Server with, you may need other
libraries installed on your system. Here is an example using a
configure line similar to the options that Percona uses to produce
binaries: ::

  $ ./configure --without-plugin-innobase --with-plugins=partition,archive,blackhole,csv,example,federated,innodb_plugin --without-embedded-server --with-pic --with-extra-charsets=complex --with-ssl --enable-assembler --enable-local-infile --enable-thread-safe-client --enable-profiling --with-readline

Now, compile using make ::

  $ make

Install: ::

  $ make install

========================================================
 Installing |Percona Server| from the Bazaar Source Tree
========================================================

Percona uses the `Bazaar <http://www.bazaar-vcs.org>`_ revision
control system for development. To build the latest Percona Server
from the source tree you will need Bazaar installed on your system.

Good practice is to use a shared repository, create one like this: ::

  $ bzr init-repo ~/percona-server

You can now fetch the latest Percona Server 5.1 sources. In the
future, we will provide instructions for fetching each specific
Percona Server version and building it, but currently we will just
talk about building the latest Percona Server 5.1 development tree. ::

  $ cd ~/percona-server
  $ bzr branch lp:percona-server/5.1

You can now change into the 5.1 directory and build Percona Server
5.1: ::

  $ make

This will fetch the upstream MySQL source tarball and apply the
Percona Server patches to it. If you have the quilt utility installed,
it will use it to apply the patches, otherwise it will just use the
standard patch utility. You will then have a directory named
Percona-Server that is ready to run the configure script and
build. ::

  $ ./configure --without-plugin-innobase --with-plugins=partition,archive,blackhole,csv,example,federated,innodb_plugin --without-embedded-server --with-pic --with-extra-charsets=complex --with-ssl --enable-assembler --enable-local-infile --enable-thread-safe-client --enable-profiling --with-readline
  $ make
  $ make install
