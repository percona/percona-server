# Save initial setting
SELECT @@global.caching_sha2_password_storage_format INTO @saved_storage_format;
SELECT @@global.caching_sha2_password_enforce_storage_format INTO @saved_enforce_storage_format;

SET GLOBAL caching_sha2_password_storage_format="CRYPT5";

# Accounts with CRYPT5
CREATE USER arthur IDENTIFIED WITH caching_sha2_password BY 'abcd';
CREATE USER marvin IDENTIFIED WITH caching_sha2_password BY 'efgh';

# Accounts with PBKDF2_SHA512
SET GLOBAL caching_sha2_password_storage_format="PBKDF2_SHA512";

CREATE USER zaphod IDENTIFIED WITH caching_sha2_password BY 'ijkl';
CREATE USER ford IDENTIFIED WITH caching_sha2_password BY 'mnop';

SELECT user, SUBSTRING(authentication_string, 1, 7) as format FROM mysql.user WHERE user IN ('arthur', 'marvin', 'zaphod', 'ford') ORDER BY user ASC;

# Try connecting - Must succeed
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uarthur -pabcd -e "SELECT CURRENT_USER()" 2>&1

# Must succeed without TLS
--connect(conn_arthur_without_ssl, localhost, arthur, abcd,,,,)
SELECT CURRENT_USER();

# Set password - Must use PBKDF2_SHA512 format
SET PASSWORD = 'abcde';

--connection default
--disconnect conn_arthur_without_ssl

SELECT SUBSTRING(authentication_string, 1, 7) FROM mysql.user WHERE user = 'arthur';

# Try connecting - Must succeed
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uarthur -pabcde -e "SELECT CURRENT_USER()" 2>&1

# Must succeed without TLS
--connect(conn_arthur_without_ssl, localhost, arthur, abcde,,,,)
SELECT CURRENT_USER();

--connection default
--disconnect conn_arthur_without_ssl

# Try connecting - Must succeed
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uzaphod -pijkl -e "SELECT CURRENT_USER()" 2>&1

# Must succeed without TLS
--connect(conn_zaphod_without_ssl, localhost, zaphod, ijkl,,,,)
SELECT CURRENT_USER();

SET PASSWORD = 'ijklm';

--connection default
--disconnect conn_zaphod_without_ssl

SELECT SUBSTRING(authentication_string, 1, 7) FROM mysql.user WHERE user = 'zaphod';

# Change format
SET GLOBAL caching_sha2_password_storage_format="CRYPT5";

FLUSH PRIVILEGES;

# Try connecting - Must succeed
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uzaphod -pijklm -e "SELECT CURRENT_USER()" 2>&1

# Must succeed without TLS
--connect(conn_zaphod_without_ssl, localhost, zaphod, ijklm,,,,)
SELECT CURRENT_USER();

SET PASSWORD = 'ijkl';

--connection default
--disconnect conn_zaphod_without_ssl

# At this point we have
# arthur, ford - Using PBKDF2_SHA512
# marvin, zaphod - Using CRYPT5

SELECT user, SUBSTRING(authentication_string, 1, 7) as format FROM mysql.user WHERE user IN ('arthur', 'marvin', 'zaphod', 'ford') ORDER BY user ASC;

# Dual password

--connection default
DROP USER arthur, marvin, zaphod, ford;

SET GLOBAL caching_sha2_password_storage_format="CRYPT5";

# Accounts with CRYPT5
CREATE USER arthur IDENTIFIED WITH caching_sha2_password BY 'abcd';
CREATE USER marvin IDENTIFIED WITH caching_sha2_password BY 'efgh';

# Accounts with PBKDF2_SHA512
SET GLOBAL caching_sha2_password_storage_format="PBKDF2_SHA512";

CREATE USER zaphod IDENTIFIED WITH caching_sha2_password BY 'ijkl';
CREATE USER ford IDENTIFIED WITH caching_sha2_password BY 'mnop';

# Set second password
ALTER USER arthur IDENTIFIED BY 'efgh' RETAIN CURRENT PASSWORD;
ALTER USER marvin IDENTIFIED BY 'abcd' RETAIN CURRENT PASSWORD;

SET GLOBAL caching_sha2_password_storage_format="CRYPT5";

ALTER USER zaphod IDENTIFIED BY 'mnop' RETAIN CURRENT PASSWORD;
ALTER USER ford IDENTIFIED BY 'ijkl' RETAIN CURRENT PASSWORD;

# Current state
# User    CRYPT5        PBKDF2_SHA512
# arthur   abcd(2nd)     efgh(1st)
# marvin  efgh(2nd)     abcd(1st)
# zaphod  mnop(1st)     ijkl(2nd)
# ford    ijkl(1st)     mnop(2nd)

SELECT user, SUBSTRING(authentication_string, 1, 7) as first, SUBSTRING(JSON_EXTRACT(User_attributes, "$.additional_password"), 2, 7) as second FROM mysql.user WHERE user IN ('arthur', 'marvin', 'zaphod', 'ford') ORDER BY user ASC;

# Try with first password
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uarthur -pefgh -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -umarvin -pabcd -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uzaphod -pmnop -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uford -pijkl -e "SELECT CURRENT_USER()" 2>&1

# Try with second password
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uarthur -pabcd -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -umarvin -pefgh -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uzaphod -pijkl -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uford -pmnop -e "SELECT CURRENT_USER()" 2>&1

FLUSH PRIVILEGES;

SET GLOBAL caching_sha2_password_storage_format="PBKDF2_SHA512";

# Try with first password
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uarthur -pefgh -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -umarvin -pabcd -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uzaphod -pmnop -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uford -pijkl -e "SELECT CURRENT_USER()" 2>&1

# Try with second password
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uarthur -pabcd -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -umarvin -pefgh -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uzaphod -pijkl -e "SELECT CURRENT_USER()" 2>&1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED -uford -pmnop -e "SELECT CURRENT_USER()" 2>&1

# Tests for caching_sha2_password_enforce_format

# Current state
# User    CRYPT5        PBKDF2_SHA512
# arthur   abcd(2nd)     efgh(1st)
# marvin  efgh(2nd)     abcd(1st)
# zaphod  mnop(1st)     ijkl(2nd)
# ford    ijkl(1st)     mnop(2nd)

--connection default
SET GLOBAL caching_sha2_password_storage_format="PBKDF2_SHA512";
SET GLOBAL caching_sha2_password_enforce_storage_format=1;

# Try connecting with password stored in non-compliant format
--connect(conn_arthur_first_pass, localhost, arthur, abcd,,,,SSL)

--error ER_MUST_CHANGE_PASSWORD
SELECT 1;

--connection default
--disconnect conn_arthur_first_pass

# Try using fast auth - Must fail
--error 1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uarthur -pabcd -e "SELECT CURRENT_USER()" 2>&1

# Try connecing with password stored in compliant format

--connect(conn_marvin_second_pass, localhost, marvin, abcd,,,,SSL)

SELECT 1;

--connection default
--disconnect conn_marvin_second_pass

# Must succeed
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -umarvin -pabcd -e "SELECT CURRENT_USER()" 2>&1

# Try with another password in compliant format
--connect(conn_arthur_second_pass, localhost, arthur, efgh,,,,SSL)

SELECT 1;

--connection default
--disconnect conn_arthur_second_pass

# Must succeed
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uarthur -pefgh -e "SELECT CURRENT_USER()" 2>&1

# Try connecing with another password stored in non-compliant format
--connect(conn_marvin_first_pass, localhost, marvin, efgh,,,,SSL)

--error ER_MUST_CHANGE_PASSWORD
SELECT 1;

--connection default
--disconnect conn_marvin_first_pass

# Try using fast auth - Must fail
--error 1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -umarvin -pefgh -e "SELECT CURRENT_USER()" 2>&1

# Try connecting with password stored in non-compliant format and reset password
--connect(conn_arthur_first_pass, localhost, arthur, abcd,,,,SSL)

--error ER_MUST_CHANGE_PASSWORD
SELECT 1;

SET PASSWORD = 'abcd';

SELECT 1;

--connection default
--disconnect conn_arthur_first_pass

# Try using fast auth - Must fail
--error 1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uarthur -pabcd -e "SELECT CURRENT_USER()" 2>&1

# Must succeed - secure channel
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED --protocol=TCP -uarthur -pabcd -e "SELECT CURRENT_USER()" 2>&1

# Must succeed with fast auth now - compliant password format
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uarthur -pabcd -e "SELECT CURRENT_USER()" 2>&1

# Try connecing with another password stored in non-compliant format and reset password
--connect(conn_marvin_first_pass, localhost, marvin, efgh,,,,SSL)

--error ER_MUST_CHANGE_PASSWORD
SELECT 1;

SET PASSWORD='efgh';

SELECT 1;

--connection default
--disconnect conn_marvin_first_pass

# Try using fast auth - Must fail
--error 1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -umarvin -pefgh -e "SELECT CURRENT_USER()" 2>&1

# Must succeed - secure channel
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED --protocol=TCP -umarvin -pefgh -e "SELECT CURRENT_USER()" 2>&1

# Must succeed with fast auth now - compliant password format
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -umarvin -pefgh -e "SELECT CURRENT_USER()" 2>&1

--connection default
SET GLOBAL caching_sha2_password_storage_format="CRYPT5";
SET GLOBAL caching_sha2_password_enforce_storage_format=1;

# Try connecting with password stored in non-compliant format
--connect(conn_zaphod_second_pass, localhost, zaphod, ijkl,,,,SSL)

--error ER_MUST_CHANGE_PASSWORD
SELECT 1;

--connection default
--disconnect conn_zaphod_second_pass

# Try using fast auth - Must fail
--error 1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uzaphod -pijkl -e "SELECT CURRENT_USER()" 2>&1

# Try connecing with password stored in compliant format

--connect(conn_ford_first_pass, localhost, ford, ijkl,,,,SSL)

SELECT 1;

--connection default
--disconnect conn_ford_first_pass

# Must succeed
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uford -pijkl -e "SELECT CURRENT_USER()" 2>&1

# Try with another password in compliant format
--connect(conn_zaphod_first_pass, localhost, zaphod, mnop,,,,SSL)

SELECT 1;

--connection default
--disconnect conn_zaphod_first_pass

# Must succeed
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uzaphod -pmnop -e "SELECT CURRENT_USER()" 2>&1

# Try connecing with another password stored in non-compliant format
--connect(conn_ford_second_pass, localhost, ford, mnop,,,,SSL)

--error ER_MUST_CHANGE_PASSWORD
SELECT 1;

--connection default
--disconnect conn_ford_second_pass

# Try using fast auth - Must fail
--error 1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uford -pmnop -e "SELECT CURRENT_USER()" 2>&1

# Try connecting with password stored in non-compliant format and reset password
--connect(conn_zaphod_second_pass, localhost, zaphod, ijkl,,,,SSL)

--error ER_MUST_CHANGE_PASSWORD
SELECT 1;

SET PASSWORD = 'ijkl';

SELECT 1;

--connection default
--disconnect conn_zaphod_second_pass

# Try using fast auth - Must fail
--error 1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uzaphod -pijkl -e "SELECT CURRENT_USER()" 2>&1

# Must succeed with fast auth now - compliant password format
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED --protocol=TCP -uzaphod -pijkl -e "SELECT CURRENT_USER()" 2>&1

# Must succeed with fast auth now - compliant password format
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uzaphod -pijkl -e "SELECT CURRENT_USER()" 2>&1

# Try connecing with another password stored in non-compliant format and reset password
--connect(conn_ford_second_pass, localhost, ford, mnop,,,,SSL)

--error ER_MUST_CHANGE_PASSWORD
SELECT 1;

SET PASSWORD='mnop';

SELECT 1;

--connection default
--disconnect conn_ford_second_pass

# Try using fast auth - Must fail
--error 1
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uford -pmnop -e "SELECT CURRENT_USER()" 2>&1

# Must succeed with fast auth now - compliant password format
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=REQUIRED --protocol=TCP -uford -pmnop -e "SELECT CURRENT_USER()" 2>&1

# Must succeed with fast auth now - compliant password format
--exec $MYSQL --host=127.0.0.1 -P $MASTER_MYPORT --ssl-mode=DISABLED --protocol=TCP -uford -pmnop -e "SELECT CURRENT_USER()" 2>&1

# -------------------------------------------------------------------

# Drop users
DROP USER arthur, marvin, zaphod, ford;

# Revert to initial setting
SET GLOBAL caching_sha2_password_enforce_storage_format=@saved_enforce_storage_format;
SET GLOBAL caching_sha2_password_storage_format=@saved_storage_format;
