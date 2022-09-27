.. _selinux:

===========================
Working with SELinux
===========================

The Linux kernel, through the Linux Security Module (LSM), supports Security-Enhanced Linux (SELinux). This module provides a way to support mandatory access control policies. SELinux defines how confined processes interact with files, network ports, directories, other processes, and additional server components. 

An SELinux policy defines the set of rules, the ``types`` for files, and the ``domains`` for processes. Rules determine how a process interacts with another type. SELinux decides whether to allow or deny an action based on the subject's context, what object initiates the action and what object is the action's target. 

A label represents the context for administrators and users. 

CentOS 7 and CentOS 8 contain a MySQL SELinux policy. *Percona Server for MySQL* is a drop-in replacement for MySQL and can use this policy without changes. 

SELinux context example
------------------------------

To view the SELinux context, add the ``-Z`` switch to many of the utilities. Here is an example of the context for ``mysqld``:

    .. sourcecode:: bash

        $ ps -eZ | grep mysqld_t
        system_u:system_r:mysqld_t:s0    3356 ?        00:00:01 mysqld

The context has the following properties:

* User - system_u

* Role - system_r

* Type or domain - mysqld_t

* Sensitivity level - s0    3356

Most SELinux policy rules are based on the type or domain. 

List SELinux Types or Domains associated with files
----------------------------------------------------

The security property that SELinux relies on is the Type security property. The type name often end with a ``_t``. A group of objects with the same type security value belongs to the same domain. 

To view the ``mysqldb_t`` types associated with the MySQL directories and files, run the following command:

    .. sourcecode:: bash

        $ ls -laZ /var/lib/ | grep mysql
        drwxr-x--x. mysql   mysql   system_u:object_r:mysqld_db_t:s0 mysql
        drwxr-x---. mysql   mysql   system_u:object_r:mysqld_db_t:s0 mysql-files
        drwxr-x---. mysql   mysql   system_u:object_r:mysqld_db_t:s0 mysql-keyring

.. note::

    If a policy type does not define the type property for an object, the default value is ``unconfined_t``. 

SELinux modes
-------------

SELinux has the following modes:

* Disabled - No SELinux policy modules loaded, which disables policies. Nothing is reported.

* Permissive - SELinux is active, but policy modules are not enforced. A policy violation is reported but does not stop the action. 

* Enforcing - SELinux is active, and violations are reported and denied. If there is no rule to allow access to a confined resource, SELinux denies the access.

Policy Types 
----------------

SELinux has several policy types:

* Targeted - Most processes operate without restriction. Specific services are contained in security domains and defined by policies.

* Strict - All processes are contained in security domains and defined by policies.

SELinux has confined processes that run in a domain and restricts everything unless explicitly allowed. An unconfined process in an unconfined domain is allowed almost all access. 

MySQL is a confined process, and the policy module defines which files are read, which ports are opened, and so on. SELinux assumes the *Percona Server for MySQL* installation uses the default file locations and default ports. 

If you change the default, you must also edit the policy. If you do not update the policy, SELinux, in enforcing mode, denies access to all non-default resources.

Check the SELinux mode
---------------------------

To check the current SELinux mode, use either of the following commands:

    .. sourcecode:: bash

        $ sestatus
        SELinux status:                 enabled
        SELinuxfs mount:                /sys/fs/selinux
        SELinux root directory:         /etc/selinux
        Loaded policy name:             targeted
        Current mode:                   enforcing
        Mode from config file:          enforcing
        Policy MLS status:              enabled
        Policy deny_unknown status:     allowed
        Memory protection checking:     actual (secure)
        Max kernel policy version:      31

or 

    .. sourcecode:: bash

        $ grep ^SELINUX= /etc/selinux/config
        SELINUX=enforcing

    .. note:: Add the ``-b`` parameter to ``sestatus`` to display the ``Policy booleans``. The boolean values for each parameter is shown. An example of using the ``b`` parameter is the following:

        .. sourcecode:: bash

            $ sestatus -b | grep mysql
            mysql_connect_any                           off
            selinuxuser_mysql_connect_enabled


The ``/etc/selinux/config`` file controls if SELinux is disabled or enabled, and if enabled, whether SELinux operates in enforcing mode or permissive mode.




Disable SELinux
------------------

If you plan to use the enforcing mode at another time, use the permissive mode instead of disabling SELinux. During the time that SELinux is disabled, the system may contain mislabeled objects or objects with no label. If you re-enable SELinux and plan to set SELinux to enforcing, you must follow the steps to :ref:`relabel`.

On boot, to disable SELinux, set the ``selinux=0`` kernel option. The kernel does not load the SELinux infrastructure. This option has the same effect as changing the ``SELINUX=disabled`` instruction in the configuration file and then rebooting the system. 

Additional SELinux tools
---------------------------------

Install the SELinux management tools, such as ``semanage`` or ``sesearch``, if needed. 

On RHEL 7 or compatible operating systems, use the following command as root:

    .. sourcecode:: bash

        $ yum -y install policycoreutils-python

On RHEL 8 or compatible operating systems, use the following command as root:

    .. sourcecode:: bash

        $ yum -y install policycoreutils-python-utils 

.. note:: You may need root privileges to run SELinux management commands.

.. _changing-selinux-mode:

Switch the mode in the configuration file
------------------------------------------------------

Switching between modes may help when troubleshooting or when modifying rules. 

To permanently change the mode, edit the ``/etc/selinux/config`` file and change the ``SELINUX=`` value. You should also verify the change. 

    .. sourcecode:: bash

        $ cat /etc/selinux/config | grep SELINUX= | grep -v ^#
        SELINUX=enforcing
        SELINUX=enforcing

        $ sudo sed -i 's/^SELINUX=.*/SELINUX=permissive/g' /etc/selinux/config

        $ cat /etc/selinux/config | grep SELINUX= | grep -v ^#
        SELINUX=permissive
        SELINUX=permissive

Reboot your system after the change.

If switching from either disabled mode or permissive mode to enforcing, see :ref:`relabel`.

Switch the mode until the next reboot
-----------------------------------------

To change the mode until the next reboot, use either of the following commands as root:

    .. sourcecode:: bash

        $ setenforce Enforcing

or 

    .. sourcecode:: bash

        $ setenforce 1

    .. note:: The following ``setenforce`` parameters are available:

            +-----------------------+----------------+
            | setenforce parameters | Also Permitted |
            +=======================+================+
            | 0                     | Permissive     |
            +-----------------------+----------------+
            | 1                     | Enforcing      |
            +-----------------------+----------------+

You can view the current mode by running either of the following commands:

    .. sourcecode:: bash

        $ getenforce
        Enforcing

or 

    .. sourcecode:: bash

        $ sestatus | grep -i mode
        Current mode:                   permissive
        Mode from config file:          enforcing


Switch the mode for a service
---------------------------------

You can move one or more services into a permissive domain. The other services remain in enforcing mode. 

To add a service to the permissive domain, run the following as root:

    .. sourcecode:: bash

        $ sudo semanage permissive -a mysqld_t

To list the current permissive domains, run the following command:

    .. sourcecode:: bash

        $ sudo semanage permissive -l 
        ...
        Customized Permissive Types

        mysqld_t

        Builtin Permissive Types

To delete a service from the permissive domain, run the following:

    .. sourcecode:: bash

        $ sudo semanage permissive -d mysqld_t

The service returns to the system's SELinux mode. Be sure to follow the steps to :ref:`relabel`.

.. _relabel:

Relabel the entire file system
-------------------------------------

Switching from disabled or permissive to enforcing requires additional steps. The enforcing mode requires the correct contexts, or labels, to function. The permissive mode allows users and processes to label files and system objects incorrectly. The disabled mode does not load the SELinux infrastructure and does not label resources or processes. 

RHEL and compatible systems, use the ``fixfiles`` application for relabeling. You can relabel the entire file system or the file contexts of an application. 

For one application, run the following command:

    .. sourcecode:: bash

        $ fixfiles -R mysqld restore

To relabel the file system without rebooting the system, use the following command:

    .. sourcecode:: bash

        $ fixfiles -f -F relabel

Another option relabels the file system during a reboot. You can either add a touch file, read during the reboot operation, or configure a kernel boot parameter. The completion of the relabeling operation automatically removes the touch file.

Add the touch file as root:

    .. sourcecode:: bash

        $ touch /.autorelabel

To configure the kernel, add the ``autorelabel=1`` kernel parameter to the boot parameter list. The parameter forces a system relabel. Reboot in permissive mode to allow the process to complete before changing to enforcing. 

.. note:: Relabeling an entire filesystem takes time. When the relabeling is complete, the system reboots again. 

.. _selinux-custom-data-directory:

Set a Custom Data directory
--------------------------------

If you do not use the default settings, SELinux, in enforcing mode, prevents access to the system.

For example, during installation, you have used the following configuration:

    .. sourcecode:: text

        datadir=/var/lib/mysqlcustom
        socket=/var/lib/mysqlcustom/mysql.sock

Restart the service.

    .. sourcecode:: bash

        $ service mysqld restart
        Redirecting to /bin/systemctl restart mysqld.service
        Job for mysqld.service failed because the control process exited with error code.
        See "systemctl status mysqld.service" and "journalctl -xe" for details.

Check the journal log to see the error code.

    .. sourcecode:: bash

        $ journalctl -xe
        ...
        SELinux is preventing mysqld from getattr access to the file /var/lib/mysqlcustom/ibdata1.
        ... 

Check the SELinux types in ``/var/lib/mysqlcustom``.

    .. sourcecode:: bash

        ls -1aZ /var/lib/mysqlcustom
        total 164288
        drwxr-x--x.  6 mysql mysql system_u:object_r:var_lib_t:s0       4096 Dec  2 07:58  .
        drwxr-xr-x. 38 root  root  system_u:object_r:var_lib_t:s0       4096 Dec  1 14:29  ..
        ...
        -rw-r-----.  1 mysql mysql system_u:object_r:var_lib_t:s0   12582912 Dec  1 14:29  ibdata1
        ...

To solve the issue, use the following methods:

* Set the proper labels for ``mysqlcustom`` files

* Change the mysqld SELinux policy to allow mysqld access to ``var_lib_t`` files.

The recommended solution is to set the proper labels. The following procedure assumes you have already created and set ownership to the custom data directory location:

1. To change the SELinux context, use ``semanage fcontext``. In this step, you define how SELinux deals with the custom paths:

    .. sourcecode:: bash

        $ semanage fcontext -a -e /var/lib/mysql /var/lib/mysqlcustom

    SELinux applies the same labeling schema, defined in the mysqld policy, for the ``/var/lib/mysql`` directory to the custom directory. Files created within the custom directory are labeled as if they were in ``/var/lib/mysql``. 

2. To ``restorecon`` command applies the change. 

    .. sourcecode:: bash

        $ restorecon -R -v /var/lib/mysqlcustom

3. Restart the mysqld service:

    .. sourcecode:: bash

        $ service mysqld start

.. _selinux-custom-logs:

Set a Custom Log Location 
------------------------------

If you do not use the default settings, SELinux, in enforcing mode, prevents access to the location. Change the log location to a custom location in my.cnf:

    .. sourcecode:: text

        log-error=/logs/mysqld.log

Verify the log location with the following command:

    .. sourcecode:: bash

        $ ls -laZ /
        ...
        drwxrwxrwx.   2 root root unconfined_u:object_r:default_t:s0    6 Dec  2 09:16 logs
        ...

Starting MySQL returns the following message:

    .. sourcecode:: bash

        $ service mysql start
        Redirecting to /bin/systemctl start mysql.service
        Job for mysqld.service failed because the control process exited with error code.
        See "systemctl status mysqld.service" and "journalctl -xe" for details.

        $ journalctl -xe
        ...
        SELinux is preventing mysqld from write access to the directory logs.
        ...

The default SELinux policy allows mysqld to write logs into a location tagged with ``var_log_t``, which is the ``/var/log`` location. You can solve the issue with either of the following methods:

* Tag the ``/logs`` location properly

* Edit the SELinux policy to allow mysqld access to all directories.

To tag the custom ``/logs`` location is the recommended method since it locks down access. Run the following commands to tag the custom location:

    .. sourcecode:: bash

        $ semanage fcontext -a -t var_log_t /logs
        $ restorecon -v /logs

You may not be able to change the ``/logs`` directory label. For example, other applications, with their own rules, use the same directory. 

To adjust the SELinux policy when a directory is shared, follow these steps:

1. Create a local policy:

    .. sourcecode:: bash

        ausearch -c 'mysqld' --raw | audit2allow -M my-mysqld

2. This command generates the my-mysqld.te and the my-mysqld.pp files. The mysqld.te is the type enforcement policy file. The my-mysqld.pp is the policy module loaded as a binary file into the SELinux subsystem.

    An example of the my-myslqd.te file:

    .. sourcecode:: text

        module my-mysqld 1.0;

        require {
            *type mysqld_t*;
            type var_lib_t;
            *type default_t*;
            class file getattr;
            *class dir write*;
        }

        #============= mysqld_t ==============
        *allow mysqld_t default_t:dir write*;
        allow mysqld_t var_lib_t:file getattr;

    The policy contains rules for the custom data directory and the custom logs directory. We have set the proper labels for the data directory location, and applying this autogenerated policy would loosen our hardening by allowing mysqld to access ``var_lib_t`` tags. 

3. SELinux-generated events are converted to rules. A generated policy may contain rules for recent violations and include unrelated rules. Unrelated rules are generated from actions, such as changing the data directory location, that are not related to the logs directory. Add the ``--start`` parameter to use log events after a specific time to filter out the unwanted events. This parameter captures events when the time stamp is equal to the specified time or later. SELinux generates a policy for the current actions.

    .. sourcecode:: bash

        $ ausearch --start 10:00:00 -c 'mysqld' --raw | audit2allow -M my-mysqld

4. This policy allows mysqld writing into the tagged directories. Open the my_mysqld file:

    .. sourcecode:: text

        module my-mysqld 1.0;

        require {
            type mysqld_t;
            type default_t;
            class dir write;
        }

        #============= mysqld_t ==============
        allow mysqld_t default_t:dir write;

5. Install the SELinux policy module:

    .. sourcecode:: bash

        $ semodule -i my-mysqld.pp

Restart the service. If you have a failure, check the journal log and follow the same procedure.

If SELinux prevents mysql from creating a log file inside the directory. You can view all the violations by changing the SELinux mode to ``permissive`` and then running mysqld. All violations are logged in the journal log. After this run, you can generate a local policy module, install it, and switch SELinux back to ``enforcing`` mode. Follow this procedure:

1. Unload the current local my-mysqld policy module:

    .. sourcecode:: bash

        $ semodule -r my-mysqld

2. You can put a single domain into permissive mode. Other domains on the system to remain in enforcing mode. Use ``semanage permissive`` with the ``-a`` parameter to change mysqld_t to permissive mode:

    .. sourcecode:: bash

        $ semanage permissive -a mysqld_t

3. Verify the mode change:

    .. sourcecode:: bash

        semdule -l | grep permissive
        ...
        permissive_mysqld_t
        ...

4. To make searching the log easier, return the time:

    .. sourcecode:: bash

        $ date

5. Start the service.

    .. sourcecode:: bash

        $ service mysqld start

6. MySQL starts, and SELinux logs the violations in the journal log. Check the journal log:

    .. sourcecode:: bash

        $ journalctl -xe

7. Stop the service:

    .. sourcecode:: bash

        $ service mysqld stop 

8. Generate a local mysqld policy, using the time returned from step 4:

    .. sourcecode:: bash

        $ ausearch --start <date> -c 'mysqld' --raw | audit2allow -M my-mysqld

9. Review the policy (the policy you generate may be different):

    .. sourcecode:: bash

        $ cat my-mysqld.te
        module my-mysqld 1.0;

        require {
        type default_t;
            type mysqld_t;
            class dir { add_name write };
            class file { append create open };
        }

        #============= mysqld_t ==============
        allow mysqld_t default_t:dir { add_name write };
        allow mysqld_t default_t:file { append create open };

10. Install the policy:

    .. sourcecode:: bash

        $ semodule -i my-mysqld.pp

11. Use ``semanage permissive`` with the ``-d`` parameter, which deletes the permissive domain for the service:

    .. sourcecode:: bash

        $ semanage permissive -d mysqld_t

12. Restart the service:

    .. sourcecode:: bash

            $ service mysqld start

.. note::

    Use this procedure to adjust the local mysqld policy module. You should review the changes which are generated to ensure the rules are not too tolerant.

.. _selinux-secure-file-priv:

Set ``secure_file_priv`` directory
---------------------------------------

Update the SELinux tags for the ``/var/lib/mysql-files/`` directory, used for ``SELECT ... INTO OUTFILE`` or similar operations, if required. The server needs only read/write access to the destination directory.

To set ``secure_file_priv`` to use this directory, run the following commands to set the context:

    .. sourcecode:: bash

        $ semanage fcontext -a -t mysqld_db_t "/var/lib/mysql-files/(/.*)?"
        $ restorecon -Rv /var/lib/mysql-files

Edit the path for a different location, if needed.

.. seealso::

    `SELinux and MySQL <https://blogs.oracle.com/mysql/selinux-and-mysql-v2>`_

    `Red Hat SELinux User's and Administrator's Guide <https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/selinux_users_and_administrators_guide/index>`_

    `CentOS HowTos SELinux <https://wiki.centos.org/HowTos/SELinux>`_


















