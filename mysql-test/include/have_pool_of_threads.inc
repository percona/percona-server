let $threadpool_enabled= `SELECT @@thread_handling LIKE 'pool-of-threads'`;
if (!$threadpool_enabled)
{
  --skip Test requires threadpool
}
