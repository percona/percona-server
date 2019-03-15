--replace_regex /.*Log tracked up to[[:space:]]*([0-9]+).*/\1/gso /InnoDB//
let $tracked_lsn=`SHOW ENGINE INNODB STATUS`;
