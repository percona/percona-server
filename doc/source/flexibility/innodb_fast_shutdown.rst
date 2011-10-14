.. _innodb_fast_shutdown:

===============
 Fast Shutdown
===============

Some |InnoDB| / |XtraDB| threads which perform various background activities are in the sleep state most of the time. They only wake up every few seconds to perform their tasks. They also check whether the server is in the shutdown phase, and if not, they go to the sleep state again. That means there could be a noticeable delay (up to 10 seconds) after a shutdown command and before all |InnoDB| / |XtraDB| threads actually notice this and terminate. This is not a big problem for most production servers, because a shutdown of a heavily loaded server normally takes much longer than 10 seconds.

The problem, however, had a significant impact on running the regression test suite, because it performs a lot of server restarts during its execution and also because there is not so much to do when shutting a test server. So it makes even less sense to wait up to 10 seconds.

This change modifies that behavior to make the sleep waiting interruptible, so that when the server is told to shutdown, threads no longer wait until the end of their sleep interval. This results in a measurably faster test suite execution (~40% in some cases).

The change was contributed by Kristian Nielsen.

Version Specific Information
============================

  * 5.1.52-12.3:
    Full functionality available.

Other Information
=================

  * Author / Origin:
    Kristian Nielsen

  * Bugs fixed:
    :bug:`643463`

Other reading
=============

  * `How to decrease InnoDB shutdown times <http://www.mysqlperformanceblog.com/2009/04/15/how-to-decrease-innodb-shutdown-times/>`_

  * `How long InnoDB shutdown may take <http://www.mysqlperformanceblog.com/2010/09/02/how-long-innodb-shutdown-may-take/>`_

