.. _data-scrubbing:

Data Scrubbing
================================================================================

:Availability: This feature is tech preview quality

The ``DELETE`` statement does the following:

* Marks the rows as deleted
* Added to ``unused``
* Records the deleted row position
* Records the time when delete was committed

The data is not immediately removed from disk storage. When a new row is inserted, the row is stored without fragmentation in a page with free space. The deleted data remains on the disk until new data overwrites it.

Using Data Scrubbing, you have two options to remove the data and reclaim the space:

+---------------------+------------------------------------------------------------------------------------+
| How data is removed | Descriptions                                                                       |
+=====================+====================================================================================+
| Immediate           | The data is removed immediately, independent of key rotation or background threads |
+---------------------+------------------------------------------------------------------------------------+
| Background          | The moment when the data is removed can be unpredictable                           |
+---------------------+------------------------------------------------------------------------------------+

Once enabled, data scrubbing works automatically on each tablespace.

To enable data scrubbing, you must enable either one or both of the following variables:

- :variable:`innodb-background-scrub-data-uncompressed`
- :variable:`innodb-background-scrub-data-compressed`

Uncompressed tables can also be scrubbed immediately, independent of key
rotation or background threads. Setting the variable
:variable:`innodb-immediate-scrub-data-uncompressed` enables this operation. This variable does not support operations on compressed tables.

For background scrubbing, you must set the :variable:`innodb_encryption_threads` variable to a value greater than **zero** when you enable data scrubbing. Intermediate scrubbing does not use encryption threads. A separate thread performs log scrubbing.

System Variables
--------------------------------------------------------------------------------

.. variable:: innodb_background_scrub_data_compressed

   :cli: ``--innodb-background-scrub-data-compressed``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``
   
Enables compressed data scrubbing.

.. variable:: innodb_background_scrub_data_uncompressed

   :cli: ``--innodb-background-scrub-data-uncompressed``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``
   
Enables uncompressed data scrubbing.

.. variable:: innodb_scrub_log

    :cli: ``--innodb-scrub-log``
    :dyn: No
    :scope: Global
    :vartype: Boolean
    :default: ``OFF``
    
Enables redo log scrubbing.

.. variable:: innodb_scrub_log_speed

    :cli: ``--innodb-scrub-log-speed``
    :dyn: Yes
    :scope: Global
    :vartype: Numeric
    :default: 256
    
Defines the scrubbing speed in bytes/sec of the redo log.

.. variable:: innodb_immediate_scrub_data_uncompressed

    :cli: ``--innodb-immediate-scrub-data-uncompressed``
    :dyn: Yes
    :scope: Global
    :vartype: Boolean
    :default: ``OFF``
    
Enables data scrubbing of uncompressed data.

.. seealso::

   Vault Documentation
      https://www.vaultproject.io/docs/index.html
   General-Purpose Keyring Key-Management Functions
      https://dev.mysql.com/doc/refman/8.0/en/keyring-udfs-general-purpose.html
