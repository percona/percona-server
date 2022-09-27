.. _enforce_engine:

========================
Enforcing Storage Engine
========================

*Percona Server for MySQL* has implemented variable which can be used for enforcing the
use of a specific storage engine.

When this variable is specified and a user tries to create a table using an
explicit storage engine that is not the specified enforced engine, the user
will get either an error if the ``NO_ENGINE_SUBSTITUTION`` SQL mode is enabled
or a warning if ``NO_ENGINE_SUBSTITUTION`` is disabled and the table
will be created anyway using the enforced engine (this is consistent with the
default *MySQL* way of creating the default storage engine if other engines
are not available unless ``NO_ENGINE_SUBSTITUTION`` is set).

In case user tries to enable :ref:`enforce_storage_engine` with engine
that isn't available, system will not start.

.. note::
 
 If you're using :ref:`enforce_storage_engine`, you must either disable
 it before doing ``mysql_upgrade`` or perform ``mysql_upgrade`` with server
 started with :option:`--skip-grants-tables`.

Version Specific Information
============================

* :ref:`8.0.13-4`: The feature was ported from *Percona Server for MySQL* 5.7.

System Variables
================

.. _enforce_storage_engine:

.. rubric:: ``enforce_storage_engine``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - NULL

.. note:: 

  This variable is not case sensitive.

Example
=======

Adding following option to :term:`my.cnf` will start the server with InnoDB as
enforced storage engine. ::  

 enforce_storage_engine=InnoDB
