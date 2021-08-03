.. _removing_tokudb:

================================
 Removing TokuDB storage engine
================================


Migrating the data to MyRocks
------------------------------

To migrate data use the `mysqldump <https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html>`__ client utility or the tools in the `MySQL Workbench <https://dev.mysql.com/downloads/workbench/>`__ to dump and restore the database.

We recommended migrating to the `MyRocks` storage engine. Follow these steps to migrate the data:

1. Use mysqldump to backup the TokuDB database into a single file.

2. Create a MyRocks instance with MyRocks tables with no data.

3. Replace the references to `TokuDB` with `MyRocks`.

4. Enable the following variable: `rocksdb_bulk_load`. This variable also enables `rocksdb_commit_in_the_middle`. 

5. Import the data into the MyRocks database.

Follow the :ref:`remove-plugins` steps.

Migrating from TokuDB to InnoDB
---------------------------------

In case you want remove the TokuDB storage engine from |Percona Server| without
causing any errors following is the recommended procedure:

Change the tables from TokuDB to InnoDB
---------------------------------------

If you still need the data in the TokuDB tables you must alter the tables
to other supported storage engine i.e., |InnoDB|: :mysql:`ALTER TABLE City
ENGINE=InnoDB;`

.. note:: 

   Do not remove the TokuDB storage engine before you've changed your
   tables to the other supported storage engine. Otherwise, you will not be able to access that
   data without reinstalling the TokuDB storage engine.

.. _remove-plugins:

Removing the plugins
--------------------

To remove the |TokuDB| storage engine with all installed plugins you can use the
|ps-admin| script:

.. code-block:: bash

   $ ps-admin --disable-tokudb -uroot -pPassw0rd

Script output should look like this: 

.. admonition:: Output

   .. code-block:: bash
   
      Checking if Percona server is running with jemalloc enabled...
      >> Percona server is running with jemalloc enabled.
    
      Checking transparent huge pages status on the system...
      >> Transparent huge pages are currently disabled on the system.
    
      Checking if thp-setting=never option is already set in config file...
      >> Option thp-setting=never is set in the config file.
    
      Checking TokuDB plugin status...
      >> TokuDB plugin is installed.
    
      Removing thp-setting=never option from /etc/mysql/my.cnf
      >> Successfuly removed thp-setting=never option from /etc/mysql/my.cnf
    
      Uninstalling TokuDB plugin...
      >> Successfuly uninstalled TokuDB plugin.

Another option is to manually remove the |TokuDB| storage engine with all installed plugins:

.. code-block:: mysql

   UNINSTALL PLUGIN tokudb; 
   UNINSTALL PLUGIN tokudb_file_map;
   UNINSTALL PLUGIN tokudb_fractal_tree_info;
   UNINSTALL PLUGIN tokudb_fractal_tree_block_map;
   UNINSTALL PLUGIN tokudb_trx;
   UNINSTALL PLUGIN tokudb_locks;
   UNINSTALL PLUGIN tokudb_lock_waits;
   UNINSTALL PLUGIN tokudb_background_job_status;

After the engine and the plugins have been uninstalled you can remove the TokuDB package by using the apt/yum commands: 

.. code-block:: bash

 [root@centos ~]# yum remove Percona-Server-tokudb-80.x86_64

or :bash:`apt remove percona-server-tokudb-8.0`
 
.. note::

   Make sure you've removed all the TokuDB specific variables from your configuration file (:file:`my.cnf`) before you restart the server, otherwise server could show errors or warnings and won't be able to start.

.. include:: ../.res/replace.program.txt
