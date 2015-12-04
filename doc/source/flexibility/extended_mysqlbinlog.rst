.. _extended_mysqlbinlog:

=========================
 Extended ``mysqlbinlog``
=========================

|Percona Server| has implemented compression support for :command:`mysqlbinlog` in :rn:`5.6.15-63.0`. This is similar to support that both ``mysql`` and ``mysqldump`` programs include (the ``-C``, ``--compress`` options "Use compression in server/client protocol"). Using the compressed protocol helps reduce the bandwidth use and speed up transfers. 

|Percona Server| has also implemented support for ``SSL``. :command:`mysqlbinlog` now accepts the ``SSL`` connection options as all the other client programs. This feature can be useful with ``--read-from-remote-server`` option. Following ``SSL`` options are now available:

 * :option:`--ssl` - Enable SSL for connection (automatically enabled with other flags).
 * :option:`--ssl-ca=name` - CA file in PEM format (check OpenSSL docs, implies --ssl).
 * :option:`--ssl-capath=name` - CA directory (check OpenSSL docs, implies --ssl).
 * :option:`--ssl-cert=name` - X509 cert in PEM format (implies --ssl).
 * :option:`--ssl-cipher=name` - SSL cipher to use (implies --ssl).
 * :option:`--ssl-key=name` - X509 key in PEM format (implies --ssl).
 * :option:`--ssl-verify-server-cert` - Verify server's "Common Name" in its cert against hostname used when connecting. This option is disabled by default.

Version Specific Information
============================

  * :rn:`5.6.15-63.0`
    :command:`mysqlbinlog` option :option:`--compress` introduced

  
  * :rn:`5.6.15-63.0`
    :command:`mysqlbinlog` now has all SSL connection options as the rest of the |MySQL| client programs.


