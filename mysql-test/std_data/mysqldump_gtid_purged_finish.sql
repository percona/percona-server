# This file will be executed at the end of mysqldump

SET DEBUG='-d,catch_show_gtid_mode';
SET DEBUG_SYNC='now SIGNAL mysqldump.finished';

