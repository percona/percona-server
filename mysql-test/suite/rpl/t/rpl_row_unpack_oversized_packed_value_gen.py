#!/usr/bin/env python3
# Generator for the forged Table_map used by
#   suite/rpl/t/rpl_row_unpack_oversized_packed_value.test  (Bug#39319907)
#
# The test needs a Table_map that declares test.t1.c1 as VARCHAR(N) while the
# accompanying Write_rows event carries a row image with a 1-byte length
# prefix of 255. That combination is not producible with plain SQL: a genuine
# source whose column really is VARCHAR(255) emits metadata 255, and the
# applier's Table_map compatibility check would then reject the narrower
# destination (VARCHAR(10)) as a lossy conversion before any row is unpacked.
#
# So we capture a *legitimate* pair from a throwaway VARCHAR(255) source and
# only forge the Table_map's 2-byte field metadata, lowering it to the
# destination width. Because the declared size then equals the destination,
# compatible_with() computes order == 0, builds no conversion table, and
# unpack_row() runs calc_field_size() over the genuine 0xFF prefix -- yielding
# a packed length (1 + 255 = 256) that overshoots the destination's
# max_packed_col_length() (10 + 1 = 11) and trips the fix.
#
# Capture step (throwaway server), producing CAPTURED_TABLE_MAP_B64 below:
#
#     CREATE TABLE t1 (c1 VARCHAR(255)) CHARACTER SET latin1;
#     INSERT INTO t1 VALUES (REPEAT('x', 255));
#     -- then: mysqlbinlog --base64-output=decode-rows -vv <binlog>
#
# VARCHAR(255) in a single-byte charset is the widest column that still stores
# its row-image length in a ONE-byte prefix; 256+ would use two bytes and the
# 1-byte read selected by the forged metadata would see only 0x00.
#
# The field metadata is a fixed-width part of the Table_map, so patching it
# does not change the event length or any later offset -- only the two
# metadata bytes are rewritten.
#
# The events are captured with binlog_checksum = NONE (SET GLOBAL
# binlog_checksum = NONE before capture), so they carry no CRC32 trailer and
# nothing else needs to be recomputed. If ever regenerated with CRC32 on, pass
# --crc32 so the trailing 4-byte checksum is recomputed after the patch; the
# BINLOG path would tolerate a stale trailer anyway, since it deserializes with
# verify_checksum = false.
#
# Usage:
#     python3 rpl_row_unpack_oversized_packed_value_gen.py [new_length] [--crc32]
# Prints the base64 to paste into section 4 of the .test (default length 10).

import base64
import struct
import sys
import zlib

# Table_map for test.t1(c1 VARCHAR(255)) latin1, captured verbatim with
# binlog_checksum = NONE (metadata 255, no CRC32 trailer).
CAPTURED_TABLE_MAP_B64 = (
    "b1+DahMBAAAALgAAACwCAAAAAIEAAAAAAAEABHRlc3QAAnQxAAEPAv8AAQIBCA=="
)

MYSQL_TYPE_VARCHAR = 0x0F


def patch_table_map(b64, new_length, recompute_crc=False):
    """Return base64 of the Table_map with its first VARCHAR column's declared
    length rewritten to new_length. When recompute_crc is set (events captured
    with CRC32 checksums), the trailing 4-byte CRC32 is recomputed too."""
    ev = bytearray(base64.b64decode(b64))

    if ev[4] != 19:  # TABLE_MAP_EVENT
        raise ValueError("not a Table_map event (type=%d)" % ev[4])

    # Skip the 19-byte common header and the 8-byte Table_map post-header
    # (6-byte table_id + 2-byte flags), then the length-prefixed db and
    # table names, to reach the column definitions.
    off = 27
    db_len = ev[off]
    off += 1 + db_len + 1        # length byte + name + NUL
    tbl_len = ev[off]
    off += 1 + tbl_len + 1       # length byte + name + NUL

    colcnt = ev[off]             # column count (single-byte for < 251 columns)
    off += 1
    coltypes = ev[off:off + colcnt]
    off += colcnt

    off += 1                     # metadata block length (single byte here)

    # Metadata is emitted only for columns that need it, in column order.
    # VARCHAR contributes 2 bytes (its max length); walk to the first one.
    for t in coltypes:
        if t == MYSQL_TYPE_VARCHAR:
            old = struct.unpack_from("<H", ev, off)[0]
            struct.pack_into("<H", ev, off, new_length)
            break
        off += _metadata_width(t)
    else:
        raise ValueError("no VARCHAR column found to patch")

    crc = None
    if recompute_crc:
        crc = zlib.crc32(bytes(ev[:-4])) & 0xFFFFFFFF
        struct.pack_into("<I", ev, len(ev) - 4, crc)

    return old, new_length, crc, base64.b64encode(bytes(ev)).decode()


def _metadata_width(coltype):
    # Width of the metadata each column type contributes, for the handful of
    # types this generator might step over before reaching the VARCHAR.
    if coltype in (0x0F, 0xF6, 0xF5):     # VARCHAR, NEWDECIMAL, ENUM/SET-as-STRING
        return 2
    if coltype in (0xFE, 0xFC, 0x10):     # STRING, BLOB, BIT
        return 2 if coltype != 0xFC else 1
    if coltype in (0x11, 0x12, 0x13):     # TIMESTAMP2, DATETIME2, TIME2
        return 1
    return 0


def main():
    args = [a for a in sys.argv[1:] if a != "--crc32"]
    recompute_crc = "--crc32" in sys.argv[1:]
    new_length = int(args[0]) if args else 10
    old, new, crc, patched = patch_table_map(
        CAPTURED_TABLE_MAP_B64, new_length, recompute_crc
    )
    note = "patched VARCHAR metadata %d -> %d" % (old, new)
    note += ", recomputed CRC32 %08x\n" % crc if crc is not None else " (no CRC32 trailer)\n"
    sys.stderr.write(note)
    print(patched)


if __name__ == "__main__":
    main()
