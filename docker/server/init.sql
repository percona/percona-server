-- Verify plugin loaded
SELECT plugin_name, plugin_status
FROM information_schema.plugins
WHERE plugin_name = 'authentication_appkey';

-- Create the appkey test account.
-- host = '%' so find_mpvio_user() matches any client IP without requiring the
-- find_mpvio_user() patch in the server binary.
-- The expected appkey is stored in the auth string: IDENTIFIED BY 'appkey:...'
-- The plugin reads info->auth_string to get the expected appkey value.
CREATE USER IF NOT EXISTS 'appkey_user'@'%'
  IDENTIFIED WITH authentication_appkey BY 'appkey:com.test.app';

GRANT SELECT ON mysql.* TO 'appkey_user'@'%';

-- Verify account created
SELECT user, host, plugin
FROM mysql.user
WHERE user = 'appkey_user';
