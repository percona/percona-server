# Contributing Guide

## Welcome to Percona Server for MySQL!

We're excited to have you join the Percona community and participate in keeping open-source open.

### Why Your Contribution Matters?

Main driving force of open-source project is community. We, as developers team, highly appreciate any contributions.

It may be advisable to initially submit bug fixes to the upstream project that will eventually become part of the next
release of Percona. Alternatively, Percona provides expedited response times, transparent development processes, and
streamlined contribution acceptance within a public repository.  
You can contribute in one of the following ways:

* Submit a bug report or a feature
   request in [ our public Jira ](https://jira.percona.com/projects/PS/issues).
* Submit a pull request (PR) with the code patch.
* Contribute to [the documentation](https://github.com/percona/psmysql-docs/blob/innovation-release/contributing.md)
* Reach us on our [ Forums ](https://forums.percona.com/c/mysql-mariadb)

This document describes the workflow for submitting pull requests.
By contributing, you agree to the [Percona Community code of conduct](https://percona.community/contribute/coc/). 

### Steps of the workflow

Steps of the workflow for submitting pull request are:

1. Creating Jira issue
2. Forking of Percona repository
3. Creating a branch for your changes
4. Building code before changes
5. Opening the code in an IDE, making changes and debugging
6. Testing changes
7. Preparing the pull request
8. Peer review and required fixes
9. Merging pull request

Description of each step is below.

Thank you for helping us enhance Percona Server for MySQL. 

## Your Code and MySQL Versions

Percona officially supports 3 version lines of MySQL:

1. 8.0.X - stable line.
2. 8.4.X - next stable line, or LTS line.
3. 9.0.X - innovation line, a test bed for everything new at the moment.

Next LTS version may be 9.7, so until then 9.X are "innovation" versions.

It is not easy to keep track of all the changes and understand how the new code will work with different versions.
Therefore, your best bet is to work with the version line you're currently using and familiar with. Nevertheless,
Percona developers team will take care of your code, and port or backport it to other version lines if it is applicable.

## Creating a Jira Issue

The creation of a Jira issue is _mandatory_ first step for a Pull Request preparation. If you're going to write code for some fix or a new feature, you should first describe in form of a Jira ticket what you are going to do. 

A comprehensive description of the bug or feature is essential. It helps the team to perform thorough testing of the solution, to include of pertinent information in the release notes, and for other relevant details. 

Additionally, linking your future pull request to the corresponding issue with a detailed description will facilitate the development process and help reviewers to understand your solution. 

Also, the Jira issue number is a required part of a branch name for your new code.

To create Jira issue, please open the  [ LINK ](https://perconadev.atlassian.net/jira/software/c/projects/PS/issues) and
authenticate with your favorite social network account (Google, Facebook, etc.). Then follow the instructions and verify
your e-mail. If Jira starts to redirect you back and forth (their bug, not ours), just open a new browser window with
the link above.

Now you're ready to an create issue. Find the big "Create" button in the top menu and press it. Choose issue type from
the second drop-down, saying it is a Bug, an Improvement, or a New feature. Fill in all the required fields below. Please, add accurate and detailed description.
Good description is a big first step to the solution. The
rest of the fields are mostly for internal use; the core team developers will fill it later.
  
  As soon as Jira issue is submitted, you are ready to start coding.
  
## Working with the Source Code

Steps of the workflow:

1. Forking the code to your GitHub account and cloning it locally
2. Creating a branch for your changes
3. Building the code before changes
4. Setting up development environment and making changes
5. Testing

### Forking code to your GitHub account.

First, you should have a GitHub account. You can not work with the Percona repository directly, you have
to [ "Fork" ](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/fork-a-repo)
our repository to your GitHub account. To do so,
open [ Percona PS repository](https://github.com/percona/percona-server) in a browser. Then, right to the repository
name you'll find the "Fork" button. Press it.
If you already have the fork, please do not forget to sync it with the Percona's one.

Then return to your GitHub home page, find your "percona-server" repository and clone it locally.
Note
that [ creating SSH keys and putting public one to the GitHub ](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account)
settings will save a lot of time for you with GitHub workflow. After it is done, you can execute the ```ssh-agent``` to authenticate to the repository automatically during entire desktop session (KDE, Gnome, etc) in all terminals:
```
ssh-add $HOME/.ssh/id_ecdsa
```

### Creating a branch for your changes

In the cloned local repository checkout the "trunk" branch you are going to work on. For 8.0.X releases is 8.0. For
8.4.X series it is 8.4, and for 9.0.X it is "trunk".

Now you are ready to create a branch for your contribution. Please name the branch according to the following pattern:

```
PS-9876-8.0-bug_in_some_module
^^^^^^^
Jira issue number
        ^^^
        Base version
            ^^^^^^^^^^^^^^^^^^^
            Very short definition of the issue 
``` 


### First build of the MySQL source code

The next step is to compile the code in your new branch before making changes.
At this point, you need to fetch third-party code which is organized as Git sub-modules. Go to the project directory and type the following:

```
git submodule update --init
``` 

The Git will fetch required third-party modules into the source tree. 

Now we can try to build the project. Below is a quick and dirty shell script for that (works with GCC 14). It gives you
hints how to do outside-the-tree build and how to set CMake flags.

```
#!/bin/bash

CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Debug \
 -DDOWNLOAD_BOOST=1 \
 -DWITH_BOOST=/home/${USER}/projects/boost\
 -DWITH_DEBUG=ON\
 -DWITH_ROCKSDB=ON\
 -DWITH_COREDUMPER=OFF\
 -DWITH_CURL=system\
 -DWITH_FIDO=system\
 -DWITH_ZLIB=bundled\
 -DCMAKE_CXX_FLAGS=-Wno-error=template-id-cdtor -Wno-error=dangling-reference \
 -Wno-error=deprecated-declarations
 
SRC_DIR=$1
if [ -z ${SRC_DIR} ] ; then
  echo "Please provide source directory as 1st parameter!"
  exit 1
fi
if [ -d $SRC_DIR ] ; then
  echo "Source dir is $SRC_DIR"
else
  echo  "Source dir $SRC_DIR does not exist"
  exit 1
fi
ABS_SRC_DIR=`realpath $SRC_DIR`
if [ -z $2 ] ; then
  BUILD_DIR="build-${SRC_DIR}"
else
  BUILD_DIR=$2
fi
echo "Build dir is $BUILD_DIR"
if [ -d $BUILD_DIR ] ; then
 echo "Build dir exists. Cmake will re-use cached data." 
else
 mkdir $BUILD_DIR
fi
cd $BUILD_DIR
cmake $CMAKE_FLAGS $ABS_SRC_DIR
make -j 16

```

Compiling from a source requires patience. MySQL has many dependencies on different libraries. The good hint is to install build dependencies for the "upstream" version of MySQL. 
For example, on Ubuntu ( don't forget to enable sources in _software-properties-gtk ) run following command:
```
apt build-dep mysql-server-core-8.0
```

Another hint is to check [ documentation on  compilation ](https://docs.percona.com/percona-server/8.0/compile-percona-server.html) .

If it does not help, we recommend a try and fail approach to installing the development packages.

When the source finally successfully compiles, you're ready to run tests and check the build. You have to run at least the "start and shutdown test" of mysqld.

```
cd build-percona-server/mysql-test
./mtr main.1st
``` 

This test ensures that you have working MySQL project. Maybe it is a good idea to execute  ```./mtr``` without parameters to run default test suite. It takes time but it will show that base MySQL functionality works fine in your compiled project.

### Opening the code in an IDE

When the tests are OK, you're ready to set up your favorite IDE for the project development. The build script above will
give you a clue what CMake parameters to add to the project in IDE.

To be able to run and debug the _mysqld_ binary in your IDE, first run the following command in the "mysql-test" directory of the
build tree:

```
./mtr --manual-debug main.1st
```

Then find the string stating with "args:". This is the command line arguments you should pass to the execution of "
mysqld" in your IDE. This step allows to run/debug "mysqld" from the IDE.

From this moment you're ready to code and debug! Happy coding!

### Testing changes

Prior to submitting your code, ensure it is thoroughly tested, both manually and with automated test suites. The maintenance of high-quality code is paramount to ensure the longevity and functionality of our project.

Testing of MySQL is a whole big thing, so please go to
the [ official documentation on MySQL testing ](https://dev.mysql.com/doc/dev/mysql-server/latest/PAGE_TESTING_TOOLS.html)
first.

If it is bug fix, make sure that corresponding tests from the test suite work fine. It's a good idea to patch existing
tests to cover your specific case. For the introduction of new features, the development of a comprehensive test suite is strongly recommended.

## Preparing the Pull Request

After finishing the tasks outlined above, you must prepare your branch for a pull request.   
Please do not forget to run ``` git clang-format ``` on your code before pushing to the GitHub repository.

Then, squash all commits to one or a couple of logically consistent commits.
For example, the fix of bug should have just one commit. If you have a new feature that requires some changes to the
existing code, then the first commit should contain required changes, and the second is a new feature and tests. It is
not a dogma, on the other hand, we value a clean and consistent Git history. Please help us maintain this standard.

Now you can push your local branch to your GitHub repository and make the pull request to the Percona repository.

Please, write meaningful title for the PR and put the number of Jira issue at the beginning of the first line in the
description. The second line is usually a link to the Jira issue. Then the short description follows. No need to write a
long description because you wrote one in the Jira issue.

Example of a pull request form:

Title:
```
PS-9876 Fix of the compression bug
```
Description:
```
PS-9876 Fix of the compression bug

https://perconadev.atlassian.net/browse/PS-9876

The first commit is update of zlib, the second is the fixes of compression function calls.
```

### Peer review and requests of changes

Peer review of the code always goes before PR merge. So, you have to add some core development team members in the "Reviewers" field of PR. Assign the PR to Yura Sorokin, our team lead, and then he will add other developers if needed. 

Our GitHub pipeline runs a lot of checks on each PR. Please make sure you fixed all comments provided by checks. Well, sometime not all of them :) , you understand...

Following this step, the development team will conduct a thorough review of your code at their earliest convenience. Our
primary objective is to ensure the highest quality standards. We encourage you to interpret our comments as
opportunities for improvement and constructive feedback. Comments may contain requests for changes in the code and suggestions how to to solve possible issues. Please, pay close attention to such comments.

### Merging pull request 

When the review is finished and all the issues are addressed, your pull request will be approved and allowed to merge into the main source tree. Usually, the merge is performed by the author, but it is a matter of permissions of the repository, so it is better to ask one of reviewers to merge PR when it is ready.

Finally, your PR is merged! Congratulations! You are now a recognized contributor.


## When will My Code be in the release?

Percona releases follow upstream MySQL releases. We merge upstream code changes and our code from the "trunk" as soon as
the next official upstream release becomes available. Then we build packages and test everything thoroughly. It takes time.

We use a "train" approach. It means, that if some code is already in one of the "trunk" branches before the official
upstream release, it will be released in Percona's packages of new release. So, if we are already in the process of
working on some fresh upstream release, PR will be merged to the "trunk" but not to the release branch. And then your code will be released after the current one.
