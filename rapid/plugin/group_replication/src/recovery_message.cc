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
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include "recovery_message.h"

Recovery_message::Recovery_message(Recovery_message_type type,
                                   const std::string& uuid)
    : Plugin_gcs_message(CT_RECOVERY_MESSAGE),
      recovery_message_type(type)
{
  member_uuid.assign(uuid);
}

Recovery_message::~Recovery_message()
{
}

Recovery_message::Recovery_message(const uchar* buf, uint64 len)
    : Plugin_gcs_message(CT_RECOVERY_MESSAGE)
{
  decode(buf, len);
}

void Recovery_message::decode_payload(const unsigned char* buffer,
                                      const unsigned char* end)
{
  DBUG_ENTER("Recovery_message::decode_payload");
  const unsigned char *slider= buffer;
  uint16 payload_item_type= 0;
  unsigned long long payload_item_length= 0;
  m_decode_error= false;

  uint16 recovery_message_type_aux= 0;
  if (decode_payload_item_int2(&slider,
                               &payload_item_type,
                               end,
                               &recovery_message_type_aux) ||
      payload_item_type != PIT_RECOVERY_MESSAGE_TYPE)
  {
    m_decode_error= true;
    DBUG_VOID_RETURN;
  }
  recovery_message_type= (Recovery_message_type)recovery_message_type_aux;

  if (decode_payload_item_string(&slider,
                                 &payload_item_type,
                                 end,
                                 &member_uuid,
                                 &payload_item_length) ||
      payload_item_type != PIT_MEMBER_UUID)
  {
    /*
      Cleared rather than left holding whatever decoded, because a member uuid
      is what handle_recovery_message() matches members on.
    */
    member_uuid.clear();
    m_decode_error= true;
    DBUG_VOID_RETURN;
  }

  DBUG_VOID_RETURN;
}

void Recovery_message::encode_payload(std::vector<unsigned char>* buffer) const
{
  DBUG_ENTER("Recovery_message::encode_payload");

  uint16 recovery_message_type_aux= (uint16)recovery_message_type;
  encode_payload_item_int2(buffer, PIT_RECOVERY_MESSAGE_TYPE,
                           recovery_message_type_aux);

  encode_payload_item_string(buffer, PIT_MEMBER_UUID,
                             member_uuid.c_str(),
                             member_uuid.length());

  /*
    Rewrite the on-the-wire length of the member uuid payload
    item so that it claims far more data than the message carries. That is the
    shape of a malformed payload from a peer, and it cannot be produced through
    any supported interface, so injecting it here is the only way an MTR test
    can drive the decode validation on a running group. Test-only: reached only
    with the debug point set.

    Upstream added the equivalent hook for the message service message on the
    8.0 line, see gr_invalid_message_service_tag_length. No such message class
    exists here, so the recovery message, which every joining member broadcasts
    when it comes online, carries the injection instead.
  */
  DBUG_EXECUTE_IF("group_replication_malformed_recovery_message_payload",
                  {
                    ulonglong invalid_payload_item_length= 1ULL << 30;
                    int8store(buffer->data() +
                              WIRE_FIXED_HEADER_SIZE +
                              WIRE_PAYLOAD_ITEM_HEADER_SIZE + 2 +
                              WIRE_PAYLOAD_ITEM_TYPE_SIZE,
                              invalid_payload_item_length);
                  };);

  /*
    The same injection point for the payload item type rather than its length:
    the member uuid item is relabelled as the recovery message type item. Every
    length on the wire stays correct, so a decoder that only validates lengths
    accepts this and assigns the uuid value to the field it believes it is
    reading - which is what the type checks in decode_payload() exist to stop,
    and what no bounds check can catch. Test-only, as above.
  */
  DBUG_EXECUTE_IF("group_replication_malformed_recovery_message_payload_type",
                  {
                    int2store(buffer->data() +
                              WIRE_FIXED_HEADER_SIZE +
                              WIRE_PAYLOAD_ITEM_HEADER_SIZE + 2,
                              (uint16)PIT_RECOVERY_MESSAGE_TYPE);
                  };);

  DBUG_VOID_RETURN;
}
