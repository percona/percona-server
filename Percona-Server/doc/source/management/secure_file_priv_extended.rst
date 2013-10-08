.. _secure_file_priv_extended:

=======================================================
Extending the :option:`secure-file-priv` server option
=======================================================

|Percona Server| has extended :option:`secure-file-priv` server option. When used with no argument, the LOAD_FILE() function will always return NULL. The LOAD DATA INFILE and SELECT INTO OUTFILE statements will fail with the following error: "The MySQL server is running with the --secure-file-priv option so it cannot execute this statement". LOAD DATA LOCAL INFILE is not affected by the --secure-file-priv option and will still work when it's used without an argument.


Version Specific Information
============================

  * :rn:`5.6.11-60.3`
    Variable :variable:`secure-file-priv` extended behavior implemented.

System Variables
================

.. variable:: secure-file-priv 

     :cli: No
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: NULL

