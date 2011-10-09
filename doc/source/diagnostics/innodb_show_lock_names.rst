.. _innodb_show_lock_names:

=================
 Show Lock Names
=================

This feature is curently undocumented except for the following example.

Example: ::

  mysql> show mutex status;
  +--------+---------------------------+---------------+
  | Type   | Name                      | Status        |
  +--------+---------------------------+---------------+
  | InnoDB | &rseg->mutex              | os_waits=210  |
  | InnoDB | &dict_sys->mutex          | os_waits=3    |
  | InnoDB | &trx_doublewrite->mutex   | os_waits=1    |
  | InnoDB | &log_sys->mutex           | os_waits=1197 |
  | InnoDB | &LRU_list_mutex           | os_waits=2    |
  | InnoDB | &fil_system->mutex        | os_waits=5    |
  | InnoDB | &kernel_mutex             | os_waits=242  |
  | InnoDB | &new_index->lock          | os_waits=2    |
  | InnoDB | &new_index->lock          | os_waits=415  |
  .....
