=============================
Development of Percona Server
=============================

|Percona Server| is an open source project to produce a distribution
of the |MySQL| server with improved performance, scalability and
diagnostics.

Submitting Changes
==================

We keep trunk in a constant state of stability to allow for a release at 
any time and to minimize wasted time by developers due to broken code 
from somebody else interfering with their day.

You should also be familiar with our |Jenkins| setup.

Overview
~~~~~~~~
At Percona we use `Bazaar <http://www.bazaar-vcs.org>`_ for source
control and `launchpad <http://www.launchpad.net>`_ for both
code hosting and release management.

Changes to our software projects could be because of a new feature
(blueprint) or fixing a bug (bug). Projects such as refactoring could
be classed as a blueprint or a bug depending on the scope of the work.

Blueprints and bugs are targeted to specific milestones (releases). A
milestone is part of a series - e.g. 1.6 is a series in Percona
XtraBackup and 1.6.1, 1.6.2 and 1.6.3 are milestones in the 1.6 series.

Code is proposed for merging in the form of merge requests on launchpad.

Some software (such as Percona Xtrabackup) we maintain both a
development branch and a stable branch. For example: Xtrabackup 1.6 is
the current stable series, and changes that should make it into bugfix
releases of 1.6 should be proposed for the 1.6 tree. However, most new
features or more invasive (or smaller) bug fixes should be targeted to
the next release, currently 1.7. If submitting something to 1.6, you
should also propose a branch that has these changes merged to the
development release (1.7). This way somebody else doesn't have to
attempt to merge your code and we get to run any extra tests that may
be in the tree (and check compatibility with all platforms).

For Percona Server, we have two current bzr branches on which
development occurs: 5.1 and 5.5. As Percona Server is not a
traditional project, instead being a set of patches against an
existing product, these two branches are not related. That is, we do
not merge from one to the other. To have your changes in both, you
must propose two branches: one for 5.1 version of patch and one for
5.5.

Making a change to a project
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In this case we're going to use percona-xtrabackup as an
example. workflow is similar for Percona Server, but patch will need
to be modified both in 5.1 and 5.5 branches.

* ``bzr branch lp:percona-xtrabackup featureX`` (where 'featureX' is a
  sensible name for the task at hand)
* (developer makes changes in featureX, testing locally)
* Developer pushes to ``lp:~username/percona-xtrabackup/featureX``
* When the developer thinks the branch may be ready to be merged, they
  will run the branch through param build.
* If there are any build or test failures, developer fixes them (in
  the case of failing tests in trunk... no more tests should
  fail. Eventually all tests will pass in trunk)
* Developer can then submit a merge proposal to lp:percona-xtrabackup,
  referencing URL for the param build showing that build and test
  passes
* Code undergoes review
* Once code is accepted, it can be merged (see other section)

If the change also applies to a stable release (e.g. 1.6) then changes
should be made on a branch of 1.6 and merged to a branch of trunk. In
this case there should be two branches run through param build and two
merge proposals (one for 1.6 and one with the changes merged to
trunk). This prevents somebody else having to guess how to merge your
changes.

Merging approved branches
~~~~~~~~~~~~~~~~~~~~~~~~~

Before code hits trunk, it goes through a "staging" branch, where some
extra tests may be run (e.g. valgrind) along with testing that all
branches behave well together (build and test) before pushing to
trunk.

To ensure quality, **DO NOT push directly to trunk!** everything must go through adequate testing first. This ensures that at any point trunk is in a releasable state.

Please note that **ALL changes must go through staging first** This is to ensure that several approved merge requests do not interact badly with each
other.

* Merge captain (for lack of a better term for the person merging
  approved code into trunk) may collate several approved branches that
  have individually passed param-build as run by the original
  developers.

  * Workflow would look something like this:

    * ``bzr branch lp:percona-xtrabackup staging``
    * ``bzr merge lp:~user/percona-xtrabackup/featureX``
    * ``bzr commit -m "merge feature X"``
    * ``bzr merge lp:~user/percona-xtrabackup/featureY``
    * ``bzr commit -m "merge feature Y"``
    * ``bzr push --overwrite lp:percona-xtrabackup/staging'``
    * Run ``lp:percona-xtrabackup/staging`` through param build (in
      future, we'll likely have a Jenkins job specifically for this)
    * If build succeeds, ``bzr push lp:percona-server`` (and branches
      will be automatically marked as 'merged'.. although bug reports
      will need to be manually changed to 'Fix Released')
    * If build or test fails, attempt to find which branch may be the
      cause, and repeat process but without that branch.

* Any failing branch will be set to 'Work in Progress' with a 'Needs
  fixing' review with the URL of the build in jenkins where the
  failure occured. This will allow developers to fix their code.

Resubmitting a merge request
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the event of a merge request being marked as 'Work In Progress' due
to build/test failures when merging, the developer should fix up the
branch, run through param build and then 'Resubmit' the merge
proposal.

There is a link on launchpad to resubmit the merge proposal, this means it appears in the list of merge requests to review again rather than off in the "work in progress" section.


Percona Server
~~~~~~~~~~~~~~

The same process for Percona Server, but we have different branches (and merge requests) for 5.1 and 5.5 series.

Upgrading MySQL base version
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Same process as other modifications.
* create local branch
* make changes
* param build
* merge request

We will need some human processes to ensure that we do not merge extra
things during the time when base MySQL version is being updated to
avoid making life harder for the person doing the update.



Making a release
================

* ``bzr branch lp:project release-project-VERSION``
* build packages
* perform any final tests (as we transition, this will already have
  been done by jenkins)
* ``bzr tag project-version``
* merge request back to lp:project including the tag (TODO: write
  exact bzr commands for this)

This way anybody can easily check out an old release by just using bzr
to branch the specific tag.

Jenkins
=======

Our Jenkins instance uses a mixture of VMs on physical hosts that
Percona runs and Virtual Machines in Amazon EC2 that are launched on
demand.

Basic Concepts
~~~~~~~~~~~~~~
We have some jobs that are activated based on source control changes
(new commits in a bzr repository). We have some that are "param
build" - that is, a user specifies parameters for the build (e.g. the
bzr tree). A param-build allows developers to ensure their branch
compiles and passes tests on all supported platforms *before*
submitting a merge request. This helps us maintain the quality of the
main bzr branches and not block other developers work.

Jenkins is a Master/Slave system and the jenkins master schedules the
builds across available machines (and may launch new VMs in EC2 to
meet demand).

Most of our jobs are what's known as "matrix builds". That is, a job
that will be run with several different configurations of the project
(e.g. release, debug) across several platforms (e.g. on a host
matching the label of "centos5-32" and a host matching label of
"ubuntu-natty-32bit"). Matrix builds show a table of lights to
indicate their status. Clicking "build now" on one of these queues up
builds for all of the combinations.

We have some integration of our regression test suites (currently
xtrabackup) with Jenkins ability to parse JUnitXML, presenting a nice
user interface to any test failures.

Because building some projects is non-trivial, in order to not
duplicate the list of compile instructions for each job, we use
template builds. You'll see builds such as percona-xtrabackup-template
which is a disabled job, but all current xtrabackup jobs point to it
for the commands to build and run the test suite.

Percona Xtrabackup
~~~~~~~~~~~~~~~~~~

`<http://jenkins.percona.com/view/Percona%20Xtrabackup/>`_

We currently build both xtrabackup 1.6 and xtrabackup trunk (will become 1.7).

There are param-builds for 1.6 and trunk too. These should be run for each merge request (and before any collection of merged branches is pushed to trunk)

Percona Server
~~~~~~~~~~~~~~

We have separate jobs for Percona Server 5.1 and Percona Server 5.5 due to the different build systems that MySQL 5.1 and 5.5 use.

The ``mysql-test-run.pl`` test suite is integrated with Jenkins through `subunit <http://launchpad.net/subunit>`_ and ``subunit2junitxml`` allowing us to easily see which tests passed/failed on any particular test run.

Percona Server 5.1
------------------

`<http://jenkins.percona.com/view/Percona%20Server%205.1/>`_

We have trunk and param jobs. We also have a valgrind job that will run after a successful trunk build.

Percona Server 5.5
------------------

`<http://jenkins.percona.com/view/Percona%20Server%205.5/>`_

Similar to 5.1, but for PS5.5 instead.

MySQL Builds
~~~~~~~~~~~~

`<http://jenkins.percona.com/view/MySQL/>`_

I've set up a few jobs in Jenkins that should help us predict the future
for Percona Server. Namely, if upstream MySQL may cause us any problems.

I wanted to see if some test failures were possibly upstream, so I set
up two jobs:

`<http://jenkins.percona.com/view/MySQL/job/mysql-5.1-url-param/>`_
`<http://jenkins.percona.com/view/MySQL/job/mysql-5.5-url-param/>`_

both of which ask for a URL to a MySQL source tarball and then do a full
build and test across the platforms we have in jenkins.

But my next thought was that we could try and do this *before* the
source tarballs come out - hopefully then being able to have MySQL
release source tarballs that do in fact pass build and test everywhere
where we're wanting to support Percona Server.

`<http://jenkins.percona.com/view/MySQL/job/mysql-5.1-trunk/>`_
`<http://jenkins.percona.com/view/MySQL/job/mysql-5.5-trunk/>`_

are scheduled to just try once per week (we can change the frequency if
we want to) to build and test from the MySQL bzr trees.

I also have a valgrind build (same configuration as for Percona Server) to help us see if there's any new valgrind warnings (or missed suppressions).

I'm hoping that these jobs will help us catch any future problems before
they become our problem. (e.g. we can easily see that the sporadic test failures we see in Percona Server are actually in upstream MySQL).
