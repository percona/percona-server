#
# Check if server has rocksdb_encryption enabled. If yes, skips the test.
#

let $have_encryption= `SELECT @@rocksdb_encryption`;
if ($have_encryption != 0)
{
  --skip Test requires 'rocksdb_encryption=0'
}

