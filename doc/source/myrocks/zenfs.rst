.. _zenfs:

============================================================================
Installing and configuring Percona Server for MySQL with ZenFS support
============================================================================

Implemented in Percona Server for MySQL 8.0.26-16.

A solid state drive (SSD) does not overwrite data like a magnetic hard disk drive. Data must be written to an empty page. An SSD issue is ``write amplification``. This issue is when the same data is written multiple times. 

An SSD is organized in pages and blocks. Data is written in pages and erased in blocks. If, for example, you have 8KB data on a page. The application updates one sector (512 Bytes) of that page. The controller reads the page in RAM, marks the old page as ``stale``, updates the sector, and then writes a new page with this 8KB of data. The process is efficient use of the storage space but also shortens the SSD lifespan because the SSD parts do wear out. 

Garbage collection can also cause large-scale write amplification. The ``stale`` data is erased in blocks, which can consist of hundreds of pages. The SSD controller searches for pages that are marked stale. Pages that are not stale but are stored in that block are moved to another block before the block is erased and marked ready for use. 

The zone storage model organizes the SSD into a set of zones that are uniform in size and uses the Zoned Namespaces (ZNS) technology. ZNS is optimized for an SSD and exposes this zoned block storage interface between the host and SSD. ZNS enables smart data placement. Writes are sequential within a zone.

ZenFS is a file system plugin for RocksDB which uses the RocksDB file system to place files into zones on a raw zoned block device (ZBD). The plugin adds native support for ZNS, avoids on device garbage collection, and minimizes write amplification. File data is stored in a set of extents. Within a zone, extents are a contiguous part of the address space. Garbage collection is an option, but this selection can cause write amplification.

ZenFS depends on the ``libzbd`` user library and requires a Linux kernel implementation that supports NVMe Zoned Namespaces. The kernel
must be configured with `zone block device support
enabled <https://zonedstorage.io/docs/linux/config>`__.

Read the `Western Digital and Percona deliver
Utrastar DC ZN540 Zoned Namespace SSD support for Percona Server for
MySQL <https://documents.westerndigital.com/content/dam/doc-library/en_us/assets/public/western-digital/collateral/company/western-digital-zns-ssd-perconal-blogpost.pdf>`__ PDF for more information.


The following procedure installs Percona Server for MySQL and then configures 
``--rocksdb-fs-uri=zenfs://dev:<short_block_device_name>`` for data storage.

.. note::

   The <block_device_name> can have a short name designation which is the <short_block_device_name>. For example, if the <block_device_name> is ``/dev/nvme0n2`` remove the ``/dev/`` portion and the <short_block_device_name> is ``nvme0n2``.
   The block device name and short block device name must be substituted with the appropriate name from your system. To indicate that such a substitution is needed in statements, we use <block_device_name> and <short_block_device_name>.

For the moment, the ZenFS plugin can be enabled in following distributions:

.. list-table::
   :widths: auto
   :header-rows: 1

   * - Distribution Name
     - Notes
   * - Debian 11.1
     - Able to run the ZenFS plugin
   * - Ubuntu 20.04.3
     - Requires the 5.11 HWE kernel patched with the ``allow blk-zoned ioctls without CAPT_SYS_ADMIN`` patch

If the ZenFS functionality is not enabled on Ubuntu 20.04, the binaries with ZenFS support can run on the standard 5.4 kernel.

`Other Linux distributions <https://zonedstorage.io/docs/distributions/linux/>`__ are adding support for ZenFS, but Percona does not provide installation packages for those distributions.

.. _zenfs-installation:

Installation
============================================================

Start with the installation of *Percona Server for MySQL*. 

1. The steps are listed here for convenience, for an explanation, see :ref:`apt-install`.


   .. code-block:: bash

      $ wget https://repo.percona.com/apt/percona-release_latest.$(lsb_release -sc)_all.deb	
      $ sudo apt install gnupg2 lsb-release ./percona-release_latest.*_all.deb
      $ sudo percona-release setup ps80

2. Install Percona Server for MySQL with MyRocks and the ZenFS plugin package. The binaries are listed in the :ref:`installing_from_binary_tarball` section of the *Percona Server for MySQL* installation instructions.
   
   .. code-block:: bash
	
      $ sudo apt install percona-server-server
	
#. Install the RocksDB plugin package. This package copies ``ha_rocksdb.so`` into a predefined location. **The RocksDB storage engine is not enabled**.

   .. code-block:: bash
	
	  $ sudo apt install percona-server-rocksdb


.. _zenfs-configure:

Configuration
============================================================

#. Identify your ZBD device, <block_device_name>, with `lsblk <https://manpages.debian.org/stretch/util-linux/lsblk.8.en.html>`__. Add the ``-o`` option and specify which columns to print. 

   In the example, the ``NAME`` column returns the block device name, the ``SIZE`` column returns the size of the device, and the ``ZONED`` column returns information if the device uses the zone model. The value, ``host-managed``, identifies a ZBD model.

   .. sourcecode:: bash

      lsblk -o NAME,SIZE,ZONED
      NAME                    SIZE  ZONED
      sda                       247.9G  none
      |-sda1                    230.9G  none
      |-sda2                        1G  none
      |-sda3                       16G  none
      <short_block_device_name>   7.2T  host-managed

		
#. Change the ownership of <block_device_name> to the ``mysql:mysql`` user account.

   .. code-block:: bash
	
	  $ sudo chown mysql:mysql <block_device_name>
		
#. Change the permissions so that the user or owner can read and write and the MySQL group can read, in case they must take a backup, for <block_device_name>.

   .. code-block:: bash
	
	  $ sudo chmod 640 <block_device_name>

#. Change the scheduler  to ``mq_deadline`` with a ``udev`` rule. Create ``/etc/udev/rules.d/60-scheduler.rules`` if the file does not exist, and add the following rule:

   .. code-block:: text

      ACTION=="add|change", KERNEL=="<short_block_device_name>", ATTR{queue/scheduler}="mq-deadline"

#. Restart the machine to apply the rule.

#. Verify if the rule was applied correctly by running the following line:

   .. code-block:: bash

    $ cat /sys/block/<short_block_device_name>/queue/scheduler

#. Review that the output of the previous command matches:

   .. code-block:: text

    [mq-deadline] none

#. Create an auxiliary directory for ZenFS. For example ``/var/lib/mysql_aux_nvme0n2``.

   The ZenFS auxiliary directory is a regular (POSIX) file directory used internally to resolve file locks and shared access. There are no strict requirements for the location but the directory must be write accessible for the `mysql:mysql` UNIX system user account. Each ZBD must have an individual auxiliary directory. This directory is recommended to be at the same level as "/var/lib/mysql", which is the default Percona Server for MySQL directory.

   .. note::

      AppArmor is enabled by default in Debian 11. If your AppArmor mode is set to ``enforce``, you must edit the profile to allow access to these locations. Add the following rules to ``usr.sbin.mysqld``:

      .. code-block:: bash

         /var/lib/mysql_aux_*/ r,
         /var/lib/mysql_aux_*/** rwk,

      Don't forget to reload the policy if you make edits:

      .. code-block:: bash

         $ sudo service apparmor reload

      For more information, see :ref:`enable-apparmor`.

   .. note::
	
		 If you must configure ZenFS to use a directory inside ``/var/lib`` (owned by ``root:root`` without write permissions for other user accounts), edit your AppArmor profile (described in an earlier step), if needed, and do the following steps manually:
		
		 #. Create the ``aux_path`` for <block_device_name>:
		
		    .. code-block:: bash
		
			    $ sudo mkdir /var/lib/mysql_aux_<short_block_device_name>
			
		 #. Change the ownership of the ``aux_path``:
		
		    .. code-block:: bash
		
			    $ sudo chown mysql:mysql /var/lib/mysql_aux_<short_block_device_name>
			
		 #. Set the permissions for the ``aux_path`` for <block_device_name>:
		
		    .. code-block:: bash
		
			    $ sudo chmod 750 /var/lib/mysql_aux_<short_block_device_name>


#. Change the ownership of <block_device_name>.

   .. code-block:: bash

          $ sudo chown mysql:mysql <block_device_name>

#. Initialize ZenFS on <block_device_name>.

   .. code-block:: bash

	  $ sudo -H -u mysql zenfs mkfs --zbd=<short_block_device_name> --aux_path=/var/lib/mysql_aux_<short_block_device_name> --finish_threshold=0 --force
		
#. Stop *Percona Server for MySQL*:

   .. code-block:: bash
	
	  sudo service mysql stop 
		
#. Edit my.cnf. Add the following line to the "[mysqld]" section:

   .. code-block:: text 
		
	  [mysqld]
	  ...
	  loose-rocksdb-fs-uri=zenfs://dev:<short_block_device_name>
	  ...
        
   .. note::
	
		The "loose-" prefix is important.
		
#. Start *Percona Server for MySQL*:

   .. code-block:: bash
	
	  $ sudo service mysql start 
		
#. Enable ``RocksDB``:

   .. code-block:: bash
	
	  $ sudo ps-admin --enable-rocksdb -u root -p

#. Enter the MyRocks password.

   Alternatively, enable RocksDB by adding the following lines to the my.cnf file:

   .. code-block:: text

	  [mysqld]
    ...
    plugin-load-add=rocksdb=ha_rocksdb.so
    default-storage-engine=rocksdb
    ...
		
#. Verify that the ".rocksdb" directory in the default data directory has only "LOG*" files:

   .. code-block:: bash
	
	  $ sudo ls -la /var/lib/mysql/.rocksdb 
		
#. Verify that ZenFS is created on "rocksdb" and has the *RocksDB* data files:

   .. code-block:: bash
	
	  $ sudo -H -u mysql zenfs list --zbd=<short_block_device_name> --path=./.rocksdb
    #OR
	  $ sudo -H -u mysql zenfs dump --zbd=<short_block_device_name>

#. You can verify if the ZenFS was successfully created with the following command:

   .. sourcecode:: bash

      $ sudo -H -u mysql zenfs ls-uuid
      ...
      13e421af-1967-435c-ab15-faf4529710b6    <short_block_device_name>
      ...

#. You can check the available storage with the following command:

   .. sourcecode:: bash

      $ sudo -H -u mysql zenfs df --zbd=<short_block_device_name>
      Free: 971453 MB
      Used: 0 MB
      Reclaimable: 0 MB
      Space amplification: 0%
      
Backup and restore
====================================================================

Shut down the server and use the following command to backup a ZenFS file system, including metadata files, to a local filesystem. The ``zenfs`` backup and restore utility must have exclusive access to the ZenFS filesystem to take a consistent snapshot. The backup command only takes logical backups.

The following command backs up everything from the root of the ZenFS drive:

.. sourcecode:: bash

    $ zenfs backup --zbd=${NULLB} --path="/home/user/bkp" --backup_path=/

The options are the following:

* The ``--path`` can be either an absolute path or a relative path. The backup command creates the directory in the ``--path`` if it does not exist. 

* The ``--backup_path`` option can use any of the following path values based on the location. 

  If the backup is for the ZenFS root drive, use any of the values in the following table:

  .. list-table:: Back up from the ZenFS root drive
     :header-rows: 1

     * - Value 
       - Description
     * - <empty_string>
       - Empty string 
     * - /
       - A forward slash
     * - .
       - A single period
     * - ./
       - A single period with a forward slash

  If the backup is for a non-root ZenFS path, use any of the values in the following table:

  .. list-table:: Back up from a non-root ZenFS path
     :header-rows: 1

     * - Value
       - Description
     * -  <directory>
       - Only the directory name
     * - /<directory>
       - A forward slash with the directory name
     * - ./<directory>
       - A single period with a forward slash and the directory name
     * -  <directory>/
       -  The directory name with a forward slash
     * - /<directory>/
       - A forward slash with the directory name and an ending forward slash
     * - ./<directory>/
       - A single period, a forward slash, the directory name, and an ending forward slash


Use the following command to restore a backup into the root of the ZenFS drive:

.. sourcecode:: bash

    $ zenfs restore --zbd=${NULLB} --path="/home/user/bkp/" --restore_path=/

* The ``--path`` can be either an absolute path or a relative path. The backup command creates the directory in the ``--path`` if it does not exist. 

* The ``--restore_path`` option can use any of the following path values based on the location.

  If the restore is for the ZenFS root drive, use any of the values in the following tables:

  .. list-table:: Restore to the ZenFS root drive
     :header-rows: 1

     * - Value 
       - Description
     * - <empty_string>
       - Empty string 
     * - /
       - A forward slash
     * - .
       - A single period
     * - ./
       - A single period with a forward slash

  If the restore is for a non-root ZenFS path, use any of the values in the following table:

  .. list-table:: Restore to a non-root ZenFS path
     :header-rows: 1

     * - Value
       - Description
     * -  <directory>
       - Only the directory name
     * - /<directory>
       - A forward slash with the directory name
     * - ./<directory>
       - A single period with a forward slash and the directory name
     * -  <directory>/
       -  The directory name with a forward slash
     * - /<directory>/
       - A forward slash with the directory name and an ending forward slash
     * - ./<directory>/
       - A single period, a forward slash, the directory name, and an ending forward slash


Known Limitations
====================================================================

After a reboot the NVME ZBD configuration ("/dev/nvme02" in our examples) can disappear. The issue is OS-dependent and can be managed by the system administrators. One or more of the following events may have occurred:

- A reboot changes the active "scheduler" from "[mq-deadline]". The following steps `reset the disk scheduler in RedHat using udev rules <https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_storage_devices/setting-the-disk-scheduler_managing-storage-devices#setting-the-disk-scheduler-using-udev-rules_setting-the-disk-scheduler>`__. For Ubuntu, see `Input/output schedulers <https://wiki.archlinux.org/title/Improving_performance#Input/output_schedulers>`__.

.. seealso:: For more information, review `Change I/O scheduler <https://www.golinuxcloud.com/how-to-change-io-scheduler-permanently-linux/>`__.

- A reboot resets the device permissions from "640/mysql:mysql" to "660/root:disk". 




