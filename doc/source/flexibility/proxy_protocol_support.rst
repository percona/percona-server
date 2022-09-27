.. _proxy_protocol_support:

============================
 Support for PROXY protocol
============================

The proxy protocol allows an intermediate proxying server speaking proxy protocol (ie. HAProxy) between the server and the ultimate client (i.e. mysql client etc) to provide the source client address to the server, which normally would only see the proxying server address instead.

As the proxy protocol amounts to spoofing the client address, it is disabled by default, and can be enabled on per-host or per-network basis for the trusted source addresses where trusted proxy servers are known to run. Unproxied connections are not allowed from these source addresses.

.. note:: 

   You need to ensure proper firewall ACL's in place when this feature is enabled. 

Proxying is supported for TCP over IPv4 and IPv6 connections only. UNIX socket connections can not be proxied and do not fall under the effect of proxy-protocol-networks='*'.

As a special exception, it is forbidden for the proxied IP address to be ``127.0.0.1`` or ``::1``.

Version Specific Information
============================

  * `8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7.

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

This variable is a global-only, read-only variable, which is either a ``*`` (to enable proxying globally, a non-recommended setting), or a list of comma-separated IPv4 and IPv6 network and host addresses, for which proxying is enabled. Network addresses are specified in CIDR notation, i.e. ``192.168.0.0/24``. To prevent source host spoofing, the setting of this variable must be as restrictive as possible to include only trusted proxy hosts.

Related Reading
===============

  * `PROXY protocol specification <http://www.haproxy.org/download/1.5/doc/proxy-protocol.txt>`_

