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

Yes, but you will need to run mysqld as root so that the PAM libraries can access /etc/shadow.
