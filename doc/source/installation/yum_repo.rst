.. _yum_repo:

===================================
 Percona :program:`yum` Repository
===================================

The |Percona| :program:`yum` repository supports popular *RPM*-based operating systems, including the *Amazon Linux AMI*.

The easiest way to install the *Percona Yum* repository is to install an *RPM* that configures :program:`yum` and installs the `Percona GPG key <http://www.percona.com/downloads/RPM-GPG-KEY-percona>`_.

Automatic Installation
======================

Execute the following command as a ``root`` user: ::

  $ yum install http://www.percona.com/downloads/percona-release/redhat/0.1-3/percona-release-0.1-3.noarch.rpm

You should see some output such as the following: ::

  Retrieving http://www.percona.com/downloads/percona-release/redhat/0.1-3/percona-release-0.1-3.noarch.rpm
  Preparing...                ########################################### [100%]
     1:percona-release        ########################################### [100%]

Testing The Repository
======================

Make sure packages are downloaded from the repository, by executing the following command as root: ::

  yum list | grep percona

You should see output similar to the following: ::

  percona-release.noarch                     0.1-3                         @/percona-release-0.1-3.noarch
  ...
  Percona-Server-client-56.x86_64            5.6.15-rel63.0.519.rhel6      percona
  Percona-Server-devel-56.x86_64             5.6.15-rel63.0.519.rhel6      percona
  Percona-Server-server-56.x86_64            5.6.15-rel63.0.519.rhel6      percona
  Percona-Server-shared-56.x86_64            5.6.15-rel63.0.519.rhel6      percona
  Percona-Server-test-56.x86_64              5.6.15-rel63.0.519.rhel6      percona
  ...
  percona-xtrabackup.x86_64                  2.2.4-5004.el6                percona


Supported Platforms
===================

  *  ``x86_64``
  *  ``i386``

Supported Releases
==================

The *CentOS* repositories should work well with *Red Hat Enterprise Linux* too, provided that :program:`yum` is installed on the server.

* *CentOS* 5 and *RHEL* 5

* *CentOS* 6 and *RHEL* 6 (Current Stable) [#f1]_ 

* *CentOS* 7 and *RHEL* 7

* *Amazon Linux AMI* (works the same as *CentOS* 5)

Percona `yum` Testing repository
================================

Percona offers pre-release builds from the testing repository. To subscribe to the testing repository, you'll need to enable the testing repository in :file:`/etc/yum.repos.d/percona-release.repo` (both ``$basearch`` and ``noarch``). **NOTE:** You'll need to install the Percona repository first if this hasn't been done already.

.. rubric:: Footnotes

.. [#f1] "Current Stable": We support only the current stable RHEL6/CentOS6 release, because there is no official (i.e. RedHat provided) method to support or download the latest OpenSSL on RHEL/CentOS versions prior to 6.5. Similarly, and also as a result thereof, there is no official Percona way to support the latest Percona Server builds on RHEL/CentOS versions prior to 6.5. Additionally, many users will need to upgrade to OpenSSL 1.0.1g or later (due to the `Heartbleed vulnerability <http://www.percona.com/resources/ceo-customer-advisory-heartbleed>`_), and this OpenSSL version is not available for download from any official RHEL/Centos repository for versions 6.4 and prior. For any officially unsupported system, src.rpm packages may be used to rebuild Percona Server for any environment. Please contact our `support service <http://www.percona.com/products/mysql-support>`_ if you require further information on this.
