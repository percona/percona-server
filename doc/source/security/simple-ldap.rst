.. _simple-ldap:


======================================================
Using Simple LDAP Authentication
======================================================

This feature was implemented in :ref:`8.0.19-10`.

LDAP (Lightweight Directory Access Protocol) provides an alternative method to
access existing directory servers, which maintain information about
individuals, groups, and organizations.

The Percona Simple LDAP plugin is a free and Open Source implementation of the
MySQL Enterprise Simple LDAP plugin.

.. rubric:: Install the plugin

Install the plugin with the following command::

    mysql> INSTALL PLUGIN authentication_ldap_simple SONAME 'authentication_ldap_simple.so';

The installation adds the following variables:

.. list-table::
    :widths: 20 15 15 6 6 10 5 8
    :header-rows: 1

    * - Variable Name
      - Description
      - Default
      - Minimum
      - Maximum
      - Scope
      - Dynamic
      - Type
    * - authentication_ldap_simple_bind_base_dn
      - Base distinguished name (DN)
      -
      -
      -
      - global
      - Yes
      - string
    * - authentication_ldap_simple_bind_root_dn
      - Root distinguished name (DN)
      -
      -
      -
      - global
      - yes
      - string
    * - authentication_ldap_simple_bind_root_pwd
      - Password for the root distinguished name
      - 
      -
      -
      - global
      - yes
      - string
    * - authentication_ldap_simple_ca_path
      - Absolute path of the certificate authority file
      -
      -
      -
      - global
      - yes
      - string
    * - authentication_ldap_simple_group_search_attr
      - Name of the attribute that specifies the group names in LDAP directory
        entries
      - cn
      -
      -
      - global
      - yes
      - string
    * - authentication_ldap_simple_group_search_filter
      - Custom group search filter
      - (|(&(objectClass=posixGroup)(memberUid={UA}))(&(objectClass=group)(member={UD})))
      -
      -
      - global
      - yes
      - string
    * - authentication_ldap_simple_init_pool_size
      - Initial size of the connection pool to the LDAP server
      - 10
      - 1
      - 32767
      - global
      - yes
      - uint
    * - authentication_ldap_simple_log_status
      - logging level
      - 1
      - 1
      - 5
      - global
      - yes
      - uint
    * - authentication_ldap_simple_max_pool_size
      - Maximum size of the pool of connections to the LDAP server
      - 1000
      - 1
      - 32767
      - global
      - yes
      - uint
    * - authentication_ldap_simple_server_host
      - LDAP server host
      -
      -
      -
      - global
      - yes
      - string
    * - authentication_ldap_simple_server_port
      - LDAP server TCP/IP port number
      - 389
      - 1
      - 65535
      - global
      - yes
      - uint
    * - authentication_ldap_simple_ssl
      - If plugin connections to the LDAP server use the SSL protocol (ldaps://)
      - OFF
      -
      -
      - global
      - yes
      - bool
    * - authentication_ldap_simple_tls
      - If plugin connections to the LDAP server are secured with STARTTLS (ldap://)
      - OFF
      -
      -
      - global
      - yes
      - bool
    * - authentication_ldap_simple_user_search_attr
      - Name of the attribute that specifies user names in LDAP directory
        entries
      - uid
      -
      -
      - global
      - yes
      - string


For simple LDAP authentication, you must specify the authentication_ldap_simple
plugin in the ``CREATE USER`` statement or the ``ALTER USER`` statement.::

    mysql> CREATE USER ... IDENTIFIED WITH authentication_ldap_simple;

or ::

    mysql> CREATE USER ... IDENTIFIED WITH authentication_ldap_simple BY 'cn=[user
    name],ou=[organization unit],dc=[domain component],dc=com'

.. note::

    If the user is created with the "BY 'cn,ou,dc,dc'" the following variables
    are not used:

    * authentication_ldap_simple_bind_base_dn
    * authentication_ldap_simple_bind_root_dn
    * authentication_ldap_simple_bind_root_pwd
    * authentication_ldap_simple_user_search_attr
    * authentication_ldap_simple_group_search_attr

    If the user is created with "IDENTIFIED BY authentication_ldap_simple" the
    listed variables are used.

If a MySQL user `rshimek` has the following entry in the LDAP directory:

.. code-block:: text

    uid=rshimek, ou=users, dc=hr, dc=com

To create a MySQL account for `rshimek`, use the following statement:

.. code-block:: MySQL

    CREATE USER 'rshimek'@'localhost'
    IDENTIFIED WITH authentication_ldap_simple
    AS 'uid=rshimek,ou=users,dc=hr,dc=com';

.. note::

    **Security** The plugin requires sending the password in clear text.

.. seealso::

    `Client-Side Cleartext Pluggable Authentication <https://dev.mysql.com/doc/refman/8.0/en/cleartext-pluggable-authentication.html>`_

.. rubric:: Uninstall the plugin

To uninstall the plugin, run the following command::

    mysql> UNINSTALL PLUGIN authentication_ldap_simple;

.. seealso::

    `LDAP Pluggable Authentication
    <https://dev.mysql.com/doc/refman/8.0/en/ldap-pluggable-authentication.html>`_
