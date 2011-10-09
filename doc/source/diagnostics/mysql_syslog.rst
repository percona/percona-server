.. _mysql_syslog:

======================================
 Log All Client Commands (``syslog``)
======================================

When enabled, this feature causes all commands run on the client to be logged to ``syslog``.


Version Specific Information
============================

  * 5.1.49-12.0:
    Full functionality available.

Other Information
=================

  * Author / Origin:
    Percona

Client Variables
================

.. variable:: syslog

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Boolean
     :default: OFF
     :range: ON/OFF

The variable enables (ON)/disables (OFF) logging to syslog.


Other Reading
=============

  * http://en.wikipedia.org/wiki/Syslog

  * http://tools.ietf.org/html/rfc5424
