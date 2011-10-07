.. rn:: 5.1.56-12.7

==============================
 |Percona Server| 5.1.56-12.7
==============================

Released on April 18, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.56-12.7/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_.)

|Percona Server| 5.1.56-12.7 is now the current stable release in the 5.1 series. It is is based on |MySQL| 5.1.56.

New Features
============

  * Expanded the applicability of |InnoDB| :ref:`innodb_fast_index_creation` to :command:`mysqldump`, ``ALTER TABLE`` and ``OPTIMIZE TABLE``. (*Alexey Kopytov*)

Variable Changes
================

  * Variable :variable:`innodb_stats_method` has been implemented in the upstream |InnoDB|, with the same name and functionality that had previously existed only in |XtraDB|. (*Yasufumi Kinoshita*)

Other Changes
=============

  * Implemented support for variable :variable:`innodb_stats_method` being implemented in the upstream |InnoDB|, including adding a column to table ``INNODB_SYS_STATS``. Bug fixed: :bug:`733317`. (*Yasufumi Kinoshita*)

  * Added ``README-windows`` with basic command to build under *Windows*, VC 2010 Express. (*Vadim Tkachenko*)

  * Added helper script ``README-Solaris`` for building on *Solaris*. (*Vadim Tkachenko*)

  * Changes were made to the post-install messages. (*Ignacio Nin*)

Bugs Fixed
==========

  * Bug :bug:`631667` - The |MySQL| binary is now built with readline support so that command line browsing is possible. (*Ignacio Nin*)

  * Bug :bug:`716575` - When using the ``innodb_import_table_from_xtrabackup`` feature, importing ``.ibd`` files smaller than 1 MB could lead to a server crash. (*Yasufumi Kinoshita*)

  * Bug :bug:`727704` - When using the :ref:`innodb_expand_import_page` feature, importing ``.ibd`` files created on |MySQL| 5.0 or |Percona Server| versions prior to 5.1.7 could crash the server. (*Yasufumi Kinoshita*)

  * Bug :bug:`735423` - File ownerships and permissions are now changed after running ``mysql_install_db`` after installation or upgrade, in order to avoid "permission denied" error when starting :command:`mysqld`. (*Aleksandr Kuzminsky*)

  * |MySQL| bugs `56433 <http://bugs.mysql.com/56433>`_ and `51325 <http://bugs.mysql.com/51325>`_ - These |MySQL| bugs have been fixed in |Percona Server|. (*Yasufumi Kinoshita*)
