.. _pam_plugin:

===========================
 PAM Authentication Plugin
===========================

Percona PAM Authentication Plugin is a free and Open Source implementation of the |MySQL|'s authentication plugin. This plugin acts as a mediator between the |MySQL| server, the |MySQL| client, and the PAM stack. The server plugin requests authentication from the PAM stack, forwards any requests and messages from the PAM stack over the wire to the client (in cleartext) and reads back any replies for the PAM stack.

 PAM plugin uses dialog as its client side plugin. Dialog plugin can be loaded to any client application that uses :file:`libperconaserverclient`/:file:`libperconaserverclient` library.

Here are some of the benefits that Percona dialog plugin offers over the default one:

  * It correctly recognizes whether PAM wants input to be echoed or not, while the default one always echoes the input on the user's console.
  * It can use the password which is passed to |MySQL| client via "-p" parameter.
  * Dialog client `installation bug <http://bugs.mysql.com/bug.php?id=60745>`_ has been fixed.
  * This plugin works on |MySQL| and |Percona Server|.

Percona offers two versions of this plugin:  

  * Full PAM plugin called *auth_pam*. This plugin uses *dialog.so*. It fully supports the PAM protocol with arbitrary communication between client and server.
  * Oracle-compatible PAM called *auth_pam_compat*. This plugin uses *mysql_clear_password* which is a part of Oracle MySQL client. It also has some limitations, such as, it supports only one password input. You must use ``-p`` option in order to pass the password to *auth_pam_compat*.

These two versions of plugins are physically different. To choose which one you want used, you must use *IDENTIFIED WITH 'auth_pam'* for auth_pam, and *IDENTIFIED WITH 'auth_pam_compat'* for auth_pam_compat.

Installation
============

This plugin requires manual installation because it isn't installed by default. :: 

 mysql> INSTALL PLUGIN auth_pam SONAME 'auth_pam.so';
 
After the plugin has been installed it should be present in the plugins list. To check if the plugin has been correctly installed and active :: 

 mysql> SHOW PLUGINS;
 ...
 ...
 | auth_pam                       | ACTIVE   | AUTHENTICATION     | auth_pam.so | GPL     |

Configuration
=============

In order to use the plugin, authentication method should be configured. Simple setup can be to use the standard UNIX authentication method (``pam_unix``).

.. note:: 

  To use ``pam_unix``, mysql will need to be added to the shadow group in order to have enough privileges to read the /etc/shadow.

A sample `/etc/pam.d/mysqld` file: ::

  auth       required     pam_unix.so
  account    required     pam_unix.so

For added information in the system log, you can expand it to be: ::

  auth       required     pam_warn.so
  auth       required     pam_unix.so audit
  account    required     pam_unix.so audit

Creating a user
================

After the PAM plugin has been configured, users can be created with the PAM plugin as authentication method :: 

  mysql> CREATE USER 'newuser'@'localhost' IDENTIFIED WITH auth_pam;

This will create a user ``newuser`` that can connect from ``localhost`` who will be authenticated using the PAM plugin. If the ``pam_unix`` method is being used user will need to exist on the system.

Supplementary groups support
============================

|Percona Server| has implemented PAM plugin support for supplementary groups. Supplementary or secondary groups are extra groups a specific user is member of. For example user ``joe`` might be a member of groups: ``joe`` (his primary group) and secondary groups ``developers`` and ``dba``. A complete list of groups and users belonging to them can be checked with ``cat /etc/group`` command.

This feature enables using secondary groups in the mapping part of the authentication string, like "``mysql, developers=joe, dba=mark``". Previously only primary groups could have been specified there. If user is a member of both ``developers`` and ``dba``, PAM plugin will map it to the ``joe`` because ``developers`` matches first. 

Version Specific Information
============================

  * :rn:`5.6.11-60.3`
    PAM authentication plugin has been integrated with |Percona Server|.
  
  * :rn:`5.6.12-60.4`
    Implemented PAM support for supplementary groups.

