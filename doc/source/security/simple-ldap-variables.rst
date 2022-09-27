.. _simple_ldap_variables:

=========================================================
Simple LDAP Variables
=========================================================

The following variables are static and can only be changed at runtime.

.. tabularcolumns:: |p{10cm}|p{2cm}|p{2cm}|p{2cm}|

.. list-table::
   :header-rows: 1
   
   * - Name
     - Command Line
     - Dynamic
     - Scope
   * - :ref:`authentication_ldap_simple_bind_root_dn`
     - Yes
     - No
     - Global
   * - :ref:`authentication_ldap_simple_bind_root_pwd`
     - Yes
     - No
     - Global
   * - :ref:`authentication_ldap_simple_ca_path`
     - Yes
     - No
     - Global
   * - :ref:`authentication_ldap_simple_server_host`
     - Yes
     - No
     - Global
   * - :ref:`authentication_ldap_simple_server_port`
     - Yes
     - No
     - Global
   * - :ref:`authentication_ldap_simple_ssl`
     - Yes
     - No
     - Global
   * - :ref:`authentication_ldap_simple_tls`
     - Yes
     - No
     - Global
     
.. _authentication_ldap_simple_bind_root_dn:

.. rubric:: ``authentication_ldap_simple_bind_root_dn``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--authentication-ldap-simple-bind-root-dn=value``
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - Null

The ``root`` credential used to authenticate against an LDAP. This variable is used with
``authentication_ldap_simple_bind_root_pwd``.

.. _authentication_ldap_simple_bind_root_pwd:

.. rubric:: ``authentication_ldap_simple_bind_root_pwd``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--authentication-ldap-simple-bind-root-pwd=value``
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - Null
    
The ``root`` password used to authenticate against an LDAP. This variable is used with
``authentication_ldap_simple_bind_root_dn``.

.. _authentication_ldap_simple_ca_path:

.. rubric:: ``authentication_ldap_simple_ca_path``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--authentication-ldap-simple-ca_path=value``
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - Null
    
The certificate authority's absolute path used to verify the LDAP certificate.

.. _authentication_ldap_simple_server_host:

.. rubric:: ``authentication_ldap_simple_server_host``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--authentication-ldap-simple-server-host=value``
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - Null
    
The LDAP server host used for LDAP authentication.

.. _authentication_ldap_simple_server_port:

.. rubric:: ``authentication_ldap_simple_server_port``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--authentication-ldap-simple-server-port=value``
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - Null
    
The LDAP server TCP/IP port number used for LDAP authentication.

.. _authentication_ldap_simple_ssl:

.. rubric:: ``authentication_ldap_simple_ssl``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--authentication-ldap-simple-ssl=value``
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - Null
    
If this variable is enabled, the plugin connects to the server with SSL.

.. _authentication_ldap_simple_tls:

.. rubric:: ``authentication_ldap_simple_tls``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--authentication-ldap-simple-tls=value``
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - Null
    
If this variable is enabled, the plugin connects to the server with TLS.

.. seealso::

    `Simple LDAP Authentication <https://dev.mysql.com/doc/mysql-security-excerpt/8.0/en/ldap-pluggable-authentication.html#ldap-pluggable-authentication-usage-simple>`_
    
