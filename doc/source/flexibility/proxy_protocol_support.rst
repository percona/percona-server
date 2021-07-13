.. _proxy_protocol_support:

============================
 Support for PROXY protocol
============================


The proxy server provides the benefits of load balancing, and security. Proxy protocol adds a header with connection information to the client's connection to the destination. Use the proxy protocol to pass the client IP address to a destination, which, normally, would only see the proxy server's address.

You can enable the protocol on a per-host or a per-network basis for the trusted source addresses, where trusted proxy servers are known. You should specify from which network the destination expects the proxy headers. The `proxy_protocol_networks` should be set to the proxy address or a dedicated network range. The protocol is supported for TCP over IPv4 and IPv6 connections only. UNIX socket connections can not use the proxy protocol and do not fall under the effect of `proxy-protocol-networks`='*'. As a special exception, it is forbidden for the proxy protocol IP address to be ``127.0.0.1`` or ``::1``.

Connections that do not use the proxy server are not allowed from the proxy-dedicated range. When the `proxy_protocol_networks` is enabled, the client communicates first. When a connection does not use the protocol, the server communicates first with an Initial Handshake packet and the client responds. If `proxy_protocol_networks` is enabled, a dedicated client address cannot connect to the destination without the proxy header. This connection stalls with no response and no error.

.. note:: 

   You need to ensure proper firewall Access Control Lists in place when this feature is enabled. 

Version Specific Information
============================

  * :rn:`5.7.10-1`:
    Feature ported from |Percona Server| 5.6

System Variables
================

.. variable:: proxy_protocol_networks

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :default: ``(empty string)``

This variable is a global-only, read-only variable. The available values are:

* Empty string, which is the default

* List of comma-separated IPv4 network and host addresses, or IPv6 network and host addresses. Network addresses are specified in CIDR notation, i.e. ``192.168.0.0/24``.

* An ``*`` (asterisk) allows the proxy headers from any account. This setting is not recommended because this setting may compromise security.

To prevent source host spoofing, the setting of this variable must be as restrictive as possible to include only trusted proxy hosts.

Related Reading
===============

  * `PROXY protocol specification <http://www.haproxy.org/download/1.5/doc/proxy-protocol.txt>`_

