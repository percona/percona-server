SSL improvements
================

|PS| have undergone several SSL compatibility improvements, distinguishing it
from the original MySQL community edition. 

Linking with OpenSSL
--------------------

As stated in `<https://dev.mysql.com/doc/refman/5.5/en/openssl-versus-yassl.html>`_,
MySQL can be compiled using either OpenSSL toolkit or yaSSL embedded library to
enable encrypted connections based on Transport Layer Security / Secure Sockets
Layer protocols. All binary packages of *Percona Server for MySQL* are linked
with system OpenSSL, whereas MySQL community edition binary is linked with
yaSSL. 

Therefore system administrator should take into account that *Percona Server
for MySQL* is not dependent on yaSSL security advisories, but instead depends
on system OpenSSL ones, which are to be tracked to make promptly upgrade as
necessary. Functionality differences between these two SSL backends are covered
in `<https://dev.mysql.com/doc/refman/5.6/en/openssl-versus-yassl.html>`_.

.. note: yaSSL-linked builds of *Percona Server for MySQL* are not supported,
   but it is still possible to build yaSSL-enabled Percona Server from source. 

Newer TLS encryption protocol support
-------------------------------------

*Percona Server for MySQL* supports TLS version 1.2, and the version 1.0 is
disabled by default. The reason is The PCI Security Standards compliance, as
PCI Security Standards Council made TLS 1.0 deprecated starting from May 2016,
and have stated 30 June 2018 as a deadline for disabling TLS 1.0 in favor of a
more secure encryption protocol, i.e. TLS 1.1 or higher (TLS v1.2 is strongly
encouraged).

Subject Alternative Names and other additional features
-------------------------------------------------------

*Percona Server for MySQL* supports additional set of features, like Subject
Alternative Names (SAN) and commonName (CN) validation, as opposed to MySQL
community edition. The following compatibility matrix illustrates these
differences:

+-------------------------+-------+-----------------+------------------+
| Feature                 | YaSSL | OpenSSL < 1.0.2 | OpenSSL >= 1.0.2 |
+=========================+=======+=================+==================+
| ‘commonName’ validation | Yes   | Yes             | Yes              |
+-------------------------+-------+-----------------+------------------+
| SAN validation          | No    | Yes             | Yes              |
+-------------------------+-------+-----------------+------------------+
| Wildcards support       | No    | No              | Yes              |
+-------------------------+-------+-----------------+------------------+

SSL support in ``mysqlbinlog``
------------------------------

Also ``mysqlbinlog`` utility in Percona Server includes support for client-side
SSL options. Other encryption-aware client tools (``mysql``, ``mysqlcheck``,
``mysqladmin``, ``mysqlimport``, ``mysqlslap``, ``mysqltest``, ``mysqlshow``,
``mysqldump``, ``mysql_upgrade``) in MySQL are sharing the same set options to
configure SSL encryption for the client-server connection. Percona Server for
MySQL improves SSL support unification, providing the same options for
``mysqlbinlog``.
