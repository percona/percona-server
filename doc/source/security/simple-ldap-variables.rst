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
   * - :variable:`authentication_ldap_simple_bind_root_dn`
     - Yes
     - No
     - Global
   * - :variable:`authentication_ldap_simple_bind_root_pwd`
     - Yes
     - No
     - Global
   * - :variable:`authentication_ldap_simple_ca_path`
     - Yes
     - No
     - Global
   * - :variable:`authentication_ldap_simple_server_host`
     - Yes
     - No
     - Global
   * - :variable:`authentication_ldap_simple_server_port`
     - Yes
     - No
     - Global
   * - :variable:`authentication_ldap_simple_ssl`
     - Yes
     - No
     - Global
   * - :variable:`authentication_ldap_simple_tls`
     - Yes
     - No
     - Global
     
.. variable:: authentication_ldap_simple_bind_root_dn

    :cli: ``--authentication-ldap-simple-bind-root-dn=value``
    :dyn: No
    :scope: Global
    :vartype: String
    :default: Null
    
The ``root`` credential used to authenticate against an LDAP. This variable is used with
``authentication_ldap_simple_bind_root_pwd``.

.. variable:: authentication_ldap_simple_bind_root_pwd

    :cli: ``--authentication-ldap-simple-bind-root-pwd=value``
    :dyn: No
    :scope: Global
    :vartype: String
    :default: Null
    
The ``root`` password used to authenticate against an LDAP. This variable is used with
``authentication_ldap_simple_bind_root_dn``.

.. variable:: authentication_ldap_simple_ca_path

    :cli: ``--authentication-ldap-simple-ca_path=value``
    :dyn: No
    :scope: Global
    :vartype: String
    :default: Null
    
The certificate authority's absolute path used to verify the LDAP certificate.

.. variable:: authentication_ldap_simple_server_host

    :cli: ``--authentication-ldap-simple-server-host=value``
    :dyn: No
    :scope: Global
    :vartype: String
    :default: Null
    
The LDAP server host used for LDAP authentication.

.. variable:: authentication_ldap_simple_server_port

    :cli: ``--authentication-ldap-simple-server-port=value``
    :dyn: No
    :scope: Global
    :vartype: String
    :default: Null
    
The LDAP server TCP/IP port number used for LDAP authentication.

.. variable:: authentication_ldap_simple_ssl

    :cli: ``--authentication-ldap-simple-ssl=value``
    :dyn: No
    :scope: Global
    :vartype: String
    :default: Null
    
If this variable is enabled, the plugin connects to the server with SSL.

.. variable:: authentication_ldap_simple_tls

    :cli: ``--authentication-ldap-simple-tls=value``
    :dyn: No
    :scope: Global
    :vartype: String
    :default: Null
    
If this variable is enabled, the plugin connects to the server with TLS.

.. seealso::

    `Simple LDAP Authentication <https://dev.mysql.com/doc/mysql-security-excerpt/8.0/en/ldap-pluggable-authentication.html#ldap-pluggable-authentication-usage-simple>`_
    
