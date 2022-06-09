============================
 Frequently Asked Questions
============================

Is there a Windows version?
===========================

No, Windows does not support PAM, so there will not be a Windows version.

Can I use it with MySQL?
========================

Yes.

Can I use it with Percona Server?
=================================

Yes.


Is it Free and Open Source Software?
====================================

Yes.


Can I use the PAM plugin to authenticate against /etc/shadow?
=============================================================

Yes, you need to add the mysql user to the shadow group. Because PAM libraries, such as 'pam_unix.so', need to access /etc/shadow.

For example this is how you can do it in *Ubuntu*: ::

   root@lucid64:/var/lib/mysql# getent group shadow
   shadow:x:42:mysql

   root@lucid64:/var/lib/mysql# ls -alhs /etc/shadow
   4.0K -rw-r----- 1 root shadow 912 Dec 21 10:39 /etc/shadow

After you restart mysqld for changes to take effect, pam_unix authentication will work.

The other option is to run mysqld as root. This should be used for testing only or as a last resort method.


I'm getting the: "ERROR 2059 (HY000): Authentication plugin 'auth_pam' cannot be loaded"
========================================================================================

This means that the default client :option:`plugin-dir` setting doesn't work or it isn't set up properly. You'll need to add the location of the plugin folder to your client configuration: :: 
  
  [client]
  plugin_dir='/usr/lib/mysql/plugin'
