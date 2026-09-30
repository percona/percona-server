#!/usr/bin/env python3
"""Regenerate the malformed WRITE_ROWS_EVENT (R1) used by
binlog_rows_event_oversized_width.test, deriving it from the well-formed R2
write-rows event so the two stay in lockstep.

R1 is R2's write-rows event with an oversized column-count width: its 19-byte
common header and 10-byte rows post-header (table_id + flags + var_header_len)
are copied byte-for-byte from R2, then the body is replaced with a packed width
> 4096 and a full (width + 7)//8 zero column bitmap and no row image. Only
event_size differs in the common header (forced by the larger body).

R2 below is the exact TABLE_MAP + WRITE_ROWS BINLOG statement captured with
checksums off (binlog_checksum = NONE) and pasted from mysqlbinlog. If you
recapture R2, replace R2_BINLOG and rerun to get the matching R1.

Usage:
    python3 gen_oversized_rows_event.py [width]      # width defaults to 4097
"""
import base64
import re
import struct
import sys

# The R2 statement from the test: TABLE_MAP (type 0x13) + WRITE_ROWS (type 0x1e),
# two independently-padded base64 chunks (MY_BASE64_DECODE_ALLOW_MULTIPLE_CHUNKS).
R2_BINLOG = (
    "+np/ahMBAAAALAAAAAsCAAAAAIEAAAAAAAEABHRlc3QAAnQxAAEDAAEBAQA=\n"
    "+np/ah4BAAAAJAAAAC8CAAAAAIEAAAAAAAEAAgAB/wABAAAA"
)

WRITE_ROWS_EVENT = 0x1E    # v2
COMMON_HEADER_LEN = 19     # timestamp + type + server_id + event_size + log_pos + flags
ROWS_POST_HEADER_LEN = 10  # table_id(6) + flags(2) + var_header_len(2)


def decode_multi_chunk(blob: str) -> bytes:
    """Decode a base64 string that may hold several independently-padded chunks."""
    s = blob.replace("\n", "")
    return b"".join(
        base64.b64decode(chunk) for chunk in re.findall(r"[A-Za-z0-9+/]+={0,2}", s)
    )


def extract_event(stream: bytes, event_type: int) -> bytes:
    """Return the first event of event_type from a concatenation of v4 events."""
    off = 0
    while off < len(stream):
        size = struct.unpack("<I", stream[off + 9:off + 13])[0]
        if stream[off + 4] == event_type:
            return stream[off:off + size]
        off += size
    raise ValueError(f"event type 0x{event_type:02x} not found")


def net_store_length(n: int) -> bytes:
    """MySQL net_field_length / net_store_length integer packing."""
    if n < 251:
        return bytes([n])
    if n < (1 << 16):
        return b"\xfc" + struct.pack("<H", n)
    if n < (1 << 24):
        return b"\xfd" + struct.pack("<I", n)[:3]
    return b"\xfe" + struct.pack("<Q", n)


def build(width: int) -> bytes:
    template = extract_event(decode_multi_chunk(R2_BINLOG), WRITE_ROWS_EVENT)
    head = COMMON_HEADER_LEN + ROWS_POST_HEADER_LEN  # copied byte-for-byte from R2

    body_tail = net_store_length(width)          # packed column-count width
    body_tail += b"\x00" * ((width + 7) // 8)    # columns-present bitmap, all zero
    # no row image

    ev = bytearray(template[:head]) + body_tail
    struct.pack_into("<I", ev, 9, len(ev))       # patch event_size for the larger body
    return bytes(ev)


if __name__ == "__main__":
    width = int(sys.argv[1]) if len(sys.argv) > 1 else 4097
    ev = build(width)
    print(f"# width={width}  event_size={len(ev)}", file=sys.stderr)
    print(base64.b64encode(ev).decode())
