.. _percona/upstream-bug-state-monitoring-agent:

Setup
********************************************************************************

To update the *upstream-bug-fixes.rst* file by using this agent, complete the
following steps:

1. In the current directory, install the dependencies listed in the
   ``requirements.txt`` file.
   
   #. If your system only has Python 3, run ``/usr/bin/pip install -r
      requirements.txt``.
   #. If you also have Python 2.7.*, your ``/usr/bin/pip`` is
      set up to update this version. In this case, run ``/usr/bin/pip3 install -r
      requirements.txt``.

#. Run ``python3 agent.py``.

.. important::

   1. While the program is running, there won't be any output to your
      terminal.
   #. The input ``upstream-bug-fixes.rst`` file will not be
      overwritten. When the program completes successfully it writes
      its output into a copy which has the tilda (~) symbol appended
      to mark the generated file as temporary.  This way, you can
      check the output and then replace the original file if you are
      happy with the result.
   

