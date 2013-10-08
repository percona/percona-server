.. _innodb_opt_lru_count:

======================================
 Reduced Buffer Pool Mutex Contention
======================================

We removed ``buffer_pool`` mutex operations on counting blocks on LRU list where it is safe to delete. As drawback we may have some inaccurate information of LRU list, but it does not affect storage engine operations. As result we have decreased contention on ``buffer_pool`` mutex.
