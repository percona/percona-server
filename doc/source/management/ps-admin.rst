.. _ps-admin:

==========================================
PS-Admin script
==========================================

You can use the ps-admin script allows :ref:`tokudb_quick_install` and :ref:`toku_backup`. If the TokuDB storage engine enables the transparent huge pages, the script adds the thp-setting=never option to my.cnf to disable transparent huge pages on runtime.  

An example of enabling the TokuDB plugin follows:

.. code-block:: bash

    $ sudo ps-admin --enable-tokudb -u root -pPassw0rd

The following example enables the TokuBackup.

.. code-block:: bash

    $ sudo ps-admin --enable-tokubackup

You are able to :ref:`enable-myrocks` and disable and uninstall the MyRocks storage engine.

An example of the enabling and disabling the MyRocksDB plugin follows:

.. code-block:: bash

    $ sudo ps-admin --enable-rocksdb -u root -pPassw0rd

    $ sudo ps-admin --disable-rocksdb -u root -pPassw0rd

The ps-admin script can also enable or disable the following:

* Audit Log plugin
* :ref:`pam_plugin`
* Query Reponse Time plugin
* MYSQLX plugin 

An example of enabling the PAM Authentication plugin follows:

.. code-block:: bash

    $ sudo ps-admin --enable-pam -u root -pPassw0rd