/*****************************************************************************

Copyright (c) 2020, 2024, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is designed to work with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have either included with
the program or referenced in the documentation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file ddl/ddl0file-reader.cc
 For reading the DDL temporary files.
 Created 2020-11-01 by Sunny Bains. */

#include "ddl0impl-file-reader.h"
#include "dict0dict.h"
#include "rem/rec.h"
#include "rem0rec.h"
#include "row0log.h"

namespace ddl {

[[nodiscard]] inline static auto get_read_len_max(
    const Write_offsets &offsets) noexcept {
  ut_a(!offsets.empty());

  os_offset_t len_max = 0;
  os_offset_t prev = 0;

  for (const auto offset : offsets) {
    auto len = offset - prev;
    if (len > len_max) len_max = len;

    prev = offset;
  }

  return len_max;
}

dberr_t File_reader::prepare() noexcept {
  ut_a(m_ptr == nullptr);
  ut_a(m_mrec == nullptr);
  ut_a(m_buffer_size > 0);
  ut_a(m_read_len == 0);

  if (m_offset == m_size) {
    return DB_END_OF_INDEX;
  }

  if (log_tmp_is_encrypted()) {
    // If encrypted, the chunk of data must be read in one go
    // so the decryption is correct
    auto max_len = get_read_len_max(m_write_offsets);
    if (max_len > m_buffer_size) {
      m_buffer_size = max_len;
    }
  }

  m_aligned_buffer = ut::make_unique_aligned<byte[]>(
      ut::make_psi_memory_key(mem_key_ddl), UNIV_SECTOR_SIZE, m_buffer_size);

  if (!m_aligned_buffer) {
    return DB_OUT_OF_MEMORY;
  }

  m_io_buffer = {m_aligned_buffer.get(), m_buffer_size};

  if (log_tmp_is_encrypted()) {
    m_aligned_buffer_crypt =
        ut::make_unique_aligned<byte[]>(ut::make_psi_memory_key(mem_key_ddl),
                                        UNIV_SECTOR_SIZE, m_io_buffer.second);

    if (!m_aligned_buffer_crypt) {
      return DB_OUT_OF_MEMORY;
    }

    m_crypt_buffer = {m_aligned_buffer_crypt.get(), m_io_buffer.second};
  }

  m_mrec = m_io_buffer.first;

  m_ptr = m_io_buffer.first;

  const auto n_fields = dict_index_get_n_fields(m_index);
  const auto n = 1 + REC_OFFS_HEADER_SIZE + n_fields;

  ut_a(m_offsets.empty());

  m_offsets.resize(n);

  m_offsets[0] = n;
  m_offsets[1] = n_fields;

  ut_a(m_aux_buf == nullptr);
  m_aux_buf = ut::new_arr_withkey<byte>(ut::make_psi_memory_key(mem_key_ddl),
                                        ut::Count{UNIV_PAGE_SIZE_MAX / 2});

  if (m_aux_buf == nullptr) {
    return DB_OUT_OF_MEMORY;
  }

  ut_a(m_size > m_offset);
  m_read_len = get_read_len_next();
  const auto err = ddl::pread(m_file.get(), m_io_buffer.first, m_read_len,
                              m_offset, m_crypt_buffer.first, m_space_id);

  if (err != DB_SUCCESS) {
    return err;
  }

  /* Fetch and advance to the next record. */
  m_ptr = m_io_buffer.first;

  /* Position m_mrec on the first record. */
  return next();
}

dberr_t File_reader::seek(os_offset_t offset) noexcept {
  ut_a(m_size > offset);

  m_offset = offset;

  m_read_len = get_read_len_next();
  const auto err = ddl::pread(m_file.get(), m_io_buffer.first, m_read_len,
                              m_offset, m_crypt_buffer.first, m_space_id);

  m_ptr = m_io_buffer.first;

  return err;
}

dberr_t File_reader::read(os_offset_t offset) noexcept {
  const auto err = seek(offset);

  if (err == DB_SUCCESS) {
    /* Position m_mrec on the first record. */
    return next();
  } else {
    return err;
  }
}

dberr_t File_reader::read_next() noexcept {
  ut_a(m_size > m_offset);
  return seek(m_offset + m_read_len);
}

dberr_t File_reader::next() noexcept {
  ut_a(m_ptr >= m_io_buffer.first && m_ptr < get_io_buffer_end());

  size_t extra_size = *m_ptr++;

  if (extra_size == 0) {
    /* Mark as eof. */
    m_offset = m_size;
    return DB_END_OF_INDEX;
  }

  if (extra_size >= 0x80) {
    /* Read another byte of extra_size. */
    if (m_ptr >= get_io_buffer_end()) {
      const auto err = read_next();
      if (err != DB_SUCCESS) {
        return err;
      }
    }

    extra_size = (extra_size & 0x7f) << 8;
    extra_size |= *m_ptr++;
  }

  /* Normalize extra_size. Above, value 0 signals "end of list". */
  --extra_size;

  /* Read the extra bytes. */

  auto rec = const_cast<byte *>(m_ptr);

  if (unlikely(rec + extra_size >= get_io_buffer_end())) {
    /* The record spans two blocks. Copy the entire record to the auxiliary
    buffer and handle this as a special case. */
    const auto partial_size = std::ptrdiff_t(get_io_buffer_end() - m_ptr);

    ut_a(static_cast<size_t>(partial_size) < UNIV_PAGE_SIZE_MAX);

    rec = m_aux_buf;

    /* Copy the partial record from the file buffer to the aux buffer. */
    memcpy(rec, m_ptr, partial_size);

    {
      const auto err = read_next();

      if (err != DB_SUCCESS) {
        return err;
      }
    }

    {
      /* Copy the remaining record from the file buffer to the aux buffer. */
      const auto len = extra_size - partial_size;

      memcpy(rec + partial_size, m_ptr, len);

      m_ptr += len;
    }

    rec_deserialize_init_offsets(rec + extra_size, m_index, &m_offsets[0]);

    const auto data_size = rec_offs_data_size(&m_offsets[0]);

    /* These overflows should be impossible given that records are much
    smaller than either buffer, and the record starts near the beginning
    of each buffer. */
    ut_a(m_ptr + data_size < get_io_buffer_end());
    ut_a(extra_size + data_size < UNIV_PAGE_SIZE_MAX);

    /* Copy the data bytes. */
    memcpy(rec + extra_size, m_ptr, data_size);

    m_ptr += data_size;

  } else {
    rec_deserialize_init_offsets(rec + extra_size, m_index, &m_offsets[0]);

    const auto data_size = rec_offs_data_size(&m_offsets[0]);

    ut_a(extra_size + data_size < UNIV_PAGE_SIZE_MAX);

    const auto required = extra_size + data_size;

    /* Check if the record fits entirely in the block. */
    if (unlikely(m_ptr + required >= get_io_buffer_end())) {
      /* The record spans two blocks. Copy prefix it to buf. */
      const auto partial_size = std::ptrdiff_t(get_io_buffer_end() - m_ptr);

      rec = m_aux_buf;

      memcpy(rec, m_ptr, partial_size);

      /* We cannot invoke rec_offs_make_valid() here, because there
      are no REC_N_NEW_EXTRA_BYTES between extra_size and data_size.
      Similarly, rec_offs_validate() would fail, because it invokes
      rec_get_status(). */
      ut_d(m_offsets[3] = (ulint)m_index);
      ut_d(m_offsets[2] = (ulint)rec + extra_size);

      {
        const auto err = read_next();

        if (err != DB_SUCCESS) {
          return err;
        }
      }

      {
        /* Copy the rest of the record. */
        const auto len = extra_size + data_size - partial_size;

        memcpy(rec + partial_size, m_ptr, len);
        m_ptr += len;
      }
    } else {
      m_ptr += required;
    }
  }

  ++m_n_rows_read;

  m_mrec = rec + extra_size;

  return DB_SUCCESS;
}

[[nodiscard]] inline static auto get_next_offset(const Write_offsets &offsets,
                                                 os_offset_t offset) noexcept {
  if (offset == 0) {  // 0 is not included
    return offsets.begin();
  }

  auto offset_it = std::find(offsets.begin(), offsets.end(), offset);
  ut_a(offset_it != offsets.end());

  return ++offset_it;
}

[[nodiscard]] size_t File_reader::get_read_len_next() const noexcept {
  if (!log_tmp_is_encrypted()) {
    return std::min(m_io_buffer.second, m_size - m_offset);
  }

  // If encrypted, read the same offset and length that was written
  // so the decryption is correct
  auto offset_it = get_next_offset(m_write_offsets, m_offset);
  ut_a(offset_it != m_write_offsets.end());

  const auto len = *offset_it - m_offset;
  ut_a(len > 0);
  ut_a(len <= m_size - m_offset);
  ut_a(len <= m_io_buffer.second);

  return len;
}

}  // namespace ddl
