/* Copyright (c) 2020, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <assert.h>
#include <gtest/gtest.h>
#include <string.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "mem_root_deque.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/field.h"
#include "sql/filesort.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_subselect.h"
#include "sql/join_optimizer/access_path.h"
#include "sql/join_optimizer/common_subexpression_elimination.h"
#include "sql/join_optimizer/explain_access_path.h"
#include "sql/join_optimizer/hypergraph.h"
#include "sql/join_optimizer/join_optimizer.h"
#include "sql/join_optimizer/make_join_hypergraph.h"
#include "sql/join_optimizer/print_utils.h"
#include "sql/join_optimizer/relational_expression.h"
#include "sql/join_optimizer/subgraph_enumeration.h"
#include "sql/join_optimizer/walk_access_paths.h"
#include "sql/join_type.h"
#include "sql/mem_root_array.h"
#include "sql/nested_join.h"
#include "sql/sort_param.h"
#include "sql/sql_class.h"
#include "sql/sql_cmd.h"
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_opt_exec_shared.h"
#include "sql/sql_optimizer.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "template_utils.h"
#include "unittest/gunit/base_mock_field.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/handler-t.h"
#include "unittest/gunit/parsertest.h"
#include "unittest/gunit/test_utils.h"

using hypergraph::NodeMap;
using std::unordered_map;
using std::vector;
using testing::_;
using testing::Pair;
using testing::Return;

static AccessPath *FindBestQueryPlanAndFinalize(THD *thd,
                                                Query_block *query_block,
                                                string *trace) {
  AccessPath *path = FindBestQueryPlan(thd, query_block, trace);
  if (path != nullptr) {
    FinalizePlanForQueryBlock(thd, query_block, path);
  }
  return path;
}

// Base class for the hypergraph unit tests. Its parent class is a type
// parameter, so that it can be used as a base class for both non-parametrized
// tests (::testing::Test) and parametrized tests (::testing::TestWithParam).
template <typename Parent>
class HypergraphTestBase : public Parent {
 public:
  void SetUp() override { m_initializer.SetUp(); }
  void TearDown() override {
    DestroyFakeTables();
    m_initializer.TearDown();
  }

 protected:
  Query_block *ParseAndResolve(const char *query, bool nullable);
  void ResolveFieldToFakeTable(Item *item);
  void ResolveAllFieldsToFakeTable(
      const mem_root_deque<TABLE_LIST *> &join_list);
  void SetJoinConditions(const mem_root_deque<TABLE_LIST *> &join_list);
  void DestroyFakeTables() {
    for (const auto &name_and_table : m_fake_tables) {
      destroy(name_and_table.second);
    }
  }
  handlerton *EnableSecondaryEngine(bool aggregation_is_unordered);

  my_testing::Server_initializer m_initializer;
  THD *m_thd = nullptr;
  std::unordered_map<std::string, Fake_TABLE *> m_fake_tables;
};

template <typename T>
Query_block *HypergraphTestBase<T>::ParseAndResolve(const char *query,
                                                    bool nullable) {
  Query_block *query_block = ::parse(&m_initializer, query, 0);
  m_thd = m_initializer.thd();

  // Create fake TABLE objects for all tables mentioned in the query.
  int num_tables = 0;
  for (TABLE_LIST *tl = query_block->get_table_list(); tl != nullptr;
       tl = tl->next_global) {
    // If we already have created a fake table with this name (for example to
    // get columns of specific types), use that one. Otherwise, create a new one
    // with two integer columns.
    Fake_TABLE *fake_table = m_fake_tables.count(tl->alias) == 0
                                 ? new (m_thd->mem_root)
                                       Fake_TABLE(/*num_columns=*/4, nullable)
                                 : m_fake_tables[tl->alias];
    fake_table->alias = tl->alias;
    fake_table->pos_in_table_list = tl;
    tl->table = fake_table;
    tl->set_tableno(num_tables++);
    m_fake_tables[tl->alias] = fake_table;
  }

  // Find all Item_field objects, and resolve them to fields in the fake tables.
  ResolveAllFieldsToFakeTable(query_block->top_join_list);

  // Also in any conditions and subqueries within the WHERE condition.
  if (query_block->where_cond() != nullptr) {
    WalkItem(query_block->where_cond(), enum_walk::POSTFIX, [&](Item *item) {
      if (item->type() == Item::SUBSELECT_ITEM) {
        Item_in_subselect *item_subselect =
            down_cast<Item_in_subselect *>(item);
        ResolveFieldToFakeTable(item_subselect->left_expr);
        Query_block *child_query_block =
            item_subselect->unit->first_query_block();
        ResolveAllFieldsToFakeTable(child_query_block->top_join_list);
        if (child_query_block->where_cond() != nullptr) {
          ResolveFieldToFakeTable(child_query_block->where_cond());
        }
        for (Item *field_item : child_query_block->fields) {
          ResolveFieldToFakeTable(field_item);
        }
        return true;  // Don't go down into item_subselect->left_expr again.
      } else if (item->type() == Item::FIELD_ITEM) {
        ResolveFieldToFakeTable(item);
      }
      return false;
    });
  }

  // And in the SELECT, GROUP BY and ORDER BY lists.
  for (Item *item : query_block->fields) {
    ResolveFieldToFakeTable(item);
  }
  for (ORDER *cur_group = query_block->group_list.first; cur_group != nullptr;
       cur_group = cur_group->next) {
    ResolveFieldToFakeTable(*cur_group->item);
  }
  for (ORDER *cur_group = query_block->order_list.first; cur_group != nullptr;
       cur_group = cur_group->next) {
    ResolveFieldToFakeTable(*cur_group->item);
  }

  query_block->prepare(m_thd, nullptr);

  // Create a fake, tiny JOIN. (This would normally be done in optimization.)
  query_block->join = new (m_thd->mem_root) JOIN(m_thd, query_block);
  query_block->join->where_cond = query_block->where_cond();
  query_block->join->having_cond = query_block->having_cond();
  query_block->join->fields = &query_block->fields;
  query_block->join->alloc_func_list();
  SetJoinConditions(query_block->top_join_list);

  if (query_block->select_limit != nullptr) {
    query_block->master_query_expression()->select_limit_cnt =
        query_block->select_limit->val_int();
  }

  return query_block;
}

template <typename T>
void HypergraphTestBase<T>::ResolveFieldToFakeTable(Item *item_arg) {
  WalkItem(item_arg, enum_walk::POSTFIX, [&](Item *item) {
    if (item->type() == Item::FIELD_ITEM) {
      Item_field *item_field = down_cast<Item_field *>(item);
      Fake_TABLE *table = m_fake_tables[item_field->table_name];
      item_field->table_ref = table->pos_in_table_list;
      if (strcmp(item_field->field_name, "x") == 0) {
        item_field->field = table->field[0];
      } else if (strcmp(item_field->field_name, "y") == 0) {
        item_field->field = table->field[1];
      } else if (strcmp(item_field->field_name, "z") == 0) {
        item_field->field = table->field[2];
      } else if (strcmp(item_field->field_name, "w") == 0) {
        item_field->field = table->field[3];
      } else {
        assert(false);
      }
      item_field->set_nullable(item_field->field->is_nullable());
      item_field->set_data_type(MYSQL_TYPE_LONG);
    }
    return false;
  });
}

template <typename T>
void HypergraphTestBase<T>::ResolveAllFieldsToFakeTable(
    const mem_root_deque<TABLE_LIST *> &join_list) {
  for (TABLE_LIST *tl : join_list) {
    if (tl->join_cond() != nullptr) {
      ResolveFieldToFakeTable(tl->join_cond());
    }
    if (tl->nested_join != nullptr) {
      ResolveAllFieldsToFakeTable(tl->nested_join->join_list);
    }
  }
}

template <typename T>
void HypergraphTestBase<T>::SetJoinConditions(
    const mem_root_deque<TABLE_LIST *> &join_list) {
  for (TABLE_LIST *tl : join_list) {
    tl->set_join_cond_optim(tl->join_cond());
    if (tl->nested_join != nullptr) {
      SetJoinConditions(tl->nested_join->join_list);
    }
  }
}

template <typename T>
handlerton *HypergraphTestBase<T>::EnableSecondaryEngine(
    bool aggregation_is_unordered) {
  auto hton = new (m_thd->mem_root) Fake_handlerton;
  hton->flags = HTON_SUPPORTS_SECONDARY_ENGINE;
  if (aggregation_is_unordered) {
    hton->secondary_engine_flags =
        MakeSecondaryEngineFlags(SecondaryEngineFlag::SUPPORTS_HASH_JOIN,
                                 SecondaryEngineFlag::AGGREGATION_IS_UNORDERED);
  } else {
    hton->secondary_engine_flags =
        MakeSecondaryEngineFlags(SecondaryEngineFlag::SUPPORTS_HASH_JOIN);
  }
  hton->secondary_engine_modify_access_path_cost = nullptr;

  for (const auto &name_and_table : m_fake_tables) {
    name_and_table.second->file->ht = hton;
  }

  m_thd->lex->m_sql_cmd->use_secondary_storage_engine(hton);

  return hton;
}

namespace {

/// An error checker which, upon destruction, verifies that a single error was
/// raised while the checker was alive, and that the error had the expected
/// error number. If an error is raised, the THD::is_error() flag will be set,
/// just as in the server. (The default error_handler_hook used by the unit
/// tests, does not set the error flag in the THD.) If expected_errno is 0, it
/// will instead check that no error was raised.
class ErrorChecker {
 public:
  ErrorChecker(const THD *thd, unsigned expected_errno)
      : m_thd(thd),
        m_errno(expected_errno),
        m_saved_error_hook(error_handler_hook) {
    // Use an error handler which sets the THD::is_error() flag.
    error_handler_hook = my_message_sql;
    EXPECT_FALSE(thd->is_error());
  }

  ~ErrorChecker() {
    error_handler_hook = m_saved_error_hook;
    if (m_errno != 0) {
      EXPECT_TRUE(m_thd->is_error());
      EXPECT_EQ(m_errno, m_thd->get_stmt_da()->mysql_errno());
      EXPECT_EQ(1, m_thd->get_stmt_da()->current_statement_cond_count());
    } else {
      EXPECT_FALSE(m_thd->is_error());
    }
  }

 private:
  const THD *m_thd;
  unsigned m_errno;
  ErrorHandlerFunctionPointer m_saved_error_hook;
};

// Sort the nodes in the given graph by name, which makes the test
// a bit more robust against irrelevant changes. Note that we don't
// sort edges, since it's often useful to correlate the code with
// the Graphviz output in the optimizer trace, which isn't sorted.
void SortNodes(JoinHypergraph *graph) {
  // Sort nodes (by alias). We sort a series of indexes first the same way
  // so that we know which went where.
  std::vector<int> node_order;
  for (unsigned i = 0; i < graph->nodes.size(); ++i) {
    node_order.push_back(i);
  }
  std::sort(node_order.begin(), node_order.end(), [graph](int a, int b) {
    return strcmp(graph->nodes[a].table->alias, graph->nodes[b].table->alias) <
           0;
  });
  std::sort(graph->nodes.begin(), graph->nodes.end(),
            [](const JoinHypergraph::Node &a, const JoinHypergraph::Node &b) {
              return strcmp(a.table->alias, b.table->alias) < 0;
            });

  // Remap hyperedges to the new node indexes. Note that we don't
  // change the neighborhood, because nothing in these tests need it.
  int node_map[MAX_TABLES];
  for (unsigned i = 0; i < graph->nodes.size(); ++i) {
    node_map[node_order[i]] = i;
  }
  for (hypergraph::Hyperedge &edge : graph->graph.edges) {
    NodeMap new_left = 0, new_right = 0;
    for (int node_idx : BitsSetIn(edge.left)) {
      new_left |= NodeMap{1} << node_map[node_idx];
    }
    for (int node_idx : BitsSetIn(edge.right)) {
      new_right |= NodeMap{1} << node_map[node_idx];
    }
    edge.left = new_left;
    edge.right = new_right;
  }

  // Remap TES.
  for (Predicate &pred : graph->predicates) {
    NodeMap new_tes = 0;
    for (int node_idx : BitsSetIn(pred.total_eligibility_set)) {
      new_tes |= NodeMap{1} << node_map[node_idx];
    }
    pred.total_eligibility_set = new_tes;
  }
}

}  // namespace

using MakeHypergraphTest = HypergraphTestBase<::testing::Test>;

TEST_F(MakeHypergraphTest, SingleTable) {
  Query_block *query_block =
      ParseAndResolve("SELECT 1 FROM t1", /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, /*trace=*/nullptr, &graph));

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(1, graph.nodes.size());
  EXPECT_EQ(0, graph.edges.size());
  EXPECT_EQ(0, graph.predicates.size());

  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
}

TEST_F(MakeHypergraphTest, InnerJoin) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x JOIN t3 ON t2.y=t3.y",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  SortNodes(&graph);

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // Simple edges; order doesn't matter.
  ASSERT_EQ(2, graph.edges.size());

  // t1/t2. There is no index information, so the default 0.1 should be used.
  EXPECT_EQ(0x01, graph.graph.edges[2].left);
  EXPECT_EQ(0x02, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[1].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[1].selectivity);

  // t2/t3.
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[0].selectivity);

  EXPECT_EQ(0, graph.predicates.size());
}

TEST_F(MakeHypergraphTest, OuterJoin) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 LEFT JOIN (t2 LEFT JOIN t3 ON t2.y=t3.y) ON t1.x=t2.x",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // Hyperedges. Order doesn't matter.
  ASSERT_EQ(2, graph.edges.size());

  // t2/t3.
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[0].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[0].selectivity);

  // t1/t2; since the predicate is null-rejecting on t2, we can rewrite.
  EXPECT_EQ(0x01, graph.graph.edges[2].left);
  EXPECT_EQ(0x02, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[1].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[1].selectivity);

  EXPECT_EQ(0, graph.predicates.size());
}

TEST_F(MakeHypergraphTest, OuterJoinNonNullRejecting) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 LEFT JOIN (t2 LEFT JOIN t3 ON t2.y=t3.y OR t2.y "
      "IS NULL) ON t1.x=t2.x",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // Hyperedges. Order doesn't matter.
  ASSERT_EQ(2, graph.edges.size());

  // t2/t3.
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[0].expr->type);
  EXPECT_FLOAT_EQ(1.0 - (0.9 * 0.9),
                  graph.edges[0].selectivity);  // OR of two conditions.

  // t1/{t2,t3}; the predicate is not null-rejecting (unlike the previous test),
  // so we need the full hyperedge.
  EXPECT_EQ(0x01, graph.graph.edges[2].left);
  EXPECT_EQ(0x06, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[1].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[1].selectivity);

  EXPECT_EQ(0, graph.predicates.size());
}

TEST_F(MakeHypergraphTest, SemiJoin) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 WHERE t1.x IN (SELECT t2.x FROM t2 JOIN t3 ON "
      "t2.y=t3.y)",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // Hyperedges. Order doesn't matter.
  ASSERT_EQ(2, graph.edges.size());

  // t2/t3.
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[0].selectivity);

  // t1/{t2,t3}.
  EXPECT_EQ(0x01, graph.graph.edges[2].left);
  EXPECT_EQ(0x06, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::SEMIJOIN, graph.edges[1].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[1].selectivity);

  EXPECT_EQ(0, graph.predicates.size());
}

TEST_F(MakeHypergraphTest, AntiJoin) {
  // NOTE: Fields must be non-nullable, or NOT IN can not be rewritten.
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 WHERE t1.x NOT IN (SELECT t2.x FROM t2 JOIN t3 ON "
      "t2.y=t3.y)",
      /*nullable=*/false);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // Hyperedges. Order doesn't matter.
  ASSERT_EQ(2, graph.edges.size());

  // t2/t3.
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[0].selectivity);

  // t1/{t2,t3}.
  EXPECT_EQ(0x01, graph.graph.edges[2].left);
  EXPECT_EQ(0x06, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::ANTIJOIN, graph.edges[1].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[1].selectivity);

  EXPECT_EQ(0, graph.predicates.size());
}

TEST_F(MakeHypergraphTest, Predicates) {
  // The OR ... IS NULL part is to keep the LEFT JOIN from being simplified
  // to an inner join.
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 LEFT JOIN t2 ON t1.x=t2.x "
      "WHERE t1.x=2 AND (t2.y=3 OR t2.y IS NULL)",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(2, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);

  // t1/t2.
  ASSERT_EQ(1, graph.edges.size());
  EXPECT_EQ(0x01, graph.graph.edges[0].left);
  EXPECT_EQ(0x02, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[0].expr->type);
  EXPECT_FLOAT_EQ(0.1, graph.edges[0].selectivity);

  ASSERT_EQ(2, graph.predicates.size());
  EXPECT_EQ("(t1.x = 2)", ItemToString(graph.predicates[0].condition));
  EXPECT_EQ(0x01, graph.predicates[0].total_eligibility_set);  // Only t1.
  EXPECT_FLOAT_EQ(0.1,
                  graph.predicates[0].selectivity);  // No specific information.

  EXPECT_EQ("((t2.y = 3) or (t2.y is null))",
            ItemToString(graph.predicates[1].condition));
  EXPECT_GT(graph.predicates[1].selectivity,
            0.1);  // More common due to the OR NULL.
  EXPECT_EQ(0x03,
            graph.predicates[1].total_eligibility_set);  // Both t1 and t2!
}

// See also the PredicatePushdown* tests below.
TEST_F(MakeHypergraphTest, AssociativeRewriteToImprovePushdown) {
  // Note that the WHERE condition needs _both_ associativity and
  // commutativity to become a proper join condition (t2 needs to be
  // pulled out; doing t1 instead would create a degenerate join).
  // The IS NULL is to keep the left join from being converted
  // into an inner join.
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM (t1 JOIN t2 ON TRUE) LEFT JOIN t3 ON TRUE "
      "WHERE t2.x=t3.x OR t3.x IS NULL",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t2", graph.nodes[0].table->alias);
  EXPECT_STREQ("t1", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // t1/t3.
  ASSERT_EQ(2, graph.edges.size());
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[0].expr->type);
  EXPECT_EQ(0, graph.edges[0].expr->join_conditions.size());
  EXPECT_FLOAT_EQ(1.0, graph.edges[0].selectivity);

  // t2/{t1,t3}. This join should also carry the predicate.
  EXPECT_EQ(0x01, graph.graph.edges[2].left);
  EXPECT_EQ(0x06, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[1].expr->type);
  EXPECT_EQ(1, graph.edges[1].expr->join_conditions.size());
  EXPECT_FLOAT_EQ(1.0, graph.edges[1].selectivity);

  EXPECT_EQ(0, graph.predicates.size());
}

TEST_F(MakeHypergraphTest, Cycle) {
  // If == is outer join and -- is inner join:
  //
  // t6 == t1 -- t2 -- t4 == t5
  //        |  /
  //        | /
  //       t3
  //
  // Note that t6 is on the _left_ side of the inner join, so we should be able
  // to push down conditions to it.

  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM "
      "((t1,t2,t3,t4) LEFT JOIN t5 ON t4.x=t5.x) LEFT JOIN t6 ON t1.x=t6.x "
      "WHERE t1.x=t2.x AND t2.x=t3.x AND t1.x=t3.x AND t2.x=t4.x",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  SortNodes(&graph);

  ASSERT_EQ(6, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);
  EXPECT_STREQ("t4", graph.nodes[3].table->alias);
  EXPECT_STREQ("t5", graph.nodes[4].table->alias);
  EXPECT_STREQ("t6", graph.nodes[5].table->alias);

  // t1/t2.
  ASSERT_EQ(6, graph.edges.size());
  EXPECT_EQ(0x01, graph.graph.edges[0].left);
  EXPECT_EQ(0x02, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);

  // t2/t3.
  EXPECT_EQ(0x04, graph.graph.edges[2].left);
  EXPECT_EQ(0x02, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[1].expr->type);

  // t4/t2.
  EXPECT_EQ(0x08, graph.graph.edges[4].left);
  EXPECT_EQ(0x02, graph.graph.edges[4].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[2].expr->type);

  // t4/t5.
  EXPECT_EQ(0x08, graph.graph.edges[6].left);
  EXPECT_EQ(0x10, graph.graph.edges[6].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[3].expr->type);

  // t1/t6.
  EXPECT_EQ(0x01, graph.graph.edges[8].left);
  EXPECT_EQ(0x20, graph.graph.edges[8].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[4].expr->type);

  // t3/t1; added last because it completes a cycle.
  EXPECT_EQ(0x04, graph.graph.edges[10].left);
  EXPECT_EQ(0x01, graph.graph.edges[10].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[5].expr->type);

  // The three predicates from the cycle should be added, but no others.
  // The TES should be equivalent to the SES, ie., the outer joins should
  // not influence this.
  ASSERT_EQ(3, graph.predicates.size());

  EXPECT_EQ("(t1.x = t2.x)", ItemToString(graph.predicates[0].condition));
  EXPECT_EQ(0x03, graph.predicates[0].total_eligibility_set);  // t1/t2.
  EXPECT_TRUE(graph.predicates[0].was_join_condition);

  EXPECT_EQ("(t2.x = t3.x)", ItemToString(graph.predicates[1].condition));
  EXPECT_EQ(0x06, graph.predicates[1].total_eligibility_set);  // t2/t3.
  EXPECT_TRUE(graph.predicates[1].was_join_condition);

  EXPECT_EQ("(t1.x = t3.x)", ItemToString(graph.predicates[2].condition));
  EXPECT_EQ(0x05, graph.predicates[2].total_eligibility_set);  // t1/t3.
  EXPECT_TRUE(graph.predicates[2].was_join_condition);
}

TEST_F(MakeHypergraphTest, NoCycleBelowOuterJoin) {
  // The OR ... IS NULL part is to keep the LEFT JOIN from being simplified
  // to an inner join.
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 LEFT JOIN (t2,t3,t4) ON t1.x=t2.x "
      "WHERE (t2.x=t3.x OR t2.x IS NULL) "
      "AND (t3.x=t4.x OR t3.x IS NULL) "
      "AND (t4.x=t2.x OR t4.x IS NULL)",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(4, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);
  EXPECT_STREQ("t4", graph.nodes[3].table->alias);

  // t2/t3.
  ASSERT_EQ(3, graph.edges.size());
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);

  // {t2,t3}/t4 (due to the Cartesian product).
  EXPECT_EQ(0x06, graph.graph.edges[2].left);
  EXPECT_EQ(0x08, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[1].expr->type);

  // t1/{t2,t3,t4} (the outer join).
  EXPECT_EQ(0x01, graph.graph.edges[4].left);
  EXPECT_EQ(0x0e, graph.graph.edges[4].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[2].expr->type);

  // The three predicates are still there; no extra predicates due to cycles.
  EXPECT_EQ(3, graph.predicates.size());
}

TEST_F(MakeHypergraphTest, CyclePushedFromOuterJoinCondition) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM "
      "t1 LEFT JOIN (t2 JOIN (t3 JOIN t4 ON t3.x=t4.x) ON t2.x=t3.x) "
      "ON t1.x=t2.x AND t2.x=t4.x",
      /*nullable=*/true);

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  SortNodes(&graph);

  ASSERT_EQ(4, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);
  EXPECT_STREQ("t4", graph.nodes[3].table->alias);

  // t3/t2.
  ASSERT_EQ(4, graph.edges.size());
  EXPECT_EQ(0x04, graph.graph.edges[2].left);
  EXPECT_EQ(0x02, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[1].expr->type);

  // t2/t4 (pushed from the ON condition).
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x08, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);

  // t1/{t2,t3,t4} (the outer join).
  EXPECT_EQ(0x01, graph.graph.edges[4].left);
  EXPECT_EQ(0x0e, graph.graph.edges[4].right);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN, graph.edges[2].expr->type);

  // t3/t4; added last because it completes a cycle.
  EXPECT_EQ(0x04, graph.graph.edges[6].left);
  EXPECT_EQ(0x08, graph.graph.edges[6].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[3].expr->type);

  // The three predicates from the cycle should be added, but no others.
  // The TES should be equivalent to the SES, ie., the outer joins should
  // not influence this.
  ASSERT_EQ(3, graph.predicates.size());

  EXPECT_EQ("(t2.x = t3.x)", ItemToString(graph.predicates[1].condition));
  EXPECT_EQ(0x06, graph.predicates[1].total_eligibility_set);  // t2/t3.
  EXPECT_TRUE(graph.predicates[1].was_join_condition);

  EXPECT_EQ("(t2.x = t4.x)", ItemToString(graph.predicates[0].condition));
  EXPECT_EQ(0x0a, graph.predicates[0].total_eligibility_set);  // t2/t4.
  EXPECT_TRUE(graph.predicates[0].was_join_condition);

  EXPECT_EQ("(t3.x = t4.x)", ItemToString(graph.predicates[2].condition));
  EXPECT_EQ(0x0c, graph.predicates[2].total_eligibility_set);  // t3/t4.
  EXPECT_TRUE(graph.predicates[2].was_join_condition);
}

TEST_F(MakeHypergraphTest, MultipleEqualitiesCauseCycle) {
  Query_block *query_block =
      ParseAndResolve("SELECT 1 FROM t1,t2,t3 WHERE t1.x=t2.x AND t2.x=t3.x",
                      /*nullable=*/true);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // t1/t2.
  ASSERT_EQ(3, graph.edges.size());
  EXPECT_EQ(0x01, graph.graph.edges[0].left);
  EXPECT_EQ(0x02, graph.graph.edges[0].right);

  // t2/t3.
  EXPECT_EQ(0x02, graph.graph.edges[2].left);
  EXPECT_EQ(0x04, graph.graph.edges[2].right);

  // t1/t3 (the cycle edge).
  EXPECT_EQ(0x01, graph.graph.edges[4].left);
  EXPECT_EQ(0x04, graph.graph.edges[4].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[2].expr->type);
}

TEST_F(MakeHypergraphTest, Flattening) {
  // This query is impossible to push cleanly without flattening,
  // or adding broad hyperedges. We want to make sure we don't try to
  // “solve” it by pushing the t2.x = t3.x condition twice.
  // Due to flattening, we also don't get any Cartesian products.
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 JOIN (t2 JOIN (t3 JOIN t4)) "
      "WHERE t1.y = t4.y AND t2.x = t3.x AND t3.x = t4.x",
      /*nullable=*/true);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));
  EXPECT_EQ("(multiple equal(t1.y, t4.y) and multiple equal(t2.x, t3.x, t4.x))",
            ItemToString(query_block->where_cond()));

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  SortNodes(&graph);

  ASSERT_EQ(4, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);
  EXPECT_STREQ("t4", graph.nodes[3].table->alias);

  ASSERT_EQ(4, graph.edges.size());

  // t2/t3.
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);
  ASSERT_EQ(1, graph.edges[1].expr->equijoin_conditions.size());
  EXPECT_EQ("(t2.x = t3.x)",
            ItemToString(graph.edges[0].expr->equijoin_conditions[0]));

  // t1/t4.
  EXPECT_EQ(0x01, graph.graph.edges[2].left);
  EXPECT_EQ(0x08, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[1].expr->type);
  ASSERT_EQ(1, graph.edges[1].expr->equijoin_conditions.size());
  EXPECT_EQ("(t1.y = t4.y)",
            ItemToString(graph.edges[1].expr->equijoin_conditions[0]));

  // t3/t4.
  EXPECT_EQ(0x04, graph.graph.edges[4].left);
  EXPECT_EQ(0x08, graph.graph.edges[4].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[2].expr->type);
  ASSERT_EQ(1, graph.edges[2].expr->equijoin_conditions.size());
  EXPECT_EQ("(t3.x = t4.x)",
            ItemToString(graph.edges[2].expr->equijoin_conditions[0]));

  // t2/t4.
  EXPECT_EQ(0x02, graph.graph.edges[6].left);
  EXPECT_EQ(0x08, graph.graph.edges[6].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[3].expr->type);
  ASSERT_EQ(1, graph.edges[3].expr->equijoin_conditions.size());
  EXPECT_EQ("(t2.x = t4.x)",
            ItemToString(graph.edges[3].expr->equijoin_conditions[0]));
}

TEST_F(MakeHypergraphTest, PredicatePromotionOnMultipleEquals) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1,t2,t3 WHERE t1.x=t2.x AND t2.x=t3.x AND t1.y=t3.y",
      /*nullable=*/true);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // t1/t2.
  ASSERT_EQ(3, graph.edges.size());
  EXPECT_EQ(0x01, graph.graph.edges[0].left);
  EXPECT_EQ(0x02, graph.graph.edges[0].right);
  EXPECT_EQ(0, graph.edges[0].expr->join_conditions.size());
  ASSERT_EQ(1, graph.edges[0].expr->equijoin_conditions.size());
  EXPECT_EQ("(t1.x = t2.x)",
            ItemToString(graph.edges[0].expr->equijoin_conditions[0]));

  // t2/t3.
  EXPECT_EQ(0x02, graph.graph.edges[2].left);
  EXPECT_EQ(0x04, graph.graph.edges[2].right);
  EXPECT_EQ(0, graph.edges[1].expr->join_conditions.size());
  ASSERT_EQ(1, graph.edges[1].expr->equijoin_conditions.size());
  EXPECT_EQ("(t2.x = t3.x)",
            ItemToString(graph.edges[1].expr->equijoin_conditions[0]));

  // t1/t3 (the cycle edge). Has both the original condition and the
  // multi-equality condition.
  EXPECT_EQ(0x01, graph.graph.edges[4].left);
  EXPECT_EQ(0x04, graph.graph.edges[4].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[2].expr->type);
  EXPECT_EQ(0, graph.edges[2].expr->join_conditions.size());
  ASSERT_EQ(2, graph.edges[2].expr->equijoin_conditions.size());
  EXPECT_EQ("(t1.y = t3.y)",
            ItemToString(graph.edges[2].expr->equijoin_conditions[0]));
  EXPECT_EQ("(t1.x = t3.x)",
            ItemToString(graph.edges[2].expr->equijoin_conditions[1]));

  // Verify that the ones coming from the multi-equality are marked with
  // the same index, so that they are properly deduplicated.
  ASSERT_EQ(4, graph.predicates.size());

  EXPECT_EQ("(t1.x = t2.x)", ItemToString(graph.predicates[0].condition));
  EXPECT_TRUE(graph.predicates[0].was_join_condition);
  EXPECT_EQ(0, graph.predicates[0].source_multiple_equality_idx);

  EXPECT_EQ("(t2.x = t3.x)", ItemToString(graph.predicates[1].condition));
  EXPECT_TRUE(graph.predicates[1].was_join_condition);
  EXPECT_EQ(0, graph.predicates[1].source_multiple_equality_idx);

  EXPECT_EQ("(t1.y = t3.y)", ItemToString(graph.predicates[2].condition));
  EXPECT_TRUE(graph.predicates[2].was_join_condition);
  EXPECT_EQ(-1, graph.predicates[2].source_multiple_equality_idx);

  EXPECT_EQ("(t1.x = t3.x)", ItemToString(graph.predicates[3].condition));
  EXPECT_TRUE(graph.predicates[3].was_join_condition);
  EXPECT_EQ(0, graph.predicates[3].source_multiple_equality_idx);
}

// Verify that multiple equalities are properly resolved to a single equality,
// and not left as a multiple one. Antijoins have a similar issue.
// Inspired by issues in a larger query (DBT-3 Q21).
TEST_F(MakeHypergraphTest, MultipleEqualityPushedFromJoinConditions) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1, t2 "
      "WHERE t1.x=t2.x AND t1.x IN (SELECT t3.x FROM t3) ",
      /*nullable=*/false);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // t1/t2.
  ASSERT_EQ(2, graph.edges.size());
  EXPECT_EQ(0x01, graph.graph.edges[0].left);
  EXPECT_EQ(0x02, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);
  ASSERT_EQ(1, graph.edges[0].expr->equijoin_conditions.size());
  EXPECT_EQ("(t1.x = t2.x)",
            ItemToString(graph.edges[0].expr->equijoin_conditions[0]));
  EXPECT_EQ(0, graph.edges[0].expr->join_conditions.size());

  // t2/t3 (semijoin). t1/t3 would also be fine. The really important part
  // is that we do not also have a t1/t2 or t1/t3 join conditions.
  EXPECT_EQ(0x02, graph.graph.edges[2].left);
  EXPECT_EQ(0x04, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::SEMIJOIN, graph.edges[1].expr->type);
  ASSERT_EQ(1, graph.edges[1].expr->equijoin_conditions.size());
  EXPECT_EQ("(t2.x = t3.x)",
            ItemToString(graph.edges[1].expr->equijoin_conditions[0]));
  EXPECT_EQ(0, graph.edges[1].expr->join_conditions.size());
}

TEST_F(MakeHypergraphTest, UnpushableMultipleEqualityCausesCycle) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1, t2, t3, t4 "
      // Two simple equalities that set up a join structure.
      "WHERE t1.y=t2.y AND t2.z=t3.z "
      // And then a multi-equality that is not cleanly pushable onto that
      // structure.
      "AND t1.x=t3.x AND t3.x=t4.x",
      /*nullable=*/false);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  SortNodes(&graph);

  ASSERT_EQ(4, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);
  EXPECT_STREQ("t4", graph.nodes[3].table->alias);

  ASSERT_EQ(5, graph.edges.size());

  // t1/t2.
  EXPECT_EQ(0x01, graph.graph.edges[0].left);
  EXPECT_EQ(0x02, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);
  ASSERT_EQ(1, graph.edges[0].expr->equijoin_conditions.size());
  EXPECT_EQ("(t1.y = t2.y)",
            ItemToString(graph.edges[0].expr->equijoin_conditions[0]));
  EXPECT_EQ(0, graph.edges[0].expr->join_conditions.size());

  // t3/t2.
  EXPECT_EQ(0x04, graph.graph.edges[2].left);
  EXPECT_EQ(0x02, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[1].expr->type);
  ASSERT_EQ(1, graph.edges[1].expr->equijoin_conditions.size());
  EXPECT_EQ("(t2.z = t3.z)",
            ItemToString(graph.edges[1].expr->equijoin_conditions[0]));
  EXPECT_EQ(0, graph.edges[1].expr->join_conditions.size());

  // t4/t3 (the first of many cycle edges from the multiple equality).
  EXPECT_EQ(0x08, graph.graph.edges[4].left);
  EXPECT_EQ(0x04, graph.graph.edges[4].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[2].expr->type);
  ASSERT_EQ(1, graph.edges[2].expr->equijoin_conditions.size());
  EXPECT_EQ("(t4.x = t3.x)",
            ItemToString(graph.edges[2].expr->equijoin_conditions[0]));
  EXPECT_EQ(0, graph.edges[2].expr->join_conditions.size());

  // t3/t1 (cycle edge).
  EXPECT_EQ(0x04, graph.graph.edges[6].left);
  EXPECT_EQ(0x01, graph.graph.edges[6].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[3].expr->type);
  ASSERT_EQ(1, graph.edges[3].expr->equijoin_conditions.size());
  EXPECT_EQ("(t1.x = t3.x)",
            ItemToString(graph.edges[3].expr->equijoin_conditions[0]));
  EXPECT_EQ(0, graph.edges[3].expr->join_conditions.size());

  // t1/t4 (cycle edge within the cycle, comes from meshing).
  EXPECT_EQ(0x01, graph.graph.edges[8].left);
  EXPECT_EQ(0x08, graph.graph.edges[8].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[4].expr->type);
  ASSERT_EQ(1, graph.edges[4].expr->equijoin_conditions.size());
  EXPECT_EQ("(t1.x = t4.x)",
            ItemToString(graph.edges[4].expr->equijoin_conditions[0]));
  EXPECT_EQ(0, graph.edges[4].expr->join_conditions.size());
}

TEST_F(MakeHypergraphTest, UnpushableMultipleEqualityWithSameTableTwice) {
  // The (t2.y, t3.x, t3.y, t4.x) multi-equality is unpushable
  // due to the t1.z = t4.w equality that's already set up;
  // we need to create a cycle from t2/t3/t4, while still not losing
  // the t3.x = t3.y condition.
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1, t1 AS t2, t1 AS t3, t1 AS t4 "
      "WHERE t1.z = t4.w "
      "AND t2.y = t3.x AND t3.x = t3.y AND t3.y = t4.x",
      /*nullable=*/false);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  SortNodes(&graph);

  ASSERT_EQ(4, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);
  EXPECT_STREQ("t4", graph.nodes[3].table->alias);

  ASSERT_EQ(4, graph.edges.size());

  // We only check that the given edges exist, and that we didn't lose
  // the t3.x = t3.y condition. All edges come from explicit
  // WHERE conditions.

  // t2/t3. Note that we get both t2.y=t3.y and t2.y=t3.x;
  // they come from the same multi-equality and we've already
  // checked t3.x=t3.y, so one is redundant, but we can't
  // figure this out yet.
  EXPECT_EQ(0x02, graph.graph.edges[0].left);
  EXPECT_EQ(0x04, graph.graph.edges[0].right);

  // t1/t4.
  EXPECT_EQ(0x01, graph.graph.edges[2].left);
  EXPECT_EQ(0x08, graph.graph.edges[2].right);

  // t3/t4.
  EXPECT_EQ(0x04, graph.graph.edges[4].left);
  EXPECT_EQ(0x08, graph.graph.edges[4].right);

  // t2/t4.
  EXPECT_EQ(0x02, graph.graph.edges[6].left);
  EXPECT_EQ(0x08, graph.graph.edges[6].right);

  bool found_predicate = false;
  for (const Predicate &pred : graph.predicates) {
    if (ItemToString(pred.condition) == "(t3.x = t3.y)") {
      found_predicate = true;
    }
  }
  EXPECT_TRUE(found_predicate);
}

// Sets up a nonsensical query, but the point is that the multiple equality
// on the antijoin can be resolved to either t1.x or t2.x, and it should choose
// the same as is already there due to the inequality in order to not create
// an overly broad hyperedge. This is similar to a situation in DBT-3 Q21.
//
// We test with the inequality referring to both tables in turn, to make sure
// that we're not just getting lucky.
using MakeHypergraphMultipleEqualParamTest =
    HypergraphTestBase<::testing::TestWithParam<int>>;

TEST_P(MakeHypergraphMultipleEqualParamTest,
       MultipleEqualityOnAntijoinGetsIdeallyResolved) {
  const int table_num = GetParam();
  string other_table = (table_num == 0) ? "t1" : "t2";
  string query_str =
      "SELECT 1 FROM t1, t2 WHERE t1.x=t2.x "
      "AND t1.x NOT IN (SELECT t3.x FROM t3 WHERE t3.x <> " +
      other_table + ".x + 1)";
  Query_block *query_block = ParseAndResolve(query_str.c_str(),
                                             /*nullable=*/false);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  JoinHypergraph graph(m_thd->mem_root, query_block);
  string trace;
  EXPECT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  EXPECT_EQ(graph.graph.nodes.size(), graph.nodes.size());
  EXPECT_EQ(graph.graph.edges.size(), 2 * graph.edges.size());

  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  // t1/t2. This one should not be too surprising.
  ASSERT_EQ(2, graph.edges.size());
  EXPECT_EQ(0x01, graph.graph.edges[0].left);
  EXPECT_EQ(0x02, graph.graph.edges[0].right);
  EXPECT_EQ(RelationalExpression::INNER_JOIN, graph.edges[0].expr->type);
  ASSERT_EQ(1, graph.edges[0].expr->equijoin_conditions.size());
  EXPECT_EQ("(t1.x = t2.x)",
            ItemToString(graph.edges[0].expr->equijoin_conditions[0]));
  EXPECT_EQ(0, graph.edges[0].expr->join_conditions.size());

  // t1/t3 (antijoin) or t2/t3. The important part is that this should _not_
  // be a hyperedge.
  if (table_num == 0) {
    EXPECT_EQ(0x01, graph.graph.edges[2].left);
  } else {
    EXPECT_EQ(0x02, graph.graph.edges[2].left);
  }
  EXPECT_EQ(0x04, graph.graph.edges[2].right);
  EXPECT_EQ(RelationalExpression::ANTIJOIN, graph.edges[1].expr->type);
  ASSERT_EQ(1, graph.edges[1].expr->equijoin_conditions.size());
  ASSERT_EQ(1, graph.edges[1].expr->join_conditions.size());
  EXPECT_EQ("(" + other_table + ".x = t3.x)",
            ItemToString(graph.edges[1].expr->equijoin_conditions[0]));
  EXPECT_EQ("(t3.x <> (" + other_table + ".x + 1))",
            ItemToString(graph.edges[1].expr->join_conditions[0]));
}

INSTANTIATE_TEST_SUITE_P(All, MakeHypergraphMultipleEqualParamTest,
                         ::testing::Values(0, 1));

// An alias for better naming.
// We don't verify costs; to do that, we'd probably need to mock out
// the cost model.
using HypergraphOptimizerTest = MakeHypergraphTest;

TEST_F(HypergraphOptimizerTest, SingleTable) {
  Query_block *query_block =
      ParseAndResolve("SELECT 1 FROM t1", /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  ASSERT_EQ(AccessPath::TABLE_SCAN, root->type);
  EXPECT_EQ(m_fake_tables["t1"], root->table_scan().table);
  EXPECT_FLOAT_EQ(100, root->num_output_rows);
}

TEST_F(HypergraphOptimizerTest,
       PredicatePushdown) {  // Also tests nested loop join.
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x WHERE t2.y=3", /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 200;
  m_fake_tables["t2"]->file->stats.records = 3;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The pushed-down filter makes the optimal plan be t2 on the left side,
  // with a nested loop.
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
  EXPECT_EQ(JoinType::INNER, root->nested_loop_join().join_type);
  EXPECT_FLOAT_EQ(6, root->num_output_rows);  // 60 rows, 10% selectivity.

  // The condition should be posted directly on t2.
  AccessPath *outer = root->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::FILTER, outer->type);
  EXPECT_EQ("(t2.y = 3)", ItemToString(outer->filter().condition));
  EXPECT_FLOAT_EQ(0.3, outer->num_output_rows);  // 10% default selectivity.

  AccessPath *outer_child = outer->filter().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer_child->type);
  EXPECT_EQ(m_fake_tables["t2"], outer_child->table_scan().table);
  EXPECT_FLOAT_EQ(3, outer_child->num_output_rows);

  // The inner part should have a join condition as a filter.
  AccessPath *inner = root->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::FILTER, inner->type);
  EXPECT_EQ("(t1.x = t2.x)", ItemToString(inner->filter().condition));
  EXPECT_FLOAT_EQ(20, inner->num_output_rows);  // 10% default selectivity.

  AccessPath *inner_child = inner->filter().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner_child->type);
  EXPECT_EQ(m_fake_tables["t1"], inner_child->table_scan().table);
}

TEST_F(HypergraphOptimizerTest, PredicatePushdownOuterJoin) {
  // The OR ... IS NULL part is to keep the LEFT JOIN from being simplified
  // to an inner join.
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 LEFT JOIN t2 ON t1.x=t2.x "
      "WHERE t1.y=42 AND (t2.y=3 OR t2.y IS NULL)",
      /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 2000;
  m_fake_tables["t2"]->file->stats.records = 3;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.

  // The t2 filter cannot be pushed down through the join, so it should be
  // on the root.
  ASSERT_EQ(AccessPath::FILTER, root->type);
  EXPECT_EQ("((t2.y = 3) or (t2.y is null))",
            ItemToString(root->filter().condition));

  AccessPath *join = root->filter().child;
  ASSERT_EQ(AccessPath::HASH_JOIN, join->type);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN,
            join->hash_join().join_predicate->expr->type);
  EXPECT_FLOAT_EQ(
      200, join->num_output_rows);  // Selectivity overridden by outer join.

  // The t1 condition should be pushed down to t1, since it's outer to the join.
  AccessPath *outer = join->hash_join().outer;
  ASSERT_EQ(AccessPath::FILTER, outer->type);
  EXPECT_EQ("(t1.y = 42)", ItemToString(outer->filter().condition));

  AccessPath *t1 = outer->filter().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, t1->type);
  EXPECT_EQ(m_fake_tables["t1"], t1->table_scan().table);

  AccessPath *inner = join->hash_join().inner;
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner->type);
  EXPECT_EQ(m_fake_tables["t2"], inner->table_scan().table);
  EXPECT_FLOAT_EQ(3, inner->num_output_rows);
}

// NOTE: We don't test selectivity here, because it's not necessarily
// correct.
TEST_F(HypergraphOptimizerTest, PartialPredicatePushdown) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1, t2 "
      "WHERE (t1.x=1 AND t2.y=2) OR (t1.x=3 AND t2.y=4)",
      /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 200;
  m_fake_tables["t2"]->file->stats.records = 30;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::HASH_JOIN, root->type);
  EXPECT_EQ(RelationalExpression::INNER_JOIN,
            root->hash_join().join_predicate->expr->type);

  // The WHERE should have been pushed down to a join condition,
  // which should not be removed despite the partial pushdown.
  const Mem_root_array<Item *> &join_conditions =
      root->hash_join().join_predicate->expr->join_conditions;
  ASSERT_EQ(1, join_conditions.size());
  EXPECT_EQ("(((t1.x = 1) and (t2.y = 2)) or ((t1.x = 3) and (t2.y = 4)))",
            ItemToString(join_conditions[0]));

  // t1 should have a partial condition.
  AccessPath *outer = root->hash_join().outer;
  ASSERT_EQ(AccessPath::FILTER, outer->type);
  EXPECT_EQ("((t1.x = 1) or (t1.x = 3))",
            ItemToString(outer->filter().condition));

  AccessPath *outer_child = outer->filter().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer_child->type);
  EXPECT_EQ(m_fake_tables["t1"], outer_child->table_scan().table);

  // t2 should have a different partial condition.
  AccessPath *inner = root->hash_join().inner;
  ASSERT_EQ(AccessPath::FILTER, inner->type);
  EXPECT_EQ("((t2.y = 2) or (t2.y = 4))",
            ItemToString(inner->filter().condition));

  AccessPath *inner_child = inner->filter().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner_child->type);
  EXPECT_EQ(m_fake_tables["t2"], inner_child->table_scan().table);
}

TEST_F(HypergraphOptimizerTest, PartialPredicatePushdownOuterJoin) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 LEFT JOIN t2 ON "
      "(t1.x=1 AND t2.y=2) OR (t1.x=3 AND t2.y=4)",
      /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 200;
  m_fake_tables["t2"]->file->stats.records = 30;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::HASH_JOIN, root->type);
  EXPECT_EQ(RelationalExpression::LEFT_JOIN,
            root->hash_join().join_predicate->expr->type);

  // The join condition should still be there.
  const Mem_root_array<Item *> &join_conditions =
      root->hash_join().join_predicate->expr->join_conditions;
  ASSERT_EQ(1, join_conditions.size());
  EXPECT_EQ("(((t1.x = 1) and (t2.y = 2)) or ((t1.x = 3) and (t2.y = 4)))",
            ItemToString(join_conditions[0]));

  // t1 should _not_ have a partial condition, as it would
  // cause NULL-complemented rows to be eaten.
  AccessPath *outer = root->hash_join().outer;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer->type);
  EXPECT_EQ(m_fake_tables["t1"], outer->table_scan().table);

  // t2 should have a partial condition.
  AccessPath *inner = root->hash_join().inner;
  ASSERT_EQ(AccessPath::FILTER, inner->type);
  EXPECT_EQ("((t2.y = 2) or (t2.y = 4))",
            ItemToString(inner->filter().condition));

  AccessPath *inner_child = inner->filter().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner_child->type);
  EXPECT_EQ(m_fake_tables["t2"], inner_child->table_scan().table);
}

TEST_F(HypergraphOptimizerTest, PredicatePushdownToRef) {
  Query_block *query_block =
      ParseAndResolve("SELECT 1 FROM t1 WHERE t1.x=3", /*nullable=*/true);
  Fake_TABLE *t1 = m_fake_tables["t1"];
  t1->create_index(t1->field[0], t1->field[1], /*unique=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The condition should be gone, and only ref access should be in its place.
  // There shouldn't be EQ_REF, since we only have a partial match.
  ASSERT_EQ(AccessPath::REF, root->type);
  EXPECT_EQ(0, root->ref().ref->key);
  EXPECT_EQ(8, root->ref().ref->key_length);
  EXPECT_EQ(1, root->ref().ref->key_parts);
}

TEST_F(HypergraphOptimizerTest, NotPredicatePushdownToRef) {
  Query_block *query_block =
      ParseAndResolve("SELECT 1 FROM t1 WHERE t1.y=3", /*nullable=*/true);
  Fake_TABLE *t1 = m_fake_tables["t1"];
  t1->create_index(t1->field[0], t1->field[1], /*unique=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // t1.y can't be pushed since t1.x wasn't.
  ASSERT_EQ(AccessPath::FILTER, root->type);
  EXPECT_EQ("(t1.y = 3)", ItemToString(root->filter().condition));
}

TEST_F(HypergraphOptimizerTest, MultiPartPredicatePushdownToRef) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 WHERE t1.y=3 AND t1.x=2", /*nullable=*/true);
  Fake_TABLE *t1 = m_fake_tables["t1"];
  t1->create_index(t1->field[0], t1->field[1], /*unique=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // Both should be pushed, and we should now use the unique index.
  ASSERT_EQ(AccessPath::EQ_REF, root->type);
  EXPECT_EQ(0, root->eq_ref().ref->key);
  EXPECT_EQ(16, root->eq_ref().ref->key_length);
  EXPECT_EQ(2, root->eq_ref().ref->key_parts);
}

TEST_F(HypergraphOptimizerTest, JoinConditionToRef) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 LEFT JOIN (t2 JOIN t3 ON t2.y=t3.y) ON t1.x=t3.x",
      /*nullable=*/true);
  Fake_TABLE *t2 = m_fake_tables["t2"];
  Fake_TABLE *t3 = m_fake_tables["t3"];
  t2->create_index(t2->field[1], /*column2=*/nullptr, /*unique=*/false);
  t3->create_index(t3->field[0], t3->field[1], /*unique=*/true);

  // Hash join between t2/t3 is attractive, but hash join between t1 and t2/t3
  // should not be.
  m_fake_tables["t1"]->file->stats.records = 1000000;
  m_fake_tables["t2"]->file->stats.records = 100;
  m_fake_tables["t3"]->file->stats.records = 1000;
  m_fake_tables["t3"]->file->stats.data_file_length = 1e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The optimal plan consists of only nested-loop joins.
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
  EXPECT_EQ(JoinType::OUTER, root->nested_loop_join().join_type);

  AccessPath *outer = root->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer->type);
  EXPECT_EQ(m_fake_tables["t1"], outer->table_scan().table);
  EXPECT_FLOAT_EQ(1000000.0, outer->num_output_rows);

  // The inner part should also be nested-loop.
  AccessPath *inner = root->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, inner->type);
  EXPECT_EQ(JoinType::INNER, inner->nested_loop_join().join_type);

  // We should have t2 on the left, and t3 on the right
  // (or we couldn't use the entire unique index).
  AccessPath *t2_path = inner->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::TABLE_SCAN, t2_path->type);
  EXPECT_EQ(m_fake_tables["t2"], t2_path->table_scan().table);
  EXPECT_FLOAT_EQ(100.0, t2_path->num_output_rows);

  // t3 should use the unique index, and thus be capped at one row.
  AccessPath *t3_path = inner->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::EQ_REF, t3_path->type);
  EXPECT_EQ(m_fake_tables["t3"], t3_path->eq_ref().table);
  EXPECT_FLOAT_EQ(1.0, t3_path->num_output_rows);

  // t2/t3 is 100 * 1, obviously.
  EXPECT_FLOAT_EQ(100.0, inner->num_output_rows);

  // The root should have t1 multiplied by t2/t3;
  // since the join predicate is already applied (and subsumed),
  // we should have no further reduction from it.
  EXPECT_FLOAT_EQ(outer->num_output_rows * inner->num_output_rows,
                  root->num_output_rows);
}

// Verify that we can make sargable predicates out of multiple equalities.
TEST_F(HypergraphOptimizerTest, MultiEqualitySargable) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1, t2, t3 WHERE t1.x = t2.x AND t2.x = t3.x",
      /*nullable=*/true);
  Fake_TABLE *t2 = m_fake_tables["t2"];
  Fake_TABLE *t3 = m_fake_tables["t3"];
  t2->create_index(t2->field[0], /*column2=*/nullptr, /*unique=*/false);
  t3->create_index(t3->field[0], /*column2=*/nullptr, /*unique=*/false);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  // The logical plan should be t1/t2/t3, with index lookups on t2 and t3.
  // should not be.
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 10000;
  m_fake_tables["t3"]->file->stats.records = 1000000;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The optimal plan consists of only nested-loop joins.
  // We don't verify costs.
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
  EXPECT_EQ(JoinType::INNER, root->nested_loop_join().join_type);

  // t1 is on the outside.
  AccessPath *outer = root->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer->type);
  EXPECT_EQ(m_fake_tables["t1"], outer->table_scan().table);

  // The inner part should also be nested-loop.
  AccessPath *inner = root->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, inner->type);
  EXPECT_EQ(JoinType::INNER, inner->nested_loop_join().join_type);

  // We have two index lookups; t2 and t3. We don't care about the order.
  ASSERT_EQ(AccessPath::REF, inner->nested_loop_join().outer->type);
  ASSERT_EQ(AccessPath::REF, inner->nested_loop_join().inner->type);
}

TEST_F(HypergraphOptimizerTest, DoNotApplyBothSargableJoinAndFilterJoin) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1, t2, t3, t4 WHERE t1.x = t2.x AND t2.x = t3.x",
      /*nullable=*/true);
  Fake_TABLE *t1 = m_fake_tables["t1"];
  t1->create_index(t1->field[0], /*column2=*/nullptr, /*unique=*/false);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  // The logical plan should be to hash-join t2/t3, then nestloop-join
  // against the index on t1. The t4 table somehow needs to be present
  // to trigger the issue; it doesn't really matter whether it's on the
  // left or right side (since it doesn't have a join condition),
  // but it happens to be put on the right.
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 100000000;
  m_fake_tables["t3"]->file->stats.records = 1000000;
  m_fake_tables["t4"]->file->stats.records = 10000;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // t4 needs to come in on the top (since we've put it as a Cartesian product);
  // either left or right side. It happens to be on the right.
  // We don't verify costs.
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
  EXPECT_EQ(JoinType::INNER, root->nested_loop_join().join_type);

  AccessPath *inner = root->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner->type);
  EXPECT_EQ(m_fake_tables["t4"], inner->table_scan().table);

  // Now for the meat of the plan. There should be a nested loop,
  // with t2/t3 on the inside and t1 on the outside.
  AccessPath *outer = root->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, outer->type);

  // We don't check the t2/t3 part very thoroughly.
  EXPECT_EQ(AccessPath::HASH_JOIN, outer->nested_loop_join().outer->type);

  // Now for the point of the test: We should have t1 on the inner side,
  // with t1=t2 pushed down into the index, and it should _not_ have a t1=t3
  // filter; even though it would seemingly be attractive to join t1=t3 against
  // the ref access, that would be double-counting the selectivity and thus
  // not permitted. (Well, it would be permitted, but we'd have to add code
  // not to apply the selectivity twice, and then it would just be extra cost
  // applying a redundant filter.)
  AccessPath *inner_inner = outer->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::REF, inner_inner->type);
  EXPECT_STREQ("t1", inner_inner->ref().table->alias);
  EXPECT_EQ(0, inner_inner->ref().ref->key);
  EXPECT_EQ("t2.x", ItemToString(inner_inner->ref().ref->items[0]));
}

static string PrintSargablePredicate(const SargablePredicate &sp,
                                     const JoinHypergraph &graph) {
  return StringPrintf(
      "%s.%s -> %s [%s]", sp.field->table->alias, sp.field->field_name,
      ItemToString(sp.other_side).c_str(),
      ItemToString(graph.predicates[sp.predicate_index].condition).c_str());
}

// Verify that when we add a cycle in the graph due to a multiple equality,
// that join predicate also becomes sargable.
using HypergraphOptimizerCyclePredicatesSargableTest =
    HypergraphTestBase<::testing::TestWithParam<const char *>>;

TEST_P(HypergraphOptimizerCyclePredicatesSargableTest,
       CyclePredicatesSargable) {
  Query_block *query_block = ParseAndResolve(GetParam(),
                                             /*nullable=*/true);
  Fake_TABLE *t1 = m_fake_tables["t1"];
  Fake_TABLE *t2 = m_fake_tables["t2"];
  Fake_TABLE *t3 = m_fake_tables["t3"];
  t1->create_index(t1->field[0], /*column2=*/nullptr, /*unique=*/false);
  t2->create_index(t2->field[0], /*column2=*/nullptr, /*unique=*/false);
  t3->create_index(t3->field[0], /*column2=*/nullptr, /*unique=*/false);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  string trace;
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  JoinHypergraph graph(m_thd->mem_root, query_block);
  ASSERT_FALSE(MakeJoinHypergraph(m_thd, &trace, &graph));
  FindSargablePredicates(m_thd, &trace, &graph);

  // Each node should have two sargable join predicates
  // (one to each of the other nodes). Verify that they are
  // correctly set up (the order does not matter, though).
  ASSERT_EQ(3, graph.nodes.size());
  EXPECT_STREQ("t1", graph.nodes[0].table->alias);
  EXPECT_STREQ("t2", graph.nodes[1].table->alias);
  EXPECT_STREQ("t3", graph.nodes[2].table->alias);

  ASSERT_EQ(2, graph.nodes[0].sargable_predicates.size());
  EXPECT_EQ(
      "t1.field_1 -> t2.x [(t1.x = t2.x)]",
      PrintSargablePredicate(graph.nodes[0].sargable_predicates[0], graph));
  EXPECT_EQ(
      "t1.field_1 -> t3.x [(t1.x = t3.x)]",
      PrintSargablePredicate(graph.nodes[0].sargable_predicates[1], graph));

  ASSERT_EQ(2, graph.nodes[1].sargable_predicates.size());
  EXPECT_EQ(
      "t2.field_1 -> t3.x [(t2.x = t3.x)]",
      PrintSargablePredicate(graph.nodes[1].sargable_predicates[0], graph));
  EXPECT_EQ(
      "t2.field_1 -> t1.x [(t1.x = t2.x)]",
      PrintSargablePredicate(graph.nodes[1].sargable_predicates[1], graph));

  ASSERT_EQ(2, graph.nodes[2].sargable_predicates.size());
  EXPECT_EQ(
      "t3.field_1 -> t2.x [(t2.x = t3.x)]",
      PrintSargablePredicate(graph.nodes[2].sargable_predicates[0], graph));
  EXPECT_EQ(
      "t3.field_1 -> t1.x [(t1.x = t3.x)]",
      PrintSargablePredicate(graph.nodes[2].sargable_predicates[1], graph));
}

INSTANTIATE_TEST_SUITE_P(
    TrueAndFalse, HypergraphOptimizerCyclePredicatesSargableTest,
    ::testing::Values(
        // With and without an explicit cycle.
        "SELECT 1 FROM t1,t2,t3 WHERE t1.x=t2.x AND t2.x=t3.x AND t1.x=t3.x",
        "SELECT 1 FROM t1,t2,t3 WHERE t1.x=t2.x AND t2.x=t3.x"));

TEST_F(HypergraphOptimizerTest, SimpleInnerJoin) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x JOIN t3 ON t2.y=t3.y",
      /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 10000;
  m_fake_tables["t2"]->file->stats.records = 100;
  m_fake_tables["t3"]->file->stats.records = 1000000;

  // Set up some large scan costs to discourage nested loop.
  m_fake_tables["t1"]->file->stats.data_file_length = 100e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t3"]->file->stats.data_file_length = 10000e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // It's pretty obvious given the sizes of these tables that the optimal
  // order for hash join is t3 hj (t1 hj t2). We don't check the costs
  // beyond that.

  ASSERT_EQ(AccessPath::HASH_JOIN, root->type);
  EXPECT_EQ(RelationalExpression::INNER_JOIN,
            root->hash_join().join_predicate->expr->type);

  AccessPath *outer = root->hash_join().outer;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer->type);
  EXPECT_EQ(m_fake_tables["t3"], outer->table_scan().table);

  AccessPath *inner = root->hash_join().inner;
  ASSERT_EQ(AccessPath::HASH_JOIN, inner->type);

  AccessPath *t1 = inner->hash_join().outer;
  ASSERT_EQ(AccessPath::TABLE_SCAN, t1->type);
  EXPECT_EQ(m_fake_tables["t1"], t1->table_scan().table);

  AccessPath *t2 = inner->hash_join().inner;
  ASSERT_EQ(AccessPath::TABLE_SCAN, t2->type);
  EXPECT_EQ(m_fake_tables["t2"], t2->table_scan().table);

  // We should have seen the other plans, too (in particular, joining
  // {t1} versus {t2,t3}; {t1,t3} versus {t2} is illegal since we don't
  // consider Cartesian products). The six subplans seen are:
  //
  // t1, t2, t3, t1-t2, t2-t3, t1-{t2,t3}, {t1,t2}-t3
  EXPECT_EQ(m_thd->m_current_query_partial_plans, 6);
}

TEST_F(HypergraphOptimizerTest, StraightJoin) {
  Query_block *query_block =
      ParseAndResolve("SELECT 1 FROM t1 STRAIGHT_JOIN t2 ON t1.x=t2.x",
                      /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 10000;

  // Set up some large scan costs to discourage nested loop.
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 100e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The optimal order would be to reorder (t2, t1), but this should be
  // disallowed due to the use of STRAIGHT_JOIN.

  ASSERT_EQ(AccessPath::HASH_JOIN, root->type);
  EXPECT_EQ(RelationalExpression::STRAIGHT_INNER_JOIN,
            root->hash_join().join_predicate->expr->type);

  AccessPath *outer = root->hash_join().outer;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer->type);
  EXPECT_EQ(m_fake_tables["t1"], outer->table_scan().table);

  AccessPath *inner = root->hash_join().inner;
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner->type);
  EXPECT_EQ(m_fake_tables["t2"], inner->table_scan().table);

  // We should see only the two table scans and then t1-t2, no other orders.
  EXPECT_EQ(m_thd->m_current_query_partial_plans, 3);
}

TEST_F(HypergraphOptimizerTest, Cycle) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM "
      "t1,t2,t3 WHERE t1.x=t2.x AND t2.x=t3.x AND t1.x=t3.x",
      /*nullable=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // We should see t1, t2, t3, {t1,t2}, {t2,t3}, {t1,t3} and {t1,t2,t3}.
  EXPECT_EQ(m_thd->m_current_query_partial_plans, 7);
}

TEST_F(HypergraphOptimizerTest, CycleFromMultipleEquality) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM "
      "t1,t2,t3 WHERE t1.x=t2.x AND t2.x=t3.x",
      /*nullable=*/true);

  // Build multiple equalities from the WHERE condition.
  COND_EQUAL *cond_equal = nullptr;
  EXPECT_FALSE(optimize_cond(m_thd, query_block->where_cond_ref(), &cond_equal,
                             &query_block->top_join_list,
                             &query_block->cond_value));

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // We should see t1, t2, t3, {t1,t2}, {t2,t3}, {t1,t3} and {t1,t2,t3}.
  EXPECT_EQ(m_thd->m_current_query_partial_plans, 7);
}

TEST_F(HypergraphOptimizerTest, SwitchesOrderToMakeSafeForRowid) {
  // Mark t1.y as a blob, to make sure we need rowids for our sort.
  Mock_field_long t1_x(/*is_unsigned=*/false);
  Base_mock_field_blob t1_y(/*length=*/1000000);
  t1_x.field_name = "x";
  t1_y.field_name = "y";

  Fake_TABLE *t1 = new (m_initializer.thd()->mem_root) Fake_TABLE(&t1_x, &t1_y);
  m_fake_tables["t1"] = t1;

  t1->set_created();
  Query_block *query_block = ParseAndResolve(
      "SELECT t1.y FROM t1 JOIN t2 ON t1.x=t2.x ORDER BY t1.y, t2.y",
      /*nullable=*/true);

  t1->create_index(t1->field[0], nullptr, /*unique=*/false);
  Fake_TABLE *t2 = m_fake_tables["t2"];
  t2->create_index(t2->field[0], nullptr, /*unique=*/false);

  // The normal case for rowid-unsafe tables are LATERAL derived tables,
  // but since we don't support derived tables in the unit test,
  // we cheat and mark t2 as unsafe for row IDs manually instead,
  // and also disallow hash join.
  handlerton *hton = EnableSecondaryEngine(/*aggregation_is_unordered=*/false);
  hton->secondary_engine_flags =
      MakeSecondaryEngineFlags(SecondaryEngineFlag::SUPPORTS_NESTED_LOOP_JOIN);
  hton->secondary_engine_modify_access_path_cost =
      [](THD *, const JoinHypergraph &, AccessPath *path) {
        if (path->type == AccessPath::REF &&
            strcmp("t2", path->ref().table->alias) == 0) {
          path->safe_for_rowid = AccessPath::SAFE_IF_SCANNED_ONCE;
        }
        return false;
      };

  m_fake_tables["t1"]->file->stats.records = 99;
  m_fake_tables["t2"]->file->stats.records = 100;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // Normally, it would be better to have t1 on the outside
  // and t2 on the inside, since t2 is the larger one, but that would create
  // a materialization, so the better version is to flip.
  ASSERT_EQ(AccessPath::SORT, root->type);
  AccessPath *join = root->sort().child;
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, join->type);
  AccessPath *outer = join->nested_loop_join().outer;
  AccessPath *inner = join->nested_loop_join().inner;

  ASSERT_EQ(AccessPath::TABLE_SCAN, outer->type);
  EXPECT_STREQ("t2", outer->table_scan().table->alias);

  ASSERT_EQ(AccessPath::REF, inner->type);
  EXPECT_STREQ("t1", inner->ref().table->alias);
}

namespace {

struct FullTextParam {
  const char *query;
  bool expect_filter;
  bool expect_index;
};

std::ostream &operator<<(std::ostream &os, const FullTextParam &param) {
  return os << param.query;
}

}  // namespace

using HypergraphFullTextTest =
    HypergraphTestBase<::testing::TestWithParam<FullTextParam>>;

TEST_P(HypergraphFullTextTest, FullTextSearch) {
  SCOPED_TRACE(GetParam().query);

  // CREATE TABLE t1(x VARCHAR(100)).
  Base_mock_field_varstring column1(/*length=*/100, /*share=*/nullptr);
  column1.field_name = "x";
  Fake_TABLE *t1 = new (m_initializer.thd()->mem_root) Fake_TABLE(&column1);
  t1->file->stats.records = 10000;
  m_fake_tables["t1"] = t1;
  t1->set_created();

  // CREATE FULLTEXT INDEX idx ON t1(x).
  down_cast<Mock_HANDLER *>(t1->file)->set_ha_table_flags(
      t1->file->ha_table_flags() | HA_CAN_FULLTEXT);
  t1->create_index(&column1, /*column2=*/nullptr, ulong{HA_FULLTEXT});

  Query_block *query_block = ParseAndResolve(GetParam().query,
                                             /*nullable=*/false);
  ASSERT_NE(nullptr, query_block);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));
  ASSERT_NE(nullptr, root);

  AccessPath *path = root;

  if (GetParam().expect_filter) {
    ASSERT_EQ(AccessPath::FILTER, path->type);
    path = path->filter().child;
  }

  if (GetParam().expect_index) {
    ASSERT_EQ(AccessPath::FULL_TEXT_SEARCH, path->type);
    // Since there is no ORDER BY in the query, expect an unordered index scan.
    EXPECT_FALSE(query_block->is_ordered());
    EXPECT_FALSE(path->full_text_search().use_order);
  } else {
    EXPECT_EQ(AccessPath::TABLE_SCAN, path->type);
  }
}

static constexpr FullTextParam full_text_queries[] = {
    // Expect a full-text index scan if the predicate returns true for positive
    // scores only. Expect the index scan to have a filter on top of it if the
    // predicate does not return true for all non-zero scores.
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc' IN BOOLEAN MODE)",
     /*expect_filter=*/false,
     /*expect_index=*/true},
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc')",
     /*expect_filter=*/false,
     /*expect_index=*/true},
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') > 0",
     /*expect_filter=*/false,
     /*expect_index=*/true},
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') > 0.5",
     /*expect_filter=*/true,
     /*expect_index=*/true},
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') >= 0.5",
     /*expect_filter=*/true,
     /*expect_index=*/true},
    {"SELECT t1.x FROM t1 WHERE 0.5 < MATCH(t1.x) AGAINST ('abc')",
     /*expect_filter=*/true,
     /*expect_index=*/true},
    {"SELECT t1.x FROM t1 WHERE 0.5 <= MATCH(t1.x) AGAINST ('abc')",
     /*expect_filter=*/true,
     /*expect_index=*/true},

    // Expect a table scan if the predicate might return true for zero or
    // negative scores. A filter node is added on top for the predicate.
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') < 0.5",
     /*expect_filter=*/true,
     /*expect_index=*/false},
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') <= 0.5",
     /*expect_filter=*/true,
     /*expect_index=*/false},
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') >= 0",
     /*expect_filter=*/true,
     /*expect_index=*/false},
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') > -1",
     /*expect_filter=*/true,
     /*expect_index=*/false},
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') <> 0.5",
     /*expect_filter=*/true,
     /*expect_index=*/false},
    {"SELECT t1.x FROM t1 WHERE 0.5 > MATCH(t1.x) AGAINST ('abc')",
     /*expect_filter=*/true,
     /*expect_index=*/false},
    {"SELECT t1.x FROM t1 WHERE 0.5 >= MATCH(t1.x) AGAINST ('abc')",
     /*expect_filter=*/true,
     /*expect_index=*/false},

    // Expect a table scan if the predicate checks for an exact score. (Not
    // because an index scan cannot be used, but because it's not a very useful
    // query, so we haven't optimized for it.)
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') = 0.5",
     /*expect_filter=*/true,
     /*expect_index=*/false},

    // Expect a table scan if the predicate is a disjunction.
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc' IN BOOLEAN MODE) "
     "OR MATCH(t1.x) AGAINST ('xyz' IN BOOLEAN MODE)",
     /*expect_filter=*/true,
     /*expect_index=*/false},

    // Expect an index scan if the predicate is a conjunction. A filter node
    // will be added for the predicate that is not subsumed by the index.
    {"SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc' IN BOOLEAN MODE) "
     "AND MATCH(t1.x) AGAINST ('xyz' IN BOOLEAN MODE)",
     /*expect_filter=*/true,
     /*expect_index=*/true},
};
INSTANTIATE_TEST_SUITE_P(FullTextQueries, HypergraphFullTextTest,
                         ::testing::ValuesIn(full_text_queries));

TEST_F(HypergraphOptimizerTest, FullTextSearchNoHashJoin) {
  // CREATE TABLE t1(x VARCHAR(100)).
  Base_mock_field_varstring column1(/*length=*/100, /*share=*/nullptr);
  column1.field_name = "x";
  Fake_TABLE *t1 = new (m_initializer.thd()->mem_root) Fake_TABLE(&column1);
  m_fake_tables["t1"] = t1;
  t1->set_created();

  // CREATE FULLTEXT INDEX idx ON t1(x).
  down_cast<Mock_HANDLER *>(t1->file)->set_ha_table_flags(
      t1->file->ha_table_flags() | HA_CAN_FULLTEXT);
  t1->create_index(&column1, /*column2=*/nullptr, ulong{HA_FULLTEXT});

  Query_block *query_block = ParseAndResolve(
      "SELECT MATCH(t1.x) AGAINST ('abc') FROM t1, t2 WHERE t1.x = t2.x",
      /*nullable=*/false);
  ASSERT_NE(nullptr, query_block);

  // Add some rows to make a hash join more tempting than a nested loop join.
  m_fake_tables["t1"]->file->stats.records = 1000;
  m_fake_tables["t2"]->file->stats.records = 1000;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));
  ASSERT_NE(nullptr, root);

  // FTS does not work well with hash join, so we force nested loop join for
  // this query.
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
}

TEST_F(HypergraphOptimizerTest, FullTextCanSkipRanking) {
  // CREATE TABLE t1(x VARCHAR(100)).
  Base_mock_field_varstring column1(/*length=*/100, /*share=*/nullptr);
  column1.field_name = "x";
  Fake_TABLE *t1 = new (m_initializer.thd()->mem_root) Fake_TABLE(&column1);
  m_fake_tables["t1"] = t1;
  t1->set_created();

  // CREATE FULLTEXT INDEX idx ON t1(x).
  down_cast<Mock_HANDLER *>(t1->file)->set_ha_table_flags(
      t1->file->ha_table_flags() | HA_CAN_FULLTEXT);
  t1->create_index(&column1, /*column2=*/nullptr, ulong{HA_FULLTEXT});

  Query_block *query_block = ParseAndResolve(
      "SELECT MATCH(t1.x) AGAINST ('a') FROM t1 WHERE "
      "MATCH(t1.x) AGAINST ('a') AND "
      "MATCH(t1.x) AGAINST ('b') AND "
      "MATCH(t1.x) AGAINST ('c') AND MATCH(t1.x) AGAINST ('c') > 0.1",
      /*nullable=*/false);
  ASSERT_NE(nullptr, query_block);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));
  ASSERT_NE(nullptr, root);

  const List<Item_func_match> *ftfuncs = query_block->ftfunc_list;
  ASSERT_EQ(5, ftfuncs->size());

  // MATCH(t1.x) AGAINST ('a') needs ranking because it is used in the
  // SELECT list.
  EXPECT_EQ("(match t1.x against ('a'))", ItemToString((*ftfuncs)[0]));
  EXPECT_EQ(nullptr, (*ftfuncs)[0]->master);
  EXPECT_FALSE((*ftfuncs)[0]->can_skip_ranking());
  EXPECT_EQ((*ftfuncs)[0], (*ftfuncs)[1]->get_master());

  // MATCH (t1.x) AGAINST ('b') does not need ranking, since it's only used
  // in a standalone predicate.
  EXPECT_EQ("(match t1.x against ('b'))", ItemToString((*ftfuncs)[2]));
  EXPECT_EQ(nullptr, (*ftfuncs)[2]->master);
  EXPECT_TRUE((*ftfuncs)[2]->can_skip_ranking());

  // MATCH (t1.x) AGAINST ('c') needs ranking because one of the predicates
  // requires it to return > 0.1.
  EXPECT_EQ("(match t1.x against ('c'))", ItemToString((*ftfuncs)[3]));
  EXPECT_EQ(nullptr, (*ftfuncs)[3]->master);
  EXPECT_FALSE((*ftfuncs)[3]->can_skip_ranking());
  EXPECT_EQ((*ftfuncs)[3], (*ftfuncs)[4]->get_master());
}

TEST_F(HypergraphOptimizerTest, FullTextAvoidDescSort) {
  // CREATE TABLE t1(x VARCHAR(100)).
  Base_mock_field_varstring column1(/*length=*/100, /*share=*/nullptr);
  column1.field_name = "x";
  Fake_TABLE *t1 = new (m_initializer.thd()->mem_root) Fake_TABLE(&column1);
  t1->file->stats.records = 10000;
  m_fake_tables["t1"] = t1;
  t1->set_created();

  // CREATE FULLTEXT INDEX idx ON t1(x).
  down_cast<Mock_HANDLER *>(t1->file)->set_ha_table_flags(
      t1->file->ha_table_flags() | HA_CAN_FULLTEXT);
  t1->create_index(&column1, /*column2=*/nullptr, ulong{HA_FULLTEXT});

  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') "
      "ORDER BY MATCH(t1.x) AGAINST ('abc') DESC",
      /*nullable=*/false);
  ASSERT_NE(nullptr, query_block);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));
  ASSERT_NE(nullptr, root);

  // Expect no sort in the plan. An ordered index scan is used.
  ASSERT_EQ(AccessPath::FULL_TEXT_SEARCH, root->type);
  EXPECT_TRUE(root->full_text_search().use_order);
}

TEST_F(HypergraphOptimizerTest, FullTextAscSort) {
  // CREATE TABLE t1(x VARCHAR(100)).
  Base_mock_field_varstring column1(/*length=*/100, /*share=*/nullptr);
  column1.field_name = "x";
  Fake_TABLE *t1 = new (m_initializer.thd()->mem_root) Fake_TABLE(&column1);
  t1->file->stats.records = 10000;
  m_fake_tables["t1"] = t1;
  t1->set_created();

  // CREATE FULLTEXT INDEX idx ON t1(x).
  down_cast<Mock_HANDLER *>(t1->file)->set_ha_table_flags(
      t1->file->ha_table_flags() | HA_CAN_FULLTEXT);
  t1->create_index(&column1, /*column2=*/nullptr, ulong{HA_FULLTEXT});

  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x FROM t1 WHERE MATCH(t1.x) AGAINST ('abc') "
      "ORDER BY MATCH(t1.x) AGAINST ('abc') ASC",
      /*nullable=*/false);
  ASSERT_NE(nullptr, query_block);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));
  ASSERT_NE(nullptr, root);

  // The full-text index can only return results in descending order, so expect
  // a SORT node on top.
  EXPECT_EQ(AccessPath::SORT, root->type);
}

TEST_F(HypergraphOptimizerTest, FullTextDescSortNoPredicate) {
  // CREATE TABLE t1(x VARCHAR(100)).
  Base_mock_field_varstring column1(/*length=*/100, /*share=*/nullptr);
  column1.field_name = "x";
  Fake_TABLE *t1 = new (m_initializer.thd()->mem_root) Fake_TABLE(&column1);
  t1->file->stats.records = 10000;
  m_fake_tables["t1"] = t1;
  t1->set_created();

  // CREATE FULLTEXT INDEX idx ON t1(x).
  down_cast<Mock_HANDLER *>(t1->file)->set_ha_table_flags(
      t1->file->ha_table_flags() | HA_CAN_FULLTEXT);
  t1->create_index(&column1, /*column2=*/nullptr, ulong{HA_FULLTEXT});

  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x FROM t1 ORDER BY MATCH(t1.x) AGAINST ('abc') DESC",
      /*nullable=*/false);
  ASSERT_NE(nullptr, query_block);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));
  ASSERT_NE(nullptr, root);

  // A full-text index scan cannot be used for ordering when there is no
  // predicate, since the index scan doesn't return all rows (only those with a
  // positive score). Expect a SORT node on top.
  EXPECT_EQ(AccessPath::SORT, root->type);
}

TEST_F(HypergraphOptimizerTest, DistinctIsDoneAsSort) {
  Query_block *query_block =
      ParseAndResolve("SELECT DISTINCT t1.y, t1.x FROM t1", /*nullable=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::SORT, root->type);
  Filesort *sort = root->sort().filesort;
  ASSERT_EQ(2, sort->sort_order_length());
  EXPECT_EQ("t1.y", ItemToString(sort->sortorder[0].item));
  EXPECT_EQ("t1.x", ItemToString(sort->sortorder[1].item));
  EXPECT_TRUE(sort->m_remove_duplicates);

  EXPECT_EQ(AccessPath::TABLE_SCAN, root->sort().child->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, DistinctIsSubsumedByGroup) {
  Query_block *query_block = ParseAndResolve(
      "SELECT DISTINCT t1.y, t1.x, 3 FROM t1 GROUP BY t1.x, t1.y",
      /*nullable=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::AGGREGATE, root->type);
  AccessPath *child = root->aggregate().child;

  EXPECT_EQ(AccessPath::SORT, child->type);
  EXPECT_FALSE(child->sort().filesort->m_remove_duplicates);
}

TEST_F(HypergraphOptimizerTest, DistinctWithOrderBy) {
  m_initializer.thd()->variables.sql_mode &= ~MODE_ONLY_FULL_GROUP_BY;
  Query_block *query_block =
      ParseAndResolve("SELECT DISTINCT t1.y FROM t1 ORDER BY t1.x, t1.y",
                      /*nullable=*/true);
  m_initializer.thd()->variables.sql_mode |= MODE_ONLY_FULL_GROUP_BY;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::SORT, root->type);
  Filesort *sort = root->sort().filesort;
  ASSERT_EQ(2, sort->sort_order_length());
  EXPECT_EQ("t1.x", ItemToString(sort->sortorder[0].item));
  EXPECT_EQ("t1.y", ItemToString(sort->sortorder[1].item));
  EXPECT_FALSE(sort->m_remove_duplicates);

  // We can't coalesce the two sorts, due to the deduplication in this step.
  AccessPath *child = root->sort().child;
  ASSERT_EQ(AccessPath::SORT, child->type);
  Filesort *sort2 = child->sort().filesort;
  ASSERT_EQ(1, sort2->sort_order_length());
  EXPECT_EQ("t1.y", ItemToString(sort2->sortorder[0].item));
  EXPECT_TRUE(sort2->m_remove_duplicates);

  EXPECT_EQ(AccessPath::TABLE_SCAN, child->sort().child->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, DistinctSubsumesOrderBy) {
  Query_block *query_block =
      ParseAndResolve("SELECT DISTINCT t1.y, t1.x FROM t1 ORDER BY t1.x",
                      /*nullable=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::SORT, root->type);
  Filesort *sort = root->sort().filesort;
  ASSERT_EQ(2, sort->sort_order_length());
  EXPECT_EQ("t1.x", ItemToString(sort->sortorder[0].item));
  EXPECT_EQ("t1.y", ItemToString(sort->sortorder[1].item));
  EXPECT_TRUE(sort->m_remove_duplicates);

  // No separate sort for ORDER BY.
  EXPECT_EQ(AccessPath::TABLE_SCAN, root->sort().child->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, SortAheadSingleTable) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x, t2.x FROM t1, t2 ORDER BY t2.x",
                      /*nullable=*/true);

  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 10000;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 100e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
  EXPECT_EQ(JoinType::INNER, root->nested_loop_join().join_type);

  // The sort should be on t2, which should be on the outer side.
  AccessPath *outer = root->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::SORT, outer->type);
  Filesort *sort = outer->sort().filesort;
  ASSERT_EQ(1, sort->sort_order_length());
  EXPECT_EQ("t2.x", ItemToString(sort->sortorder[0].item));
  EXPECT_FALSE(sort->m_remove_duplicates);

  AccessPath *outer_child = outer->sort().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer_child->type);
  EXPECT_STREQ("t2", outer_child->table_scan().table->alias);

  // The inner side should just be t1, no sort.
  AccessPath *inner = root->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner->type);
  EXPECT_STREQ("t1", inner->table_scan().table->alias);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, CannotSortAheadBeforeBothTablesAreAvailable) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x, t2.x FROM t1, t2 ORDER BY t1.x, t2.x",
                      /*nullable=*/true);

  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 10000;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 100e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The sort should be at the root, because the sort cannot be pushed
  // to e.g. t2 (unlike in the previous test); t1.x isn't available yet.
  ASSERT_EQ(AccessPath::SORT, root->type);

  // Check that there is no pushed sort in the tree.
  WalkAccessPaths(root->sort().child, /*join=*/nullptr,
                  WalkAccessPathPolicy::ENTIRE_TREE,
                  [&](const AccessPath *path, const JOIN *) {
                    EXPECT_NE(AccessPath::SORT, path->type);
                    return false;
                  });

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, SortAheadTwoTables) {
  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x, t2.x, t3.x FROM t1, t2, t3 ORDER BY t1.x, t2.x",
      /*nullable=*/true);

  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 100;
  m_fake_tables["t3"]->file->stats.records = 10000;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t3"]->file->stats.data_file_length = 100e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
  EXPECT_EQ(JoinType::INNER, root->nested_loop_join().join_type);

  // There should be a sort pushed down, with t1 and t2 below.
  AccessPath *outer = root->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::SORT, outer->type);
  Filesort *sort = outer->sort().filesort;
  ASSERT_EQ(2, sort->sort_order_length());
  EXPECT_EQ("t1.x", ItemToString(sort->sortorder[0].item));
  EXPECT_EQ("t2.x", ItemToString(sort->sortorder[1].item));
  EXPECT_FALSE(sort->m_remove_duplicates);

  // We don't check that t1 and t2 are actually below there
  // (and we don't care about the join type chosen, even though
  // it should usually be hash join), but we do check
  // that there are no more sorts.
  WalkAccessPaths(outer->sort().child, /*join=*/nullptr,
                  WalkAccessPathPolicy::ENTIRE_TREE,
                  [&](const AccessPath *path, const JOIN *) {
                    EXPECT_NE(AccessPath::SORT, path->type);
                    return false;
                  });

  // The inner side should just be t3, no sort.
  AccessPath *inner = root->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner->type);
  EXPECT_STREQ("t3", inner->table_scan().table->alias);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, NoSortAheadOnNondeterministicFunction) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x, t2.x FROM t1, t2 ORDER BY t1.x + RAND()",
                      /*nullable=*/true);

  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 100;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 1e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The sort should _not_ be pushed to t1, but kept at the top.
  // We don't care about the rest of the plan.
  ASSERT_EQ(AccessPath::SORT, root->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, SortAheadDueToEquivalence) {
  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x, t2.x FROM t1 JOIN t2 ON t1.x=t2.x ORDER BY t1.x, t2.x "
      "LIMIT 10",
      /*nullable=*/true);

  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 10000;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 100e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::LIMIT_OFFSET, root->type);
  EXPECT_EQ(10, root->limit_offset().limit);

  // There should be no sort at the limit; join directly.
  AccessPath *join = root->limit_offset().child;
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, join->type);

  // The outer side should have a sort, on t1 only.
  AccessPath *outer = join->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::SORT, outer->type);
  Filesort *sort = outer->sort().filesort;
  ASSERT_EQ(1, sort->sort_order_length());
  EXPECT_EQ("t1.x", ItemToString(sort->sortorder[0].item));
  EXPECT_FALSE(sort->m_remove_duplicates);

  // And it should indeed be t1 that is sorted, since it's the
  // smallest one.
  AccessPath *t1 = outer->sort().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, t1->type);
  EXPECT_STREQ("t1", t1->table_scan().table->alias);

  // The inner side should be t2, with the join condition as filter.
  AccessPath *inner = join->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::FILTER, inner->type);
  EXPECT_EQ("(t1.x = t2.x)", ItemToString(inner->filter().condition));

  AccessPath *t2 = inner->filter().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, t2->type);
  EXPECT_STREQ("t2", t2->table_scan().table->alias);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, SortAheadDueToUniqueIndex) {
  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x, t2.x FROM t1 JOIN t2 ON t1.x=t2.x "
      "ORDER BY t1.x, t2.x, t2.y LIMIT 10",
      /*nullable=*/true);

  // Create a unique index on t2.x. This means that t2.y is now
  // redundant, and can (will) be reduced away when creating the homogenized
  // order.
  m_fake_tables["t2"]->create_index(m_fake_tables["t2"]->field[0],
                                    /*column2=*/nullptr, /*unique=*/true);

  m_fake_tables["t1"]->file->stats.records = 200;
  m_fake_tables["t2"]->file->stats.records = 10000;
  m_fake_tables["t1"]->file->stats.data_file_length = 2e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 100e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::LIMIT_OFFSET, root->type);
  EXPECT_EQ(10, root->limit_offset().limit);

  // There should be no sort at the limit; join directly.
  AccessPath *join = root->limit_offset().child;
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, join->type);

  // The outer side should have a sort, on t1 only.
  AccessPath *outer = join->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::SORT, outer->type);
  Filesort *sort = outer->sort().filesort;
  ASSERT_EQ(1, sort->sort_order_length());
  EXPECT_EQ("t1.x", ItemToString(sort->sortorder[0].item));
  EXPECT_FALSE(sort->m_remove_duplicates);

  // And it should indeed be t1 that is sorted, since it's the
  // smallest one.
  AccessPath *t1 = outer->sort().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, t1->type);
  EXPECT_STREQ("t1", t1->table_scan().table->alias);

  // The inner side should be t2, with the join condition pushed down into an
  // EQ_REF.
  AccessPath *inner = join->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::EQ_REF, inner->type);
  EXPECT_STREQ("t2", inner->eq_ref().table->alias);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, NoSortAheadOnNonUniqueIndex) {
  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x, t2.x FROM t1 JOIN t2 ON t1.x=t2.x "
      "ORDER BY t1.x, t2.x, t2.y LIMIT 10",
      /*nullable=*/true);

  // With a non-unique index, there is no functional dependency,
  // and we should resort to sorting the largest table (t2).
  // The rest of the test is equal to SortAheadDueToUniqueIndex,
  // and we don't really verify it.
  m_fake_tables["t2"]->create_index(m_fake_tables["t2"]->field[0],
                                    /*column2=*/nullptr, /*unique=*/false);

  m_fake_tables["t1"]->file->stats.records = 200;
  m_fake_tables["t2"]->file->stats.records = 10000;
  m_fake_tables["t1"]->file->stats.data_file_length = 2e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 100e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  ASSERT_EQ(AccessPath::LIMIT_OFFSET, root->type);
  EXPECT_EQ(10, root->limit_offset().limit);

  AccessPath *join = root->limit_offset().child;
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, join->type);

  // The outer side should have a sort, on t2 only.
  AccessPath *outer = join->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::SORT, outer->type);
  Filesort *sort = outer->sort().filesort;
  ASSERT_EQ(2, sort->sort_order_length());
  EXPECT_EQ("t2.x", ItemToString(sort->sortorder[0].item));
  EXPECT_EQ("t2.y", ItemToString(sort->sortorder[1].item));
  EXPECT_FALSE(sort->m_remove_duplicates);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, ElideSortDueToBaseFilters) {
  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x, t1.y FROM t1 WHERE t1.x=3 ORDER BY t1.x, t1.y",
      /*nullable=*/true);

  m_fake_tables["t1"]->create_index(m_fake_tables["t1"]->field[0],
                                    /*column2=*/nullptr, /*unique=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The sort should be elided entirely due to the unique index
  // and the constant lookup.
  ASSERT_EQ(AccessPath::EQ_REF, root->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, ElideSortDueToDelayedFilters) {
  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x, t1.y FROM t1 LEFT JOIN t2 ON t1.y=t2.y WHERE t2.x IS NULL "
      "ORDER BY t2.x, t2.y ",
      /*nullable=*/true);

  m_fake_tables["t2"]->create_index(m_fake_tables["t2"]->field[0],
                                    /*column2=*/nullptr, /*unique=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.records = 10000;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t2"]->file->stats.data_file_length = 100e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // We should have the IS NULL at the root, and no sort, due to the
  // functional dependency from t2.x to t2.y.
  ASSERT_EQ(AccessPath::FILTER, root->type);
  EXPECT_EQ("(t2.x is null)", ItemToString(root->filter().condition));
  WalkAccessPaths(root->filter().child, /*join=*/nullptr,
                  WalkAccessPathPolicy::ENTIRE_TREE,
                  [&](const AccessPath *path, const JOIN *) {
                    EXPECT_NE(AccessPath::SORT, path->type);
                    return false;
                  });

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, ElideSortDueToIndex) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x FROM t1 ORDER BY t1.x DESC",
                      /*nullable=*/true);

  m_fake_tables["t1"]->create_index(m_fake_tables["t1"]->field[0],
                                    /*column2=*/nullptr, /*unique=*/false);
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;

  // Mark the index as returning ordered results.
  ON_CALL(*down_cast<Mock_HANDLER *>(m_fake_tables["t1"]->file),
          index_flags(_, _, _))
      .WillByDefault(Return(HA_READ_ORDER | HA_READ_NEXT | HA_READ_PREV));

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The sort should be elided entirely due to index.
  ASSERT_EQ(AccessPath::INDEX_SCAN, root->type);
  EXPECT_STREQ("t1", root->index_scan().table->alias);
  EXPECT_EQ(0, root->index_scan().idx);
  EXPECT_TRUE(root->index_scan().use_order);
  EXPECT_TRUE(root->index_scan().reverse);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, ElideConstSort) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x FROM t1 ORDER BY 'a', 'b', CONCAT('c')",
                      /*nullable=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The sort should be elided entirely.
  ASSERT_EQ(AccessPath::TABLE_SCAN, root->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

// This case is tricky; the order given by the index is (x, y), but the
// interesting order is just (y). Normally, we only grow orders into interesting
// orders, but here, we have to reduce them as well.
TEST_F(HypergraphOptimizerTest, IndexTailGetsUsed) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x, t1.y FROM t1 WHERE t1.x=42 ORDER BY t1.y",
                      /*nullable=*/true);

  m_fake_tables["t1"]->create_index(m_fake_tables["t1"]->field[0],
                                    m_fake_tables["t1"]->field[1],
                                    /*unique=*/false);
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;

  // Mark the index as returning ordered results.
  ON_CALL(*down_cast<Mock_HANDLER *>(m_fake_tables["t1"]->file),
          index_flags(_, _, _))
      .WillByDefault(Return(HA_READ_ORDER | HA_READ_NEXT | HA_READ_PREV));

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The sort should be elided entirely due to index.
  ASSERT_EQ(AccessPath::REF, root->type);
  EXPECT_STREQ("t1", root->ref().table->alias);
  EXPECT_EQ(0, root->ref().ref->key);
  EXPECT_EQ(true, root->ref().use_order);
  EXPECT_EQ(false, root->ref().reverse);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, SortAheadByCoverToElideSortForGroup) {
  Query_block *query_block = ParseAndResolve(
      "SELECT t1.x FROM t1, t2 GROUP BY t1.x, t1.y ORDER BY t1.y DESC",
      /*nullable=*/true);

  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;
  m_fake_tables["t2"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.data_file_length = 1e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The root should be a group, and it should _not_ have a sort beneath it
  // (it should be elided due to sortahead).
  ASSERT_EQ(AccessPath::AGGREGATE, root->type);
  AccessPath *join = root->aggregate().child;
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, join->type);
  AccessPath *outer = join->nested_loop_join().outer;

  // The outer table should be sorted on (y↓, x); it is compatible with the
  // grouping (even though it was on {x, y}), and also compatible with the
  // ordering.
  ASSERT_EQ(AccessPath::SORT, outer->type);
  Filesort *filesort = outer->sort().filesort;
  ASSERT_EQ(2, filesort->sort_order_length());
  EXPECT_EQ("t1.y", ItemToString(filesort->sortorder[0].item));
  EXPECT_TRUE(filesort->sortorder[0].reverse);
  EXPECT_EQ("t1.x", ItemToString(filesort->sortorder[1].item));
  EXPECT_FALSE(filesort->sortorder[1].reverse);

  // We don't test the inner side.

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, SatisfyGroupByWithIndex) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x FROM t1 GROUP BY t1.x",
                      /*nullable=*/true);

  m_fake_tables["t1"]->create_index(m_fake_tables["t1"]->field[0],
                                    /*column2=*/nullptr, /*unique=*/false);
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;

  // Mark the index as returning ordered results.
  ON_CALL(*down_cast<Mock_HANDLER *>(m_fake_tables["t1"]->file),
          index_flags(_, _, _))
      .WillByDefault(Return(HA_READ_ORDER | HA_READ_NEXT | HA_READ_PREV));

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The root is a group node, of course.
  ASSERT_EQ(AccessPath::AGGREGATE, root->type);
  AccessPath *inner = root->aggregate().child;

  // The grouping should be taking care of by the ordered index.
  EXPECT_EQ(AccessPath::INDEX_SCAN, inner->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, SatisfyGroupingForDistinctWithIndex) {
  Query_block *query_block =
      ParseAndResolve("SELECT DISTINCT t1.y, t1.x FROM t1",
                      /*nullable=*/true);

  m_fake_tables["t1"]->create_index(m_fake_tables["t1"]->field[0],
                                    m_fake_tables["t1"]->field[1],
                                    /*unique=*/false);
  m_fake_tables["t1"]->file->stats.records = 100;
  m_fake_tables["t1"]->file->stats.data_file_length = 1e6;

  // Mark the index as returning ordered results.
  ON_CALL(*down_cast<Mock_HANDLER *>(m_fake_tables["t1"]->file),
          index_flags(_, _, _))
      .WillByDefault(Return(HA_READ_ORDER | HA_READ_NEXT | HA_READ_PREV));

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The root should be a duplicate removal node; no sort.
  // Order of the group items doesn't matter.
  ASSERT_EQ(AccessPath::REMOVE_DUPLICATES, root->type);
  ASSERT_EQ(2, root->remove_duplicates().group_items_size);
  EXPECT_EQ("t1.y", ItemToString(root->remove_duplicates().group_items[0]));
  EXPECT_EQ("t1.x", ItemToString(root->remove_duplicates().group_items[1]));

  // The grouping should be taking care of by the ordered index.
  AccessPath *inner = root->remove_duplicates().child;
  EXPECT_EQ(AccessPath::INDEX_SCAN, inner->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, SemiJoinThroughLooseScan) {
  Query_block *query_block =
      ParseAndResolve("SELECT 1 FROM t1 WHERE t1.x IN (SELECT t2.x FROM t2)",
                      /*nullable=*/true);

  // Make t1 large and with a relevant index, and t2 small
  // and with none. The best plan then will be to remove
  // duplicates from t2 and then do lookups into t1.
  m_fake_tables["t1"]->create_index(m_fake_tables["t1"]->field[0],
                                    /*column2=*/nullptr,
                                    /*unique=*/true);
  m_fake_tables["t1"]->file->stats.records = 1000000;
  m_fake_tables["t1"]->file->stats.data_file_length = 10000e6;
  m_fake_tables["t2"]->file->stats.records = 100;
  m_fake_tables["t2"]->file->stats.data_file_length = 1e6;

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // The join should be changed to an _inner_ join, and the inner side
  // should be an EQ_REF on t1.
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
  EXPECT_EQ(JoinType::INNER, root->nested_loop_join().join_type);

  AccessPath *inner = root->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::EQ_REF, inner->type);
  EXPECT_STREQ("t1", inner->eq_ref().table->alias);

  // The outer side is slightly trickier. There should first be
  // a duplicate removal on the join key...
  AccessPath *outer = root->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::REMOVE_DUPLICATES, outer->type);
  ASSERT_EQ(1, outer->remove_duplicates().group_items_size);
  EXPECT_EQ("t2.x", ItemToString(outer->remove_duplicates().group_items[0]));

  // ...then a sort to get the grouping...
  AccessPath *sort = outer->remove_duplicates().child;
  ASSERT_EQ(AccessPath::SORT, sort->type);
  Filesort *filesort = sort->sort().filesort;
  ASSERT_EQ(1, filesort->sort_order_length());
  EXPECT_EQ("t2.x", ItemToString(filesort->sortorder[0].item));

  // Note that ideally, we'd have true here instead of the duplicate removal,
  // but we can't track duplicates-removed status through AccessPaths yet.
  EXPECT_FALSE(filesort->m_remove_duplicates);

  // ...and then finally a table scan.
  AccessPath *t2 = sort->sort().child;
  ASSERT_EQ(AccessPath::TABLE_SCAN, t2->type);
  EXPECT_STREQ("t2", t2->table_scan().table->alias);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphOptimizerTest, ImpossibleJoinConditionGivesZeroRows) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 LEFT JOIN (t2 JOIN t3 ON t2.x=t3.x AND 1=2) ON "
      "t1.x=t2.x",
      /*nullable=*/false);

  // We don't need any statistics; the best plan is quite obvious.
  // But we'd like to confirm the estimated row count for the join.
  m_fake_tables["t1"]->file->stats.records = 10;
  m_fake_tables["t2"]->file->stats.records = 1000;
  m_fake_tables["t3"]->file->stats.records = 1000;

  string trace;
  AccessPath *root = FindBestQueryPlan(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));

  // Since there are no rows on the right side, we should have a nested loop
  // with t1 on the left side.
  ASSERT_EQ(AccessPath::NESTED_LOOP_JOIN, root->type);
  EXPECT_EQ(JoinType::OUTER, root->nested_loop_join().join_type);
  EXPECT_FLOAT_EQ(10.0, root->num_output_rows);

  AccessPath *outer = root->nested_loop_join().outer;
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer->type);
  EXPECT_STREQ("t1", outer->table_scan().table->alias);

  // On the right side, we should have pushed _up_ the 1=2 condition,
  // and seen that it kills all the rows on the right side.
  AccessPath *inner = root->nested_loop_join().inner;
  ASSERT_EQ(AccessPath::ZERO_ROWS, inner->type);

  // Just verify that we indeed have a join under there.
  // (It is needed to get the zero row flags set on t2 and t3.)
  EXPECT_EQ(AccessPath::NESTED_LOOP_JOIN, inner->zero_rows().child->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

// An alias for better naming.
using HypergraphSecondaryEngineTest = HypergraphOptimizerTest;

TEST_F(HypergraphSecondaryEngineTest, SingleTable) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x FROM t1", /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;

  // Install a hook that doubles the row count estimate of t1.
  handlerton *hton = EnableSecondaryEngine(/*aggregation_is_unordered=*/false);
  hton->secondary_engine_modify_access_path_cost =
      [](THD *, const JoinHypergraph &, AccessPath *path) {
        EXPECT_EQ(AccessPath::TABLE_SCAN, path->type);
        EXPECT_STREQ("t1", path->table_scan().table->alias);
        path->num_output_rows = 200;
        return false;
      };

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  ASSERT_NE(nullptr, root);

  ASSERT_EQ(AccessPath::TABLE_SCAN, root->type);
  EXPECT_EQ(m_fake_tables["t1"], root->table_scan().table);
  EXPECT_FLOAT_EQ(200, root->num_output_rows);
}

TEST_F(HypergraphSecondaryEngineTest, SimpleInnerJoin) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x JOIN t3 ON t2.y=t3.y",
      /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 10000;
  m_fake_tables["t2"]->file->stats.records = 100;
  m_fake_tables["t3"]->file->stats.records = 1000000;

  // Install a hook that changes the row count estimate for t3 to 1.
  handlerton *hton = EnableSecondaryEngine(/*aggregation_is_unordered=*/false);
  hton->secondary_engine_modify_access_path_cost =
      [](THD *, const JoinHypergraph &, AccessPath *path) {
        // Nested-loop joins have been disabled for the secondary engine.
        EXPECT_NE(AccessPath::NESTED_LOOP_JOIN, path->type);
        if (path->type == AccessPath::TABLE_SCAN &&
            string(path->table_scan().table->alias) == "t3") {
          path->num_output_rows = 1;
        }
        return false;
      };

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  ASSERT_NE(nullptr, root);

  // Expect the biggest table to be the outer one. The table statistics tell
  // that this is t3, but the secondary engine cost hook changes the estimate
  // for t3 so that t1 becomes the biggest one.
  ASSERT_EQ(AccessPath::HASH_JOIN, root->type);
  ASSERT_EQ(AccessPath::TABLE_SCAN, root->hash_join().outer->type);
  EXPECT_STREQ("t1", root->hash_join().outer->table_scan().table->alias);
}

TEST_F(HypergraphSecondaryEngineTest, OrderedAggregation) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x FROM t1 GROUP BY t1.x", /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;

  EnableSecondaryEngine(/*aggregation_is_unordered=*/false);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  ASSERT_NE(nullptr, root);

  ASSERT_EQ(AccessPath::AGGREGATE, root->type);
  ASSERT_EQ(AccessPath::SORT, root->aggregate().child->type);
}

TEST_F(HypergraphSecondaryEngineTest, UnorderedAggregation) {
  Query_block *query_block =
      ParseAndResolve("SELECT t1.x FROM t1 GROUP BY t1.x", /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;

  EnableSecondaryEngine(/*aggregation_is_unordered=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  ASSERT_NE(nullptr, root);

  ASSERT_EQ(AccessPath::AGGREGATE, root->type);
  ASSERT_EQ(AccessPath::TABLE_SCAN, root->aggregate().child->type);
}

TEST_F(HypergraphSecondaryEngineTest,
       OrderedAggregationCoversDistinctWithOrder) {
  Query_block *query_block =
      ParseAndResolve("SELECT DISTINCT t1.x, t1.y FROM t1 ORDER BY t1.y",
                      /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;

  EnableSecondaryEngine(/*aggregation_is_unordered=*/false);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));
  ASSERT_NE(nullptr, root);

  ASSERT_EQ(AccessPath::SORT, root->type);
  Filesort *sort = root->sort().filesort;
  ASSERT_EQ(2, sort->sort_order_length());
  EXPECT_EQ("t1.y", ItemToString(sort->sortorder[0].item));
  EXPECT_EQ("t1.x", ItemToString(sort->sortorder[1].item));
  EXPECT_TRUE(sort->m_remove_duplicates);

  ASSERT_EQ(AccessPath::TABLE_SCAN, root->sort().child->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphSecondaryEngineTest, UnorderedAggregationDoesNotCover) {
  Query_block *query_block =
      ParseAndResolve("SELECT DISTINCT t1.x, t1.y FROM t1 ORDER BY t1.y",
                      /*nullable=*/true);
  m_fake_tables["t1"]->file->stats.records = 100;

  EnableSecondaryEngine(/*aggregation_is_unordered=*/true);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  // Prints out the query plan on failure.
  SCOPED_TRACE(PrintQueryPlan(0, root, query_block->join,
                              /*is_root_of_join=*/true));
  ASSERT_NE(nullptr, root);
  ASSERT_NE(nullptr, root);

  // The final sort is just a regular sort, no duplicate removal.
  ASSERT_EQ(AccessPath::SORT, root->type);
  Filesort *sort = root->sort().filesort;
  ASSERT_EQ(1, sort->sort_order_length());
  EXPECT_EQ("t1.y", ItemToString(sort->sortorder[0].item));
  EXPECT_FALSE(sort->m_remove_duplicates);

  // Below that, there's a duplicate-removing sort for DISTINCT.
  // Order does not matter, but it happens to choose the cover here.
  AccessPath *distinct = root->sort().child;
  ASSERT_EQ(AccessPath::SORT, distinct->type);
  sort = distinct->sort().filesort;
  ASSERT_EQ(2, sort->sort_order_length());
  EXPECT_EQ("t1.y", ItemToString(sort->sortorder[0].item));
  EXPECT_EQ("t1.x", ItemToString(sort->sortorder[1].item));
  EXPECT_TRUE(sort->m_remove_duplicates);

  ASSERT_EQ(AccessPath::TABLE_SCAN, distinct->sort().child->type);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_F(HypergraphSecondaryEngineTest, RejectAllPlans) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x JOIN t3 ON t2.y=t3.y",
      /*nullable=*/true);

  handlerton *hton = EnableSecondaryEngine(/*aggregation_is_unordered=*/false);
  hton->secondary_engine_modify_access_path_cost =
      [](THD *, const JoinHypergraph &, AccessPath *path) {
        // Nested-loop joins have been disabled for the secondary engine.
        EXPECT_NE(AccessPath::NESTED_LOOP_JOIN, path->type);
        // Reject all plans.
        return true;
      };

  // No plans will be found, so expect an error.
  ErrorChecker error_checker{m_thd, ER_SECONDARY_ENGINE};

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  EXPECT_EQ(nullptr, root);
}

TEST_F(HypergraphSecondaryEngineTest, RejectAllCompletePlans) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x JOIN t3 ON t2.y=t3.y",
      /*nullable=*/true);

  handlerton *hton = EnableSecondaryEngine(/*aggregation_is_unordered=*/false);
  hton->secondary_engine_modify_access_path_cost =
      [](THD *, const JoinHypergraph &, AccessPath *path) {
        // Reject the path if all three tables are referenced.
        return GetUsedTableMap(path, /*include_pruned_tables=*/true) == 0b111;
      };

  // No plans will be found, so expect an error.
  ErrorChecker error_checker{m_thd, ER_SECONDARY_ENGINE};

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  EXPECT_EQ(nullptr, root);
}

TEST_F(HypergraphSecondaryEngineTest, RejectJoinOrders) {
  Query_block *query_block = ParseAndResolve(
      "SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x JOIN t3 ON t2.y=t3.y",
      /*nullable=*/true);

  // Install a hook that only accepts hash joins where the outer table is a
  // table scan and the inner table is a table scan or another hash join, and
  // which only accepts join orders where the tables are ordered alphabetically
  // by their names.
  handlerton *hton = EnableSecondaryEngine(/*aggregation_is_unordered=*/false);
  hton->secondary_engine_modify_access_path_cost =
      [](THD *, const JoinHypergraph &, AccessPath *path) {
        // Nested-loop joins have been disabled for the secondary engine.
        EXPECT_NE(AccessPath::NESTED_LOOP_JOIN, path->type);
        if (path->type == AccessPath::HASH_JOIN) {
          if (path->hash_join().outer->type != AccessPath::TABLE_SCAN) {
            return true;
          }
          string outer = path->hash_join().outer->table_scan().table->alias;
          string inner;
          if (path->hash_join().inner->type == AccessPath::TABLE_SCAN) {
            inner = path->hash_join().inner->table_scan().table->alias;
          } else {
            EXPECT_EQ(AccessPath::HASH_JOIN, path->hash_join().inner->type);
            EXPECT_EQ(AccessPath::TABLE_SCAN,
                      path->hash_join().inner->hash_join().inner->type);
            inner = path->hash_join()
                        .inner->hash_join()
                        .inner->table_scan()
                        .table->alias;
          }
          // Reject plans where the join order is not alphabetical.
          return outer > inner;
        }
        return false;
      };

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  ASSERT_NE(nullptr, root);

  /*
    Expect the plan to have the following structure, because of the cost hook:

       HJ
      /  \
     t1  HJ
        /  \
       t2  t3
   */

  ASSERT_EQ(AccessPath::HASH_JOIN, root->type);
  const auto &outer_hash = root->hash_join();
  ASSERT_EQ(AccessPath::TABLE_SCAN, outer_hash.outer->type);
  ASSERT_EQ(AccessPath::HASH_JOIN, outer_hash.inner->type);
  const auto &inner_hash = outer_hash.inner->hash_join();
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner_hash.inner->type);
  ASSERT_EQ(AccessPath::TABLE_SCAN, inner_hash.outer->type);

  EXPECT_STREQ("t1", outer_hash.outer->table_scan().table->alias);
  EXPECT_STREQ("t2", inner_hash.outer->table_scan().table->alias);
  EXPECT_STREQ("t3", inner_hash.inner->table_scan().table->alias);
}

namespace {
struct RejectionParam {
  // The query to test.
  string query;
  // Path type to reject in the secondary engine cost hook.
  AccessPath::Type rejected_type;
  // Whether or not to expect an error if the specified path type always gives
  // an error or is rejected.
  bool expect_error;
};

std::ostream &operator<<(std::ostream &os, const RejectionParam &param) {
  return os << param.query << '/' << param.rejected_type << '/'
            << param.expect_error;
}
}  // namespace

using HypergraphSecondaryEngineRejectionTest =
    HypergraphTestBase<::testing::TestWithParam<RejectionParam>>;

TEST_P(HypergraphSecondaryEngineRejectionTest, RejectPathType) {
  const RejectionParam &param = GetParam();
  Query_block *query_block = ParseAndResolve(param.query.data(),
                                             /*nullable=*/true);

  handlerton *hton = EnableSecondaryEngine(/*aggregation_is_unordered=*/false);
  hton->secondary_engine_modify_access_path_cost =
      [](THD *thd, const JoinHypergraph &, AccessPath *path) {
        EXPECT_FALSE(thd->is_error());
        return path->type == GetParam().rejected_type;
      };

  ErrorChecker error_checker(m_thd,
                             param.expect_error ? ER_SECONDARY_ENGINE : 0);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  EXPECT_EQ(param.expect_error, root == nullptr);

  query_block->cleanup(m_thd, /*full=*/true);
}

TEST_P(HypergraphSecondaryEngineRejectionTest, ErrorOnPathType) {
  const RejectionParam &param = GetParam();
  Query_block *query_block = ParseAndResolve(param.query.data(),
                                             /*nullable=*/true);

  handlerton *hton = EnableSecondaryEngine(/*aggregation_is_unordered=*/false);
  hton->secondary_engine_modify_access_path_cost =
      [](THD *thd, const JoinHypergraph &, AccessPath *path) {
        EXPECT_FALSE(thd->is_error());
        if (path->type == GetParam().rejected_type) {
          my_error(ER_SECONDARY_ENGINE_PLUGIN, MYF(0), "");
          return true;
        } else {
          return false;
        }
      };

  ErrorChecker error_checker(
      m_thd, param.expect_error ? ER_SECONDARY_ENGINE_PLUGIN : 0);

  string trace;
  AccessPath *root = FindBestQueryPlanAndFinalize(m_thd, query_block, &trace);
  SCOPED_TRACE(trace);  // Prints out the trace on failure.
  EXPECT_EQ(param.expect_error, root == nullptr);

  query_block->cleanup(m_thd, /*full=*/true);
}

INSTANTIATE_TEST_SUITE_P(
    ErrorCases, HypergraphSecondaryEngineRejectionTest,
    ::testing::ValuesIn(std::initializer_list<RejectionParam>({
        {"SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x", AccessPath::TABLE_SCAN, true},
        {"SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x", AccessPath::HASH_JOIN, true},
        {"SELECT 1 FROM t1 JOIN t2 ON t1.x=t2.x ORDER BY t1.x",
         AccessPath::SORT, true},
        {"SELECT DISTINCT t1.x FROM t1", AccessPath::SORT, true},
        {"SELECT t1.x FROM t1 GROUP BY t1.x HAVING COUNT(*) > 5",
         AccessPath::FILTER, true},
        {"SELECT t1.x FROM t1 GROUP BY t1.x HAVING COUNT(*) > 5 ORDER BY t1.x",
         AccessPath::FILTER, true},
    })));

INSTANTIATE_TEST_SUITE_P(
    SuccessCases, HypergraphSecondaryEngineRejectionTest,
    ::testing::ValuesIn(std::initializer_list<RejectionParam>(
        {{"SELECT 1 FROM t1 WHERE t1.x=1", AccessPath::HASH_JOIN, false},
         {"SELECT 1 FROM t1 WHERE t1.x=1", AccessPath::SORT, false},
         {"SELECT DISTINCT t1.y, t1.x, 3 FROM t1 GROUP BY t1.x, t1.y",
          AccessPath::SORT, false}})));

/*
  A hypergraph receiver that doesn't actually cost any plans;
  it only counts the number of possible plans that would be
  considered.
 */
struct CountingReceiver {
  CountingReceiver(const JoinHypergraph &graph, size_t num_relations)
      : m_graph(graph), m_num_subplans(new size_t[1llu << num_relations]) {
    std::fill(m_num_subplans.get(),
              m_num_subplans.get() + (1llu << num_relations), 0);
  }

  bool HasSeen(NodeMap subgraph) { return m_num_subplans[subgraph] != 0; }

  bool FoundSingleNode(int node_idx) {
    NodeMap map = TableBitmap(node_idx);
    ++m_num_subplans[map];
    return false;
  }

  bool FoundSubgraphPair(NodeMap left, NodeMap right, int edge_idx) {
    const JoinPredicate *edge = &m_graph.edges[edge_idx];
    if (!PassesConflictRules(left | right, edge->expr)) {
      return false;
    }
    size_t n = m_num_subplans[left] * m_num_subplans[right];
    if (OperatorIsCommutative(*edge->expr)) {
      m_num_subplans[left | right] += 2 * n;
    } else {
      m_num_subplans[left | right] += n;
    }
    return false;
  }

  size_t count(NodeMap map) const { return m_num_subplans[map]; }

  const JoinHypergraph &m_graph;
  std::unique_ptr<size_t[]> m_num_subplans;
};

RelationalExpression *CloneRelationalExpr(THD *thd,
                                          const RelationalExpression *expr) {
  RelationalExpression *new_expr =
      new (thd->mem_root) RelationalExpression(thd);
  new_expr->type = expr->type;
  new_expr->tables_in_subtree = expr->tables_in_subtree;
  if (new_expr->type == RelationalExpression::TABLE) {
    new_expr->table = expr->table;
  } else {
    new_expr->left = CloneRelationalExpr(thd, expr->left);
    new_expr->right = CloneRelationalExpr(thd, expr->right);
  }
  return new_expr;
}

// Generate all possible complete binary trees of (exactly) the given size,
// consisting only of inner joins, and with fake tables at the leaves.
vector<RelationalExpression *> GenerateAllCompleteBinaryTrees(
    THD *thd, size_t num_relations, size_t start_idx) {
  assert(num_relations != 0);

  vector<RelationalExpression *> ret;
  if (num_relations == 1) {
    TABLE *table =
        new (thd->mem_root) Fake_TABLE(/*num_columns=*/1, /*nullable=*/true);
    table->pos_in_table_list->set_tableno(start_idx);

    // For debugging only.
    char name[32];
    snprintf(name, sizeof(name), "t%zu", start_idx + 1);
    table->alias = sql_strdup(name);
    table->pos_in_table_list->alias = table->alias;

    RelationalExpression *expr = new (thd->mem_root) RelationalExpression(thd);
    expr->type = RelationalExpression::TABLE;
    expr->table = table->pos_in_table_list;
    expr->tables_in_subtree = table->pos_in_table_list->map();

    ret.push_back(expr);
    return ret;
  }

  for (size_t num_left = 1; num_left <= num_relations - 1; ++num_left) {
    size_t num_right = num_relations - num_left;
    vector<RelationalExpression *> left =
        GenerateAllCompleteBinaryTrees(thd, num_left, start_idx);
    vector<RelationalExpression *> right =
        GenerateAllCompleteBinaryTrees(thd, num_right, start_idx + num_left);

    // Generate all pairs of trees, cloning as we go.
    for (size_t i = 0; i < left.size(); ++i) {
      for (size_t j = 0; j < right.size(); ++j) {
        RelationalExpression *expr =
            new (thd->mem_root) RelationalExpression(thd);
        expr->type = RelationalExpression::INNER_JOIN;
        expr->left = CloneRelationalExpr(thd, left[i]);
        expr->right = CloneRelationalExpr(thd, right[j]);
        expr->tables_in_subtree =
            expr->left->tables_in_subtree | expr->right->tables_in_subtree;
        ret.push_back(expr);
      }
    }
  }
  return ret;
}

// For each join operation (starting from idx), try all join types
// and all possible simple, non-degenerate predicaes, calling func()
// for each combination.
template <class Func>
void TryAllPredicates(
    const vector<RelationalExpression *> &join_ops,
    const vector<Item_field *> &fields,
    const vector<RelationalExpression::Type> &join_types,
    unordered_map<RelationalExpression *, table_map> *generated_nulls,
    size_t idx, const Func &func) {
  if (idx == join_ops.size()) {
    func();
    return;
  }

  RelationalExpression *expr = join_ops[idx];
  for (RelationalExpression::Type join_type : join_types) {
    expr->type = join_type;

    // Check which tables are visible after this join
    // (you can't have a predicate pointing into the right side
    // of an antijoin).
    const table_map left_map = expr->left->tables_in_subtree;
    const table_map right_map = expr->right->tables_in_subtree;
    if (join_type == RelationalExpression::ANTIJOIN ||
        join_type == RelationalExpression::SEMIJOIN) {
      expr->tables_in_subtree = left_map;
    } else {
      expr->tables_in_subtree = left_map | right_map;
    }

    (*generated_nulls)[expr] =
        (*generated_nulls)[expr->left] | (*generated_nulls)[expr->right];
    if (join_type == RelationalExpression::LEFT_JOIN) {
      (*generated_nulls)[expr] |= right_map;
    } else if (join_type == RelationalExpression::FULL_OUTER_JOIN) {
      (*generated_nulls)[expr] |= left_map | right_map;
    }

    // Find all pairs of tables under this operation, and construct an equijoin
    // predicate for them.
    for (Item_field *field1 : fields) {
      if (!IsSubset(field1->used_tables(), left_map)) {
        continue;
      }
      if ((join_type == RelationalExpression::INNER_JOIN ||
           join_type == RelationalExpression::SEMIJOIN) &&
          IsSubset(field1->used_tables(), (*generated_nulls)[expr->left])) {
        // Should have be simplified away. (See test comment.)
        continue;
      }
      for (Item_field *field2 : fields) {
        if (!IsSubset(field2->used_tables(), right_map)) {
          continue;
        }
        if ((join_type == RelationalExpression::INNER_JOIN ||
             join_type == RelationalExpression::SEMIJOIN ||
             join_type == RelationalExpression::LEFT_JOIN ||
             join_type == RelationalExpression::ANTIJOIN) &&
            IsSubset(field2->used_tables(), (*generated_nulls)[expr->right])) {
          // Should have be simplified away. (See test comment.)
          continue;
        }

        Item_func_eq *pred = new Item_func_eq(field1, field2);
        pred->update_used_tables();
        pred->quick_fix_field();
        expr->equijoin_conditions[0] = pred;
        expr->conditions_used_tables =
            field1->used_tables() | field2->used_tables();

        TryAllPredicates(join_ops, fields, join_types, generated_nulls, idx + 1,
                         func);
      }
    }
  }
}

std::pair<size_t, size_t> CountTreesAndPlans(
    THD *thd, int num_relations,
    const std::vector<RelationalExpression::Type> &join_types) {
  size_t num_trees = 0, num_plans = 0;

  vector<RelationalExpression *> roots =
      GenerateAllCompleteBinaryTrees(thd, num_relations, /*start_idx=*/0);
  for (RelationalExpression *expr : roots) {
    vector<RelationalExpression *> join_ops;
    vector<Item_field *> fields;
    vector<TABLE *> tables;

    // Which tables can get NULL-complemented rows due to outer joins.
    // We use this to reject inner joins against them, on the basis
    // that they would be simplified away and thus don't count.
    unordered_map<RelationalExpression *, table_map> generated_nulls;

    // Collect lists of all ops, and create tables where needed.
    ForEachOperator(expr, [&join_ops, &fields, &generated_nulls,
                           &tables](RelationalExpression *op) {
      if (op->type == RelationalExpression::TABLE) {
        Item_field *field = new Item_field(op->table->table->field[0]);
        field->quick_fix_field();
        fields.push_back(field);
        op->tables_in_subtree = op->table->map();
        generated_nulls.emplace(op, 0);
        tables.push_back(op->table->table);
      } else {
        join_ops.push_back(op);
        op->equijoin_conditions.clear();
        op->equijoin_conditions.push_back(nullptr);
      }
    });

    TryAllPredicates(
        join_ops, fields, join_types, &generated_nulls, /*idx=*/0, [&] {
          JoinHypergraph graph(thd->mem_root, /*query_block=*/nullptr);
          for (RelationalExpression *op : join_ops) {
            op->conflict_rules.clear();
          }
          MakeJoinGraphFromRelationalExpression(thd, expr, /*trace=*/nullptr,
                                                &graph);
          CountingReceiver receiver(graph, num_relations);
          ASSERT_FALSE(EnumerateAllConnectedPartitions(graph.graph, &receiver));
          ++num_trees;
          num_plans += receiver.count(TablesBetween(0, num_relations));
        });

    // Clean up allocated memory.
    for (TABLE *table : tables) {
      destroy(table);
    }
  }

  return {num_trees, num_plans};
}

/*
  Reproduces tables 4 and 5 from [Moe13]; builds all possible complete
  binary trees, fills them with all possible join operators from a given
  set, adds a simple (non-degenerate) equality predicate for each,
  and counts the number of plans. By getting numbers that match exactly,
  we can say with a fairly high degree of certainty that we've managed to
  get all the associativity etc. tables correct.

  The paper makes a few unspoken assumptions that are worth noting:

  1. After an antijoin or semijoin, the right side “disappears” and
     can not be used for further join predicates. This is consistent
     with the typical EXISTS / NOT EXISTS formulation in SQL.
  2. Outer joins are assumed simplified away wherever possible, so
     queries like (a JOIN (b LEFT JOIN c ON ...) a.x=c.x) are discarded
     as meaningless -- since the join predicate would discard any NULLs
     generated for c, the LEFT JOIN could just as well be an inner join.
  3. All predicates are assumed to be NULL-rejecting.

  Together, these explain why we have e.g. 26 queries with n=3 and the
  small operator set, instead of 36 (which would be logical for two shapes
  of binary trees, three operators for the top node, three for the bottom node
  and two possible top join predicates) or even more (if including non-nullable
  outer join predicates).

  We don't match the number of empty and nonempty rule sets given, but ours
  are correct and the paper's have a bug that prevents some simplification
  (Moerkotte, personal communication).
 */
TEST(ConflictDetectorTest, CountPlansSmallOperatorSet) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();
  THD *thd = initializer.thd();
  current_thd = thd;

  vector<RelationalExpression::Type> join_types{
      RelationalExpression::INNER_JOIN, RelationalExpression::LEFT_JOIN,
      RelationalExpression::ANTIJOIN};
  EXPECT_THAT(CountTreesAndPlans(thd, 3, join_types), Pair(26, 88));
  EXPECT_THAT(CountTreesAndPlans(thd, 4, join_types), Pair(344, 4059));
  EXPECT_THAT(CountTreesAndPlans(thd, 5, join_types), Pair(5834, 301898));

  // This takes too long to run for a normal unit test run (~10s in optimized
  // mode).
  if (false) {
    EXPECT_THAT(CountTreesAndPlans(thd, 6, join_types), Pair(117604, 32175460));
    EXPECT_THAT(CountTreesAndPlans(thd, 7, join_types),
                Pair(2708892, 4598129499));
  }
  initializer.TearDown();
}

TEST(ConflictDetectorTest, CountPlansLargeOperatorSet) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();
  THD *thd = initializer.thd();
  current_thd = thd;

  vector<RelationalExpression::Type> join_types{
      RelationalExpression::INNER_JOIN, RelationalExpression::LEFT_JOIN,
      RelationalExpression::FULL_OUTER_JOIN, RelationalExpression::SEMIJOIN,
      RelationalExpression::ANTIJOIN};
  EXPECT_THAT(CountTreesAndPlans(thd, 3, join_types), Pair(62, 203));
  EXPECT_THAT(CountTreesAndPlans(thd, 4, join_types), Pair(1114, 11148));

  // These take too long to run for a normal unit test run (~80s in optimized
  // mode).
  if (false) {
    EXPECT_THAT(CountTreesAndPlans(thd, 5, join_types), Pair(25056, 934229));
    EXPECT_THAT(CountTreesAndPlans(thd, 6, join_types),
                Pair(661811, 108294798));
    EXPECT_THAT(CountTreesAndPlans(thd, 7, join_types),
                Pair(19846278, 16448441514));
  }
  initializer.TearDown();
}

class CSETest : public HypergraphTestBase<::testing::Test> {
 protected:
  string TestCSE(const string &expression);
};

string CSETest::TestCSE(const string &expression) {
  // Abuse ParseAndResolve() to get the expression parsed.
  Query_block *query_block = ParseAndResolve(
      ("SELECT 1 FROM t1, t2, t3, t4, t5 WHERE " + expression).c_str(),
      /*nullable=*/true);
  return ItemToString(
      CommonSubexpressionElimination(query_block->join->where_cond));
}

TEST_F(CSETest, NoopSimpleItem) {
  EXPECT_EQ(TestCSE("t1.x=t2.x"), "(t1.x = t2.x)");
}

TEST_F(CSETest, NoopANDNoOR) {
  EXPECT_EQ(TestCSE("t1.x=t2.x AND t2.x = t3.x"),
            "((t1.x = t2.x) and (t2.x = t3.x))");
}

TEST_F(CSETest, NoopORNoAND) {
  EXPECT_EQ(TestCSE("t1.x=t2.x OR t2.x = t3.x"),
            "((t1.x = t2.x) or (t2.x = t3.x))");
}

TEST_F(CSETest, NoopNoCommon) {
  EXPECT_EQ(TestCSE("t1.x=t2.x OR (t2.x = t3.x AND t3.x > 4)"),
            "((t1.x = t2.x) or ((t2.x = t3.x) and (t3.x > 4)))");
}

TEST_F(CSETest, BasicSplit) {
  EXPECT_EQ(TestCSE("(t1.x=t2.x AND t2.x > 3) OR (t1.x=t2.x AND t2.x < 0)"),
            "((t1.x = t2.x) and ((t2.x > 3) or (t2.x < 0)))");
}

TEST_F(CSETest, SplitFromRecursedORGroups) {
  EXPECT_EQ(TestCSE("(t1.x=0 AND t2.x>1) OR ((t1.x=0 AND t2.y>1) OR (t1.x=0 "
                    "AND t2.z>0))"),
            "((t1.x = 0) and ((t2.x > 1) or (t2.y > 1) or (t2.z > 0)))");
}

TEST_F(CSETest, SplitFromRecursedANDGroups) {
  EXPECT_EQ(TestCSE("(t2.x>1 AND (t2.y>1 AND (t1.x=0))) OR "
                    "(t3.x>1 AND (t3.y>1 AND (t1.x=0)))"),
            "((t1.x = 0) and "
            "(((t2.x > 1) and (t2.y > 1)) or ((t3.x > 1) and (t3.y > 1))))");
}

// Split out t1.x > 1 and t2.y < 2, ie., more than one element,
// and they are in different orders. There are multiple items left
// in the rightmost OR group, too.
TEST_F(CSETest, SplitOutMoreThanOneElement) {
  EXPECT_EQ(TestCSE("(t1.x > 1 AND t2.y < 2 AND t2.x > 3) OR ((t2.y < 2 AND "
                    "t1.x > 1 AND t2.x < 1 AND t2.z >= 4))"),
            "((t1.x > 1) and (t2.y < 2) and "
            "((t2.x > 3) or ((t2.x < 1) and (t2.z >= 4))))");
}

TEST_F(CSETest, ShortCircuit) {
  EXPECT_EQ(TestCSE("t1.x=t2.x OR (t1.x=t2.x AND t2.x < 0)"), "(t1.x = t2.x)");
}

TEST_F(CSETest, ShortCircuitWithMultipleElements) {
  EXPECT_EQ(TestCSE("(t1.x=0 AND t1.y=1) OR (t1.x=0 AND t1.y=1)"),
            "((t1.x = 0) and (t1.y = 1))");
}
