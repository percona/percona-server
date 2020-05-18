.. _docker:

============================================
Running Percona Server in a Docker Container
============================================

Docker images of Percona Server are hosted publicly on Docker Hub at
https://hub.docker.com/r/percona/percona-server/.

For more information about using Docker, see the `Docker Docs`_.

.. _`Docker Docs`: https://docs.docker.com/

.. note:: Make sure that you are using the latest version of Docker.
   The ones provided via ``apt`` and ``yum``
   may be outdated and cause errors.

.. note:: By default, Docker will pull the image from Docker Hub
   if it is not available locally.

Using the Percona Server Images
===============================

The following procedure describes how to run and access Percona Server 5.6
using Docker.

Starting a Percona Server Instance in a Container
-------------------------------------------------

To start a container named ``ps``
running the latest version in the Percona Server 5.6 series,
with the root password set to ``root``::

 [root@docker-host] $ docker run -d \
   --name ps \
   -e MYSQL_ROOT_PASSWORD=root \
   percona/percona-server:5.6

.. note:: ``root`` is not a secure password.

.. note::

    The `docker stop` command sends a `TERM` signal. Docker waits 10 seconds
    and sends a `KILL` signal. Very large instances cannot dump the data from
    memory to disk in 10 seconds. If you plan to run a very large instance, add
    the following option to the `docker run` command.

    --stop-timeout 600

Accessing the Percona Server Container
--------------------------------------

To access the shell in the container::

 [root@docker-host] $ docker exec -it ps /bin/bash

From the shell, you can view the error log::

 [mysql@ps] $ more /var/log/mysql/error.log
 2017-08-29T04:20:22.190474Z 0 [Warning] 'NO_ZERO_DATE', 'NO_ZERO_IN_DATE' and 'ERROR_FOR_DIVISION_BY_ZERO' sql modes should be used with strict mode. They will be merged with strict mode in a future release.
 2017-08-29T04:20:22.190520Z 0 [Warning] 'NO_AUTO_CREATE_USER' sql mode was not set.
 ...

You can also run the MySQL command-line client
to access the database directly::

 [mysql@ps] $ mysql -uroot -proot
 mysql: [Warning] Using a password on the command line interface can be insecure.
 Welcome to the MySQL monitor.  Commands end with ; or \g.
 Your MySQL connection id is 4
 Server version: 5.7.19-17 Percona Server (GPL), Release '17', Revision 'e19a6b7b73f'

 Copyright (c) 2009-2017 Percona LLC and/or its affiliates
 Copyright (c) 2000, 2017, Oracle and/or its affiliates. All rights reserved.

 Oracle is a registered trademark of Oracle Corporation and/or its affiliates. Other names may be trademarks of their respective owners.

 Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

 mysql>

Accessing Percona Server from Application in Another Container
--------------------------------------------------------------

The image exposes the standard MySQL port 3306,
so container linking makes Percona Server instance available
from other containers.
To link a container running your application
(in this case, from image named ``app/image``)
with the Percona Server container,
run it with the following command::

 [root@docker-host] $ docker run -d \
   --name app \
   --link ps \
   app/image:latest

This application container will be able to access the Percona Server container
via port 3306.

Environment Variables
=====================

When running a Docker container with Percona Server,
you can adjust the configuration of the instance
by passing one or more environment variables with the ``docker run`` command.

.. note:: These variables will not have any effect
   if you start the container with a data directory
   that already contains a database:
   any pre-existing database will always remain untouched on container startup.

The variables are optional,
except that you must specify at least one of the following:

* :variable:`MYSQL_ALLOW_EMPTY_PASSWORD`: least secure, use only for testing.

* :variable:`MYSQL_ROOT_PASSWORD`: more secure,
  but setting the password on the command line is not recommended
  for sensitive production setups.

* :variable:`MYSQL_RANDOM_ROOT_PASSWORD`: most secure,
  recommended for production.

  .. note:: To further secure your instance,
     use the :variable:`MYSQL_ONETIME_PASSWORD` variable
     if you are running version 5.6 or later.

.. variable:: MYSQL_ALLOW_EMPTY_PASSWORD

  Specifies whether to allow the container
  to be started with a blank password for the MySQL root user.
  Disabled by default.
  To enable, set ``MYSQL_ALLOW_EMPTY_PASSWORD=yes``.

  .. note:: Allowing empty root password is not recommended for production,
     because anyone will have full superuser access to the database.

.. variable:: MYSQL_DATABASE

  Specifies the name of the database to be created when running the container.
  To create a user with full access to this database (``GRANT ALL``),
  set the :variable:`MYSQL_USER` and :variable:`MYSQL_PASSWORD` variables.

.. variable:: MYSQL_ONETIME_PASSWORD

  Specifies whether the password for the MySQL root user
  should be set as expired.
  Disabled by default.
  If enabled using ``MYSQL_ONETIME_PASSWORD=yes``,
  the MySQL root password must be changed before using it to log in.

.. variable:: MYSQL_PASSWORD

  Specifies the password for the user with full access to the database
  specified by the :variable:`MYSQL_DATABASE` variable.
  Setting the :variable:`MYSQL_USER` variable is also required.

.. variable:: MYSQL_RANDOM_ROOT_PASSWORD

  Specifies whether a random password for the MySQL root user
  should be generated.
  Disabled by default.
  To enable, set ``MYSQL_RANDOM_ROOT_PASSWORD=yes``.

  The password will be printed to ``stdout`` in the container,
  and it can be viewed using the ``docker logs`` command.

.. variable:: MYSQL_ROOT_PASSWORD

  Specifies the password for the MySQL root user.

  .. note:: Setting the MySQL root password on the command line is insecure.
     It is recommended to set a random password
     using the :variable:`MYSQL_RANDOM_ROOT_PASSWORD` variable.

.. variable:: MYSQL_USER

  Specifies the name for the user with full access to the database
  specified by the :variable:`MYSQL_DATABASE` variable.
  Setting the :variable:`MYSQL_PASSWORD` variable is also required.

Storing Data
============

There are two ways to store data used by applications
that run in Docker containers:

* Let Docker manage the storage of your data
  by writing the database files to disk on the host system
  using its own internal volume management.

* Create a data directory on the host system
  (outside the container on high performance storage)
  and mount it to a directory visible from inside the container.
  This places the database files in a known location on the host system,
  and makes it easy for tools and applications on the host system
  to access the files.
  The user should make sure that the directory exists,
  and that permissions and other security mechanisms on the host system
  are set up correctly.

For example, if you create a data directory on a suitable volume
on your host system named ``/local/datadir``,
you run the container with the following command::

 [root@docker-host] $ docker run -d \
   --name ps \
   -e MYSQL_ROOT_PASSWORD=root \
   -v /local/datadir:/var/lib/mysql \
   percona/percona-server:5.6

The ``-v /local/datadir:/var/lib/mysql`` option
mounts the ``/local/datadir`` directory on the host
to ``/var/lib/mysql`` in the container,
which is the default data directory used by Percona Server.

.. note:: If you the Percona Server container instance
   with a data directory that already contains data
   (the ``mysql`` subdirectory where all our system tables are stored),
   the :variable:`MYSQL_ROOT_PASSWORD` variable should be omitted
   from the ``docker run`` command.

.. note:: If you have SELinux enabled,
   assign the relevant policy type to the new data directory,
   so that the container will be allowed to access it::

    [root@docker-host] $ chcon -Rt svirt_sandbox_file_t /local/datadir

Port Forwarding
===============

Docker allows mapping ports on the container to ports on the host system
using the ``-p`` option.
If you run the container with this option,
you can connect to the database by connecting your client
to a port on the host machine.
This can greatly simplify consolidating many instances to a single host.

To map the standard MySQL port 3306 to port 6603 on the host::

  [root@docker-host] $ docker run -d \
   --name ps \
   -e MYSQL_ROOT_PASSWORD=root \
   -p 6603:3306 \
   percona/percona-server:5.6

Passing Options to Percona Server
=================================

You can pass options to Percona Server when running the container
by appending them to the ``docker run`` command.
For example, to start run Percona Server with UTF-8
as the default setting for character set
and collation for all databases::

  [root@docker-host] $ docker run -d \
   --name ps \
   -e MYSQL_ROOT_PASSWORD=root \
   percona/percona-server:5.6 \
   --character-set-server=utf8 \
   --collation-server=utf8_general_ci

