==========
 Glossary
==========

.. glossary::

  ACID
    Set of properties that guarantee database transactions are 
    processed reliably. Stands for :term:`Atomicity`,
    :term:`Consistency`, :term:`Isolation`, :term:`Durability`.

  Atomicity
    Atomicity means that database operations are applied following a
    "all or nothing" rule. A transaction is either fully applied or not
    at all.

  Consistency
    Consistency means that each transaction that modifies the database
    takes it from one consistent state to another.

  Durability
    Once a transaction is committed, it will remain so.

  Foreign Key
    A referential constraint between two tables. Example: A purchase
    order in the purchase_orders table must have been made by a customer
    that exists in the customers table.

  Isolation
    The Isolation requirement means that no transaction can interfere
    with another.

  InnoDB
    A :term:`Storage Engine` for MySQL and derivatives (:term:`Percona
    Server`, :term:`MariaDB`) originally written by Innobase Oy, since 
    acquired by Oracle. It provides :term:`ACID` compliant storage engine
    with :term:`foreign key` support. As of :term:`MySQL` version 5.5, 
    InnoDB became the default storage engine on all platforms.

  Jenkins
    `Jenkins <http://www.jenkins-ci.org>`_ is a continuous integration
    system that we use to help ensure the continued quality of the
    software we produce. It helps us achieve the aims of:

     * no failed tests in trunk on any platform,
     * aid developers in ensuring merge requests build and test on all platforms,
     * no known performance regressions (without a damn good explanation).

  LSN
    Log Serial Number. A term used in relation to the :term:`InnoDB` or
    :term:`XtraDB` storage engines.

  MariaDB
    A fork of :term:`MySQL` that is maintained primarily by Monty
    Program AB. It aims to add features, fix bugs while maintaining 100%
    backwards compatibility with MySQL.

  my.cnf
    The file name of the default MySQL configuration file.

  MyISAM
    A :term:`MySQL` :term:`Storage Engine` that was the default until
    MySQL 5.5.

  MySQL
    An open source database that has spawned several distributions and
    forks. MySQL AB was the primary maintainer and distributor until
    bought by Sun Microsystems, which was then acquired by Oracle. As
    Oracle owns the MySQL trademark, the term MySQL is often used for
    the Oracle distribution of MySQL as distinct from the drop-in
    replacements such as :term:`MariaDB` and :term:`Percona Server`.

  NUMA
    Non-Uniform Memory Access 
    (`NUMA <http://en.wikipedia.org/wiki/Non-Uniform_Memory_Access>`_) is a
    computer memory design used in multiprocessing, where the memory access
    time depends on the memory location relative to a processor. Under NUMA,
    a processor can access its own local memory faster than non-local memory,
    that is, memory local to another processor or memory shared between
    processors. The whole system may still operate as one unit, and all memory
    is basically accessible from everywhere, but at a potentially higher latency
    and lower performance.

  Percona Server for MySQL
    Percona's branch of :term:`MySQL` with performance and management improvements.

  Percona Server
    See :term:`Percona Server for MySQL`

  Storage Engine
    A :term:`Storage Engine` is a piece of software that implements the
    details of data storage and retrieval for a database system. This
    term is primarily used within the :term:`MySQL` ecosystem due to it
    being the first widely used relational database to have an
    abstraction layer around storage. It is analogous to a Virtual File
    System layer in an Operating System. A VFS layer allows an operating
    system to read and write multiple file systems (e.g. FAT, NTFS, XFS,
    ext3) and a Storage Engine layer allows a database server to access
    tables stored in different engines (e.g. :term:`MyISAM`, InnoDB).

  XtraDB
    Percona's improved version of :term:`InnoDB` providing performance,
    features and reliability above what is shipped by Oracle in InnoDB.
