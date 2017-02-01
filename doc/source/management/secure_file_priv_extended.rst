.. _secure_file_priv_extended:

======================================================
Extending the :option:`secure-file-priv` server option
======================================================

|Percona Server| has extended :option:`secure-file-priv` server option. When
used with no argument, the ``LOAD_FILE()`` function will always return
``NULL``. The ``LOAD DATA INFILE`` and ``SELECT INTO OUTFILE`` statements will
fail with the following error: ``"The MySQL server is running with the
--secure-file-priv option so it cannot execute this statement"``. ``LOAD DATA
LOCAL INFILE`` is not affected by the ``--secure-file-priv`` option and will
still work when it's used without an argument.

In |Percona Server| :rn:`5.6.34-79.1` empty ``secure-file-priv`` became an
alias for ``NULL`` value: both disable ``LOAD_FILE()``, ``LOAD DATA INFILE``,
and ``SELECT INTO OUTFILE``. With this change it is no longer possible to
disable security checks by omitting the option as that would take the default
value (:file:`/var/lib/mysql-files/` for ``.deb`` and ``.rpm`` and ``NULL`` for
``.tar.gz`` packages. Instead, ``--secure-file-priv=''`` (or ``=/``) should be
used.


Version Specific Information
============================

  * :rn:`5.6.11-60.3`
    Variable :variable:`secure-file-priv` extended behavior implemented.
  * :rn:`5.6.34-79.1`
    Default value for :variable:`secure-file-priv` has been changed from
    ``NULL`` to ``/var/lib/mysql-files/`` when installed from ``.deb`` and
    ``.rpm`` packages.

System Variables
================

.. variable:: secure-file-priv 

     :cli: No
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: ``/var/lib/mysql-files/`` - for ``.deb`` and ``.rpm`` packages
     :default: ``NULL`` - for ``.tar.gz`` packages
