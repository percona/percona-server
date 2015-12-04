#ifndef _SQL_QUERY_STRIPC_COMMENTS_H_
#define _SQL_QUERY_STRIPC_COMMENTS_H_

// implemented in sql_cache.cc
class QueryStripComments
{
private:
  QueryStripComments(const QueryStripComments&);
  QueryStripComments& operator=(const QueryStripComments&);
public:
  QueryStripComments();
  ~QueryStripComments();
  void set(LEX_CSTRING query, uint a_additional_length);

  char* query()        { return buffer; }
  uint  query_length() { return length; }
private:
  void cleanup();
private:
  char* buffer;
  uint  length /*query length, not buffer length*/;
  uint  buffer_length;
};

#endif // _SQL_QUERY_STRIPC_COMMENTS_H_
