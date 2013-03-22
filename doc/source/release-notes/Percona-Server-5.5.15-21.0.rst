.. rn:: 5.5.15-21.0

============================
|Percona Server| 5.5.15-21.0
============================

Percona is glad to announce the release of |Percona Server| 5.5.15-21.0 on August 31, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.15-21.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.5/installation.html>`_).

Based on |MySQL| 5.5.15, including all the bug fixes in it, |Percona Server| 5.5.15-21.0 is now the current stable release in the 5.5 series. All of Percona's software is open-source and free, all the details of the release can be found in the `5.5.15-21.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.15-21.0>`_.


New features
=============

As of MySQL 5.5.15, a *Fixed Row Format* (FRF) is still being used in the ``MEMORY`` storage engine. The fixed row format imposes restrictions on the type of columns as it assigns on advance a limited amount of memory per row. This renders a ``VARCHAR`` field in a ``CHAR`` field in practice, making impossible to have a TEXT or BLOB field with that engine implementation.

To overcome this limitation, the :ref:`improved_memory_engine` is introduced in this release for supporting true ``VARCHAR``, ``VARBINARY``, ``TEXT`` and ``BLOB`` fields in ``MEMORY`` tables.
This implementation is based on the *Dynamic Row Format* (DFR) introduced by the mysql-heap-dynamic-rows patch.
DFR is used to store column values in a variable-length form, thus helping to decrease memory footprint of those columns and making possible BLOB and TEXT fields and real `VARCHAR` and `VARBINARY`.

For performance reasons, a mixed solution is implemented: the fixed format is used at the beginning of the row, while the dynamic one is used for the rest of it. All values for columns used in indexes are stored in fixed format at the first block of the row, then the following columns are handled with DRF.

