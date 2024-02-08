eval INSTALL PLUGIN mecab SONAME '$MECAB';

--let $ipadic_charset = utf-8
--let $mysql_charset = utf8mb4
--let $mecab_charset = `SELECT variable_value FROM performance_schema.global_status WHERE VARIABLE_NAME = 'mecab_charset'`

if ($mecab_charset == '')
{
  # restart with package dictionary
  --let $MYSQL_DATADIR = `SELECT @@datadir`
  --let $MYSQL_BASEDIR = `SELECT @@basedir`

  --let $mecabrc = $MYSQL_DATADIR/mecabrc
  --let $dicdir = $MYSQL_BASEDIR/lib/mecab/dic/ipadic_$ipadic_charset

  --exec echo "dicdir=$dicdir" > $mecabrc

  --let $restart_parameters = "restart: --loose-mecab-rc-file=$mecabrc $MECAB_OPT --innodb-ft-min-token_size=2"
  --let $do_not_echo_parameters = 1
  --source include/restart_mysqld.inc

  eval INSTALL PLUGIN mecab SONAME '$MECAB';

  --let $mecab_charset = `SELECT variable_value FROM performance_schema.global_status WHERE VARIABLE_NAME = 'mecab_charset'`
}

if ($mecab_charset != $mysql_charset)
{
  --let $skip_message = Test fail to load mecab parser, please set correct 'loose_mecab_rc_file'.
  if ($mecab_charset != '')
  {
    UNINSTALL PLUGIN mecab;
    --let $skip_message = Test mecab charset mismatch (mecab_charset=$mecab_charset, mysql_charset=$mysql_charset).
  }
  if ($mecabrc != '')
  {
    --remove_file $mecabrc
    --let $mecabrc =
  }
  --skip $skip_message
}

SHOW STATUS LIKE 'mecab_charset';
