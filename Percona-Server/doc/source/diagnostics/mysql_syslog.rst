.. _mysql_syslog:

======================================
 Log All Client Commands (``syslog``)
======================================

When enabled, this feature causes all commands run by the command line client to be logged to syslog. If you want to enable this option permanently, add it to the [mysql] group in my.cnf.

Version Specific Information
============================

  * :rn:`5.6.11-60.3`:
    Feature ported from |Percona Server| 5.5.

Other Information
=================

  * Author / Origin:
    Percona

Client Variables
================

.. variable:: syslog

     :cli: Yes
     :conf: Yes
     :server: No
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
