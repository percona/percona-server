.. _development:

================================================================================
Development of |Percona Server|
================================================================================

|Percona Server| is an open source project to produce a distribution
of the |MySQL| Server with improved performance, scalability and
diagnostics.

Submitting Changes
================================================================================

We keep trunk in a constant state of stability to allow for a release at
any time and to minimize wasted time by developers due to broken code.

Overview
--------------------------------------------------------------------------------

At Percona we use `Git <https://git-scm.com>`_ for source control, `GitHub
<https://github.com/percona>`_ for code hosting, and `Jira
<https://jira.percona.com>`_ for release management.

We change our software to implement new features and/or to fix bugs. Refactoring
could be classed either as a new feature or a bug depending on the scope of
work.

New features and bugs are targeted to specific releases. A
release is part of a series. For example, 2.4 is a series in Percona
XtraBackup and 2.4.15, 2.4.16 and 2.4.17 are releases in this series.

Code is proposed for merging in the form of pull requests on GitHub.

For |Percona Server|, we have several Git branches on which development occurs:
5.5, 5.6, 5.7, and 8.0. As |Percona Server| is not a traditional project, instead
of being a set of patches against an existing product, these branches are not
related. In other words, we do not merge from one release branch to another. To
have your changes in several branches, you must propose branches to each release
branch.

Making a Change to a Project
--------------------------------------------------------------------------------

In this case, we are going to use ``percona-xtrabackup`` as an example. The
workflow is similar for |Percona Server|, but the patch will need to be modified
in all release branches of |Percona Server|.

* ``git branch https://github.com/percona/percona-xtrabackup/featureX`` (where 'featureX' is a
  sensible name for the task at hand)
* (developer makes changes in featureX, testing locally)
* The Developer pushes to ``https://github.com/percona/username/percona-xtrabackup/featureX``
* The developer can submit a pull request to https://github.com/percona/percona-xtrabackup,
* Code undergoes a review
* Once code is accepted, it can be merged

If the change also applies to a stable release (e.g. 2.4) then changes should be
made on a branch of 2.4 and merged to a branch of trunk. In this case there
should be two branches run through the param build and two merge proposals (one
for the stable release and one with the changes merged to trunk). This prevents
somebody else having to guess how to merge your changes.

|Percona Server|
--------------------------------------------------------------------------------

The same process for |Percona Server|, but we have several different branches
(and merge requests).
