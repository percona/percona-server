# Percona Server contributing guide

We welcome contributions from all users and the community. By contributing, you agree to the [Percona Community code of conduct](https://percona.community/contribute/coc/). Thank you for deciding to contribute and help us improve Percona Server for MySQL.

You can contribute to the server in a number of ways:



* Fix a bug or typo.
* Implement a new feature or improvement.
* [Contribute to the documentation.](https://github.com/percona/pdmysql-docs/blob/8.0/CONTRIBUTING.md)

The fastest way to contribute is to simply make a pull request against the Percona Server GitHub project at [https://github.com/percona/percona-server](https://github.com/percona/percona-server).  From there our team will review and request changes and eventually adopt or politely reject your contribution.  In some cases we may need to close your pull request and create one of our own.  If this is necessary, you will still receive full credit for your work.

If you would like to assist us further to ensure a faster turnaround time from the time you make your submission to the time your contribution lands in an official release, follow these steps to contribute to Percona Server for MySQL:



* Choose an improvement
* Prepare your starting work branch
* Implement and test your change
* Submit it for review


# Choose the product, upstream first

Percona Server for MySQL is derived from Oracle MySQL Community and contains some components that are derived from other external sources such as the MyRocks storage engine which is based on Facebooks MySQL MyRocks storage engine.  These products and components that we derive our work from are referred to as our upstreams.

When considering a bug fix or an improvement, think about the best place to send it.  For example, a bug fix that applies to both Percona Server for MySQL and Oracle MySQL community is best to send to Oracle first.  Percona Server for MySQL will then “inherit” the fix during our next upstream merge cycle.  We are always perfectly willing to take your change and submit it to Oracle ourselves but that may extend the amount of time that it takes for the change to appear in both products.


# Choose an improvement

Visit our [JIRA project for Percona Server](https://jira.percona.com/issues/?jql=project%20%3D%20PS%20AND%20resolution%20%3D%20Unresolved%20ORDER%20BY%20priority%20DESC%2C%20updated%20DESC) and select an issue that interests you.  If no issue exists yet for your issue or idea, create one of the appropriate type (**Bug**, **Feature Request**, or **Improvement**).  Once you have a JIRA issue that you would like to work on, please comment on that issue so that we know that you are willing to contribute.  This comment will inform us of your intention so that we will not duplicate our efforts and may be able to collaborate on the effort.

Here are some basic tips and requirements to go by to ensure that your contribution will be accepted:



* New features and improvements should be 100% backwards compatible with earlier minor series versions and not change any default or existing behaviors of the server.  Users have the expectation that a simple upgrade will not change or break existing applications or deployments.  A good indicator is that all MySQL Test Run test cases pass without the need to modify them to account for an unexpected change in behavior or user interface.
* Ideal improvements are minimally invasive to the server, storage engines, and existing plugins and components.  For example, a minimally invasive contribution would be something like a new UDF or plugin that stands on its own with no code changes required to the server core.  An invasive change would be one with many individual lines of code changed across the server and engines, i.e. something that is not isolated to a few contiguous code blocks.  These invasive type changes usually cause conflicts during our upstream merge process and are also likely to be sensitive to breaking when other server behavior or code changes.  Changes that are overly invasive may incur additional maintenance and merge costs and may be rejected.  If you have a concern about a contribution, please reach out to us before you invest too much time.  There are usually ways to achieve a desired outcome while remaining minimally invasive.
* Target your work to the correct major series version(s).  New features and improvements should only go into the newest major release series, currently the 8.0.x series.  Bug fixes that impact multiple major release series should start at the lowest currently supported major release series, currently the 5.7.x series.  Once the fix has been accepted for the lower series, it can then be merged forward to the next upper series and submitted as a separate, but related contribution.  If a fix only applies to a lower series, the change will need to be “NULL merged” to the upper series in order to maintain a clean commit history.


# Prepare your starting work branch

First, using GitHub Web User Interface, fork the Percona Server project into your own GitHub repository.  You will need this forked project as a place to host your branch(s) when you make your pull request(s) later on.  If you are using an existing fork, make sure that your HEAD branches of 5.7 and 8.0 are up to date.

On your local machine, clone the percona server repo:


```
    > git clone git@github.com:/<yourgituser>/percona-server –recurse-submodules -j8
    > cd percona-server
```


This will set you up on the default branch of the Percona repo, currently **8.0**.


```
    > git branch
      * 8.0
```


Prepare your starting point branch or branches to work from.  If you are only working on something that will go to the newest major release series, currently 8.0, then you can start your work on a branch based off of the current 8.0 HEAD.  You should use your JIRA issue id to base your branch name off of.  For our example we will use the JIRA issue PS-8675 and create a working branch off of the current 8.0 HEAD:


```
    > git checkout -b ps-8.0-8675 origin/8.0
      Switched to a new branch 'ps-8.0-8675'
```


Now you may jump to “**Implement and test your change**”.

If you are fixing a bug that exists in a lower major release series such as 5.7, you should begin there first and then cherry pick it up to newer major release series such as 8.0.

## Code style and format requirements

Generally we follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).  As you may notice, the format and style of the code within the MySQL and Percona Server source trees varies widely.  This is the result of a codebase that is 22 years old.  Things get written in one style, they work, they never get touched again.  Other things are a source of constant improvement and get refactored multiple times.  You will find a mixture of legacy “C” style coding and various levels of C++ standards.  The rule is to follow closely the style of the file/function of which you are working, this includes things like number of spaces per tab, function signature styles, etc…  In the 8.0 series, **_.clang-format_** file has been introduced so you should be able to safely use the llvm clang-format tool but you must be aware of the current version which the server is formatted with otherwise you will still have formatting issues.  The exact version of clang-format to use is identified within the **_.clang-format_** file located in the root of the source tree.

New code and implementation of ‘external’ things like plugins and components can make use of whatever language level that you like as long as the style conforms to the Google C++ Style and clang-format rules.  Most of the server code in the 8.0 series uses C++17 whereas some areas are still C++ 11 based.  We have some older plugins that are still basically “C” and some newer components that have a fully modern C++ framework.  Code style is heavily scrutinized during the code review process.


## Test suite coverage requirements

All contributions require a suitable addition or alteration of a covering MySQL Test Run (MTR) test case.  The MTR runtime and language reference can be found online [here](https://dev.mysql.com/doc/dev/mysql-server/latest/PAGE_MYSQL_TEST_RUN.html).  Looking through existing tests is also a great reference.  The tests and their results are located within the mysql binary installation directory under the **_mysql-test_** directory.  The tests are broken down into suites.  For new features or implementations such as plugins or components, a new suite can be created within the plugin or component source tree.  For others, the test additions/alterations should go under the **_mysql-test_** directory.  Test coverage is also heavily scrutinized during the code review process.  You will almost certainly be asked to improve or extend your test coverage during the code review process.  No contribution can be accepted without an acceptable level of test coverage.


## Commit message requirements

When you are ready to commit your changes, your pull request should contain exactly one commit for your changes (or one original and one merge commit if a change originated in a lower major series and merged forward to a higher major series).  We typically expect to see only one change or issue per pull request.  Sometimes we discover two or more JIRA issues that are very tightly coupled and need to be fixed together.  In this case, multiple issues in a single commit are permitted.

Commit messages should fully explain the change and/or feature implementation and should not rely on a link to an external resource or document.  Commit messages must have a very specific format, the first line(s) must have the JIRA issue numbers and JIRA issue description(s) followed by one empty line, followed by the detailed commit message.  Here is an example of a commit message for our fake JIRA issue PS-8675:


```
    PS-8675 : Use binary search in widget dictionaries

    As dictionaries are generated only once, but used many times, this patch improves the lookup performance by sorting them during load time, and using binary search during lookups.  As this is purely a repetitive efficiency improvement, no additional tests are needed.
```



## Documentation requirements on JIRA issue

If you are developing any new feature or behavior, you should provide enough seed information within the JIRA issue and commit message for a Documentation Engineer to translate into product documentation.  There is a good chance that our Documentation Engineers will need clarification on behavioral details and they will ask for these through the JIRA issue, so please be responsive to any inquiries.


## Testing

When you have completed the work on your contribution, you should be sure to locally run the MTR test suite.  The complete suite is not always necessary.  You can try to localize just to the area that you are working on.  If you are working on a completely new feature or plugin, you may be able to limit it to just the newly added suite.  Otherwise, for example, if you are adding a bug fix to InnoDB, you should probably run the **main** suite along with the appropriate **innodb*** suite.  If there are any test regressions (tests that fail only with your changes in place), you will need to debug and fix the cause and explain any unrelated test result changes in your commit message.


## Submit it for review

Now that you have your contribution tested, your changes committed to your local git repository with a conforming commit message, you can push your changes to your personal git repository:


```
    > git push origin ps-8.0-8675
```


Next, submit it to us for review.  This will be done entirely through the GitHub Web User Interface.  Logon to your git repository via the web, navigate to your forked percona-server repository, find your recently pushed branch, click **Compare & pull request**.  Before you confirm, take this opportunity to review your diff and make any changes.  Once you are happy with your changes and commit message, confirm by clicking **Create pull request**.

Once you have confirmation of your pull request, it is a good idea to copy the link and paste it into the JIRA issue as a new comment.  This makes it easier to find it later on.

From here, one of our engineers will adopt your contribution.  It will begin with a thorough code review for functionality, behavioral changes, coding style, security, test coverage, etc…  Chances are very good that we will ask for changes during the review process.  When you make additional changes, please do not re-commit and force push your git branch.  We prefer that you just make additional commits to your original work and push to your working branch within your personal git repo.  The pull request will be automatically updated with your additional commits.  Usually just replying to or adding a new comment within the PR to let us know of your change is enough.

Once our Engineers are satisfied with the state of the work, we will run the change through our internal CI/CD system and post a comment or link to the results to the pull request and/or JIRA issue (you will not be able to view the results).  If we notice any unexpected failures in the CI/CD test jobs, it will be communicated to you within the pull request what tests failed and you will be asked to correct the failures.

Now we are ready to merge, but wait, there is one more thing.  Remember those additional corrective commits that you made to your working branch?  Well, you will now be asked to “squash” all of those into a single commit and force push your branch.  If your fork allows changes to be made by the maintainers, one of our engineers can do this for you as well as format the commit message.

And that is it!  We will select a release target for your work and make sure that it is all properly documented and listed within our release notes.


## What if I was working on something in a lower major series?

If you were working on a bug fix that exists in all or most versions, for example both the 5.7 series and 8.0 series and you started your fix on the 5.7 series as advised above, you have a few more things to do.

We first must assume that your 5.7 fix was accepted and is pending merge.  Now return to the **Prepare your starting work branch** and prepare a starting branch on the next upper major release series branch in your same local GitHub working repository, for now this is probably 8.0.  Now you can try to merge your changes from your accepted 5.7 branch to your new 8.0 working branch.  This sequence might look something like this:


```
    > git checkout -b ps-8.0-8675 origin/8.0
    > git submodule deinit --all --force && git submodule init --recursive
```


At this point, you may have some extra stuff in your working tree that was leftover by the switch to your working 8.0 branch.  You can simply use `git clean -df `and `rm -rf` on any directories that still show in the `git status` “Untracked files:” list.

Now let’s try to merge:


```
    > git merge ps-5.7-8675
      stuff…
```


If you are having a good day, things will merge cleanly with no conflicts, but, chances are that there will be conflicts.  If so, you need to fix them and finalize your merge following the instructions from git.  With this branch, you can now go back up to the **Testing** and **Submit it for review** sections above.

That is it!  Thank you for your interest in our Percona products and for making the effort to contribute!!

