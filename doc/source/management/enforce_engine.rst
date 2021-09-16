.. _enforce_engine:

========================
Enforcing Storage Engine
========================

|Percona Server| has implemented variable which can be used for enforcing the use of a specific storage engine.

When this variable is specified and a user tries to create a table using an explicit storage engine that is not the specified enforced engine, he will get either an error if the ``NO_ENGINE_SUBSTITUTION`` SQL mode is enabled or a warning if ``NO_ENGINE_SUBSTITUTION`` is disabled and the table will be created anyway using the enforced engine (this is consistent with the default MySQL way of creating the default storage engine if other engines aren't available unless ``NO_ENGINE_SUBSTITUTION`` is set).

In case user tries to enable :variable:`enforce_storage_engine` with engine that isn't available, system will not start.

.. note::
 
 If you're using :variable:`enforce_storage_engine`, you must either disable it before doing ``mysql_upgrade`` or perform ``mysql_upgrade`` with server started with :option:`--skip-grants-tables`.

Version Specific Information
============================

  * :rn:`5.7.10-1`
    Feature ported from |Percona Server| 5.6

System Variables
================

.. variable:: enforce_storage_engine

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: NULL

.. note:: 

  This variable is not case sensitive.

Example
=======

Adding following option to source/glossary.rst`my.cnf` will start the server with InnoDB as enforced storage engine. ::  

 enforce_storage_engine=InnoDB
