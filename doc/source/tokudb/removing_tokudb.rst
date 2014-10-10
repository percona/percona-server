.. _removing_tokudb:

================================
 Removing TokuDB storage engine
================================

In case you want remove the TokuDB storage engine from |Percona Server| without causing any errors following is the recommended procedure:

Change the tables from TokuDB to InnoDB
---------------------------------------

If you still need the data in the TokuDB tables you'll need to alter the tables to other supported storage engine i.e., |InnoDB|:

.. code-block:: mysql

   mysql> ALTER TABLE City ENGINE=InnoDB;

.. note:: 

   In case you remove the TokuDB storage engine before you've changed your tables to other supported storage engine you won't be able to access that data without re-installing the TokuDB storage engine.

Removing the plugins
--------------------

Storage engine requires manual removal: 

.. code-block:: mysql

 UNINSTALL PLUGIN tokudb; 
 UNINSTALL PLUGIN tokudb_file_map;
 UNINSTALL PLUGIN tokudb_fractal_tree_info;
 UNINSTALL PLUGIN tokudb_fractal_tree_block_map;
 UNINSTALL PLUGIN tokudb_trx;
 UNINSTALL PLUGIN tokudb_locks;
 UNINSTALL PLUGIN tokudb_lock_waits;

After the engine and the plugins have been uninstalled you can remove the TokuDB package by using the apt/yum commands: 

.. code-block:: bash

 [root@centos ~]# yum remove Percona-Server-tokudb-56.x86_64

or

.. code-block:: bash

 root@wheezy:~# apt-get remove percona-server-tokudb-5.6
 
.. note::

   Make sure you've removed all the TokuDB specific variables from your configuration file (:file:`my.cnf`) before you restart the server, otherwise server could show errors or warnings and won't be able to start.



