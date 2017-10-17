.. _docker_images:

=======================================
Setting up Percona Server Docker images
=======================================

Percona Server Docker images, are created and maintained by the Percona
engineering team.

You can see available versions in the `full list of tags
<https://registry.hub.docker.com/u/percona/percona-server/tags/manage/>`_.

These images are updated when new releases are published with older
versions still available.

How to Use the Images
=====================

This documents shows how to download and set up latest |Percona Server| 5.6
image.

Start a Percona Server Instance
-------------------------------

Start a Percona Server container as follows:

.. code-block:: bash

  $ docker run --name container-name -e MYSQL_ROOT_PASSWORD=secret -d percona/percona-server:5.6

Where ``container-name`` is the name you want to assign to your container,
``secret`` is the password to be set for the root user and tag is the tag
specifying the version you want.

Connecting from an Application in Another Docker Container
----------------------------------------------------------

This image exposes the standard |MySQL| port (3306), so container linking makes
the instance available to other containers. Start other containers like this
in order to link it to a |Percona Server| container:

.. code-block:: bash

  $ docker run --name app-container-name --link container-name -d app-that-uses-mysql

In this example ``container-name`` is the name of your database container and
the ``app-that-uses-mysql`` the name of the application.

Connecting from the MySQL Command Line Client
---------------------------------------------

You can use the following command to start another container instance and run
the |MySQL| command line client against your original container. This allows
you to execute SQL statements against your database:

.. code-block:: bash

  $ docker run -it --link container-name --rm percona/percona-server:tag mysql -h container-name -P 3306 -uroot -psecret

In this example ``container-name`` is the name of your database container.

Container Shell Access and Viewing MySQL Log Files
--------------------------------------------------

You can use the ``docker exec`` command to run commands inside a Docker
container. To access shell you can run the following command:

.. code-block:: bash

  $ docker exec -it my-container-name /bin/bash

Once you have the shell access you now have access to the |Percona Server|
log file. Log file can be found in :file:`/var/lib/mysql/errorr.log`.

You can use ``more`` to check its content:

.. code-block:: bash

  $ more /var/log/mysqld.log

Environment Variables
=====================

When you start a |Percona Server| container, you can adjust the configuration
of the instance by passing one or more environment variables on the docker run
command line. Do note that none of the variables below will have any effect if
you start the container with a data directory that already contains a database:
any pre-existing database will always be left untouched on container startup.

Most of the variables listed below are optional, but one of the variables:

 * :variable:`MYSQL_ROOT_PASSWORD`,
 * :variable:`MYSQL_ALLOW_EMPTY_PASSWORD`, or
 * :variable:`MYSQL_RANDOM_ROOT_PASSWORD` must be given.

.. variable:: MYSQL_ROOT_PASSWORD

  This variable specifies a password that will be set for the root superuser
  account. In the above example, it was set to secret. NOTE: Setting the
  |MySQL| root user password on the command line is insecure.

.. variable:: MYSQL_RANDOM_ROOT_PASSWORD

  When this variable is set to yes, a random password for the server's root
  user will be generated. The password will be printed to stdout in the
  container, and it can be obtained by using the command docker logs
  container-name.

.. variable:: MYSQL_ONETIME_PASSWORD

  This variable is optional. When set to yes, the root user's password will be
  set as expired, and must be changed before we can login normally.

.. variable:: MYSQL_DATABASE

  This variable is optional. It allows you to specify the name of a database to
  be created on image startup. If a user/password was supplied (see below) then
  that user will be granted superuser access (corresponding to ``GRANT ALL``)
  to this database.

.. variable:: MYSQL_USER, MYSQL_PASSWORD

  These variables are optional, used in conjunction to create a new user and
  set that user's password. This user will be granted superuser permissions
  (see above) for the database specified by the :variable:`MYSQL_DATABASE`
  variable. Both variables are required for a user to be created.

  Do note that there is no need to use this mechanism to create the root
  superuser, this user gets created by default with the password set by either
  of the mechanisms (given or generated) discussed above.

.. variable:: MYSQL_ALLOW_EMPTY_PASSWORD

  Set to yes to allow the container to be started with a blank password for the
  root user. NOTE: Setting this variable to yes is not recommended unless you
  really know what you are doing, since this will leave your instance
  completely unprotected, allowing anyone to gain complete superuser access.

Notes, Tips, Gotchas
====================

Secure Container Startup
------------------------

In many use cases, employing the :variable:`MYSQL_ROOT_PASSWORD` variable to
specify the |MySQL| root user password on initial container startup is
insecure. Instead, to keep your setup as secure as possible, we strongly
recommend using the :variable:`MYSQL_RANDOM_ROOT_PASSWORD` option. To further
secure your instance, we also recommend using the
:variable:`MYSQL_ONETIME_PASSWORD` variable.

Storing Data
------------

There are many two ways to store data used by applications that run in Docker
containers. We maintain our usual stance and encourage users to investigate
the options and use the method that best suits their use case. Here are some
of the options available:

* Let Docker manage the storage of your database data by writing the database
  files to disk on the host system using its own internal volume management.
  The current solutions, ``devicemapper``, ``aufs`` and ``overlayfs`` have
  negative performance records.

* Create a data directory on the host system (outside the container on high
  performance storage) and mount this to a directory visible from inside the
  container. This places the database files in a known location on the host
  system, and makes it easy for tools and applications on the host system to
  access the files. The user needs to make sure that the directory exists, and
  that permissions and other security mechanisms on the host system are set up
  correctly.

The `Docker documentation
<https://docs.docker.com/engine/userguide/storagedriver/>`_ is a good starting
point for understanding the different storage options and variations, and there
are multiple blog and forum postings that discuss and give advice in this area.
We will simply show the basic procedure here for the latter option above:

1. Create a data directory on a suitable volume on your host system, like
   :file:`/local/datadir`.

2. Start your container like this:

.. code-block:: bash

  $ docker run --name container-name -v /local/datadir:/var/lib/mysql -e MYSQL_ROOT_PASSWORD=secret -d percona/percona-server:tag

The ``-v /local/datadir:/var/lib/mysql`` part of the command mounts the
:file:`/local/datadir` directory from the underlying host system as
:file:`/var/lib/mysql` inside the container, where |MySQL| by default will
write its data files.

Note that users on systems with *SELinux* enabled may experience problems with
this. The current workaround is to assign the relevant *SELinux* policy type
to the new data directory so that the container will be allowed to access it:

.. code-block:: bash

  $ chcon -Rt svirt_sandbox_file_t /local/datadir

Existing Data
-------------

If you start your |MySQL| container instance with a data directory that already
contains a data (specifically, a mysql subdirectory where all our system tables
live), the :variable:`MYSQL_ROOT_PASSWORD` variable should be omitted from the
docker run command.

Port forwarding
---------------

Docker allows mapping of ports on the container to ports on the host system by
using the ``-p`` option. If you start the container as follows, you can connect
to the database by connecting your client to a port on the host machine. This
can greatly simplify consolidating many instances to a single host. In this
example port ``6603``, the we use the address of the docker host to connect to
the TCP port the docker deamon is forwarding from:

.. code-block:: bash

  $ docker run --name container-name `-p 6603:3306` -d percona/percona-server mysql -h docker_host_ip -P 6603

Passing options to the server
-----------------------------

You can pass arbitrary command line options to the |MySQL| server by appending
them to the run command:

.. code-block:: bash

  $ docker run --name my-container-name -d percona/percona-server --option1=value --option2=value

In this case, the values of ``option1`` and ``option2`` will be passed directly
to the server when it is started. The following command will for instance start
your container with UTF-8 as the default setting for character set and
collation for all databases in MySQL:

.. code-block:: bash

  $ docker run --name container-name -d  percona/percona-server --character-set-server=utf8 --collation-server=utf8_general_ci

Using a Custom MySQL Configuration File
---------------------------------------

The |MySQL| startup configuration in these Docker images is specified in the
:file:`/var/lib/mysql/my.cnf` file. If you want to customize this configuration
for your own purposes, you can make changes to this file.

Supported Docker Versions
-------------------------

These images are officially supported by the Percona engineering team on Docker
version 1.9.

