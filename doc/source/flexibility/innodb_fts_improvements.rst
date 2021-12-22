.. _innodb_fts_improvements:

====================================
InnoDB Full-Text Search improvements
====================================

.. contents::
   :local:

.. _ignoring_stopword_list:

Ignoring Stopword list
======================

By default all Full-Text Search indexes check the `stopwords list
<https://dev.mysql.com/doc/refman/5.7/en/fulltext-stopwords.html>`_,
to see if any indexed elements *contain* one of the words on that list.

Using this list for n-gram indexes isn't always suitable, as an example, any
item that contains ``a`` or ``i`` will be ignored. Another word that can't be
searched is ``east``, this one will find no matches because ``a`` is on the
FTS stopword list.

To resolve this issue, in |Percona Server| :rn:`5.7.20-18` a new
:variable:`innodb_ft_ignore_stopwords` variable has been implemented
which controls whether InnoDB Full-Text Search should ignore stopword list.

Although this variable is introduced to resolve n-gram issues, it affects
all Full-Text Search indexes as well.

Being a stopword doesn't just mean to be a one of the predefined
words from the list. Tokens shorter than `innodb_ft_min_token_size
<https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_ft_min_token_size>`_
or longer than `innodb_ft_max_token_size
<https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_ft_max_token_size>`_
are also considered stopwords. Therefore, when
:variable:`innodb_ft_ignore_stopwords` is set to ``ON`` even for non-ngram
FTS, ``innodb_ft_min_token_size`` / ``innodb_ft_max_token_size`` will be
ignored meaning that in this case very short and very long words will
also be indexed.

System Variables
----------------

.. variable:: innodb_ft_ignore_stopwords

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ``OFF``

When enabled, this variable will instruct InnoDB Full Text Search
parser to ignore the stopword list when building/updating an FTS index.

.. _punctuation_marks:

Punctuation Marks in Full-Text Search
=====================================

By default, full text search is unable to find words with various punctuation
characters in boolean search mode, although those characters are
indexed with ngram parser. A new variable :variable:`ft_query_extra_word_chars`
was introduced in |Percona Server| :rn:`5.7.21-20` to solve this issue.

When it's enabled, all the non-whitespace symbols are considered to be
word symbols by FTS query parser, except for the boolean search syntax
symbols (which are specified by `ft_boolean_syntax <https://dev.mysql.com/doc/refman/5.7/en/server-system-variables.html#sysvar_ft_boolean_syntax>`_ variable). The latter ones are also considered to be word symbols inside
double quotes. This only applies for the query tokenizer, and the
indexing tokenizer is not changed in any way. Because of this, the
double quote symbol itself is never considered a word symbol, as no
existing indexing tokenizer does so, thus searching for it would never
return documents.

System Variables
----------------

.. variable:: ft_query_extra_word_chars

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ``OFF``

When enabled, this variable will make all non-whitespace symbols (including
punctuation marks) to be treated as word symbols in full-text search queries.

