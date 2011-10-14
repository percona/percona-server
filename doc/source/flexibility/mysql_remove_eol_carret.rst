.. _mysql_remove_eol_carret:

=============================
 Handle ``BLOB`` End of Line
=============================

At some point in the past, the |MySQL| command line client was modified to remove ``\r`` before ``\n`` in its input.

This caused problems in some workloads, specifically when loading ``BLOB`` fields containing ``\r`` characters. |Percona Server| solves this by implementing a new command line client option, :variable:`no-remove-eol-carret`.

When the :variable:`no-remove-eol-carret` option is specified, ``\r`` before ``\n`` is not removed.


Version Specific Information
============================

  * 5.1.50-12.1:
    Full functionality.

Client Command Line Parameter
=============================

.. variable:: no-remove-eol-carret

     :cli: Yes
     :conf: Yes
     :scope: Local
     :dyn: No
     :vartype: Boolean
     :default: Off
     :range: On/Off
