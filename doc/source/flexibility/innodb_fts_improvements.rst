.. _innodb_fts_improvements:

====================================
InnoDB Full-Text Search Improvements
====================================

.. contents::
   :local:

.. _ignoring_stopword_list:

======================
Ignoring Stopword List
======================

By default all Full-Text Search indexes check the `stopwords list
<https://dev.mysql.com/doc/refman/8.0/en/fulltext-stopwords.html>`_,
to see if any indexed elements *contain* one of the words on that list.

Using this list for n-gram indexes isn't always suitable, as an example, any
item that contains ``a`` or ``i`` will be ignored. Another word that can't be
searched is ``east``, this one will find no matches because ``a`` is on the
FTS stopword list.

To resolve this issue, *Percona Server for MySQL* has the
:ref:`innodb_ft_ignore_stopwords` variable to control whether
*InnoDB* Full-Text Search should ignore the stopword list.

Although this variable is introduced to resolve n-gram issues, it affects
all Full-Text Search indexes as well.

Being a stopword doesn't just mean to be a one of the predefined
words from the list. Tokens shorter than `innodb_ft_min_token_size
<https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_ft_min_token_size>`_
or longer than `innodb_ft_max_token_size
<https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_ft_max_token_size>`_
are also considered stopwords. Therefore, when
:ref:`innodb_ft_ignore_stopwords` is set to ``ON`` even for non-ngram
FTS, ``innodb_ft_min_token_size`` / ``innodb_ft_max_token_size`` will be
ignored meaning that in this case very short and very long words will
also be indexed.

System Variables
================

.. _innodb_ft_ignore_stopwords:

.. rubric:: ``innodb_ft_ignore_stopwords``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Session, Global
   * - Dynamic
     - Yes
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

When enabled, this variable will instruct *InnoDB* Full Text Search
parser to ignore the stopword list when building/updating an FTS index.
