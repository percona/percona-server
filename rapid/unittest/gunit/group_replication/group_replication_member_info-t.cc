/* Copyright (c) 2014, 2023, Oracle and/or its affiliates.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"
#include <gtest/gtest.h>

#include "member_info.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace gcs_member_info_unittest {

class ClusterMemberInfoTest : public ::testing::Test
{
protected:
  ClusterMemberInfoTest() { };

  virtual void SetUp()
  {
    string hostname("pc_hostname");
    string uuid("781f947c-db4a-11e3-98d1-f01faf1a1c44");
    uint port= 4444;
    uint plugin_version= 0x000400;
    uint write_set_algorithm= 1;
    uint lower_case_table_names= 0;
    string executed_gtid("aaaa:1-10");
    string retrieved_gtid("bbbb:1-10");
    ulonglong gtid_assignment_block_size= 9223372036854775807ULL;
    bool in_primary_mode= false;
    bool has_enforces_update_everywhere_checks= false;
    uint member_weight= 70;

    gcs_member_id= new Gcs_member_identifier("stuff");

    Group_member_info::Group_member_status status=
        Group_member_info::MEMBER_OFFLINE;

    Member_version local_member_plugin_version(plugin_version);
    local_node= new Group_member_info((char*)hostname.c_str(), port,
                                      (char*)uuid.c_str(),write_set_algorithm,
                                      gcs_member_id->get_member_id(), status,
                                      local_member_plugin_version,
                                      gtid_assignment_block_size,
                                      Group_member_info::MEMBER_ROLE_PRIMARY,
                                      in_primary_mode,
                                      has_enforces_update_everywhere_checks,
                                      member_weight, lower_case_table_names);
    local_node->update_gtid_sets(executed_gtid,retrieved_gtid);
  }

  virtual void TearDown()
  {
    delete gcs_member_id;
    delete local_node;
  }

  Group_member_info* local_node;
  Gcs_member_identifier* gcs_member_id;
};

TEST_F(ClusterMemberInfoTest, EncodeDecodeIdempotencyTest)
{
  vector<uchar>* encoded= new vector<uchar>();
  local_node->encode(encoded);

  Group_member_info decoded_local_node(&encoded->front(), encoded->size());


  ASSERT_EQ(local_node->get_port(),
            decoded_local_node.get_port());
  ASSERT_EQ(local_node->get_hostname(),
            decoded_local_node.get_hostname());
  ASSERT_EQ(local_node->get_uuid(),
            decoded_local_node.get_uuid());
  ASSERT_EQ(local_node->get_write_set_extraction_algorithm(),
            decoded_local_node.get_write_set_extraction_algorithm());
  ASSERT_EQ(local_node->get_gcs_member_id().get_member_id(),
            decoded_local_node.get_gcs_member_id().get_member_id());
  ASSERT_EQ(local_node->get_recovery_status(),
            decoded_local_node.get_recovery_status());
  ASSERT_EQ(local_node->get_member_version().get_version(),
            decoded_local_node.get_member_version().get_version());
  ASSERT_EQ(local_node->get_gtid_executed(),
            decoded_local_node.get_gtid_executed());
  ASSERT_EQ(local_node->get_gtid_retrieved(),
            decoded_local_node.get_gtid_retrieved());
  ASSERT_EQ(local_node->get_gtid_assignment_block_size(),
            decoded_local_node.get_gtid_assignment_block_size());
  ASSERT_EQ(local_node->get_role(),
            decoded_local_node.get_role());
  ASSERT_EQ(local_node->get_member_weight(),
            decoded_local_node.get_member_weight());

  delete encoded;
}

class ClusterMemberInfoManagerTest : public ::testing::Test
{
protected:
  ClusterMemberInfoManagerTest() { };

  virtual void SetUp()
  {
    string hostname("pc_hostname");
    string uuid("8d7r947c-dr4a-17i3-59d1-f01faf1kkc44");
    uint port= 4444;
    uint write_set_algorithm= 1;
    uint lower_case_table_names= 0;
    uint plugin_version= 0x000400;
    gcs_member_id= new Gcs_member_identifier("stuff");
    ulonglong gtid_assignment_block_size= 9223372036854775807ULL;
    bool in_primary_mode= false;
    bool has_enforces_update_everywhere_checks= false;
    uint member_weight= 80;

    Group_member_info::Group_member_status status=
        Group_member_info::MEMBER_OFFLINE;

    Member_version local_member_plugin_version(plugin_version);
    local_node= new Group_member_info((char*)hostname.c_str(), port,
                                      (char*)uuid.c_str(), write_set_algorithm,
                                      gcs_member_id->get_member_id(), status,
                                      local_member_plugin_version,
                                      gtid_assignment_block_size,
                                      Group_member_info::MEMBER_ROLE_SECONDARY,
                                      in_primary_mode,
                                      has_enforces_update_everywhere_checks,
                                      member_weight, lower_case_table_names);

    cluster_member_mgr= new Group_member_info_manager(local_node);
  }

  virtual void TearDown()
  {
    delete cluster_member_mgr;
    delete gcs_member_id;
    delete local_node;
  }

  Group_member_info_manager_interface* cluster_member_mgr;
  Group_member_info* local_node;
  Gcs_member_identifier* gcs_member_id;
};

TEST_F(ClusterMemberInfoManagerTest, GetLocalInfoByUUIDTest)
{
  //Add another member info in order to make this test more realistic
  string hostname("pc_hostname2");
  string uuid("781f947c-db4a-22e3-99d4-f01faf1a1c44");
  uint port= 4444;
  uint write_set_algorithm= 1;
  uint lower_case_table_names= 0;
  uint plugin_version= 0x000400;
  Gcs_member_identifier gcs_member_id("another_stuff");
  string executed_gtid("aaaa:1-11");
  string retrieved_gtid("bbbb:1-11");
  ulonglong gtid_assignment_block_size= 9223372036854775807ULL;
  bool in_primary_mode= false;
  bool has_enforces_update_everywhere_checks= false;
  uint member_weight= 90;

  Group_member_info::Group_member_status status=
      Group_member_info::MEMBER_OFFLINE;

  Member_version local_member_plugin_version(plugin_version);
  Group_member_info* new_member= new Group_member_info((char*)hostname.c_str(),
                                                       port,
                                                       (char*)uuid.c_str(),
                                                       write_set_algorithm,
                                                       gcs_member_id.get_member_id(),
                                                       status,
                                                       local_member_plugin_version,
                                                       gtid_assignment_block_size,
                                                       Group_member_info::MEMBER_ROLE_PRIMARY,
                                                       in_primary_mode,
                                                       has_enforces_update_everywhere_checks,
                                                       member_weight,
                                                       lower_case_table_names);
  new_member->update_gtid_sets(executed_gtid,retrieved_gtid);

  cluster_member_mgr->add(new_member);

  string uuid_to_get("8d7r947c-dr4a-17i3-59d1-f01faf1kkc44");

  Group_member_info* retrieved_local_info=
      cluster_member_mgr->get_group_member_info(uuid_to_get);

  ASSERT_TRUE(retrieved_local_info != NULL);
  ASSERT_EQ(retrieved_local_info->get_uuid(),
            uuid_to_get);

  delete retrieved_local_info;
}

TEST_F(ClusterMemberInfoManagerTest, UpdateStatusOfLocalObjectTest)
{
  cluster_member_mgr->update_member_status(local_node->get_uuid(),
                                           Group_member_info::MEMBER_ONLINE);

  ASSERT_EQ(Group_member_info::MEMBER_ONLINE,
            local_node->get_recovery_status());
}

TEST_F(ClusterMemberInfoManagerTest, UpdateGtidSetsOfLocalObjectTest)
{
  string executed_gtid("aaaa:1-10");
  string retrieved_gtid("bbbb:1-10");

  cluster_member_mgr->update_gtid_sets(local_node->get_uuid(),
                                       executed_gtid,
                                       retrieved_gtid);

  ASSERT_EQ(executed_gtid,
            local_node->get_gtid_executed());
  ASSERT_EQ(retrieved_gtid,
            local_node->get_gtid_retrieved());
}

TEST_F(ClusterMemberInfoManagerTest, GetLocalInfoByUUIDAfterEncodingTest)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  vector<Group_member_info*>* decoded_members=
      cluster_member_mgr->decode(&encoded->front(), encoded->size());

  cluster_member_mgr->update(decoded_members);

  delete decoded_members;
  delete encoded;

  string uuid_to_get("8d7r947c-dr4a-17i3-59d1-f01faf1kkc44");

  Group_member_info* retrieved_local_info=
      cluster_member_mgr->get_group_member_info(uuid_to_get);

  ASSERT_TRUE(retrieved_local_info != NULL);

  ASSERT_EQ(local_node->get_port(),
            retrieved_local_info->get_port());
  ASSERT_EQ(local_node->get_hostname(),
            retrieved_local_info->get_hostname());
  ASSERT_EQ(local_node->get_uuid(),
            retrieved_local_info->get_uuid());
  ASSERT_EQ(local_node->get_gcs_member_id().get_member_id(),
            retrieved_local_info->get_gcs_member_id().get_member_id());
  ASSERT_EQ(local_node->get_recovery_status(),
            retrieved_local_info->get_recovery_status());
  ASSERT_EQ(local_node->get_write_set_extraction_algorithm(),
            retrieved_local_info->get_write_set_extraction_algorithm());
  ASSERT_EQ(local_node->get_gtid_executed(),
            retrieved_local_info->get_gtid_executed());
  ASSERT_EQ(local_node->get_gtid_retrieved(),
            retrieved_local_info->get_gtid_retrieved());
  ASSERT_EQ(local_node->get_gtid_assignment_block_size(),
            retrieved_local_info->get_gtid_assignment_block_size());
  ASSERT_EQ(local_node->get_role(),
            retrieved_local_info->get_role());
  ASSERT_EQ(local_node->get_member_weight(),
            retrieved_local_info->get_member_weight());

  delete retrieved_local_info;
}

TEST_F(ClusterMemberInfoManagerTest, UpdateStatusOfLocalObjectAfterExchangeTest)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  vector<Group_member_info*>* decoded_members=
      cluster_member_mgr->decode(&encoded->front(), encoded->size());

  cluster_member_mgr->update(decoded_members);

  delete decoded_members;
  delete encoded;

  cluster_member_mgr->update_member_status(local_node->get_uuid(),
                                           Group_member_info::MEMBER_ONLINE);

  ASSERT_EQ(Group_member_info::MEMBER_ONLINE,
            local_node->get_recovery_status());

  string executed_gtid("cccc:1-11");
  string retrieved_gtid("dddd:1-11");

  cluster_member_mgr->update_gtid_sets(local_node->get_uuid(),
                                       executed_gtid,
                                       retrieved_gtid);

  ASSERT_EQ(executed_gtid,
            local_node->get_gtid_executed());
  ASSERT_EQ(retrieved_gtid,
            local_node->get_gtid_retrieved());

  Group_member_info* retrieved_local_info=
      cluster_member_mgr->get_group_member_info(local_node->get_uuid());

  ASSERT_EQ(Group_member_info::MEMBER_ONLINE,
            retrieved_local_info->get_recovery_status());

  ASSERT_EQ(executed_gtid,
            retrieved_local_info->get_gtid_executed());
  ASSERT_EQ(retrieved_gtid,
            retrieved_local_info->get_gtid_retrieved());

  delete retrieved_local_info;
}


TEST_F(ClusterMemberInfoManagerTest, EncodeDecodeLargeSets)
{
  //Add another member info in order to make this test more realistic
  string hostname("pc_hostname2");
  string uuid("781f947c-db4a-22e3-99d4-f01faf1a1c44");
  uint port= 4444;
  uint write_set_algorithm= 1;
  uint lower_case_table_names= 0;
  uint plugin_version= 0x000400;
  Gcs_member_identifier gcs_member_id("another_stuff");
  string executed_gtid("aaaa:1-11:12-14:16-20:22-30");
  //Add an huge gtid string (bigger then 16 bits )
  string retrieved_gtid(70000, 'a');
  ulonglong gtid_assignment_block_size= 9223372036854775807ULL;
  bool in_primary_mode= false;
  bool has_enforces_update_everywhere_checks= false;
  uint member_weight= 40;

  Group_member_info::Group_member_status status=
      Group_member_info::MEMBER_OFFLINE;

  Member_version local_member_plugin_version(plugin_version);
  Group_member_info* new_member= new Group_member_info((char*)hostname.c_str(),
                                                       port,
                                                       (char*)uuid.c_str(),
                                                       write_set_algorithm,
                                                       gcs_member_id.get_member_id(),
                                                       status,
                                                       local_member_plugin_version,
                                                       gtid_assignment_block_size,
                                                       Group_member_info::MEMBER_ROLE_PRIMARY,
                                                       in_primary_mode,
                                                       has_enforces_update_everywhere_checks,
                                                       member_weight,
                                                       lower_case_table_names);
  new_member->update_gtid_sets(executed_gtid,retrieved_gtid);

  cluster_member_mgr->add(new_member);

  string uuid_to_get("8d7r947c-dr4a-17i3-59d1-f01faf1kkc44");

  Group_member_info* retrieved_local_info=
      cluster_member_mgr->get_group_member_info(uuid_to_get);

  ASSERT_TRUE(retrieved_local_info != NULL);
  ASSERT_EQ(retrieved_local_info->get_uuid(),
            uuid_to_get);

  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  vector<Group_member_info*>* decoded_members=
      cluster_member_mgr->decode(&encoded->front(), encoded->size());
  delete encoded;

  cluster_member_mgr->update(decoded_members);

  delete decoded_members;

  ASSERT_EQ(2U,
            cluster_member_mgr->get_number_of_members());

  delete retrieved_local_info;
  retrieved_local_info=
      cluster_member_mgr->get_group_member_info(uuid);

  ASSERT_TRUE(retrieved_local_info != NULL);

  ASSERT_EQ(port,
            retrieved_local_info->get_port());
  ASSERT_EQ(hostname,
            retrieved_local_info->get_hostname());
  ASSERT_EQ(executed_gtid,
            retrieved_local_info->get_gtid_executed());
  ASSERT_EQ(retrieved_gtid,
            retrieved_local_info->get_gtid_retrieved());
  ASSERT_EQ(write_set_algorithm,
            retrieved_local_info->get_write_set_extraction_algorithm());

  delete retrieved_local_info;
  retrieved_local_info=
      cluster_member_mgr->get_group_member_info(uuid_to_get);

  ASSERT_TRUE(retrieved_local_info != NULL);

  ASSERT_EQ(local_node->get_port(),
            retrieved_local_info->get_port());
  ASSERT_EQ(local_node->get_hostname(),
            retrieved_local_info->get_hostname());
  ASSERT_EQ(local_node->get_uuid(),
            retrieved_local_info->get_uuid());
  ASSERT_EQ(local_node->get_gcs_member_id().get_member_id(),
            retrieved_local_info->get_gcs_member_id().get_member_id());
  ASSERT_EQ(local_node->get_recovery_status(),
            retrieved_local_info->get_recovery_status());
  ASSERT_EQ(local_node->get_write_set_extraction_algorithm(),
            retrieved_local_info->get_write_set_extraction_algorithm());
  ASSERT_EQ(local_node->get_gtid_executed(),
            retrieved_local_info->get_gtid_executed());
  ASSERT_EQ(local_node->get_gtid_retrieved(),
            retrieved_local_info->get_gtid_retrieved());
  ASSERT_EQ(local_node->get_gtid_assignment_block_size(),
            retrieved_local_info->get_gtid_assignment_block_size());
  ASSERT_EQ(local_node->get_role(),
            retrieved_local_info->get_role());
  ASSERT_EQ(local_node->get_member_weight(),
            retrieved_local_info->get_member_weight());
  ASSERT_EQ(local_node->get_lower_case_table_names(),
            retrieved_local_info->get_lower_case_table_names());

  delete retrieved_local_info;
}

/*
  Group Replication malformed payload decode validation.

  The decode_payload_item_* helpers advanced the read pointer and copied
  `length` bytes out of the received buffer without checking against the end of
  that buffer. For the variable-length variants the length comes straight off
  the wire, so a crafted GCS message from a peer could drive reads past the end
  of the buffer - an out-of-bounds read.

  Group Replication messages arrive over GCS/XCom from other members, so this is
  externally influenced input. These tests feed truncated encodings, which is
  exactly the shape of a malformed payload, and assert the decoder reports the
  failure instead of reading past the end.
*/

TEST_F(ClusterMemberInfoTest, TruncatedPayloadIsRejected)
{
  vector<uchar>* encoded= new vector<uchar>();
  local_node->encode(encoded);

  /* A complete encoding must still decode cleanly. */
  Group_member_info whole(&encoded->front(), encoded->size());
  ASSERT_FALSE(whole.is_decode_error());

  /*
    A prefix too short to hold the items this decoder reads must be reported as
    a failure. Note a long prefix may legitimately decode clean: the decoder
    stops once it has read the items it knows about, so trailing bytes of the
    encoding are not required. What must never happen is a read past the end,
    which is what the bounds checks in decode_payload_item_* now prevent.
  */
  Group_member_info short_prefix(&encoded->front(), 8);
  ASSERT_TRUE(short_prefix.is_decode_error())
    << "an 8 byte prefix of a " << encoded->size()
    << " byte message cannot hold the required payload items";

  /*
    Sweep every truncation. This asserts nothing on its own beyond "does not
    crash and terminates" - its value is as a vehicle for the sanitiser builds,
    where an out-of-bounds read here is a hard failure. Before the fix this
    sweep reads past the end of the buffer for a large fraction of the lengths.
  */
  for (size_t len= 1; len < encoded->size(); len++)
  {
    Group_member_info truncated(&encoded->front(), len);
    /* Touch the result so the decode cannot be optimised away. */
    ASSERT_TRUE(truncated.is_decode_error() || !truncated.is_decode_error());
  }

  delete encoded;
}

TEST_F(ClusterMemberInfoTest, EmptyPayloadIsRejected)
{
  /* Nothing at all to decode: must fail, not read from the pointer. */
  vector<uchar>* encoded= new vector<uchar>();
  local_node->encode(encoded);

  Group_member_info empty(&encoded->front(), 0);
  ASSERT_TRUE(empty.is_decode_error());

  delete encoded;
}

TEST_F(ClusterMemberInfoTest, OversizedPayloadItemLengthIsRejected)
{
  /*
    The direct reproducer for the out-of-bounds read. A payload item is
    type (2 bytes) + length (8 bytes) + data. Take a valid encoding and rewrite
    the first item's length field to claim far more data than the buffer holds,
    which is what a malicious peer would send. The decoder must reject it rather
    than assign() from past the end of the buffer.
  */
  vector<uchar>* encoded= new vector<uchar>();
  local_node->encode(encoded);

  Group_member_info whole(&encoded->front(), encoded->size());
  ASSERT_FALSE(whole.is_decode_error());

  /* Offset of the first payload item's length field. */
  size_t const len_off= Group_member_info::WIRE_FIXED_HEADER_SIZE +
                        Group_member_info::WIRE_PAYLOAD_ITEM_TYPE_SIZE;
  ASSERT_LT(len_off + Group_member_info::WIRE_PAYLOAD_ITEM_LEN_SIZE,
            encoded->size());

  uchar* raw= &encoded->front();
  /* Claim a 0x00FFFFFF byte string inside a few-hundred byte buffer. */
  int8store(raw + len_off, (ulonglong)0x00FFFFFFULL);

  Group_member_info hostile(raw, encoded->size());
  ASSERT_TRUE(hostile.is_decode_error())
    << "a payload item claiming more data than the buffer holds must be "
       "rejected, not read past the end";

  delete encoded;
}

/*
  The two entry points that read a peer supplied buffer at fixed
  offsets without decoding the whole message. Both are reached before any
  Plugin_gcs_message instance exists, so neither is covered by the decode
  validation tests above.
*/

TEST_F(ClusterMemberInfoManagerTest, GetCargoTypeRejectsTruncatedFixedHeader)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  ASSERT_GT(encoded->size(), Plugin_gcs_message::WIRE_FIXED_HEADER_SIZE);

  Plugin_gcs_message::enum_cargo_type cargo_type=
      Plugin_gcs_message::CT_UNKNOWN;
  bool const error= Plugin_gcs_message::get_cargo_type(
      &encoded->front(),
      Plugin_gcs_message::WIRE_FIXED_HEADER_SIZE - 1,
      &cargo_type);

  EXPECT_TRUE(error)
    << "a buffer shorter than the fixed header must be rejected, not read "
       "past the end";

  delete encoded;
}

TEST_F(ClusterMemberInfoManagerTest, GetCargoTypeRejectsOutOfRangeCargoType)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  size_t const cargo_type_offset= Plugin_gcs_message::WIRE_VERSION_SIZE +
                                  Plugin_gcs_message::WIRE_HD_LEN_SIZE +
                                  Plugin_gcs_message::WIRE_MSG_LEN_SIZE;
  ASSERT_LT(cargo_type_offset + Plugin_gcs_message::WIRE_CARGO_TYPE_SIZE,
            encoded->size());

  uint16 const invalid_cargo_types[]=
    { (uint16)Plugin_gcs_message::CT_UNKNOWN,
      (uint16)Plugin_gcs_message::CT_MAX,
      (uint16)(Plugin_gcs_message::CT_MAX + 1) };

  for (size_t i= 0; i < array_elements(invalid_cargo_types); i++)
  {
    int2store(&encoded->front() + cargo_type_offset, invalid_cargo_types[i]);

    Plugin_gcs_message::enum_cargo_type cargo_type=
        Plugin_gcs_message::CT_UNKNOWN;
    bool const error= Plugin_gcs_message::get_cargo_type(
        &encoded->front(), encoded->size(), &cargo_type);

    EXPECT_TRUE(error) << "cargo type " << invalid_cargo_types[i]
                       << " is outside the known range and must not be "
                          "dispatched";
  }

  delete encoded;
}

TEST_F(ClusterMemberInfoManagerTest, GetCargoTypeAcceptsValidMessage)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  Plugin_gcs_message::enum_cargo_type cargo_type=
      Plugin_gcs_message::CT_UNKNOWN;
  bool const error= Plugin_gcs_message::get_cargo_type(
      &encoded->front(), encoded->size(), &cargo_type);

  EXPECT_FALSE(error);
  EXPECT_EQ(Plugin_gcs_message::CT_MEMBER_INFO_MANAGER_MESSAGE, cargo_type);

  delete encoded;
}

TEST_F(ClusterMemberInfoManagerTest, GetFirstPayloadItemRejectsTruncatedHeader)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  size_t const minimum_size=
      Plugin_gcs_message::WIRE_FIXED_HEADER_SIZE +
      Plugin_gcs_message::WIRE_PAYLOAD_ITEM_HEADER_SIZE;
  ASSERT_GT(encoded->size(), minimum_size);

  const unsigned char* payload_data= NULL;
  uint64 payload_length= 0;
  bool const error= Plugin_gcs_message::get_first_payload_item_raw_data(
      &encoded->front(), minimum_size - 1, &payload_data, &payload_length);

  EXPECT_TRUE(error)
    << "a buffer too short to hold the first payload item header must be "
       "rejected";

  delete encoded;
}

TEST_F(ClusterMemberInfoManagerTest, GetFirstPayloadItemRejectsOverrunningLength)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  size_t const payload_item_len_offset=
      Plugin_gcs_message::WIRE_FIXED_HEADER_SIZE +
      Plugin_gcs_message::WIRE_PAYLOAD_ITEM_TYPE_SIZE;
  ASSERT_LT(payload_item_len_offset +
                Plugin_gcs_message::WIRE_PAYLOAD_ITEM_LEN_SIZE,
            encoded->size());

  size_t const bytes_after_item_header=
      encoded->size() - (payload_item_len_offset +
                         Plugin_gcs_message::WIRE_PAYLOAD_ITEM_LEN_SIZE);

  /* One byte more than the buffer holds must be rejected. */
  int8store(&encoded->front() + payload_item_len_offset,
            (ulonglong)(bytes_after_item_header + 1));

  const unsigned char* payload_data= NULL;
  uint64 payload_length= 0;
  bool error= Plugin_gcs_message::get_first_payload_item_raw_data(
      &encoded->front(), encoded->size(), &payload_data, &payload_length);

  EXPECT_TRUE(error)
    << "a declared payload item length larger than the buffer must be "
       "rejected, not handed to the applier";

  /* Exactly what the buffer holds must still be accepted. */
  int8store(&encoded->front() + payload_item_len_offset,
            (ulonglong)bytes_after_item_header);

  error= Plugin_gcs_message::get_first_payload_item_raw_data(
      &encoded->front(), encoded->size(), &payload_data, &payload_length);

  EXPECT_FALSE(error);
  EXPECT_EQ(bytes_after_item_header, payload_length);

  delete encoded;
}

TEST_F(ClusterMemberInfoManagerTest, GetFirstPayloadItemAcceptsValidMessage)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  const unsigned char* payload_data= NULL;
  uint64 payload_length= 0;
  bool const error= Plugin_gcs_message::get_first_payload_item_raw_data(
      &encoded->front(), encoded->size(), &payload_data, &payload_length);

  EXPECT_FALSE(error);
  EXPECT_TRUE(payload_data != NULL);
  EXPECT_GT(payload_length, 0U);
  EXPECT_LE(payload_data + payload_length,
            &encoded->front() + encoded->size());

  delete encoded;
}

/*
  The two cases below are ports of the gunit tests upstream ships with
  Bug#39253416 (7206c72ea29): DecodeRejectsMemberEntryWithInvalidPortLength and
  DecodeRejectsUnexpectedMemberInfoManagerEntryType. The third upstream case,
  GetPitDataRejectsUnexpectedMemberInfoManagerEntryType, has no counterpart here
  because get_pit_data() reads payload items this line does not have.

  They cover Group_member_info_manager_message::decode_payload(), the state
  exchange path, which no MTR test on this line can reach: injecting there needs
  a debug point in Group_member_info::encode_payload(), and that encode runs on a
  GCS engine thread, which has no DBUG state, so the hook never fires.

  Both walk a Group_member_info_manager_message encoding:

    fixed header
    PIT_MEMBERS_NUMBER item   header + 2 bytes
    PIT_MEMBER_DATA item      header + a whole nested Group_member_info
*/
static size_t get_first_member_entry_offset()
{
  size_t const int2_payload_size= 2;
  return Group_member_info::WIRE_FIXED_HEADER_SIZE +
         Group_member_info::WIRE_PAYLOAD_ITEM_HEADER_SIZE +
         int2_payload_size;
}

static size_t get_first_member_port_length_offset(size_t hostname_length)
{
  /* Where the nested member's own encoding starts. */
  size_t const first_member_payload_offset=
      get_first_member_entry_offset() +
      Group_member_info::WIRE_PAYLOAD_ITEM_HEADER_SIZE;
  /* Its first item is the hostname, and the port item follows it. */
  size_t const hostname_item_size=
      Group_member_info::WIRE_PAYLOAD_ITEM_HEADER_SIZE + hostname_length;

  return first_member_payload_offset +
         Group_member_info::WIRE_FIXED_HEADER_SIZE + hostname_item_size +
         Group_member_info::WIRE_PAYLOAD_ITEM_TYPE_SIZE;
}

TEST_F(ClusterMemberInfoManagerTest,
       DecodeRejectsMemberEntryWithInvalidPortLength)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  size_t const port_length_offset=
      get_first_member_port_length_offset(local_node->get_hostname().length());
  ASSERT_LT(port_length_offset + Group_member_info::WIRE_PAYLOAD_ITEM_LEN_SIZE,
            encoded->size() + 1);

  /* A port item declaring one byte where the decoder needs two. */
  int8store(&encoded->front() + port_length_offset, 1ULL);

  std::vector<Group_member_info*>* decoded_members=
      cluster_member_mgr->decode(&encoded->front(), encoded->size());

  EXPECT_TRUE(decoded_members == NULL)
    << "a nested member whose port item declares the wrong length must fail "
       "the whole message, not yield a partially decoded member list";

  delete decoded_members;
  delete encoded;
}

TEST_F(ClusterMemberInfoManagerTest,
       DecodeRejectsUnexpectedMemberInfoManagerEntryType)
{
  vector<uchar>* encoded= new vector<uchar>();
  cluster_member_mgr->encode(encoded);

  size_t const member_entry_offset= get_first_member_entry_offset();
  ASSERT_LT(member_entry_offset +
                Group_member_info::WIRE_PAYLOAD_ITEM_TYPE_SIZE,
            encoded->size() + 1);

  /*
    Relabel the member entry as something that is not PIT_MEMBER_DATA. Upstream
    relabels it PIT_MEMBER_ACTIONS, which this line does not have;
    PIT_MEMBERS_NUMBER is the equivalent - a valid member of the enum that does
    not belong at this position. Every length stays correct, so only the type
    check can reject it.
  */
  int2store(&encoded->front() + member_entry_offset,
            (uint16)Group_member_info_manager_message::PIT_MEMBERS_NUMBER);

  std::vector<Group_member_info*>* decoded_members=
      cluster_member_mgr->decode(&encoded->front(), encoded->size());

  EXPECT_TRUE(decoded_members == NULL)
    << "a member entry that is not PIT_MEMBER_DATA must fail the whole message";

  delete decoded_members;
  delete encoded;
}

}
