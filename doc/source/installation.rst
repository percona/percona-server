===========================================
 Installing |Percona Server| from Binaries
===========================================

Before installing, you might want to read the :doc:`release-notes`.

Ready-to-use binaries are available from the |Percona Server| `download page <http://www.percona.com/downloads/Percona-Server-5.5/>`_, including:

 * ``RPM`` packages for *RHEL* 4 and *RHEL* 5

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

Once the repository is set up, use the following commands:

  * For 5.0 series: ::

      yum install Percona-SQL-client-50 Percona-SQL-server-50

  * For 5.1 series: ::

      yum install Percona-Server-client-51 Percona-Server-server-51

  * For 5.5 series: ::

      yum install Percona-Server-client-55 Percona-Server-server-55
     
``DEB``-Based Systems
---------------------

Once the repository is set up, use the following commands:

  * For 5.0 series: ::
      
      sudo apt-get install percona-server-server-5.0

  * For 5.1 series: ::
      
      sudo apt-get install percona-server-server-5.1

  * For 5.5 series: ::
  
      sudo apt-get install percona-server-server-5.5

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

and all dependencies will be installed and the Percona Server installation will be finished by :command:`apt`.
