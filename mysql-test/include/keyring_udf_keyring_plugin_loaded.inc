--replace_regex /\.dll/.so/
eval create function keyring_key_fetch returns string soname '$KEYRING_UDF';
--replace_regex /\.dll/.so/
eval create function keyring_key_type_fetch returns string soname '$KEYRING_UDF';
--replace_regex /\.dll/.so/
eval create function keyring_key_length_fetch returns integer soname '$KEYRING_UDF';
--replace_regex /\.dll/.so/
eval create function keyring_key_remove returns integer soname '$KEYRING_UDF';
--replace_regex /\.dll/.so/
eval create function keyring_key_generate returns integer soname '$KEYRING_UDF';

# Error cases -- wrong argument count
--error ER_CANT_INITIALIZE_UDF
select keyring_key_fetch('Key_1','AES');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_type_fetch('Key_1','AES');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_length_fetch('Key_1','AES');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_remove('Key_1','AES');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Key_1');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Key_1','AES');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Key_1','');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_fetch('Key_1',NULL);
# End of wrong count

# Error cases -- wrong argument type
--error ER_CANT_INITIALIZE_UDF
select keyring_key_fetch(1);
select keyring_key_fetch(NULL);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_type_fetch(1);
select keyring_key_type_fetch(NULL);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_length_fetch(1);
select keyring_key_length_fetch(NULL);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Key_1','AES','123');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate(NULL,'AES','123');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Key_1',NULL,'123');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Key_1','AES',NULL);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate(NULL,NULL,NULL);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate(1,'AES',123);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Key_1',123,'123');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Key_invalid_key_type',123,123);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_remove(1);
select keyring_key_remove(NULL);
# End of wrong argument type

#Check the plugins

select PLUGIN_NAME,PLUGIN_AUTHOR from information_schema.plugins where PLUGIN_NAME like '%keyring%';

# Genuine cases

# AES
# Generate AES_128
let key_id = `select CONCAT('Rob_AES_128', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','AES',16);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
# Generate AES_192
let key_id = `select CONCAT('Rob_AES_192', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id', 'AES', 24);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
# Generate AES_256
let key_id = `select CONCAT('Rob_AES_256', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','AES',32);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
# RSA
# Generate RSA_1024
let key_id = `select CONCAT('Rob_RSA_1024', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','RSA',128);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
#Generate RSA_2048
let key_id = `select CONCAT('Rob_RSA_2048', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','RSA',256);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
#Generate RSA_4098
let key_id = `select CONCAT('Rob_RSA_4096', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','RSA',512);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
# DSA
# Generate DSA 1024
let key_id = `select CONCAT('Rob_DSA_1024', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','DSA',128);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
# Generate DSA 2048
let key_id = `select CONCAT('Rob_DSA_2048', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','DSA',256);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
# Generate DSA 3072
let key_id = `select CONCAT('Rob_DSA_3072', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','DSA',384);
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');

# Fetch not existing:
select keyring_key_fetch('key') into @x;
select @x;
select keyring_key_type_fetch('key') into @x;
select @x;
select keyring_key_length_fetch('key') into @x;
select @x;

--echo #Testing with AES_ENCRYPT/AES_DECRYPT

let key_id = `select CONCAT('AES_128', '$server_uuid')`;
--replace_result $key_id key_id
select keyring_key_generate('$key_id','AES',16);
--replace_result $key_id key_id
select AES_ENCRYPT('secret message', keyring_key_fetch('$key_id')) into @cipher;
--replace_result $key_id key_id
select AES_DECRYPT(@cipher, keyring_key_fetch('$key_id'));

--replace_result $key_id key_id
select keyring_key_remove('$key_id');

# Fetch non-existent key - we should get empty string as result
select keyring_key_fetch('Rob1');
# Fetch non-existent type
select keyring_key_type_fetch('Rob1');
# Fetch length not existing
select keyring_key_length_fetch('Rob1');

# Errors comming from keyring
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_fetch('') into @x;
select @x;
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_length_fetch('') into @x;
select @x;
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_type_fetch('') into @x;
select @x;
# Remove not existing
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_remove('Rob_not_existing') into @x;
select @x;
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_remove('') into @x;
select @x;
# Generate wrong key type
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_generate('Wrong_type','xxx', 16) into @x;
select @x;
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_generate('','AES', 16) into @x;
select @x;

--echo # Testing privileges

let roots_key_id = `select CONCAT('roots_key', '$server_uuid')`;
--replace_result $roots_key_id roots_key_id
eval select keyring_key_generate('$roots_key_id', 'AES',16);

create definer=root@localhost procedure shared_key() select keyring_key_fetch('$roots_key_id');

CREATE USER user_execute_test@localhost;
connect(conn_no_execute, localhost, user_execute_test);

--error ER_CANT_INITIALIZE_UDF
select keyring_key_generate('Rob_DSA_no_privilege','DSA',384);

connection default;
disconnect conn_no_execute;
GRANT EXECUTE ON *.* TO 'user_execute_test'@'localhost';

connect(conn_with_execute, localhost, user_execute_test);

let key_id = `select CONCAT('Rob_DSA_no_privilege', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','DSA',384);
let key_id = `select CONCAT('another_Rob_DSA_no_privilege', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','DSA',384);

--echo #Check if user you can access another user's keys
--replace_result $roots_key_id roots_key_id
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
eval select keyring_key_remove('$roots_key_id');

--echo #Check that the user is able to access the key via procedure
--replace_column 1 #
call shared_key();

connection default;

DROP PROCEDURE shared_key;

--echo #Check if a user can create a key with id that already exist but belongs
--echo #to another user.

let key_id = `select CONCAT('Rob_DSA_no_privilege', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_generate('$key_id','DSA',384);

--echo #Check if user with larger privileges can access another user's key

let key_id = `select CONCAT('another_Rob_DSA_no_privilege', '$server_uuid')`;

--replace_result $key_id key_id
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
eval select keyring_key_remove('$key_id');

--echo #Cleanup

let key_id = `select CONCAT('Rob_DSA_no_privilege', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
--replace_result $roots_key_id roots_key_id
eval select keyring_key_remove('$roots_key_id');
connection conn_with_execute;
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');
let key_id = `select CONCAT('another_Rob_DSA_no_privilege', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');

connection default;
disconnect conn_with_execute;
DROP USER 'user_execute_test'@'localhost';

# End of genuine cases

