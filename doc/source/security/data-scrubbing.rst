.. _data-scrubbing:

Data Scrubbing
================================================================================

:Availability: This feature is **Experimental** quality

While data encryption ensures that the existing data are not stored in plain
form, the data scrubbing literally removes the data once the user decides they
should be deleted. Compare this behavior with how the ``DELETE`` statement works
which only marks the affected data as *deleted* - the space claimed by this data
is overwritten with new data later.

Once enabled, data scrubbing works automatically on each tablespace
separately. To enable data scrubbing, you need to set the following variables:

- :variable:`innodb-background-scrub-data-uncompressed`
- :variable:`innodb-background-scrub-data-compressed`

Uncompressed tables can also be scrubbed immediately, independently of key
rotation or background threads. This can be enabled by setting the variable
:variable:`innodb-immediate-scrub-data-uncompressed`. This option is not supported for
compressed tables.

Note that data scrubbing is made effective by setting the
:variable:`innodb_online_encryption_threads` variable to a value greater than
**zero**.

System Variables
--------------------------------------------------------------------------------

.. variable:: innodb_background_scrub_data_compressed

   :cli: ``--innodb-background-scrub-data-compressed``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

.. variable:: innodb_background_scrub_data_uncompressed

   :cli: ``--innodb-background-scrub-data-uncompressed``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

.. seealso::

   Vault Documentation
      https://www.vaultproject.io/docs/index.html
   General-Purpose Keyring Key-Management Functions
      https://dev.mysql.com/doc/refman/8.0/en/keyring-udfs-general-purpose.html
