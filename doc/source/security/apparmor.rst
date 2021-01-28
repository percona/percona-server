.. _enable-apparmor:

=================================================
Working with AppArmor 
=================================================

The operating system has a Discretionary Access Controls (DAC) system. AppArmor supplements the DAC with a Mandatory Access Control (MAC) system. AppArmor is the default security module for Ubuntu or Debian systems and uses profiles to define how programs access resources. 

AppArmor is path-based and restricts processes by using profiles. Each profile contains a set of policy rules. Some applications may install their profile along with the application. If an installation does not also install a profile, then that application is not part of the AppArmor subsystem. You can also create profiles since they are simple text files stored in the ``/etc/apparmor.d`` directory. 

Profiles are one of the following modes:

* Enforce - the default setting, applications are prevented from taking actions restricted by the profile rules.

* Complain - applications are allowed to take restricted actions, and the actions are logged.

* Disabled - Applications are allowed to take restricted actions, and the actions are not logged. 

 You can mix enforce profiles and complain profiles. 

Checking the Current Status
-----------------------------

As root or using ``sudo``, you can check the AppArmor status:

.. sourcecode:: bash

    $ sudo aa-status
    apparmor module is loaded.
    34 profiles are loaded.
    32 profiles in enforce mode.
    ...
        /usr/sbin/mysqld
    ...
    2 profiles in complain mode.
    ...
    3 profiles have profiles defined.
    ...
    0 processes are in complain mode.
    0 processes are unconfined but have a profile defined.

.. _complain-one:

Change a Profile to Complain mode
-----------------------------------------

To improve a profile, change to the ``complain`` mode and then run any actions that generates log entries. Update the profile from the log information.

To change a profile to complain mode and the program is in your path:

.. sourcecode:: bash

    $ sudo aa-complain <program>

If needed, specify the path:

.. sourcecode:: bash

    $ sudo aa-complain /sbin/<program>

If the profile is not in stored in /etc/apparmor.d, use the following command:

.. sourcecode:: bash

    $ sudo aa-complain /path/to/profiles/<program>

.. _enforce-one:

Change a Profile to Enforce mode
---------------------------------

To change a profile to enforce mode and the program is in your path:

.. sourcecode:: bash

    $ sudo aa-enforce <program>

If needed, specify the path:

.. sourcecode:: bash

    $ sudo aa-enforce /sbin/<program>

To change enforce mode and the profile is not stored in /etc/apparmor, use the following command:

.. sourcecode:: bash

    $ sudo aa-enforce /path/to/profile

.. _disable-one:

Disabling one profile
------------------------------

You can disable a profile but change the profile to the ``complain`` mode if you are troubleshooting. Use either of the following methods:

.. sourcecode:: bash

    $ sudo ln -s /etc/apparmor.d/usr.sbin.mysqld /etc/apparmor.d/disable/ 
    $ sudo apparmor_parser -R /etc/apparmor.d/usr.sbin.mysqld

or

.. sourcecode:: bash

    $ aa-disable /etc/apparmor.d/usr.sbin.mysqld

Reloading all profiles
-----------------------

Run the following command to reload all profiles:

.. sourcecode:: bash

    $ sudo service apparmor reload

.. _reload-one:

Reloading one profile
----------------------

To reload one profile, run the following:

.. sourcecode:: bash

    $ sudo apparmor_parser -r /etc/apparmor.d/<profile>

For some changes to take effect, you may need to restart the program.

Disabling AppArmor
--------------------

AppArmor should not be disabled, if possible. If you have a profile with issues, change the profile to ``complain`` mode during troubleshooting. If AppArmor must be disabled, run the following commands:

1. Check the status.

.. sourcecode:: bash

    $ sudo apparmor_status

2. Stop and disable AppArmor.

.. sourcecode:: bash

    $ sudo systemctl stop apparmor
    $ sudo systemctl disable apparmor

Modifying the mysqld profile
---------------------------------

Install the mysqld profile with the following procedure: 

1. Install the ``apparmor-utils`` package to work with profiles:

    .. sourcecode:: bash

        $ sudo apt-get -y install apparmor-utils
        Reading package lists... Done
        Building dependency tree
        ...
        The following additional packages will be installed:
            python3-apparmor python3-libapparmor
        ...

2. Download the current version of the AppArmor from upstream:

    ..  sourcecode:: bash

        $ wget https://raw.githubusercontent.com/mysql/mysql-server/8.0/packaging/deb-in/extra/apparmor-profile
        ...
        Saving to 'apparamor-profile`
        ...

3. Move the file to `/etc/apparmor.d/usr.sbin.mysqld`

    .. sourcecode:: bash

        $ mv apparmor-profile /etc/apparmor.d/usr.sbin.mysqld

4. Create an empty file for editing:

    .. sourcecode:: bash

        $ touch /etc/apparmor.d/local/usr.sbin.mysqld

5. Load the profile:

    .. sourcecode:: bash

        $ apparmor_parser -r -T -W /etc/apparmor.d/usr.sbin.mysqld

6. Restart |Percona Server|:

    .. sourcecode:: bash

        $ systemctl restart mysql

7. Check the profile status:

    .. sourcecode:: bash

        $ sudo aa-status
        ...
        processes are in enforce mode
        ...
        /usr/sbin/mysqld (100840)
        ...

Editing the mysqld profile
---------------------------

Only edit :file:`/etc/apparmor.d/local/usr.sbin.mysql`. Follow the steps to :ref:`complain-one`. Open the file in any text editor to make the changes. Follow the steps in :ref:`reload-one` to enable the profile. When you are satisfied with the update, follow the steps to :ref:`enforce-one`.

Configuring a custom data directory
------------------------------------

You can change the data directory to a non-default location, like `/var/lib/mysqlcustom`. You should enable audit mode and edit the profile to allow access.

.. sourcecode:: bash

    $ cat /etc/mysql/mysql.conf.d/mysqld.cnf 
    #
    # The Percona Server 8.0 configuration file.
    #
    # For explanations see
    # http://dev.mysql.com/doc/mysql/en/server-system-variables.html

    [mysqld]
    pid-file    = /var/run/mysqld/mysqld.pid
    socket        = /var/run/mysqld/mysqld.sock
    *datadir    = /var/lib/mysqlcustom*
    log-error    = /var/log/mysql/error.log

Enable audit mode for mysqld. The security policy is enforced and all access is logged.

.. sourcecode:: bash

    $ aa-audit mysqld

Restart Percona Server for MySQL.

.. sourcecode:: bash

    $ systemctl mysql restart

The restart fails because AppArmor blocked access to the custom data directory.  

.. sourcecode:: bash

    ...
    Dec 07 12:17:08 ubuntu-s-4vcpu-8gb-nyc1-01-aa-ps audit[16013]: AVC apparmor="DENIED" operation="mknod" profile="/usr/sbin/mysqld" name="/var/lib/mysqlcustom/binlog.index" pid=16013 comm="mysqld" requested_mask="c" denied_mask="c" fsuid=111 ouid=111
    Dec 07 12:17:08 ubuntu-s-4vcpu-8gb-nyc1-01-aa-ps kernel: audit: type=1400 audit(1607343428.022:36): apparmor="DENIED" operation="mknod" profile="/usr/sbin/mysqld" name="/var/lib/mysqlcustom/mysqld_tmp_file_case_insensitive_test.lower-test" pid=16013 comm="mysqld" requested_mask="c" denied_mask="c" fsuid=111 ouid=111
    ...

In :file:`/etc/apparmor.d/local/usr.sbin.mysqld`, change the following entries in this section in your text editor:

..  sourcecode:: text

        # Allow data dir access
        /var/lib/mysqlcustom/ r,
        /var/lib/mysqlcustom/** rwk,

In :file:`etc/apparmor.d/usr.sbin.mysqld`, comment out, using the `#` symbol, the entries in the `Allow data dir access` section. This step is optional. If you skip this step, mysqld continues to have access to the default data directory location.

.. note::

    Edit the local version of the file instead of the main profile. Separating the changes makes maintenance easier. 

Reload the profile:

.. sourcecode:: bash

    $apparmor_parser -r -T /etc/apparmor.d/usr.sbin.mysqld

Restart mysql:

.. sourcecode:: bash

    $ systemctl restart mysqld


Setting up a custom log location
----------------------------------

To move your logs to a custom location, you must edit my.cnf and edit the profile to allow access:
    

.. sourcecode:: text

    cat /etc/mysql/mysql.conf.d/mysqld.cnf 
    #
    # The Percona Server 8.0 configuration file.
    #
    # For explanations see
    # http://dev.mysql.com/doc/mysql/en/server-system-variables.html

    [mysqld]
    pid-file    = /var/run/mysqld/mysqld.pid
    socket        = /var/run/mysqld/mysqld.sock
    datadir    = /var/lib/mysql
    log-error    = /*custom-log-dir*/mysql/error.log

Verify the directory exists.

.. sourcecode:: bash 

    $ ls -la /custom-log-dir/
    total 12
    drwxrwxrwx  3 root root 4096 Dec  7 13:09 .
    drwxr-xr-x 24 root root 4096 Dec  7 13:07 ..
    drwxrwxrwx  2 root root 4096 Dec  7 13:09 mysql

Restart Percona Server.

..  sourcecode:: bash

    $ service mysql start
    Job for mysql.service failed because the control process exited with error code.
    See "systemctl status mysql.service" and "journalctl -xe" for details.


    root@ubuntu-s-4vcpu-8gb-nyc1-01-aa-ps:~# journalctl -xe
    ...
    AVC apparmor="DENIED" operation="mknod" profile="/usr/sbin/mysqld" name="/custom-log-dir/mysql/error.log"
    ...

The access has been denied by AppArmor. Edit the local profile to allow the custom data directory.

..  sourcecode:: bash

    $ cat /etc/apparmor.d/local/usr.sbin.mysqld 
    # Site-specific additions and overrides for usr.sbin.mysqld..
    # For more details, please see /etc/apparmor.d/local/README.

    # Allow log file access
    /custom-log-dir/mysql/ r,
    /custom-log-dir/mysql/** rw,

Reload the profile:

.. sourcecode:: bash

    $apparmor_parser -r -T /etc/apparmor.d/usr.sbin.mysqld

Restart mysql:

.. sourcecode:: bash

    $ systemctl restart mysqld

Setting ``secure_file_priv`` directory
---------------------------------------

By default, `secure_file_priv` points to the following location:

..  sourcecode:: mysql

    mysql> show variables like 'secure_file_priv';
    +------------------+-----------------------+
    | Variable_name    | Value                 |
    +------------------+-----------------------+
    | secure_file_priv | /var/lib/mysql-files/ |
    +------------------+-----------------------+

To allow access, in a text editor, open the apparmor profile. Review the current setting:

..  sourcecode:: text

    # Allow data dir access
    /var/lib/mysql/ r,
    /var/lib/mysql/** rwk,

Edit the local profile to allow the custom data directory.

..  sourcecode:: bash

    $ cat /etc/apparmor.d/local/usr.sbin.mysqld 
    # Site-specific additions and overrides for usr.sbin.mysqld..
    # For more details, please see /etc/apparmor.d/local/README.

    # Allow data dir access
    /var/lib/mysqlcustom/ r,
    /var/lib/mysqlcustom/** rwk,

Reload the profile:

.. sourcecode:: bash

    $apparmor_parser -r -T /etc/apparmor.d/usr.sbin.mysqld

Restart mysql:

.. sourcecode:: bash

    $ systemctl restart mysqld


.. seealso::

    `Ubuntu and AppArmor <https://ubuntu.com/server/docs/security-apparmor>`_
    
    `Ubuntu Wiki AppArmor <https://wiki.ubuntu.com/AppArmor>`_



