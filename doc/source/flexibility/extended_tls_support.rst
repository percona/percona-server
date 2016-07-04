.. _extended_tls_support:

=============================
Support for TLS v1.1 and v1.2
=============================

|Percona Server| has implemented TLS v1.1 and v1.2 protocol support and at the same time disabled TLS v1.0 support (support for TLS v1.0 can be enabled by adding the ``TLSv1`` to :variable:`tls_version` variable). Support for TLS v1.1 and v1.2 protocols has been implemented by porting the :variable:`tls_version` variable from 5.7 server. TLS v1.0 protocol has been disabled because it will no longer be viable for `PCI after June 30th 2016 <https://www.pcisecuritystandards.org/documents/Migrating_from_SSL_Early_TLS_Information%20Supplement_v1.pdf>`_. Variable default has been changed from ``TLSv1,TLSv1.1,TLSv1.2`` to ``TLSv1.1,TLSv1.2`` to disable the support for TLS v1.0 by default. 

The client-side has the ability to make TLSv1.1 and 1.2 connections, but the option to allow only some protocol versions (``--tls-version``, ``MYSQL_OPT_TLS_VERSION`` in C API) has not been backported due to compatibility concerns and relatively easy option to use 5.7 clients instead if needed. **Note:** ``MASTER_TLS_VERSION`` clause of ``CHANGE MASTER TO`` statement has not been backported.

Version Specific Information
============================

  * :rn:`5.6.31-77.0`:
    Implemented support for TLS v1.1 and TLS v1.2 protocols

System Variables
================

.. variable:: tls_version

     :version 5.6.31-77.0: Introduced
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: ``TLSv1.1,TLSv1.2``

This variable defines protocols permitted by the server for encrypted connections. 

.. variable:: have_tlsv1_2

     :version 5.6.31-77.0: Introduced
     :cli: Yes
     :conf: No
     :scope: Global
     :dyn: No
     :vartype: Boolean 

This server variable is set to ``ON`` if the server has been compiled with a SSL library providing TLSv1.2 support.
