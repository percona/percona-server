.. _ssl:

================================================================================
SSL Improvements
================================================================================

By default, *Percona Server for MySQL* passes elliptic-curve crypto-based
ciphers to OpenSSL, such as ECDHE-RSA-AES128-GCM-SHA256.

.. note::

   Although documented as supported, elliptic-curve crypto-based ciphers do not work with *MySQL*.

   .. seealso::

      MySQL Bug System (solved for *Percona Server for MySQL*):
         `#82935 Cipher ECDHE-RSA-AES128-GCM-SHA256 listed in man/Ssl_cipher_list, not supported <https://bugs.mysql.com/bug.php?id=82935>`_

.. |openssl| replace:: OpenSSL
