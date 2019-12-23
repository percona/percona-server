# This file will be executed at the end of mysqldump

SET DEBUG_SYNC='now SIGNAL mysqldump.finished';

