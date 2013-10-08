.. _atomic_fio:

============================================
 Atomic write support for Fusion-io devices
============================================

.. note::

 This feature implementation is considered BETA quality.

DirectFS filesystem on `Fusion-io <http://www.fusionio.com/>`_ devices supports atomic writes. Atomic writes can be used instead of |InnoDB| doublewrite buffer to guarantee that the |InnoDB| data pages will be written to disk entirely or not at all. When atomic writes are enabled the device will take care of protecting the data against partial writes. In case the doublewrite buffer is enabled it will be disabled automatically. This will improve the write performance, because data doesn't need to be written twice anymore, and make the recovery simpler.

  
Version Specific Information
============================

 * :rn:`5.6.11-60.3`
    ``Atomic write support for Fusion-io`` feature implemented. This feature was ported from |MariaDB|.

System Variables
================

.. variable:: innodb_use_atomic_writes

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: 0 (OFF)

This variable can be used to enable or disable atomic writes instead of the doublewrite buffer. When this option is enabled (set to ``1``), doublewrite buffer will be disabled on |InnoDB| initialization and the file flush method will be set to ``O_DIRECT`` if it's not ``O_DIRECT`` or ``O_DIRECT_NO_FSYNC`` already.

.. warning::

  :variable:`innodb_use_atomic_writes` should only be enabled on supporting devices, otherwise |InnoDB| will fail to start.

Other Reading
=============
 
 * For general |InnoDB| tuning :ref:`innodb_io_page` documentation is available.

 * `FusionIO DirectFS atomic write support in *MariaDB* <https://kb.askmonty.org/en/fusionio-directfs-atomic-write-support/>`_

 * `Atomic Writes Accelerate MySQL Performance <http://www.fusionio.com/blog/atomic-writes-accelerate-mysql-performance/>`_

