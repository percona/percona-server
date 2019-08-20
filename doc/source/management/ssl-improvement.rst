.. _percona-server.management.ssl:

================================================================================
SSL Improvements
================================================================================

|Percona Server| enables transmitting data in the encrypted form by using the
TLSv1.2 protocol. By default, |Percona Server| disables TLSv1.0 and deprecates
TLSv1.1.

.. rubric:: Implemented Using |openssl|

|Percona Server| binaries link with the |openssl| library to implement the
support of TLS protocols. |Percona Server| supports |openssl| version 1.1.

|Percona Server| binaries do not link with the |yassl.def| as |MySQL| Community
Edition does. The |yassl| library only supports TSLv1.0 and TSL1.1 protocols
which are less secure than TLSv1.2. You may still link with |yassl| when
building |Percona Server| from source.

As part of its implementation, |Percona Server| offers correct diagnostic
messages in cases like ciphers on the client and the server mismatching, 
the required TLS version not enabled, and so on. For more information, see
`#75311 Error for SSL cipher is unhelpful
<https://bugs.mysql.com/bug.php?id=75311>`_.

.. important::

   As |Percona Server| does not use |yassl|, |yassl| security advisories are not
   applicable to |Percona Server|. System administrators should track the
   security advisories relevant to |openssl| and upgrade their operating system
   promptly.

.. seealso::

   More information about |yassl.def| 
      https://www.wolfssl.com/products/yassl/
   |MySQL| Documentation: OpenSSL Versus yaSSL
      https://dev.mysql.com/doc/refman/5.6/en/openssl-versus-yassl.html
   |MySQL| Bug System (solved for |Percona Server|): 
       `#75311 Error for SSL cipher is unhelpful <https://bugs.mysql.com/bug.php?id=75311>`_

By default, |Percona Server| passes elliptic curve crypto-based
ciphers to OpenSSL, such as ECDHE-RSA-AES128-GCM-SHA256.

.. note:: 

   Although documented as supported, elliptic curve crypto-based ciphers do not work with |MySQL|.

   .. seealso::

      |MySQL| Bug System (solved for |Percona Server|):
         `#82935 Cipher ECDHE-RSA-AES128-GCM-SHA256 listed in man/Ssl_cipher_list, not supported <https://bugs.mysql.com/bug.php?id=82935>`_

.. _percona-server.management.ssl.multi-domain-certificate:

Multi-Domain Certificates
================================================================================
      
|Percona Server| supports multi-domain certificates (:abbr:`SAN (Subject
Alternative Name)`). This feature is useful to help manage the storage better,
for building high availability clusters, or as part of a backup solution.

.. seealso::

   |Percona| Blog Post: When would you use SAN with MySQL?
      https://www.percona.com/blog/2009/03/09/when-would-you-use-san-with-mysql/
   |MySQL| Bug System (solved for |Percona Server|):
      `#68052 SSL Certificate Subject ALT Names with IPs not respected with --ssl-verify-serve <https://bugs.mysql.com/bug.php?id=68052>`_


.. _percona-server.management.ssl.compatibility-matrix:

Compatibility Matrix 
================================================================================

==========================================  =======  ==================  ================
Feature                                     YaSSL    OpenSSL < 1.0.2     OpenSSL >= 1.0.2
==========================================  =======  ==================  ================
Validation of SSL certificate common name   Yes      Yes                 Yes
Validation of |san.abbr|                    No       Yes                 Yes
Support for wildcard names                  No       No                  Yes
==========================================  =======  ==================  ================

.. _percona-server.management.ssl.mysqlbinlog:

SSL Improvements in ``mysqlbinlog``
================================================================================
	    
|Percona Server| extends :command:`mysqlbinlog` to accept the ``SSL`` connection
options as all the other client programs.

.. seealso::

   How |Percona Server| extends the functionality of :command:`mysqlbinlog`
      :ref:`extended_mysqlbinlog`
   |MySQL| Bug System (solved for |Percona Server|):
      `#41975 Support for SSL options not included in mysqlbinlog <https://bugs.mysql.com/bug.php?id=41975>`_

.. |openssl| replace:: OpenSSL
.. |yassl| replace:: yaSSL
.. |yassl.def| replace:: :abbr:`yaSSL embedded SSL library`
.. |san.abbr| replace:: :abbr:`SAN (Subject Alternative Name)`
