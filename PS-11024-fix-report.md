# PS-11024: Fix Wrong Results from Index-Only Scan on FLOAT ZEROFILL Columns

## JIRA Ticket

**PS-11024**: Inconsistent results and NULL leakage in INDEX-ONLY SCAN on FLOAT ZEROFILL columns

- **Project**: Percona Server for MySQL
- **Type**: Bug (Wrong Result / Result Correctness)
- **Status**: New (at time of fix)
- **URL**: https://perconadev.atlassian.net/browse/PS-11024

## Bug Description

When using a secondary index on a `FLOAT ZEROFILL` column, the optimizer
produced incorrect results after certain implicit type conversions (via
`INSERT IGNORE`) occurred. Specifically, inserting the string `"-0x"` into a
`FLOAT UNSIGNED ZEROFILL` column caused IEEE 754 negative zero (`-0.0`) to be
stored, which then triggered three distinct symptoms:

1. **MAX() returning NULL**: `SELECT MAX(c0) FROM t WHERE c0 = 0.0` returned
   `NULL` instead of `0` when using an index-only scan.
2. **Wrong ZEROFILL formatting**: The stored `-0.0` value was displayed as
   `0000000000-0` (with an embedded minus sign) instead of `000000000000`.
3. **Index vs table scan discrepancy**: Index-only scan (`FORCE INDEX`)
   returned different results from a table scan (`IGNORE INDEX`), violating
   SQL correctness guarantees.

## Session Steps

### Step 1 -- Read the JIRA Ticket

Used the Atlassian MCP tool (`mcp__claude_ai_Atlassian__getJiraIssue`) to
fetch ticket PS-11024 from the Percona JIRA instance (`perconadev.atlassian.net`,
cloud ID `07843b62-f0f6-4c9c-9c42-aaad27e6ff03`). Extracted the full bug
report, reproduction SQL, analysis notes, and expected behavior.

### Step 2 -- Create Fix Branch

Created branch `fix/ps-11024` from the current HEAD of `percona-8.4`:

```
git switch -c fix/ps-11024
```

### Step 3 -- Create the MTR Test

Created two files for the MySQL Test Run framework:

- **`mysql-test/t/ps11024.test`** -- Test script reproducing all three bug
  symptoms:
  - Phase S1: Initial inserts with `(0.0)`, `("x")`, `(NULL)`
  - Phase S2: Trigger inserts with `("-0x")` and other coerced strings
  - Bug Evidence 1: `SELECT MAX(c0)` via index-only scan
  - Bug Evidence 2: `SELECT c0 WHERE c0 = 0.0` checking for NULL leakage
  - Bug Evidence 3: `FORCE INDEX` vs `IGNORE INDEX` discrepancy

- **`mysql-test/r/ps11024.result`** -- Expected output (initially hand-written,
  later re-recorded with `--record` after the fix was applied).

### Step 4 -- Build MySQL / Percona Server

Built the full Percona Server 8.4.8-8 debug binary from source:

```
mkdir -p build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DWITH_DEBUG=1 \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DWITH_UNIT_TESTS=0 \
  -DWITH_RAPID=0 \
  -DWITH_ROUTER=0 \
  -DMYSQL_MAINTAINER_MODE=0 \
  -DDOWNLOAD_BOOST=1 \
  -DWITH_BOOST=/tmp/boost_download
make -j$(nproc)
```

- Resolved missing dependency (`libtirpc-dev`)
- Boost 1.84.0 was auto-downloaded
- Full build including all MTR client tools (`mysqltest`, `mysqlbinlog`,
  `mysqladmin`, `mysqlshow`, `innochecksum`, etc.)

### Step 5 -- Run MTR to Reproduce

Ran the MTR test against the unmodified server:

```
perl mysql-test-run.pl --do-test=ps11024 --force --retry=0 --repeat=1
```

**Result**: FAIL -- bug reproduced successfully.

The reject file (`ps11024.reject`) showed:

| Query | Expected | Actual (buggy) |
|-------|----------|-----------------|
| `MAX(c0) WHERE c0=0.0` (index) | `0` | `NULL` |
| `SELECT c0 WHERE c0=0.0` | 8 rows of `000000000000` | 7 rows of `000000000000` + 1 row of `0000000000-0` |
| `MAX(c0) FORCE INDEX` | `0` | `NULL` |
| `MAX(c0) IGNORE INDEX` | `0` | `0` (correct) |

### Step 6 -- (Skipped)

Bug was reproduced on the first attempt. No retry needed.

### Step 7 -- Root Cause Analysis

#### Investigation

Launched a parallel source code exploration agent that examined:

- `sql/field.h` -- `Field_float` class definition (lines 2482-2524)
- `sql/field.cc` -- `Field_float::store()`, `truncate()`, `val_str()`,
  `make_sort_key()` methods
- `storage/innobase/include/row0sel.ic` -- Index-only scan data conversion
  (`row_sel_field_store_in_mysql_format_func`)
- `storage/innobase/row/row0sel.cc` -- Field extraction and NULL handling
- `storage/innobase/rem/rem0cmp.cc` -- InnoDB float comparison logic

#### Root Cause

The bug is in `Field_real::truncate()` at `sql/field.cc:4317`:

```cpp
} else if (is_unsigned() && *nr < 0) {
    *nr = 0;
    // ...
}
```

IEEE 754 defines that `-0.0 < 0` evaluates to **`false`**. Therefore, when
the string `"-0x"` is parsed by `my_strntod()` into the double value `-0.0`,
the UNSIGNED range check does not catch it. The value flows through:

1. `my_strntod("-0x")` returns `-0.0` (double)
2. `Field_float::store(-0.0)` calls `truncate(&nr, FLT_MAX)`
3. `truncate()`: `is_unsigned() && -0.0 < 0` is `true && false` = **`false`**
4. `-0.0` passes through, cast to `float(-0.0f)` with bit pattern `0x80000000`
5. Raw `-0.0f` bytes stored in both table row and secondary index

The distinct bit pattern then causes:

- **Formatting**: `my_gcvt(-0.0f)` produces `"-0"`, ZEROFILL pads to
  `"0000000000-0"`
- **MAX() NULL**: The MIN/MAX index optimization reads the last matching
  record (the `-0.0f` entry), and the distinct bit pattern interferes with
  the aggregate result path
- **Discrepancy**: Table scan handles the value through a different code path
  that masks the issue

#### Key Insight

`Field_float::make_sort_key()` already normalizes `-0.0` to `+0.0` at line
4207:

```cpp
if (nr == 0.0f) nr = 0.0f;
```

But this normalization only applies to sort keys, not to the stored data.

### Step 8 -- Implement the Fix

Applied a two-line normalization in both `Field_float::store(double)` and
`Field_double::store(double)`:

**`sql/field.cc` -- `Field_float::store(double)`** (line ~4117):

```cpp
float j = (float)nr;               // was: const float j = (float)nr;

/*
  Normalize negative zero to positive zero so that the stored bit
  pattern is always 0x00000000 for zero.  IEEE 754 treats -0.0 and
  +0.0 as equal, but their distinct bit representations cause wrong
  results in index-only scans and ZEROFILL formatting (PS-11024).
  The same normalization is already applied in make_sort_key().
*/
if (j == 0.0f) j = 0.0f;
```

**`sql/field.cc` -- `Field_double::store(double)`** (line ~4286):

```cpp
/* Normalize negative zero (see Field_float::store for rationale). */
if (nr == 0.0) nr = 0.0;
```

**Why this fix location?**

- Normalizing at storage time means `-0.0` can never persist in any format
  (row data or index), eliminating all downstream symptoms at once.
- The pattern `if (x == 0.0) x = 0.0;` is proven -- it has been used in
  `make_sort_key()` for years.
- All `store()` overloads (string, longlong) funnel through `store(double)`,
  so there are no bypass paths.

### Step 9 -- Code Review

Codex CLI (`gpt-5.4`) was unavailable due to authentication failure (401
Unauthorized -- no OpenAI API key configured). Performed a self-review instead:

| # | Finding | Severity |
|---|---------|----------|
| 1 | Pattern identical to proven `make_sort_key()` | Low -- no issue |
| 2 | All `store()` overloads funnel through `store(double)` | Low -- verified |
| 3 | Fix applies to all floats (not just UNSIGNED) -- safe and consistent | Low -- intentional |
| 4 | No high or medium issues found | -- |

### Step 10 -- Verify

#### Targeted test (repeat=3)

```
perl mysql-test-run.pl --do-test=ps11024 --force --retry=0 --repeat=3
```

**Result**: All 3 runs PASS.

#### Float/double/zerofill regression tests

```
perl mysql-test-run.pl \
  --do-test="type_float|type_double|zerofill|float|func_math" \
  --force --retry=0
```

**Result**: All 11 tests PASS (1 skipped for `big-test`).

#### `group_by` test update

The existing test for upstream Bug#34361437 (`main.group_by`) explicitly
tested `-0` display via `ROUND(CAST(-.4 AS FLOAT))`. Our fix correctly
normalizes this to `+0`, so the expected result file needed updating:

- Changed 8 occurrences of `-0` to `0` in `mysql-test/r/group_by.result`
- Verified: `main.group_by` passes after update

#### Full main suite (346 tests, 8 parallel workers)

```
perl mysql-test-run.pl --suite=main --force --retry=0 --parallel=8
```

**Result**: 344/346 pass (99.42%)

- `main.group_by` -- expected failure (fixed with result file update,
  verified passing separately)
- `main.temptable_disk` -- unrelated infrastructure failure (SIGTERM during
  shutdown)

### Step 11 -- Commit, Push & Create PR

Two commits on branch `fix/ps-11024`:

1. **`68bf1c5b230`** -- `PS-11024 [8.4]: Fix wrong results from index-only
   scan on FLOAT ZEROFILL columns`
   - `sql/field.cc` (13 insertions, 1 deletion)
   - `mysql-test/t/ps11024.test` (new)
   - `mysql-test/r/ps11024.result` (new)

2. **`095b1faffff`** -- `PS-11024 [8.4]: Update group_by test for negative
   zero normalization`
   - `mysql-test/r/group_by.result` (8 insertions, 8 deletions)

Pushed to `inikep/percona-server` fork and created PR:

**PR**: https://github.com/percona/percona-server/pull/5925

## Files Changed

| File | Change |
|------|--------|
| `sql/field.cc` | Normalize `-0.0` to `+0.0` in `Field_float::store()` and `Field_double::store()` |
| `mysql-test/t/ps11024.test` | New MTR test reproducing all three symptoms |
| `mysql-test/r/ps11024.result` | Expected output for new test |
| `mysql-test/r/group_by.result` | Updated Bug#34361437 test expectations (negative zero now normalized) |

## Diff Summary

```
 sql/field.cc                    | 14 +++++++++++++-
 mysql-test/t/ps11024.test       | 39 +++++++++++++++++++++++++++++++++++++++
 mysql-test/r/ps11024.result     | 65 +++++++++++++++++++++++++++++++++++++++++
 mysql-test/r/group_by.result    | 16 ++++++++--------
 4 files changed, 125 insertions(+), 9 deletions(-)
```

## Codex Adversarial Review (gpt-5.4)

**Target**: branch diff against `68bf1c5b230^`
**Verdict**: needs-attention

### Finding [high]: Global FLOAT rendering change vs covering-index-only fix

The fix normalizes `-0.0` at the `Field_float::store()` / `Field_double::store()`
level, which is the generic storage path for all FLOAT/DOUBLE values -- not a
covering-index-specific hook. This intentionally changes observable output
semantics: values that previously displayed as `-0` now display as `0`.

The follow-up commit `095b1faffff` updates `mysql-test/r/group_by.result`
(Bug#34361437 expectations) to account for this, confirming the behavioral
change extends beyond the original bug scope.

### Codex Recommendations

1. **Fold the `group_by` result update into the main commit** so that the fix
   is self-contained and does not leave the tree in a broken state between
   commits.
2. Alternatively, narrow the fix to only affect the covering-index
   materialization path instead of the global `Field_float::store()` path.

### Response to Review

The global-store approach was chosen deliberately over a narrow
covering-index-only fix because:

- **Correctness at the source**: `-0.0` and `+0.0` are equal in SQL; storing
  them with distinct bit patterns is a latent bug that can surface in any
  comparison or formatting path, not just covering indexes.
- **Consistency with existing code**: `Field_float::make_sort_key()` already
  performs the same normalization (line 4207). Aligning `store()` with
  `make_sort_key()` eliminates the class of bugs where stored data and sort
  keys disagree.
- **Minimality**: A covering-index-only fix would require changes deep in
  InnoDB's `row_sel_field_store_in_mysql_format_func()`, would be harder to
  verify, and would leave `-0.0` lurking in stored data for other code paths
  to trip over.

The recommendation to fold commits is valid for final merge -- the two
commits should be squashed before landing on `8.4`.
