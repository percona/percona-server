if (`SELECT @@GLOBAL.replica_parallel_workers > 1`)
{
     skip Test cannot run with MTS;
}
