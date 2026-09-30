/* Copyright (c) 2018, 2026, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "gcs_base_test.h"

#include <cstring>
#include <string>
#include <vector>
#include "app_data.h"
#include "get_synode_app_data.h"
#include "pax_msg.h"
#include "xcom_base.h"
#include "xcom_cache.h"
#include "xcom_memory.h"
#include "xcom_transport.h"

#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/xcom/network/network_provider_manager.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/xcom/network/xcom_network_provider.h"

namespace xcom_base_unittest {

class XcomBase : public GcsBaseTest {
 protected:
  XcomBase() {
    ::init_cache();

    auto &net_manager = Network_provider_manager::getInstance();

    auto xcom_network_provider = std::make_shared<Xcom_network_provider>();
    net_manager.add_network_provider(xcom_network_provider);
  }
  ~XcomBase() { ::deinit_cache(); }
};

TEST_F(XcomBase, XcomSendClientAppDataUpgradeScenario) {
  app_data a;
  std::string address("127.0.0.1:12345");

  char const *names[]{address.c_str()};
  node_address *na = new_node_address(1, names);

  a.body.c_t = add_node_type;
  a.body.app_u_u.nodes.node_list_len = 1;
  a.body.app_u_u.nodes.node_list_val = na;

  int result = 0;
  result = are_we_allowed_to_upgrade_to_v6(&a);

  ASSERT_EQ(result, 1);
  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomSendClientAppDataUpgradeScenarioV6) {
  app_data a;
  std::string address("[::1]:12345");

  char const *names[]{address.c_str()};
  node_address *na = new_node_address(1, names);

  a.body.c_t = add_node_type;
  a.body.app_u_u.nodes.node_list_len = 1;
  a.body.app_u_u.nodes.node_list_val = na;

  int result = 0;
  result = are_we_allowed_to_upgrade_to_v6(&a);

  ASSERT_EQ(result, 0);

  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomSendClientAppDataUpgradeScenarioMalformed) {
  app_data a;
  std::string address("::1]:12345");

  char const *names[]{address.c_str()};
  node_address *na = new_node_address(1, names);

  a.body.c_t = add_node_type;
  a.body.app_u_u.nodes.node_list_len = 1;
  a.body.app_u_u.nodes.node_list_val = na;

  int result = 0;
  result = are_we_allowed_to_upgrade_to_v6(&a);

  ASSERT_EQ(result, 0);

  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomNewClientEligibleDowngradeScenario) {
  std::string address("127.0.0.1:12345");

  char const *names[]{address.c_str()};
  node_address *na = new_node_address(1, names);

  site_def *sd = new_site_def();
  init_site_def(1, na, sd);

  xcom_proto incoming = x_1_4;
  int result = is_new_node_eligible_for_ipv6(incoming, sd);

  ASSERT_EQ(result, 0);

  free_site_def(sd);
  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomNewClientEligibleDowngradeScenarioFail) {
  std::string address("[::1]:12345");

  char const *names[]{address.c_str()};
  node_address *na = new_node_address(1, names);

  site_def *sd = new_site_def();
  init_site_def(1, na, sd);

  xcom_proto incoming = x_1_4;
  int result = is_new_node_eligible_for_ipv6(incoming, sd);

  ASSERT_EQ(result, 1);

  free_site_def(sd);
  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomNewClientEligibleDowngradeScenarioNullSiteDef) {
  xcom_proto incoming = x_1_4;
  int result = is_new_node_eligible_for_ipv6(incoming, nullptr);

  ASSERT_EQ(result, 0);
}

TEST_F(XcomBase, XcomNewClientEligibleDowngradeScenarioVersionSame) {
  xcom_proto incoming = my_xcom_version;
  int result = is_new_node_eligible_for_ipv6(incoming, nullptr);

  ASSERT_EQ(result, 0);
}

TEST_F(XcomBase, GetSynodeAppDataNotCached) {
  synode_no synode;
  synode.group_id = 12345;
  synode.msgno = 0;
  synode.node = 0;

  synode_no_array synodes;
  synodes.synode_no_array_len = 1;
  synodes.synode_no_array_val = &synode;

  synode_app_data_array result;
  result.synode_app_data_array_len = 0;
  result.synode_app_data_array_val = nullptr;

  auto error_code = ::xcom_get_synode_app_data(&synodes, &result);
  ASSERT_EQ(error_code, XCOM_GET_SYNODE_APP_DATA_NOT_CACHED);
}

TEST_F(XcomBase, GetSynodeAppDataNotDecided) {
  synode_no synode;
  synode.group_id = 12345;
  synode.msgno = 0;
  synode.node = 0;

  synode_no_array synodes;
  synodes.synode_no_array_len = 1;
  synodes.synode_no_array_val = &synode;

  synode_app_data_array result;
  result.synode_app_data_array_len = 0;
  result.synode_app_data_array_val = nullptr;

  /* Add the synode to the cache, but it is undecided. */
  get_cache(synode);

  auto error_code = ::xcom_get_synode_app_data(&synodes, &result);
  ASSERT_EQ(error_code, XCOM_GET_SYNODE_APP_DATA_NOT_DECIDED);
}

TEST_F(XcomBase, GetSynodeAppDataSuccessful) {
  synode_no synode;
  synode.group_id = 12345;
  synode.msgno = 0;
  synode.node = 0;

  synode_no origin;
  origin.group_id = 12345;
  origin.msgno = 0;
  origin.node = 1;

  synode_no_array synodes;
  synodes.synode_no_array_len = 1;
  synodes.synode_no_array_val = &synode;

  synode_app_data_array result;
  result.synode_app_data_array_len = 0;
  result.synode_app_data_array_val = nullptr;

  /* Add the synode to the cache, and set it as decided. */
  char const *const payload = "Message in a bottle";
  app_data_ptr a = new_app_data();
  a->unique_id = origin;
  a->body.c_t = app_type;
  a->body.app_u_u.data.data_len = std::strlen(payload) + 1;
  a->body.app_u_u.data.data_val = const_cast<char *>(payload);

  pax_msg *p = pax_msg_new_0(synode);
  p->op = learn_op;
  p->a = a;
  p->refcnt = 1;

  pax_machine *paxos = get_cache(synode);
  paxos->learner.msg = p;

  auto error_code = ::xcom_get_synode_app_data(&synodes, &result);
  ASSERT_EQ(error_code, XCOM_GET_SYNODE_APP_DATA_OK);

  ASSERT_EQ(result.synode_app_data_array_len, 1);
  ASSERT_EQ(synode_eq(result.synode_app_data_array_val[0].synode, synode), 1);
  ASSERT_EQ(synode_eq(result.synode_app_data_array_val[0].origin, origin), 1);
  ASSERT_EQ(result.synode_app_data_array_val[0].data.data_len,
            p->a->body.app_u_u.data.data_len);
  ASSERT_EQ(std::strcmp(result.synode_app_data_array_val[0].data.data_val,
                        p->a->body.app_u_u.data.data_val),
            0);

  /* Cleanup */
  a->body.app_u_u.data.data_len = 0;
  a->body.app_u_u.data.data_val = nullptr;

  unchecked_replace_pax_msg(&paxos->learner.msg, nullptr);

  xdr_free(reinterpret_cast<xdrproc_t>(xdr_synode_app_data_array),
           reinterpret_cast<char *>(&result));
}

TEST_F(XcomBase, GetSynodeAppDataTooManySynodes) {
  /*
   Bypass protocol negotiation because we are not actually connected to
   anything.
  */
  connection_descriptor con;
  con.connected_ = CON_PROTO;
  con.x_proto = get_latest_common_proto();

  constexpr uint32_t group_id = 1;
  /* Unserializable message. Exceeds array size. */
  u_int constexpr nr_synodes = MAX_SYNODE_ARRAY + 1;

  synode_no_array synodes;
  synodes.synode_no_array_len = nr_synodes;
  synodes.synode_no_array_val =
      static_cast<synode_no *>(std::calloc(nr_synodes, sizeof(synode_no)));
  ASSERT_NE(synodes.synode_no_array_val, nullptr);

  synode_app_data_array reply;
  ASSERT_EQ(xcom_client_get_synode_app_data(&con, group_id, &synodes, &reply),
            0);

  std::free(synodes.synode_no_array_val);
}

/* Disable on Windows. The test outcome varies wildly on our test environment,
 * likely due to different configurations of the stack size. */
#if !defined(_WIN32)
TEST_F(XcomBase, ProposerBatchDeserialization) {
  pax_msg *p = nullptr;
  unchecked_replace_pax_msg(&p, pax_msg_new_0(null_synode));

  for (auto i = 0; i < MAX_BATCH_APP_DATA; i++) {
    app_data_ptr a = new_app_data();
    a->body.c_t = app_type;
    a->next = p->a;

    p->a = a;
  }

  char *buffer = nullptr;
  uint32_t buffer_len = 0;

  int const serialized =
      serialize_msg(p, get_latest_common_proto(), &buffer_len, &buffer);
  ASSERT_EQ(serialized, 1);

  unchecked_replace_pax_msg(&p, pax_msg_new_0(null_synode));

  int const deserialized =
      deserialize_msg(p, get_latest_common_proto(), buffer, buffer_len);
  ASSERT_EQ(deserialized, 1);

  std::free(p);
  std::free(buffer);
}
#endif  // !defined(_WIN32)

// clang-format off
/*
This test validates the fix of
Bug #28966455 APPLIER LOG MISSES A TRANSACTION IN GR.

Situation
---------
S0 is the leader, trying to get consensus on a transaction T for its own slot.
S1 is the follower, trying to take over the slot and get consensus on a no_op.

Legend
------
SX: Server X
PX: Proposer of SX
ALX: Acceptor/learner of SX
O: Event on the respective server
X->: Message sent from the server on X to the server on ->
###: Comment/observation
E: The part where P1 is deviating from the Paxos protocol

Diagram
-------
  S0           S1         S2
P0  AL0      P1  AL1      AL2
|    O       |    O        O  AL{0,1,2}.promise = (0,0)
|    |       |    |        |
############################  P0 starts trying consensus for T
|    |       |    |        |
O    |       |    |        |  P0.ballot = (0,0); P0.value = T
X--->|-------|--->|------->|  accept_op[ballot=(0,0),
|    |       |    |        |            value=T (P0.value)]
|    |       |    |        |
|    O       |    O        O  AL{0,1,2}.value = T
|<---X-------|----X--------X  ack_accept_op[ballot=(0,0)]
|    |       |    |        |
############################  P0 got majority of accepts for (0,0) T
|    |       |    |        |
############################  P1 starts trying consensus for no_op
|    |       |    |        |
|    |       O    |        |  P1.ballot = (1,1); P1.value = no_op
|    |<------X--->|------->|  prepare_op[ballot=(1,1)]
|    |       |    |        |
|    O       |    O        O  AL{0,1,2}.promise = (1,1)
|    X------>|<---X--------X  ack_prepare_op[ballot=(1,1),
|    |       |    |        |                 accepted={(0,0) T}]
|    |       |    |        |
############################  P1 got a majority of prepares for (1,1)
|    |       |    |        |
|    |       E    |        |  P1.value should be set to T here.
|    |       E    |        |  According to the Paxos protocol, if any
|    |       E    |        |  acceptor replies with a previously
|    |       E    |        |  accepted value, one must use it. But
|    |       E    |        |  handle_ack_prepare did not do it because
|    |       E    |        |  handle_ack_prepare has the following code:
|    |       E    |        |
|    |       E    |        |  if (gt_ballot(m->proposal,
|    |       E    |        |                p->proposer.msg->proposal))
|    |       E    |        |  {
|    |       E    |        |    replace_pax_msg(&p->proposer.msg, m);
|    |       E    |        |    ...
|    |       E    |        |  }
|    |       E    |        |
|    |       E    |        |  And p->proposer.msg->proposal was
|    |       E    |        |  initialized to (0,1) on P1, meaning that:
|    |       E    |        |
|    |       E    |        |  if (0,0) > (0,1): P1.value = T
|    |       E    |        |
|    |       E    |        |  Therefore, P1.value = no_op.
|    |       E    |        |  (see handle_ack_prepare)
|    |       |    |        |
|    |  ...--X--->|------->|  accept_op[ballot=(1,1),
|    |       |    |        |            value=no_op (P1.value)]
|    |       |    |        |
|    |       |    O        O  AL{1,2}.value = no_op
|    |       |<---X--------X  ack_accept_op[ballot=(1,1)]
|    |       |    |        |
############################  P1 got majority of accepts for (1,1) no_op
############################  Values accepted for P{0,1} don't agree
|    |       |    |        |
|    |  ...--X--->|------->|  tiny_learn_op[ballot=(1,1), no_op]
|    |       |    |        |
|    |       |    O        O  AL{1,2} learn no_op
|    |       |    O        O  Executor task of S{1,2} delivers no_op
|    |       |    |        |
X--->|--...  |    |        |  tiny_learn_op[ballot=(0,0)]
|    |       |    |        |
|    O       |    |        |  AL0 learns T
|    O       |    |        |  Executor task of S0 delivers T
|    |       |    |        |
############################  S0 delivered T, S{1,2} delivered no_op
|    |       |    |        |
*/
// clang-format on
TEST_F(XcomBase, PaxosLearnSameValue) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // pax_machine for each "server."
  auto *s0_paxos =
      static_cast<pax_machine *>(std::calloc(1, sizeof(pax_machine)));
  init_pax_machine(s0_paxos, nullptr, synod);

  auto *s1_paxos =
      static_cast<pax_machine *>(std::calloc(1, sizeof(pax_machine)));
  init_pax_machine(s1_paxos, nullptr, synod);

  auto *s2_paxos =
      static_cast<pax_machine *>(std::calloc(1, sizeof(pax_machine)));
  init_pax_machine(s2_paxos, nullptr, synod);

  // site_def for each "server."
  auto *s0_config = new_site_def();
  s0_config->nodeno = 0;
  s0_config->nodes.node_list_len = 3;
  s0_config->global_node_set.node_set_len = 3;

  auto *s1_config = new_site_def();
  s1_config->nodeno = 1;
  s1_config->nodes.node_list_len = 3;
  s1_config->global_node_set.node_set_len = 3;

  auto *s2_config = new_site_def();
  s2_config->nodeno = 2;
  s2_config->nodes.node_list_len = 3;
  s2_config->global_node_set.node_set_len = 3;

  // clang-format off
  /****************************************************************************
     S0           S1         S2
   P0  AL0      P1  AL1      AL2
   |    |       |    |        |
   O    |       |    |        |  P0.ballot = (0,0); P0.value = T
   X--->|-------|--->|------->|  accept_op[ballot=(0,0),
   |    |       |    |        |            value=T (P0.value)]
   |    |       |    |        |
   |    O       |    O        O  AL{0,1,2}.value = T
   |<---X-------|----X--------X  ack_accept_op[ballot=(0,0)]
   |    |       |    |        |
   ****************************************************************************/
  // clang-format on
  pax_msg *tx = pax_msg_new(synod, nullptr);
  tx->a = new_app_data();
  tx->a->body.c_t = app_type;
  tx->a->body.app_u_u.data.data_len = 1;
  tx->a->body.app_u_u.data.data_val = static_cast<char *>(malloc(sizeof(char)));
  replace_pax_msg(&s0_paxos->proposer.msg, tx);
  prepare_push_2p(s0_config, s0_paxos);
  pax_msg *s0_accept_tx = s0_paxos->proposer.msg;
  init_propose_msg(s0_accept_tx);
  s0_accept_tx->from = 0;

  ballot ballot_tx;
  ballot_tx.cnt = 0;
  ballot_tx.node = 0;
  ASSERT_TRUE(eq_ballot(s0_paxos->proposer.msg->proposal, ballot_tx));

  // S0 sends s0_accept_tx to AL{0,1,2}

  // AL{0,1,2} receive s0_accept_tx

  pax_msg *s0_accept_tx_s0 = clone_pax_msg(s0_accept_tx);
  pax_msg *s0_ack_accept_tx =
      handle_simple_accept(s0_paxos, s0_accept_tx_s0, s0_accept_tx_s0->synode);
  ASSERT_NE(nullptr, s0_ack_accept_tx);
  s0_ack_accept_tx->from = 0;

  pax_msg *s0_accept_tx_s1 = clone_pax_msg(s0_accept_tx);
  pax_msg *s1_ack_accept_tx =
      handle_simple_accept(s1_paxos, s0_accept_tx_s1, s0_accept_tx_s1->synode);
  ASSERT_NE(nullptr, s1_ack_accept_tx);
  s1_ack_accept_tx->from = 1;

  pax_msg *s0_accept_tx_s2 = clone_pax_msg(s0_accept_tx);
  pax_msg *s2_ack_accept_tx =
      handle_simple_accept(s2_paxos, s0_accept_tx_s2, s0_accept_tx_s2->synode);
  ASSERT_NE(nullptr, s2_ack_accept_tx);
  s2_ack_accept_tx->from = 2;

  // AL{0,1,2} send s{0,1,2}_ack_accept_tx to P0

  // P0 receives s{0,1,2}_ack_accept_tx

  pax_msg *s0_ack_accept_tx_s0 = clone_pax_msg(s0_ack_accept_tx);
  ASSERT_EQ(nullptr,
            handle_simple_ack_accept(s0_config, s0_paxos, s0_ack_accept_tx_s0));

  pax_msg *s1_ack_accept_tx_s0 = clone_pax_msg(s1_ack_accept_tx);
  pax_msg *s0_learn_tx =
      handle_simple_ack_accept(s0_config, s0_paxos, s1_ack_accept_tx_s0);
  ASSERT_NE(nullptr, s0_learn_tx);
  ASSERT_EQ(tiny_learn_op, s0_learn_tx->op);
  s0_learn_tx->from = 0;

  pax_msg *s2_ack_accept_tx_s0 = clone_pax_msg(s2_ack_accept_tx);
  ASSERT_EQ(nullptr,
            handle_simple_ack_accept(s0_config, s0_paxos, s2_ack_accept_tx_s0));

  // clang-format off
  /****************************************************************************
     S0           S1         S2
   P0  AL0      P1  AL1      AL2
   |    |       |    |        |
   |    |       O    |        |  P1.ballot = (1,1); P1.value = no_op
   |    |<------X--->|------->|  prepare_op[ballot=(1,1)]
   |    |       |    |        |
   |    |       |    |        |
   |    O       |    O        O  AL{0,1,2}.promise = (1,1)
   |    X------>|<---X--------X  ack_prepare_op[ballot=(1,1),
   |    |       |    |        |                 accepted={(0,0) T}]
   |    |       |    |        |
   ****************************************************************************/
  // clang-format on
  replace_pax_msg(&s1_paxos->proposer.msg, pax_msg_new(synod, s1_config));
  create_noop(s1_paxos->proposer.msg);
  pax_msg *s1_prepare_noop = clone_pax_msg(s1_paxos->proposer.msg);
  prepare_push_3p(s1_config, s1_paxos, s1_prepare_noop, synod, no_op);
  init_prepare_msg(s1_prepare_noop);
  s1_prepare_noop->from = 1;

  ballot ballot_noop;
  ballot_noop.cnt = 1;
  ballot_noop.node = 1;
  ASSERT_TRUE(eq_ballot(s1_prepare_noop->proposal, ballot_noop));

  // P1 sends s1_prepare_noop to AL{0,1,2}

  // AL{0,1,2} receive s1_prepare_noop

  pax_msg *s1_prepare_noop_s0 = clone_pax_msg(s1_prepare_noop);
  pax_msg *s0_ack_prepare_noop =
      handle_simple_prepare(s0_paxos, s1_prepare_noop_s0, synod);
  ASSERT_NE(nullptr, s0_ack_prepare_noop);
  ASSERT_EQ(ack_prepare_op, s0_ack_prepare_noop->op);
  ASSERT_TRUE(eq_ballot(s0_ack_prepare_noop->proposal, ballot_tx));
  ASSERT_EQ(normal, s0_ack_prepare_noop->msg_type);
  s0_ack_prepare_noop->from = 0;

  pax_msg *s1_prepare_noop_s1 = clone_pax_msg(s1_prepare_noop);
  pax_msg *s1_ack_prepare_noop =
      handle_simple_prepare(s1_paxos, s1_prepare_noop_s1, synod);
  ASSERT_NE(nullptr, s1_ack_prepare_noop);
  ASSERT_EQ(ack_prepare_op, s1_ack_prepare_noop->op);
  ASSERT_TRUE(eq_ballot(s1_ack_prepare_noop->proposal, ballot_tx));
  ASSERT_EQ(normal, s1_ack_prepare_noop->msg_type);
  s1_ack_prepare_noop->from = 1;

  pax_msg *s1_prepare_noop_s2 = clone_pax_msg(s1_prepare_noop);
  pax_msg *s2_ack_prepare_noop =
      handle_simple_prepare(s2_paxos, s1_prepare_noop_s2, synod);
  ASSERT_NE(nullptr, s2_ack_prepare_noop);
  ASSERT_EQ(ack_prepare_op, s2_ack_prepare_noop->op);
  ASSERT_TRUE(eq_ballot(s2_ack_prepare_noop->proposal, ballot_tx));
  ASSERT_EQ(normal, s2_ack_prepare_noop->msg_type);
  s2_ack_prepare_noop->from = 2;

  // AL{0,1,2} send s{0,1,2}_ack_prepare_noop to P1

  // P1 receives s{0,1,2}_ack_prepare_noop

  pax_msg *s0_ack_prepare_noop_s1 = clone_pax_msg(s0_ack_prepare_noop);
  ASSERT_FALSE(
      handle_simple_ack_prepare(s1_config, s1_paxos, s0_ack_prepare_noop_s1));

  pax_msg *s1_ack_prepare_noop_s1 = clone_pax_msg(s1_ack_prepare_noop);
  bool can_send_accept =
      handle_simple_ack_prepare(s1_config, s1_paxos, s1_ack_prepare_noop_s1);
  ASSERT_TRUE(can_send_accept);
  pax_msg *s1_accept_noop = s1_paxos->proposer.msg;
  ASSERT_NE(nullptr, s1_accept_noop);
  ASSERT_EQ(accept_op, s1_accept_noop->op);
  s1_accept_noop->from = 1;

  pax_msg *s2_ack_prepare_noop_s1 = clone_pax_msg(s2_ack_prepare_noop);
  ASSERT_FALSE(
      handle_simple_ack_prepare(s1_config, s1_paxos, s2_ack_prepare_noop_s1));

  // clang-format off
  /****************************************************************************
     S0           S1         S2
   P0  AL0      P1  AL1      AL2
   |    |       |    |        |
   |    |       E    |        |  P1.value should be set to T here.
   |    |       E    |        |  According to the Paxos protocol, if any
   |    |       E    |        |  acceptor replies with a previously
   |    |       E    |        |  accepted value, one must use it. But
   |    |       E    |        |  handle_ack_prepare will not do it because
   |    |       E    |        |  handle_ack_prepare has the following code:
   |    |       E    |        |
   |    |       E    |        |  if (gt_ballot(m->proposal,
   |    |       E    |        |                p->proposer.msg->proposal))
   |    |       E    |        |  {
   |    |       E    |        |    replace_pax_msg(&p->proposer.msg, m);
   |    |       E    |        |    ...
   |    |       E    |        |  }
   |    |       E    |        |
   |    |       E    |        |  However, p->proposer.msg->proposal is initialized
   |    |       E    |        |  to (0,1) on P1, meaning that:
   |    |       E    |        |
   |    |       E    |        |  if (0,0) > (0,1): P1.value = no_op
   |    |       E    |        |
   |    |       E    |        |  Therefore, P1.value = no_op.
   |    |       E    |        |  (see handle_ack_prepare)
   |    |       |    |        |
   |    |  ...--X--->|------->|  accept_op[ballot=(1,1),
   |    |       |    |        |            value=no_op (P1.value)]
   |    |       |    |        |
   |    |       |    O        O  AL{0,1,2}.value = no_op
   |    |       |<---X--------X  ack_accept_op[ballot=(1,1)]
   |    |       |    |        |
   |    |       |    |        |
   |    |  ...--X--->|------->|  tiny_learn_op[ballot=(1,1), no_op]
   |    |       |    |        |
   |    |       |    O        O  AL{1,2} learn no_op
   |    |       |    O        O  Executor task of S{1,2} delivers no_op
   |    |       |    |        |
   ****************************************************************************/
  // clang-format on
  /*
   Here was the problem. P1 should have inherited T from one of AL{0,1,2}. But
   it did not because, s1_paxos->proposer.msg->proposal was initialized to
   (0,1).
   This lead to gt_ballot(m->proposal, p->proposer.msg->proposal) being false:

     gt_ballot(m->proposal, p->proposer.msg->proposal) <=>
     gt_ballot((0,0), (0,1)) <=>
     false

   The assert below would fire on mysql-trunk before the fix for
   Bug #28966455 APPLIER LOG MISSES A TRANSACTION IN GR.
   */
  // assert(s1_paxos->proposer.msg->msg_type != no_op);

  // P1 sends s1_accept_noop to AL{1,2}
  // P1 alsos sends s1_accept_noop to AL0, but it gets delayed

  // AL{1,2} receive s1_accept_noop
  pax_msg *s1_ack_accept_noop =
      handle_simple_accept(s1_paxos, clone_pax_msg(s1_accept_noop), synod);
  s1_ack_accept_noop->from = 1;

  pax_msg *s2_ack_accept_noop =
      handle_simple_accept(s2_paxos, clone_pax_msg(s1_accept_noop), synod);
  s2_ack_accept_noop->from = 2;

  // AL{1,2} send s{1,2}_ack_accept_noop to P1

  // P1 receives s{1,2}_ack_accept_noop
  pax_msg *s1_ack_accept_noop_s1 = clone_pax_msg(s1_ack_accept_noop);
  ASSERT_EQ(nullptr, handle_simple_ack_accept(s1_config, s1_paxos,
                                              s1_ack_accept_noop_s1));

  pax_msg *s2_ack_accept_noop_s1 = clone_pax_msg(s2_ack_accept_noop);
  pax_msg *s1_learn_noop =
      handle_simple_ack_accept(s1_config, s1_paxos, s2_ack_accept_noop_s1);
  ASSERT_NE(nullptr, s1_learn_noop);
  ASSERT_EQ(tiny_learn_op, s1_learn_noop->op);
  s1_learn_noop->from = 1;

  // P1 sends s1_learn_noop to AL{1,2}
  // P1 alsos sends s1_learn_noop to AL0, but it gets delayed

  // AL{1,2} receive s1_learn_noop
  pax_msg *s1_learn_noop_s1 = clone_pax_msg(s1_learn_noop);
  handle_learn(s1_config, s1_paxos, s1_learn_noop_s1);

  pax_msg *s1_learn_noop_s2 = clone_pax_msg(s1_learn_noop);
  handle_learn(s2_config, s2_paxos, s1_learn_noop_s2);

  ASSERT_TRUE(pm_finished(s1_paxos));

  ASSERT_TRUE(pm_finished(s2_paxos));

  // S1 and S2 would deliver no_op... (but deliver tx after the fix)

  // clang-format off
  /****************************************************************************
     S0           S1         S2
   P0  AL0      P1  AL1      AL2
   |    |       |    |        |
   X--->|--...  |    |        |  tiny_learn_op[ballot=(0,0)]
   |    |       |    |        |
   |    O       |    |        |  AL0 learns T
   |    O       |    |        |  Executor task of S0 delivers T
   |    |       |    |        |
   ****************************************************************************/
  // clang-format on

  // P0 sends s0_learn_tx to AL0
  // P1 alsos sends s0_learn_tx to AL{1,2}, but it doesn't matter

  // AL0 receive s0_learn_tx
  pax_msg *s0_learn_tx_s0 = clone_pax_msg(s0_learn_tx);
  handle_tiny_learn(s0_config, s0_paxos, s0_learn_tx_s0);

  ASSERT_TRUE(pm_finished(s0_paxos));

  // ...and S0 delivers tx

  bool const every_executor_delivered_same_value =
      (s0_paxos->learner.msg->msg_type == s1_paxos->learner.msg->msg_type &&
       s1_paxos->learner.msg->msg_type == s2_paxos->learner.msg->msg_type);
  ASSERT_TRUE(every_executor_delivered_same_value);

  // Cleanup.
  auto free_pax_msg = [](pax_msg *p) {
    p->refcnt = 1;
    replace_pax_msg(&p, nullptr);
  };

  init_pax_machine(s0_paxos, nullptr, synod);
  std::free(s0_paxos->proposer.prep_nodeset->bits.bits_val);
  std::free(s0_paxos->proposer.prep_nodeset);
  std::free(s0_paxos->proposer.prop_nodeset->bits.bits_val);
  std::free(s0_paxos->proposer.prop_nodeset);
  std::free(s0_paxos);
  std::free(s0_config);
  free_pax_msg(s0_ack_accept_tx);
  free_pax_msg(s0_ack_accept_tx_s0);
  free_pax_msg(s0_learn_tx);
  free_pax_msg(s1_prepare_noop_s0);
  free_pax_msg(s0_ack_prepare_noop);
  free_pax_msg(s0_learn_tx_s0);

  init_pax_machine(s1_paxos, nullptr, synod);
  std::free(s1_paxos->proposer.prep_nodeset->bits.bits_val);
  std::free(s1_paxos->proposer.prep_nodeset);
  std::free(s1_paxos->proposer.prop_nodeset->bits.bits_val);
  std::free(s1_paxos->proposer.prop_nodeset);
  std::free(s1_paxos);
  std::free(s1_config);
  free_pax_msg(s1_ack_accept_tx);
  free_pax_msg(s1_ack_accept_tx_s0);
  free_pax_msg(s1_prepare_noop);
  free_pax_msg(s1_prepare_noop_s1);
  free_pax_msg(s1_ack_prepare_noop);
  free_pax_msg(s1_ack_prepare_noop_s1);
  free_pax_msg(s2_ack_prepare_noop_s1);
  free_pax_msg(s1_ack_accept_noop);
  free_pax_msg(s1_ack_accept_noop_s1);
  free_pax_msg(s2_ack_accept_noop_s1);
  free_pax_msg(s1_learn_noop);

  init_pax_machine(s2_paxos, nullptr, synod);
  std::free(s2_paxos->proposer.prep_nodeset->bits.bits_val);
  std::free(s2_paxos->proposer.prep_nodeset);
  std::free(s2_paxos->proposer.prop_nodeset->bits.bits_val);
  std::free(s2_paxos->proposer.prop_nodeset);
  std::free(s2_paxos);
  std::free(s2_config);
  free_pax_msg(s2_ack_accept_tx);
  free_pax_msg(s2_ack_accept_tx_s0);
  free_pax_msg(s1_prepare_noop_s2);
  free_pax_msg(s2_ack_prepare_noop);
  free_pax_msg(s2_ack_accept_noop);
}

TEST_F(XcomBase, HandleBootWithoutIdentity) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // Fake node information.
  char const *names[]{"127.0.0.1:10001"};
  blob uuid;
  uuid.data.data_len = 1;
  uuid.data.data_val = const_cast<char *>("1");
  blob uuids[] = {uuid};

  // site_def for the "server."
  auto *config = new_site_def();
  config->nodeno = 0;
  config->nodes.node_list_len = 1;
  config->nodes.node_list_val = ::new_node_address_uuid(1, names, uuids);

  pax_msg *need_boot = pax_msg_new(synod, nullptr);
  // need_boot_op without an identity.
  init_need_boot_op(need_boot, nullptr);
  ASSERT_TRUE(should_handle_need_boot(config, need_boot));

  // Cleanup.
  need_boot->refcnt = 1;
  replace_pax_msg(&need_boot, nullptr);

  ::delete_node_address(1, config->nodes.node_list_val);

  std::free(config);
}

TEST_F(XcomBase, HandleBootWithIdentityOfExistingMember) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // Fake node information.
  char const *names[]{"127.0.0.1:10001"};
  blob uuid;
  uuid.data.data_len = 1;
  uuid.data.data_val = const_cast<char *>("1");
  blob uuids[] = {uuid};

  // site_def for the "server."
  auto *config = new_site_def();
  config->nodeno = 0;
  config->nodes.node_list_len = 1;
  config->nodes.node_list_val = ::new_node_address_uuid(1, names, uuids);

  pax_msg *need_boot = pax_msg_new(synod, nullptr);
  // need_boot_op with an identity.
  node_address *identity = ::new_node_address_uuid(1, names, uuids);
  init_need_boot_op(need_boot, identity);
  ASSERT_TRUE(should_handle_need_boot(config, need_boot));

  // Cleanup.
  need_boot->refcnt = 1;
  replace_pax_msg(&need_boot, nullptr);

  ::delete_node_address(1, config->nodes.node_list_val);
  ::delete_node_address(1, identity);

  std::free(config);
}

TEST_F(XcomBase, HandleBootWithIdentityOfNonExistingMember) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // Fake node information.
  char const *names[]{"127.0.0.1:10001"};
  blob uuid;
  uuid.data.data_len = 1;
  uuid.data.data_val = const_cast<char *>("1");
  blob uuids[] = {uuid};

  // site_def for the "server."
  auto *config = new_site_def();
  config->nodeno = 0;
  config->nodes.node_list_len = 1;
  config->nodes.node_list_val = ::new_node_address_uuid(1, names, uuids);

  pax_msg *need_boot = pax_msg_new(synod, nullptr);
  // need_boot_op with an identity.
  blob unknown_uuid;
  unknown_uuid.data.data_len = 1;
  unknown_uuid.data.data_val = const_cast<char *>("2");
  blob unknown_uuids[] = {unknown_uuid};
  node_address *identity = ::new_node_address_uuid(1, names, unknown_uuids);
  init_need_boot_op(need_boot, identity);
  ASSERT_FALSE(should_handle_need_boot(config, need_boot));

  // Cleanup.
  need_boot->refcnt = 1;
  replace_pax_msg(&need_boot, nullptr);

  ::delete_node_address(1, config->nodes.node_list_val);
  ::delete_node_address(1, identity);

  std::free(config);
}

TEST_F(XcomBase, HandleBootWithMoreThanOneIdentity) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // Fake node information.
  char const *name{"127.0.0.1:10001"};
  char const *names[]{name};
  blob uuid;
  uuid.data.data_len = 1;
  uuid.data.data_val = const_cast<char *>("1");
  blob uuids[] = {uuid};

  // site_def for the "server."
  auto *config = new_site_def();
  config->nodeno = 0;
  config->nodes.node_list_len = 1;
  config->nodes.node_list_val = ::new_node_address_uuid(1, names, uuids);

  pax_msg *need_boot = pax_msg_new(synod, nullptr);
  // need_boot_op with two identities.
  char const *other_name{"127.0.0.1:10002"};
  char const *two_names[] = {name, other_name};
  blob two_uuids[] = {uuid, uuid};
  node_address *identity = ::new_node_address_uuid(2, two_names, two_uuids);
  need_boot->op = need_boot_op;
  if (identity != nullptr) {
    need_boot->a = new_app_data();
    need_boot->a->body.c_t = xcom_boot_type;
    init_node_list(2, identity, &need_boot->a->body.app_u_u.nodes);
  }
  ASSERT_FALSE(should_handle_need_boot(config, need_boot));

  // Cleanup.
  need_boot->refcnt = 1;
  replace_pax_msg(&need_boot, nullptr);

  ::delete_node_address(1, config->nodes.node_list_val);
  ::delete_node_address(2, identity);

  std::free(config);
}

/**
 * This test will check the logic implemented in pre_process_incoming_ping
 *
 * It will create all necessary support structures and:
 * - Call pre_process_incoming_ping 4 times
 * - On the first and second try it must:
 * -- increment the number of pings
 * -- Make sure that we do not shutdown the connection
 * - On the third attempt it must:
 * -- Have incremented the number of pings
 * -- Shutdown the connection
 * - On the fourth attempt
 * -- Have incremented the number of pings
 * -- Make sure that we do not shutdown the connection
 */
TEST_F(XcomBase, ProcessPingToUsFullSmokeTest) {
  site_def site;
  pax_msg pm;

  server srv_from;
  srv_from.last_ping_received = 0.0;
  srv_from.number_of_pings_received = 0;

  srv_from.con = new_connection(0, nullptr);
  srv_from.con->connected_ = CON_PROTO;

  char srv_addr[1024] = "test";
  srv_from.srv = &srv_addr[0];
  srv_from.port = 12345;

  site.nodeno = 1;
  site.global_node_set.node_set_len = 3;
  site.nodes.node_list_len = 3;
  site.servers[0] = &srv_from;

  pm.from = 0;
  pm.op = are_you_alive_op;

  bool has_disconnected = false;
  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 1.0);
  ASSERT_EQ(1, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 2.0);
  ASSERT_EQ(2, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 3.0);
  ASSERT_EQ(3, srv_from.number_of_pings_received);
  ASSERT_TRUE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 5.0);
  ASSERT_EQ(4, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  free(srv_from.con);
}

/**
 * This test will check the logic implemented in pre_process_incoming_ping when
 * the node has not booted
 *
 * It will create all necessary support structures and:
 * - Call pre_process_incoming_ping 4 times
 * - On every time it must:
 * -- NOT increment the number of pings
 * -- Make sure that we do NOT shutdown the connection
 */
TEST_F(XcomBase, ProcessPingToUsDoNothingIfNodeIsBooting) {
  site_def site;
  pax_msg pm;

  server srv_from;
  srv_from.last_ping_received = 0.0;
  srv_from.number_of_pings_received = 0;
  srv_from.con = new_connection(0, nullptr);
  srv_from.con->connected_ = CON_PROTO;

  char srv_addr[1024] = "test";
  srv_from.srv = &srv_addr[0];
  srv_from.port = 12345;

  site.nodeno = 1;
  site.global_node_set.node_set_len = 3;
  site.nodes.node_list_len = 3;
  site.servers[0] = &srv_from;

  pm.from = 0;
  pm.op = are_you_alive_op;

  bool has_disconnected = false;
  has_disconnected = pre_process_incoming_ping(&site, &pm, false, 1.0);
  ASSERT_EQ(0, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, false, 2.0);
  ASSERT_EQ(0, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, false, 3.0);
  ASSERT_EQ(0, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, false, 5.0);
  ASSERT_EQ(0, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  free(srv_from.con);
}

/**
 * This test will check the logic implemented in pre_process_incoming_ping
 * with an inactive connection
 *
 * It will create all necessary support structures and:
 * - Call pre_process_incoming_ping 4 times
 * - On the first and second try it must:
 * -- increment the number of pings
 * -- Make sure that we do not shutdown the connection
 * - On the third attempt it must:
 * -- Have incremented the number of pings
 * -- DO NOT Shutdown the connection
 * - On the fourth attempt
 * -- Have incremented the number of pings
 * -- Make sure that we do not shutdown the connection
 */
TEST_F(XcomBase, ProcessPingToUsDoNotShutdownInactiveConnection) {
  site_def site;
  pax_msg pm;

  server srv_from;
  srv_from.last_ping_received = 0.0;
  srv_from.number_of_pings_received = 0;
  srv_from.con = new_connection(-1, nullptr);
  srv_from.con->connected_ = CON_NULL;

  site.nodeno = 1;
  site.global_node_set.node_set_len = 3;
  site.nodes.node_list_len = 3;
  site.servers[0] = &srv_from;

  pm.from = 0;
  pm.op = are_you_alive_op;

  bool has_disconnected = false;
  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 1.0);
  ASSERT_EQ(1, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 2.0);
  ASSERT_EQ(2, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 3.0);
  ASSERT_EQ(3, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 5.0);
  ASSERT_EQ(4, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  free(srv_from.con);
}

/**
 * This test will check the logic implemented in pre_process_incoming_ping
 * making sure that we are able to reset the ping number value
 *
 * It will create all necessary support structures and:
 * - Call pre_process_incoming_ping 3 times
 * - On the first and second try it must:
 * -- increment the number of pings
 * -- Make sure that we do not shutdown the connection
 * - Wait for 6 seconds
 * - On the third attempt it must:
 * -- Have reset number of pings
 * -- DO NOT Shutdown the connection
 */
TEST_F(XcomBase, ProcessPingToUsDoNotShutdownResetPings) {
  site_def site;
  pax_msg pm;

  server srv_from;
  srv_from.last_ping_received = 0.0;
  srv_from.number_of_pings_received = 0;
  srv_from.con = new_connection(-1, nullptr);
  srv_from.con->connected_ = CON_NULL;

  site.nodeno = 1;
  site.global_node_set.node_set_len = 3;
  site.nodes.node_list_len = 3;
  site.servers[0] = &srv_from;

  pm.from = 0;
  pm.op = are_you_alive_op;

  bool has_disconnected = false;
  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 1.0);
  ASSERT_EQ(1, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 2.0);
  ASSERT_EQ(2, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm, true, 10.0);
  ASSERT_EQ(1, srv_from.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  free(srv_from.con);
}

/**
 * This test will check the logic implemented in pre_process_incoming_ping
 * and it will receive pings from 2 different servers.
 *
 * It will create all necessary support structures and:
 * - Call pre_process_incoming_ping 4 times using server 1
 * - On the first and second try using server 1 it must:
 * -- increment the number of pings
 * -- Make sure that we do not shutdown the connection
 * - Call pre_process_incoming_ping once using server 2
 * - On the first and second try it must:
 * -- increment the number of pings
 * -- Make sure that we do not shutdown the connection
 * - On the third attempt using server 1 it must:
 * -- Have incremented the number of pings
 * -- Shutdown the connection
 * - On the fourth attempt using server 1
 * -- Have incremented the number of pings
 * -- Make sure that we do not shutdown the connection
 */
TEST_F(XcomBase, ProcessPingToUsTwoServersSendingPings) {
  site_def site;
  pax_msg pm1, pm2;

  char srv_addr[1024] = "test";

  server srv_from1, srv_from2;
  srv_from1.last_ping_received = 0.0;
  srv_from1.number_of_pings_received = 0;
  srv_from1.con = new_connection(0, nullptr);
  srv_from1.con->connected_ = CON_PROTO;

  srv_from1.srv = &srv_addr[0];
  srv_from1.port = 12345;

  srv_from2.last_ping_received = 0.0;
  srv_from2.number_of_pings_received = 0;
  srv_from2.con = new_connection(0, nullptr);
  srv_from2.con->connected_ = CON_PROTO;

  srv_from2.srv = &srv_addr[0];
  srv_from2.port = 12346;

  site.nodeno = 1;
  site.global_node_set.node_set_len = 3;
  site.nodes.node_list_len = 3;
  site.servers[0] = &srv_from1;
  site.servers[2] = &srv_from2;

  pm1.from = 0;
  pm1.op = are_you_alive_op;

  pm2.from = 2;
  pm2.op = are_you_alive_op;

  bool has_disconnected = false;
  has_disconnected = pre_process_incoming_ping(&site, &pm1, true, 1.0);
  ASSERT_EQ(1, srv_from1.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm1, true, 2.0);
  ASSERT_EQ(2, srv_from1.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm2, true, 3.0);
  ASSERT_EQ(1, srv_from2.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm1, true, 4.0);
  ASSERT_EQ(3, srv_from1.number_of_pings_received);
  ASSERT_TRUE(has_disconnected);

  has_disconnected = pre_process_incoming_ping(&site, &pm1, true, 5.0);
  ASSERT_EQ(4, srv_from1.number_of_pings_received);
  ASSERT_FALSE(has_disconnected);

  free(srv_from1.con);
  free(srv_from2.con);
}

/*
  Bug#39234600 - Unauthenticated XCom client_msg injection executes arbitrary
  replicated SQL on Group Replication members.

  acceptor_learner_task() used to hand an externally received client_msg to the
  same dispatch path as locally submitted Group Replication input, so a peer
  able to reach the XCom port could submit app_type cargo that is applied as
  replicated SQL on every group member. The fix gates that path with an
  allowlist of cargo types acceptable from an external client.
*/

/* Build a single-element app_data list carrying the given cargo type. */
static app_data_ptr new_app_data_with_cargo(cargo_type cargo) {
  app_data_ptr a = new_app_data();
  a->body.c_t = cargo;
  return a;
}

/*
  XCom client/control and membership configuration requests, which must keep
  working over an external client connection.
*/
static cargo_type const allowed_external_client_cargo[] = {
    add_node_type,
    disable_arbitrator,
    enable_arbitrator,
    force_config_type,
    get_event_horizon_type,
    get_leaders_type,
    get_synode_app_data_type,
    remove_node_type,
    set_cache_limit,
    set_event_horizon_type,
    set_leaders_type,
    set_max_leaders,
    unified_boot_type};

/*
  Application delivery, transaction/view handling, reset and termination cargo,
  which must never be accepted from an external client. app_type is the vehicle
  for arbitrary replicated content and is the actual vulnerability.
*/
static cargo_type const disallowed_external_client_cargo[] = {
    abort_trans,       app_type,
    begin_trans,       convert_into_local_server_type,
    exit_type,         prepared_trans,
    remove_reset_type, reset_type,
    view_msg,          x_terminate_and_exit,
    xcom_boot_type,    xcom_set_group};

TEST_F(XcomBase, ExternalClientCargoAllowlistAcceptsControlCargo) {
  for (auto cargo : allowed_external_client_cargo) {
    ASSERT_TRUE(unittest_only_is_allowed_external_client_cargo_type(cargo))
        << "cargo type " << static_cast<int>(cargo)
        << " must remain allowed on an external client connection";
  }
}

TEST_F(XcomBase, ExternalClientCargoAllowlistRejectsDeliveryCargo) {
  for (auto cargo : disallowed_external_client_cargo) {
    ASSERT_FALSE(unittest_only_is_allowed_external_client_cargo_type(cargo))
        << "cargo type " << static_cast<int>(cargo)
        << " must be rejected on an external client connection";
  }
}

TEST_F(XcomBase, ExternalClientCargoAllowlistCoversEveryCargoType) {
  /*
    The two lists above must together account for every member of cargo_type.
    The production switch is exhaustive, so the compiler already rejects a
    *forgotten* cargo type - but nothing stops a newly added one being placed in
    the allowed arm by mistake, or this test drifting out of step with the enum.
    If cargo_type grows, this count must be updated deliberately, alongside a
    decision about which arm the new type belongs in.

    Note cargo_type is NOT contiguous: values 3, 5 and 6 were retired with
    xcom_recover, query_type and query_next_log (they still exist in 5.7), so
    iterating 0..get_leaders_type would visit non-members. Counting the lists is
    the reliable check.
  */
  size_t const n_allowed = sizeof(allowed_external_client_cargo) /
                           sizeof(allowed_external_client_cargo[0]);
  size_t const n_disallowed = sizeof(disallowed_external_client_cargo) /
                              sizeof(disallowed_external_client_cargo[0]);
  ASSERT_EQ(13u, n_allowed);
  ASSERT_EQ(12u, n_disallowed);
  ASSERT_EQ(25u, n_allowed + n_disallowed);
}

TEST_F(XcomBase, ExternalClientCargoAllowlistFailsClosedOnRetiredValues) {
  /*
    The retired enum values 3, 5 and 6 have no case in the switch, so they fall
    through to the closing `return FALSE`. Pin that fail-closed behaviour: an
    unrecognised cargo type must never be treated as allowed.
  */
  for (int retired : {3, 5, 6}) {
    ASSERT_FALSE(unittest_only_is_allowed_external_client_cargo_type(
        static_cast<cargo_type>(retired)))
        << "unrecognised cargo type " << retired << " must fail closed";
  }
}

TEST_F(XcomBase, ExternalClientRejectsAppTypeInAppDataList) {
  app_data_ptr data = new_app_data_with_cargo(app_type);
  ASSERT_TRUE(unittest_only_has_disallowed_external_client_cargo_type(data));
  _replace_app_data_list(&data, nullptr);
}

TEST_F(XcomBase, ExternalClientAcceptsMembershipAppDataList) {
  app_data_ptr data = new_app_data_with_cargo(add_node_type);
  ASSERT_FALSE(unittest_only_has_disallowed_external_client_cargo_type(data));
  _replace_app_data_list(&data, nullptr);
}

TEST_F(XcomBase, ExternalClientRejectsAppTypeAnywhereInList) {
  /* The scan must not stop at the first element: an attacker can prepend
     legitimate cargo to smuggle app_type further down the list. */
  app_data_ptr data = new_app_data_with_cargo(add_node_type);
  data->next = new_app_data_with_cargo(remove_node_type);
  data->next->next = new_app_data_with_cargo(app_type);

  ASSERT_TRUE(unittest_only_has_disallowed_external_client_cargo_type(data));
  _replace_app_data_list(&data, nullptr);
}

TEST_F(XcomBase, ExternalClientRejectsMixedDisallowedCargoInList) {
  /* Same for a non-app_type disallowed cargo behind an allowed one, which
     exercises the second loop rather than the app_type fast path. */
  app_data_ptr data = new_app_data_with_cargo(add_node_type);
  data->next = new_app_data_with_cargo(exit_type);

  ASSERT_TRUE(unittest_only_has_disallowed_external_client_cargo_type(data));
  _replace_app_data_list(&data, nullptr);
}

TEST_F(XcomBase, ExternalClientAcceptsEmptyAppDataList) {
  /* A null list carries no disallowed cargo; the gate must not invent one. */
  ASSERT_FALSE(
      unittest_only_has_disallowed_external_client_cargo_type(nullptr));
}

/*
  Bug#39234600 - demonstrates the attack path, and pins the gate that closes it.

  acceptor_learner_task() did not distinguish an externally received client_msg
  application payload from local Group Replication input.
  Both paths end in the same place:

    local     xcom_send()             -> channel_put(&prop_input_queue, ...)
    external  acceptor_learner_task() -> dispatch_op()
                                      -> case client_msg
                                      -> handle_client_msg()
                                      -> channel_put(&prop_input_queue, ...)

  This test builds the hostile message - a client_msg carrying app_type, which
  is the cargo that becomes replicated content - and establishes two things:

  1. dispatch_op() accepts it onto prop_input_queue on exactly the same terms as
     a locally submitted message. dispatch_op() is precisely the call
     acceptor_learner_task() makes for a message read off the wire, and the
     pax_msg carries no provenance, so once queued the proposer cannot tell the
     two apart. Nothing downstream of the gate protects against this.

  2. The gate rejects that same message. This is the regression assertion: if
     the allowlist is removed or misclassifies app_type, step 2 fails.

  SCOPE. Step 1 passes with or without the fix, by design - the gate sits
  upstream in acceptor_learner_task(), not in dispatch_op(). It is here to show
  what the gate is protecting, not to detect its absence; step 2 does that.
  Driving acceptor_learner_task() itself would need the XCom scheduler
  (task_loop() does not return) and a socketpair, which no test in this tree
  does, so the accept -> task -> dispatch link is established by reading
  incoming_connection_task() in xcom_transport.cc rather than by execution.
*/
TEST_F(XcomBase, ExternalClientMsgReachesProposerQueueAndIsGated) {
  /*
    The fixture already init_cache()d. xcom_thread_init() calls init_cache()
    again, which would orphan that first allocation, so release it here and
    let xcom_thread_init() own the cache for the rest of the test.
  */
  ::deinit_cache();

  /* xcom_thread_init() is what channel_init()s prop_input_queue. */
  xcom_thread_init();

  int const base = unittest_only_prop_input_queue_len();

  /* Locally submitted Group Replication input. */
  app_data_ptr local_a = new_app_data();
  local_a->body.c_t = app_type;
  pax_msg *local_msg = pax_msg_new_0(null_synode);
  local_msg->refcnt = 1;
  xcom_send(local_a, local_msg);

  int const after_local = unittest_only_prop_input_queue_len();
  ASSERT_EQ(base + 1, after_local)
      << "local xcom_send() must queue exactly one message";

  /* The same application cargo, arriving as an external client_msg. */
  app_data_ptr ext_a = new_app_data();
  ext_a->body.c_t = app_type;
  pax_msg *ext_msg = pax_msg_new_0(null_synode);
  ext_msg->refcnt = 1;
  ext_msg->op = client_msg;
  ext_msg->a = ext_a;

  linkage reply_queue;
  link_init(&reply_queue, TYPE_HASH("msg_link"));

  /* 1. It is accepted on the same terms as the local one. */
  dispatch_op(nullptr, ext_msg, &reply_queue);
  ASSERT_EQ(after_local + 1, unittest_only_prop_input_queue_len())
      << "an external client_msg carrying app_type reached the proposer queue "
         "on the same terms as locally submitted input - nothing downstream "
         "distinguishes them. This is the path Bug#39234600 opens.";

  /*
    2. The gate rejects exactly that message. Regression assertion.

    The gate has two independent layers and both are asserted, because either
    alone leaves a hole:
      - an app_type fast path, which does not consult the allowlist
      - the allowlist loop, for every other disallowed cargo
    Asserting only app_type would keep passing if the allowlist were broken,
    since the fast path would still catch it.
  */
  ASSERT_TRUE(unittest_only_has_disallowed_external_client_cargo_type(ext_a))
      << "the app_type fast path must reject this message; without it the "
         "payload above is applied as replicated SQL on every group member";

  app_data_ptr other_a = new_app_data();
  other_a->body.c_t = exit_type;
  ASSERT_TRUE(unittest_only_has_disallowed_external_client_cargo_type(other_a))
      << "the allowlist loop must reject non-app_type cargo that has no place "
         "on an external client connection";
  ASSERT_FALSE(unittest_only_is_allowed_external_client_cargo_type(app_type))
      << "app_type must never be classified as allowed, independently of the "
         "fast path that also catches it";
  _replace_app_data_list(&other_a, nullptr);
}

}  // namespace xcom_base_unittest
