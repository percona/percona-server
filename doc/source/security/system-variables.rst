.. _secure-variables:

================================================================================
Server variables
================================================================================

Use system variables to configure the server operation. 
    
.. _secure_log_path:

.. rubric:: `secure_log_path`


Implemented in :ref:`8.0.28-19`.

.. list-table::
   :header-rows: 1
   
   * - Variable Name
     - Description
   * - Command-line
     - ``--secure-log-path``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - empty string
     
This variable restricts the dynamic log file locations. The variable is read-only and must be set up in a configuration file or the command line. 

The accepted value is the directory name as a string. The default value is an empty string. When the value is an empty string, the variable only adds a warning to the error log and does nothing. If the value contains a directory name, then the slow query log and the general log must be located in that directory. An attempt to move either of these files outside of the specified directory results in an error.


