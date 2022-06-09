.. _user-manual:

==========================================================
 *Percona PAM authentication plugin for MySQL* User Manual
==========================================================

.. toctree::
   :maxdepth: 1
   :hidden:

Configuring PAM for MySQL
=========================

You will need to configure PAM on your system for how it should authenticate for MySQL. A simple setup can be to use the standard UNIX authentication method.

*NOTE:* Using pam_unix means the MySQL Server needs to read the `/etc/shadow` file, which usually means it has to be run as `root` - usually not a recommended configuration.

A sample `/etc/pam.d/mysqld` file: ::

  auth       required     pam_unix.so
  account    required     pam_unix.so

For added information in the system log, you can expand it to be: ::

  auth       required     pam_warn.so
  auth       required     pam_unix.so audit
  account    required     pam_unix.so audit


Creating A User
===============

You will need to execute `CREATE USER` with specifying the PAM plugin. For example: ::

  CREATE USER 'username'@'host' IDENTIFIED WITH auth_pam;

This creates a user `username` that can connect from `host` and will be authenticated using the PAM plugin. If you are using the `pam_unix` method in PAM (or similar) you will need to have an account for `username` existing on the system.

Supplementary groups support
============================

|Percona Server| has implemented PAM plugin support for supplementary groups. Supplementary or secondary groups are extra groups a specific user is member of. For example user ``joe`` might be a member of groups: ``joe`` (his primary group) and secondary groups ``developers`` and ``dba``. A complete list of groups and users belonging to them can be checked with ``cat /etc/group`` command.

This feature enables using secondary groups in the mapping part of the authentication string, like "``mysql, developers=joe, dba=mark``". Previously only primary groups could have been specified there. If user is a member of both ``developers`` and ``dba``, PAM plugin will map it to the ``joe`` because ``developers`` matches first.

