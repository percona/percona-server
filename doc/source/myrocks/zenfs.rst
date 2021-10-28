.. _zenfs:

=========================================
MyRocks with ZenFS
=========================================

Implemented in Percona Server for MySQL 8.0.26-16.

`ZenFS <https://zonedstorage.io/projects/zenfs/>`__ is a file system plugin
which places files into zones on a raw zoned block device (ZBD) using
the MyRocks File System interface. ZenFS depends on ``libzbd`` and requires
a Linux kernel implementation which supports NVMe Zoned Namespaces. The kernel
must be configured with `zone block device support
enabled <https://zonedstorage.io/linux/config/#kernel-configuration>`__.

The ``libzbd`` library provides functions and manages the state of the zones
of zoned block devices. You can download the
library `here <https://ubuntu.pkgs.org/21.04/ubuntu-universe-amd64/libzbd1_1.2.0-1_amd64.deb.html>`__ and follow the installation instructions on the Web site. For more information, see `libzbd User Library <https://zonedstorage.io/projects/libzbd/>`__.

For more information, see `Western Digital and Percona deliver
Utrastar DC ZN540 Zoned Namespace SSD support for Percona Server for
MySQL <https://documents.westerndigital.com/content/dam/doc-library/en_us/assets/public/western-digital/collateral/company/western-digital-zns-ssd-perconal-blogpost.pdf>`__.

.. _zenfs-install:

Installing MyRocks with ZenFS packages
------------------------------------------

The MyRocks with ZenFS packages are listed in
the :ref:`installing_from_binary_tarball` section.

To install the packages, run the following command:
   
.. code-block:: bash
    
    $ sudo apt install percona-server-server-zenfs
    $ sudo apt install percona-server-rocksdb-zenfs

Running the Installation
----------------------------

The following steps successfully run Percona Server with MySQL on the  ``--rocksdb-fs-uri=zenfs://dev:<short_block_device_name>``:

.. note::

    The ``<block_device_name>`` can have a short name desigination which is the ``<short_block_device_name>``. For example, the ``block_device_name`` is ``/dev/nvme1n2`` and the short name is ``nvme1n2``.

#. Identify your ZBD device, ``<block_device_name>``, with ``lsblk``. Use the ``-o`` option to specify which columns are returned. The ``NAME``, ``SIZE``, and ``ZONED`` parameters display the block device names, size, and if the device uses the zone model. A ZBD device is identified as ``host-managed``.

    .. sourcecode:: bash

     lsblk -o NAME,SIZE,ZONED
     NAME        SIZE  ZONED
     sda       247.9G  none
     |-sda1    230.9G  none
     |-sda2        1G  none
     |-sda3       16G  none
     sdb        15.5T  host-managed


#. Create an auxiliary directory for the ZenFS, for example, ``/var/lib/mysql_aux``. The ZenFS auxiliary directory is a regular (POSIX) file system used internally to resolve file locks and shared access. There are no strict requirements for the auxiliary directory location but the directory must be write accessible for the `mysql:mysql` UNIX system user account. Each ZBD must have an individual auxiliary directory.


#. Run ``/zenfs mkfs --zbd <short_block_device_name> --aux_path=<path_to_aux_zenfs_dir>`` in the context of the mysql:mysql account to initialize ZenFS on the ZBD.

    .. sourcecode:: bash

        zenfs mkfs --zbd=nvme1n2 --aux_path=/var/lib/mysql_aux

    .. note::

        This step initializes the metadata on the block device, ``<block_device_name>``, and creates the ``<path_to_aux_zenfs_dir>`` owned by the `mysql:mysql` account. For example, the ``<block_device_name>`` could be ``/dev/nvme1n2``. The ``<path_to_aux_zenfs_dir>`` is a unique, dedicated directory which allows read and write access to `mysql:mysql`.

        If you are re-initializing an already initialized ZBD, you must manually clear the ``/var/lib/mysql_aux`` directory and add the additional ``--force`` parameter.

        .. sourcecode:: bash

            zenfs mkfs --zbd=nvme1n2 --aux_path=/var/lib/mysql_aux  --force



#. You can start Percona Server for MySQL with any of the known methods. For example, add this option to ``my.cnf``:

    .. sourcecode:: text

        [mysqld]
        ...
        rocksdb-fs-uri=zenfs://dev:nvme1n2
        ...

You can verify if the ZenFS was successfully created with the following command:

.. sourcecode:: bash

    zenfs ls-uuid
    ...
    13e421af-1967-435c-ab15-faf4529710b6    nvme1n2
    ...

You can check the available storage with the following command:

.. sourcecode:: bash

    zenfs df --zbd=nvme1n2
    Free: 7563 MB
    Used: 0 MB
    Reclaimable: 0 MB
    Space amplification: 0%

Shut down the server and use the following command to backup a ZenFS file system, including metadata files, to a local filesystem. The ``zenfs`` utility must have exclusive access to the ZenFS filesystem to take a consistent snapshot.

.. sourcecode:: bash

    zenfs backup --zbd=nvme1n2 --path=/storage/backup --backup_path=./

.. note::

    At this time, a logical backup is the only backup type supported by the Percona Server MyRocks tables located on ``ZenFS``.

Use the following command to restore a backup:

.. sourcecode:: bash

    zenfs restore --zbd=nvme1n2 --path=/storage/backup/rocksdb-backup \
    --restore_path=./
