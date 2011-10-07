.. _udf_maatkit:

======================================
 *Percona Toolkit* and *Maatkit* UDFs
======================================

Three *Percona Toolkit* and *Maatkit* UDFs that provide faster checksums are provided:

  * ``fnv_udf``

  * ``fnv1a_udf``

  * ``murmur_udf``

Version Specific Information
============================

  * 5.1.53-12.4:
    Began distributing ``fnv_udf``, ``fnv1a_udf``, and ``murmur_udf``.

Other Information
=================

  * Author / Origin:
    Baron Schwartz

Installation
============

Use of the Percona Software Repositories simplifies the installation of *Percona Toolkit*. Once the repository has been set up on your system, *Percona Toolkit* can be installed by executing: ::

  $ yum install percona-toolkit

This will place the *Percona Toolkit* UDFs onto your system. However, they will not yet be installed into the |MySQL| server. To install one of the UDF``s into the server, execute one of the following commands, depending on which UDF you want to install: ::

  $ mysql -e "CREATE FUNCTION fnv_64 RETURNS INTEGER SONAME ``fnv_udf.so``" 
  $ mysql -e "CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME ``fnv1a_udf.so``" 
  $ mysql -e "CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME ``murmur_udf.so``"

Executing each of these commands will install its respective UDF into the server.

If you have any difficulty, or require more detailed information, refer to the *Maatkit* `Installing UDFs <http://code.google.com/p/maatkit/wiki/InstallingUdfs>`_ documentation.


Other Reading
=============

  * *Percona Toolkit* - http://www.percona.com/doc/percona-toolkit
  * *Maatkit* - http://www.maatkit.org/
