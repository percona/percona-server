--source include/rpl/init_source_replica.inc

INSTALL COMPONENT 'file://component_masking_functions';
--source include/rpl/connection_replica.inc
INSTALL COMPONENT 'file://component_masking_functions';
--source include/rpl/connection_source.inc


--echo #
--echo # Random number generation functions
--echo #

--echo # gen_range()
--let $check_determinism_expression = gen_range(0, 1048575)
--source check_determinism.inc


--echo #
--echo # Random string generation functions
--echo #

--echo # gen_rnd_canada_sin()
--let $check_determinism_expression = gen_rnd_canada_sin()
--source check_determinism.inc

--echo # gen_rnd_email()
--let $check_determinism_expression = gen_rnd_email()
--source check_determinism.inc
--let $check_determinism_expression = gen_rnd_email(4)
--source check_determinism.inc
--let $check_determinism_expression = gen_rnd_email(4, 6)
--source check_determinism.inc
--let $check_determinism_expression = gen_rnd_email(4, 6, 'foo.com')
--source check_determinism.inc

--echo # gen_rnd_iban()
--let $check_determinism_expression = gen_rnd_iban()
--source check_determinism.inc
--let $check_determinism_expression = gen_rnd_iban('UK')
--source check_determinism.inc
--let $check_determinism_expression = gen_rnd_iban('UK', 20)
--source check_determinism.inc

--echo # gen_rnd_pan()
--let $check_determinism_expression = gen_rnd_pan()
--source check_determinism.inc

--echo # gen_rnd_ssn()
--let $check_determinism_expression = gen_rnd_ssn()
--source check_determinism.inc

--echo # gen_rnd_uk_nin()
--let $check_determinism_expression = gen_rnd_uk_nin()
--source check_determinism.inc

--echo # gen_rnd_us_phone()
--let $check_determinism_expression = gen_rnd_us_phone()
--source check_determinism.inc

--echo # gen_rnd_uuid()
--let $check_determinism_expression = gen_rnd_uuid()
--source check_determinism.inc


# Currently MySQL Server considers any UDF (including those that have 'true'
# in the 'const_item') as not safe for statement-based replication. So,
# despite the fact that all masking ('maks_xxx()') functions are deterministic,
# MySQL still generates a warning when they are used. This is why the following
# section does the same checks as for the 'gen_rnd_xxx()'.

--echo #
--echo # Masking functions
--echo #

--echo # mask_canada_sin()
--let $check_determinism_expression = mask_canada_sin('046-454-286')
--source check_determinism.inc
--let $check_determinism_expression = mask_canada_sin('046-454-286', '_')
--source check_determinism.inc

--echo # mask_iban()
--let $check_determinism_expression = mask_iban('IE12BOFI90000112345678')
--source check_determinism.inc
--let $check_determinism_expression = mask_iban('IE12BOFI90000112345678', '_')
--source check_determinism.inc

--echo # mask_inner()
--let $check_determinism_expression = mask_inner('abcdefgh', 2, 4)
--source check_determinism.inc
--let $check_determinism_expression = mask_inner('abcdefgh', 2, 4, '_')
--source check_determinism.inc

--echo # mask_outer()
--let $check_determinism_expression = mask_outer('abcdefgh', 2, 4)
--source check_determinism.inc
--let $check_determinism_expression = mask_outer('abcdefgh', 2, 4, '_')
--source check_determinism.inc

--echo # mask_pan()
--let $check_determinism_expression = mask_pan('1234567887654321')
--source check_determinism.inc
--let $check_determinism_expression = mask_pan('1234567887654321', '_')
--source check_determinism.inc

--echo # mask_pan_relaxed()
--let $check_determinism_expression = mask_pan_relaxed('1234567887654321')
--source check_determinism.inc
--let $check_determinism_expression = mask_pan_relaxed('1234567887654321', '_')
--source check_determinism.inc

--echo # mask_ssn()
--let $check_determinism_expression = mask_ssn('909-63-6922')
--source check_determinism.inc
--let $check_determinism_expression = mask_ssn('909-63-6922', '_')
--source check_determinism.inc

--echo # mask_uk_nin()
--let $check_determinism_expression = mask_uk_nin('QQ123456C')
--source check_determinism.inc
--let $check_determinism_expression = mask_uk_nin('QQ123456C', '_')
--source check_determinism.inc

--echo # mask_uuid()
--let $check_determinism_expression = mask_uuid('8594a73a-5190-4f75-bafc-12b3153309eb')
--source check_determinism.inc
--let $check_determinism_expression = mask_uuid('8594a73a-5190-4f75-bafc-12b3153309eb', '_')
--source check_determinism.inc


--echo #
--echo # Dictionary functions
--echo #

--source include/rpl/connection_source.inc
CREATE TABLE mysql.masking_dictionaries(
    Dictionary VARCHAR(256) NOT NULL,
    Term VARCHAR(256) NOT NULL,
    UNIQUE INDEX dictionary_term_idx (Dictionary, Term)
) ENGINE = InnoDB DEFAULT CHARSET=utf8mb4;

--let $current_user = `SELECT USER()`
eval GRANT MASKING_DICTIONARIES_ADMIN ON *.* TO $current_user;

--echo # masking_dictionary_term_add()
SELECT masking_dictionary_term_add('dict1', 'word11');
SELECT masking_dictionary_term_add('dict1', 'word12');
SELECT masking_dictionary_term_add('dict2', 'word21');
SELECT masking_dictionary_term_add('dict2', 'word22');

--source include/rpl/sync_to_replica.inc
--let $assert_cond = [ SELECT COUNT(*) FROM mysql.masking_dictionaries ] = 4
--let $assert_text =  mysql.masking_dictionaries must have 4 records
--source include/assert.inc

--echo # gen_dictionary()
--let $check_determinism_expression = gen_dictionary('dict1')
--source check_determinism.inc

--echo # gen_blocklist()
--let $check_determinism_expression = gen_blocklist('word11', 'dict1', 'dict2')
--source check_determinism.inc

--echo # masking_dictionary_term_remove()
--source include/rpl/connection_source.inc
SELECT masking_dictionary_term_remove('dict2', 'word21');
SELECT masking_dictionary_term_remove('dict2', 'word22');

--source include/rpl/sync_to_replica.inc
--let $assert_cond = [ SELECT COUNT(*) FROM mysql.masking_dictionaries ] = 2
--let $assert_text =  mysql.masking_dictionaries must have 2 records
--source include/assert.inc

--echo # masking_dictionary_remove()
--source include/rpl/connection_source.inc
SELECT masking_dictionary_remove('dict1');

--source include/rpl/sync_to_replica.inc
--let $assert_cond = [ SELECT COUNT(*) FROM mysql.masking_dictionaries ] = 0
--let $assert_text =  mysql.masking_dictionaries must have no records
--source include/assert.inc


--source include/rpl/connection_source.inc
eval REVOKE MASKING_DICTIONARIES_ADMIN ON *.* FROM $current_user;
DROP TABLE mysql.masking_dictionaries;

--source include/rpl/connection_replica.inc
UNINSTALL COMPONENT 'file://component_masking_functions';
--source include/rpl/connection_source.inc
UNINSTALL COMPONENT 'file://component_masking_functions';

--source include/rpl/deinit.inc
