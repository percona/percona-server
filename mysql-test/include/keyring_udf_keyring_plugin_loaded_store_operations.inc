--replace_regex /\.dll/.so/
eval create function keyring_key_store returns integer soname '$KEYRING_UDF';

# Error cases -- wrong argument count
--error ER_CANT_INITIALIZE_UDF
select keyring_key_store('Key_1');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_store('Key_1','AES');

# Error cases -- wrong argument type
--error ER_CANT_INITIALIZE_UDF
select keyring_key_store('Key_1','AES',123);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_store(NULL,'AES',123);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_store('Key_1',NULL,123);
select keyring_key_store('Key_1','AES',NULL);
select keyring_key_store(NULL,NULL,NULL);
--error ER_CANT_INITIALIZE_UDF
select keyring_key_store(1234,NULL,'53247@#$%^');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_store(1,'AES','123');
--error ER_CANT_INITIALIZE_UDF
select keyring_key_store('Key_1',123,'123');

# Store AES_128
let key_id = `select CONCAT('Rob_AES_128', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_store('$key_id','AES',"0123456789012345");
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');

# Store AES_192
let key_id = `select CONCAT('Rob_AES_192', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_store('$key_id','AES',"012345678901234567890%@3");
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');

# Store AES_256
let key_id = `select CONCAT('Rob_AES_256', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_store('$key_id','AES',"01234567890123456789012345678901");
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');

# Store RSA_1024
let key_id = `select CONCAT('Rob_RSA_1024', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_store('$key_id','RSA',"01234567890123456789012345678901234567890123456789012345678901230123456789012345678901234567890123456789012345678901234567890123");
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');

# Store DSA 1024
let key_id = `select CONCAT('Rob_DSA_1024', '$server_uuid')`;
--replace_result $key_id key_id
eval select keyring_key_store('$key_id','DSA',"01234567890123456789012345678901234567890123456789012345678901230123456789012345678901234567890123456789012345678901234567890123");
--replace_result $key_id key_id
eval select keyring_key_fetch('$key_id') into @x;
select LENGTH(@x);
--replace_result $key_id key_id
eval select keyring_key_type_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_length_fetch('$key_id');
--replace_result $key_id key_id
eval select keyring_key_remove('$key_id');

# Store wrong key type
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_store('Wrong_type','xxx', '0123456789012345') into @x;
select @x;
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
select keyring_key_store('','AES', '0123456789012345') into @x;
select @x;

# Store really long key
select repeat('abcdefgh', 2049) into @x;
--error ER_CLIENT_KEYRING_UDF_KEY_TOO_LONG
select keyring_key_store('long_key', 'AES', @x);
select repeat('abcdefgh', 17) into @x;
