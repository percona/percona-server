--let $indexed =
--let $generated_storage_type = VIRTUAL
--let $generated_dictionary_name =
--source ../include/xtradb_compressed_columns_virtual.inc

--let $indexed =
--let $generated_storage_type = STORED
--let $generated_dictionary_name =
--source ../include/xtradb_compressed_columns_virtual.inc

--let $indexed =
--let $generated_storage_type = STORED
--let $generated_dictionary_name = $source_dictionary_name
--source ../include/xtradb_compressed_columns_virtual.inc


--let $indexed = 1
--let $generated_storage_type = VIRTUAL
--let $generated_dictionary_name =
--source ../include/xtradb_compressed_columns_virtual.inc

--let $indexed = 1
--let $generated_storage_type = STORED
--let $generated_dictionary_name =
--source ../include/xtradb_compressed_columns_virtual.inc
