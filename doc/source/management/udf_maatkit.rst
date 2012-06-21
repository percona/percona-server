.. _udf_maatkit:

======================================
 *Percona Toolkit* and *Maatkit* UDFs
======================================

Three *Percona Toolkit* and *Maatkit* UDFs that provide faster checksums are provided:

  * ``libfnv_udf``

  * ``libfnv1a_udf``

  * ``libmurmur_udf``

Tools that do checksumming of data (pt-table-checksum, pt-table-sync) will use these functions if they are installed. These are much faster and have better data distribution than CRC32() or MD5() or similar, which are the default functions built into |MySQL|.

Version Specific Information
============================

  * 5.5.8-20.0:
    Began distributing ``libfnv_udf``, ``libfnv1a_udf``, and ``libmurmur_udf``.

Other Information
=================

  * Author / Origin:
    Baron Schwartz

Installation
============

These functions are  distributed with the |Percona Server| installation. Installing the |Percona Server| will place the UDFs onto your system. However, they will not yet be installed into the |MySQL| server. To install one of the UDF's into the server, execute one of the following commands, depending on which UDF you want to install: ::

  $ mysql -e "CREATE FUNCTION fnv_64 RETURNS INTEGER SONAME 'libfnv_udf.so'" 
  $ mysql -e "CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME 'libfnv1a_udf.so'" 
  $ mysql -e "CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME 'libmurmur_udf.so'"

Executing each of these commands will install its respective UDF into the server.

Confirm that the installation succeeded by running a command such as :: 

  mysql> SELECT fnv_64('hello world');

Other Reading
=============

  * *Percona Toolkit* - http://www.percona.com/doc/percona-toolkit
  * *Maatkit* - http://www.maatkit.org/
