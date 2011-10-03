==========
 Glossary
==========

.. glossary::

  ACID
    :term:`Atomicity` :term:`Consistency` :term:`Isolation`
    :term:`Durability`

  Atomicity
    Atomicity means that database operations are applied following a
    "all or nothing" rule. A transaction is either fully applied or not
    at all.

  Consistency
    Consistency means that each transaction that modifies the database
    takes it from one consistent state to another.

  Drizzle
    Drizzle: a database for the cloud.

    Drizzle is a community-driven open source project that is forked
    from the popular MySQL database. The Drizzle team has removed
    non-essential code, re-factored the remaining code into a
    plugin-based architecture and modernized the code base moving to
    C++.

    Drizzle Charter
    * A database optimized for Cloud infrastructure and Web applications.
    * Design for massive concurrency on modern multi-cpu architecture
    * Optimize memory for increased performance and parallelism
    * Open source, open community, open design
    Scope
    * Re-designed modular architecture providing plugins with defined APIs
    * Simple design for ease of use and administration
    * Reliable, ACID transactional

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
    Server`, :term:`MariaDB`, :term:`Drizzle`) originally written by
    Innobase Oy, since acquired by Oracle. It provides :term:`ACID`
    compliant storage engine with :term:`foreign key` support. As of
    :term:`MySQL` version 5.5, InnoDB became the default storage engine
    on all platforms.

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


  Percona Server
    Percona's branch of :term:`MySQL` with performance and management improvements.

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
