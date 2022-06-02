.. _proxy_protocol_support:

============================
 Support for PROXY protocol
============================

The proxy protocol transports connection information in a safe way to an
intermediate proxy server (for example, HAProxy) between a server and a client
(i.e., mysql client, etc.). Since the proxy protocol is a way to spoof the
client address, the proxy protocol is disabled by default. The protocol can be
enabled on a per-host or a per-network basis for the trusted source addresses
where trusted proxy servers are known to run.

Unproxied connections are not allowed from these source addresses.

.. note::

   You need to ensure proper firewall Access Control List (ACL) is in place
   when this feature is enabled.
   
Proxying is supported for TCP over IPv4 and IPv6 connections only. UNIX socket connections can not be proxied and do not fall under the effect of proxy-protocol-networks='*'.
   
As a special exception, it is forbidden for the proxied IP address to be either ``127.0.0.1`` or ``::1``.

Version Specific Information
============================

  * :ref:`5.7.10-1`:
    Feature ported from *Percona Server for MySQL* 5.6

System Variables
================

.. _proxy_protocol_networks:

.. rubric:: ``proxy_protocol_networks``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Default
     - ``(empty string)``

This variable is a global-only, read-only variable. The available values are:

* Empty string, which is the default

* List of comma-separated IPv4 network and host addresses, or IPv6 network and host addresses. Network addresses are specified in CIDR notation, i.e. ``192.168.0.0/24``.

* An ``*`` (asterisk) allows the proxy headers from any account. This setting is not recommended because this setting may compromise security.

To prevent source host spoofing, the setting of this variable must be as restrictive as possible to include only trusted proxy hosts.

.. note::

    If the `proxy_protocol_networks` is set to a value that is not ``*``, you
    must add ``bind_address`` with the MySQL server IP in my.cnf.

    If you set the proxy_protocol_networks to an IPv4-mapped address, the
    variable works without ``bind_address``.

Related Reading
===============

  * `PROXY protocol specification <http://www.haproxy.org/download/1.5/doc/proxy-protocol.txt>`_

