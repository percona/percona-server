.. _innodb_fast_checksum_page:

========================
 Fast |InnoDB| Checksum
========================

.. warning::

  This feature has been deprecated after |Percona Server| :rn:`5.5.28-29.2` and it will not be available in |Percona Server| 5.6, because the innodb_checksum_algorithm feature in |MySQL| 5.6 makes it redundant.

|InnoDB| writes a checksum at the end of each data page in order to detect data files corruption. However computing this checksum requires CPU cycles and in some circumstances this extra overhead can become significant.

|XtraDB| can use a more CPU-efficient algorithm, based on 4-byte words, which can be beneficial for some workloads (for instance write-heavy workloads on servers that can perform lots of IO).

The original algorithm is checked after the new one, so you can have data pages with old checksums and data pages with new checksums. However in this case, you may experience slow reads from pages having old checksums. If you want to have the entire benefit of this change, you will need to recreate all your |InnoDB| tables, for instance by dumping and reloading all |InnoDB| tables.

Once enabled, turning it off will require table(s) to be dump/imported, since it will fail to start on data files created when ``innodb_fast_checksums`` was enabled. In this case ALTER TABLE won't work due to its implementation. 


System Variables
================

.. variable:: innodb_fast_checksum

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: No
   :vartype: BOOL
   :default: 0

 
