=======================================================
 Installing Percona PAM Authentication Plugin for MySQL
=======================================================

.. toctree::
   :hidden:

Compiling from Source
=====================

You will need both the PAM headers and the MySQL 5.5 headers and corresponding `mysql_config` binary available on your system.

If you are not using one of the pre-built binary packages, you will need to compile the plugin from source. You can either use a source tarball or the source repository.

For getting a copy of the latest development bzr tree: ::

  $ bzr branch lp:percona-pam-for-mysql

If you are building from bzr, you will need to generate the configure script: ::

  $ ./bootstrap

You do not need to run `bootstrap` if you are using a source tarball.

You then need to build the plugin: ::

  $ ./configure
  $ make

To install, you can simply run (as root or using sudo or similar): ::

  $ make install

Installing server-side plugin
=============================

The shared library that holds the plugin, auth_pam.so, needs to be stored in the plugindir directory of mysql. You can get this value via the command: ::

  $ mysql_config --plugindir

Make sure that after installed, the library has got the appropiate permissions (file execution is required).

Most packages should do this for you, so this is likely only required with the binary tarballs.

In order to load the plugin into the working server, issue the following command: ::

 mysql> INSTALL PLUGIN auth_pam SONAME 'auth_pam.so';


You can now create a PAM configuration for the MySQL server and create users that are authenticated by PAM.
