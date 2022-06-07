if (`SELECT @@GLOBAL.replica_parallel_workers <= 1`)
{
  skip Test makes sense only to run with MTS;
}
