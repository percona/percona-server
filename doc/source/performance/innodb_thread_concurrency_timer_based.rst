.. _innodb_thread_concurrency_timer_based_page:

===========================================
InnoDB timer-based Concurrency Throttling
===========================================

If the variable :variable:`innodb_thread_concurrency_timer_based` has been set to ``TRUE``, lock-free timer-based |InnoDB| method of handling thread concurrency will be used instead of original mutex-based method.

System Variables
================

.. variable:: innodb_thread_concurrency_timer_based

   :cli: Yes
   :conf: Yes
   :scope: Global  	 
   :dyn: No	
   :vartype: BOOL
   :default: FALSE
   :range: TRUE/FALSE

.. note:: 
 This feature depends on atomic op builtins being available. 

