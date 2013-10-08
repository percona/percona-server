==================================================
 About Percona PAM Authentication Plugin for MySQL
==================================================

Percona PAM Authentication Plugin is a free and Open Source implementation of the MySQL's authentication plugin. This plugin acts as a mediator between the MySQL server, the MySQL client, and the PAM stack. The server plugin requests authentication from the PAM stack, forwards any requests and messages from the PAM stack over the wire to the client (in cleartext) and reads back any replies for the PAM stack.

 PAM plugin uses dialog as its client side plugin. Dialog plugin can be loaded to any client application that uses libmysqlclient library.

Here are some of the benefits that Percona dialog plugin offers over the default one:

  * It correctly recognizes whether PAM wants input to be echoed or not, while the default one always echoes the input on the user's console.
  * It can use the password which is passed to |MySQL| client via "-p" parameter.
  * Dialog client `installation bug <http://bugs.mysql.com/bug.php?id=60745>`_ has been fixed.
  * This plugin works on |MySQL| and |Percona Server|.

Percona offers two versions of this plugin:  

  * Full PAM plugin called *auth_pam*. This plugin uses *dialog.so*. It fully supports the PAM protocol with arbitrary communication between client and server.
  * Oracle-compatible PAM called *auth_pam_compat*. This plugin uses *mysql_clear_password* which is a part of Oracle MySQL client. It also has some limitations, such as, it supports only one password input. You must use "-p" option in order to pass the password to auth_pam_compat.

These two versions of plugins are physically different. To choose which one you want used, you must use *IDENTIFIED WITH 'auth_pam'* for auth_pam, and *IDENTIFIED WITH 'auth_pam_compat'* for auth_pam_compat.


