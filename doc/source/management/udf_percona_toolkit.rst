.. _udf_percona_toolkit:

========================
 *Percona Toolkit* UDFs
========================

Three *Percona Toolkit* UDFs that provide faster checksums are provided:

  * ``libfnv1a_udf``

  * ``libfnv_udf``

  * ``libmurmur_udf``

Version Specific Information
============================

  * :rn:`5.6.11-60.3`:
    Began distributing ``libfnv1a_udf``, ``libfnv_udf``, and ``libmurmur_udf``.

Other Information
=================

  * Author / Origin:
    Baron Schwartz

Installation
============

These UDFs are part of the |Percona Server| packages. To install one of the UDFs into the server, execute one of the following commands, depending on which UDF you want to install: ::

  $ mysql -e "CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME ``libfnv1a_udf.so``" 
  $ mysql -e "CREATE FUNCTION fnv_64 RETURNS INTEGER SONAME ``libfnv_udf.so``" 
  $ mysql -e "CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME ``libmurmur_udf.so``"

Executing each of these commands will install its respective UDF into the server.

Troubleshooting
================

If you get the error: :: 

  ERROR 1126 (HY000): Can't open shared library 'fnv_udf.so' (errno: 22 fnv_udf.so: cannot open shared object file: No such file or directory)

Then you may need to copy the .so file to another location in your system. Try both ``/lib`` and ``/usr/lib``. Look at your environment's ``$LD_LIBRARY_PATH`` variable for clues. If none is set, and neither ``/lib`` nor ``/usr/lib`` works, you may need to set ``LD_LIBRARY_PATH`` to ``/lib`` or ``/usr/lib``.

Other Reading
=============

  * *Percona Toolkit* `documentation <http://www.percona.com/doc/percona-toolkit/>`_
