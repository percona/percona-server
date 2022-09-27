=====================================
 The Percona XtraDB Storage Engine
=====================================

.. image:: percona-xtradb.png
   :alt: Percona XtraDB
   :align: right

|xtradb.product| is an enhanced version of the |InnoDB| storage engine, designed
to better scale on modern hardware.  It also includes a variety of other
features useful in high-performance environments. It is fully backwards
compatible, and so can be used as a drop-in replacement for standard |InnoDB|.


|xtradb.product| includes all of |InnoDB| 's robust, reliable ``ACID``-compliant
design and advanced ``MVCC`` architecture, and builds on that solid foundation
with more features, more tunability, more metrics, and more scalability. In
particular, it is designed to scale better on many cores, to use memory more
efficiently, and to be more convenient and useful. The new features are
especially designed to alleviate some of |InnoDB|'s limitations. We choose
features and fixes based on customer requests and on our best judgment of
real-world needs as a high-performance consulting company.

|xtradb.product| engine will not have further binary releases, it is
distributed as part of |Percona Server|.

.. include:: .res/replace.txt
