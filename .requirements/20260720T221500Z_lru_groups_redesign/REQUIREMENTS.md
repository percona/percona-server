# Grouped LRU redesign (PS-11141, 8.4)

## As Is

InnoDB's buffer pool LRU is a single intrusive doubly-linked list of
`buf_page_t` (`UT_LIST_BASE_NODE_T(buf_page_t, LRU) buf_pool_t::LRU`, node
member `buf_page_t::LRU` at `storage/innobase/include/buf0buf.h:1675`). One
per-pool `LRU_list_mutex` (`buf0buf.h:2371`) serializes every mutation:
inserting a freshly-read page, moving a page to MRU on make-young, removing a
page on eviction/free, and walking the `LRU_old` boundary
(`buf_pool_t::LRU_old`, `buf0buf.h:2561`, plus `LRU_old_len` at `:2567`) that
splits the list into a young and an old sublist (default ~37% old, via
`LRU_old_ratio`).

On top of `PS-11141-8.4-lru-mutex-narrow` (narrows the mutex's hold window
around `buf_page_init_for_read`), branch `PS-11141-8.4-lru-make-young-defer`
adds a lock-free per-pool Treiber-stack promotion queue
(`buf_pool_t::LRU_promote_head`, `buf_page_t::LRU_promote_next` /
`LRU_in_promote_queue`) so make-young no longer takes `LRU_list_mutex`
immediately; instead `buf_LRU_enqueue_promote()` pushes onto the queue, and
`buf_LRU_drain_promote_queue()` (`storage/innobase/buf/buf0lru.cc:2034`)
periodically detaches the whole queue under `LRU_list_mutex` and, via
`buf_LRU_promote_block_batched()` (`buf0lru.cc:1973`), moves each drained page
individually to the MRU head with a single deferred `buf_LRU_old_adjust_len()`
fix-up at the end of the batch.

These two branches are merged into working branch `PS-11141-8.4-lru-groups`
(merge commit `6ad5d6071a4`, already pushed to the `pawel` remote). Even with
deferred/batched promotion, every LRU mutation still ultimately serializes on
the single `LRU_list_mutex` — this task attacks that structurally.

## To Be

The LRU list becomes a list of **groups** of up to `BUF_LRU_GROUP_SIZE` (K, a
compile-time constant) pages:

- `buf_pool_t::LRU` becomes a list of `buf_lru_group_t` nodes; `LRU_list_mutex`
  protects only the links **between** groups.
- Each `buf_lru_group_t` owns a mutex protecting its own contents (its page
  slots, count, `old` flag); page-level operations that stay within one group
  use only that group's mutex, not `LRU_list_mutex`.
- `LRU_old` becomes a group boundary (`buf_lru_group_t*`), splitting the list
  into young and old sublists of *groups*. `LRU_old_len` keeps counting pages
  (existing metrics/heuristics depend on a page count), but the boundary
  itself only moves in whole-group steps.
- Eviction picks a tail group and evicts best-effort: ready-for-replace pages
  are evicted, dirty pages flushed (as today), buffer-fixed pages skipped. The
  group shrinks as pages leave it; once empty it is unlinked from the group
  list — but only when `LRU_list_mutex` is held (see lock ordering below), so
  removal is deferred rather than inline.
- Make-young removes a page from its group under that group's mutex alone
  (the contention win: no `LRU_list_mutex`) and enqueues it into the existing
  promotion queue, unchanged.
- Draining the promotion queue builds **one new group** out of the drained
  pages and links that whole group at the LRU young head, instead of
  reinserting each page individually into the flat list.
- Freshly-read pages (not from promotion) accumulate into a partial "fill
  group" anchored at the `LRU_old` boundary (old-sublist head), preserving
  InnoDB's existing midpoint-insertion / scan-resistance guarantee. The fill
  group seals at K pages and a new one starts.

### Lock ordering (new `SYNC_BUF_LRU_GROUP` level)

```
LRU_list_mutex  >  buf_lru_group_t::mutex (SYNC_BUF_LRU_GROUP)  >  block mutex / zip_mutex
```

A holder of `LRU_list_mutex` may acquire a group mutex; the converse is
forbidden — a thread holding only a group mutex must never try to acquire
`LRU_list_mutex` (lock upgrade is illegal). This is why emptying a group under
its own mutex must defer group-unlink rather than do it inline.

## Requirements

1. Introduce `BUF_LRU_GROUP_SIZE` as a compile-time constant.
2. Introduce `buf_lru_group_t`: a mutex (new `SYNC_BUF_LRU_GROUP` latch
   level), a list-link to sibling groups, a fixed array of up to
   `BUF_LRU_GROUP_SIZE` page slots, a live-page count, and an `old` flag.
3. Register `SYNC_BUF_LRU_GROUP` in `storage/innobase/include/sync0types.h`
   between `SYNC_BUF_LRU_LIST` and `SYNC_BUF_BLOCK` (consistent with the
   ordering above), and update `LatchDebug` internals in `sync0debug.cc` per
   the enum's own maintenance comment.
4. Add `buf_page_t` back-pointer fields (owning group + slot index) so a page
   can find and lock its group without a list scan.
5. Provide an ordered, group-aware page iterator/visitor so the ~10
   page-granular flat-list traversal sites outside `buf0lru.cc` (dump, AHI
   drop, buddy relocate, `i_s.cc` LRU table, debug validators,
   `buf_get_total_list_len`, `buf_relocate`, the `LRUHp`/`LRUItr`
   hazard-pointer/iterator classes) can walk pages in LRU order without each
   learning group internals directly.
6. Rewrite `buf_LRU_add_block_low`, `buf_LRU_remove_block`,
   `buf_LRU_make_block_young`, `buf_LRU_make_block_old`,
   `buf_LRU_old_adjust_len`, `buf_LRU_old_init` in terms of groups per the
   semantics above (fill-group insertion, deferred empty-group unlink, group
   boundary walk).
7. Rewrite `buf_LRU_drain_promote_queue` / `buf_LRU_promote_block_batched` to
   build one new group from the drained batch and link it at the young head.
8. Rewrite the eviction scan (`buf_LRU_free_from_common_LRU_list`,
   `buf_flush_LRU_list_batch`) to operate on tail groups with best-effort
   partial eviction.
9. Preserve the existing per-page `bpage->old` flag semantics (still readable
   by `i_s` `IS_OLD`, `buf_page_peek_if_too_old`, flush neighbor checks)
   derived from the owning group's side of the `LRU_old` boundary.
10. Preserve all existing sysvars (`innodb_old_blocks_pct`,
    `innodb_old_blocks_time`, `innodb_lru_make_young_drain_threshold`) and
    their observable behavior/tuning range.
11. Debug validators (`buf_LRU_validate_instance`, `buf_validate`) must assert
    the new invariants: every page belongs to exactly one group, every group's
    `n_pages` matches its live slot count, group order matches page LRU order,
    `LRU_old` sits at a group boundary and its side-of-boundary agrees with
    each member page's `old` flag.

## Acceptance Criteria

1. `BUF_LRU_GROUP_SIZE` and `buf_lru_group_t` compile standalone (P1) without
   changing `buf_pool_t::LRU`'s element type yet — additive foundation only.
2. `SYNC_BUF_LRU_GROUP` is accepted by `LatchDebug`; a debug build with
   `--innodb-sync-debug` does not flag the new level as unknown.
3. After the full rewrite, a debug build with `UNIV_DEBUG` (and ideally
   `UNIV_LRU_DEBUG`) passes `buf_LRU_validate` / `buf_validate` under normal
   server operation (startup, DML load, shutdown).
4. A thread never holds a group mutex while attempting to acquire
   `LRU_list_mutex` (enforced by the sync-level ordering + code review; no
   automatic detector exists for this specific direction beyond level
   ordering, so this is also a manual review criterion).
5. `buf_page_peek_if_too_old`, `i_s.INNODB_BUFFER_PAGE.IS_OLD`, and
   `i_s.INNODB_BUFFER_POOL_STATS.OLD_LRU_LEN` report values consistent with
   pre-change semantics (a page's `old`-ness matches which side of `LRU_old`
   its group is on; `OLD_LRU_LEN` is a page count, not a group count).
6. `mysql-test/suite/sys_vars/t/innodb_old_blocks_pct_basic.test`,
   `innodb_old_blocks_time_basic.test`, `innodb_lru_make_young_drain_threshold_basic.test`,
   `mysql-test/suite/innodb_zip/t/lru_mutex_narrow_stress_debug.test`,
   `mysql-test/suite/percona_innodb/t/lru_flusher_debug.test`, and
   `mysql-test/r/all_persisted_variables.result` all pass.
7. A newly-read page enters the LRU via the old-head fill group, not the MRU
   head (verifiable via `i_s.INNODB_BUFFER_PAGE_LRU` ordering + `IS_OLD` on a
   freshly-read, never-accessed page).
8. Eviction of a tail group with one buffer-fixed page still evicts the other
   ready pages in that group and does not stall (best-effort partial).

## Testing Plan

- **Compile-verify every step** against the Debug + `WITH_DEBUG=ON` Ninja
  build configured in `build/` in this worktree (target `mysqld` first, since
  it covers all touched translation units; `ninja` for the rest as needed for
  MTR).
- **Debug validators as the primary correctness oracle**: `buf_LRU_validate`
  / `buf_LRU_validate_instance` (already exist, extended per Requirement 11)
  run automatically in debug builds and are the cheapest way to catch a
  broken group/page invariant during manual smoke testing and MTR.
- **Focused MTR** (existing suites, no new sysvar introduced so no new sysvar
  matrix per the user's testing rule):
  `sys_vars/innodb_old_blocks_pct_basic`,
  `sys_vars/innodb_old_blocks_time_basic`,
  `sys_vars/innodb_lru_make_young_drain_threshold_basic`,
  `innodb_zip/lru_mutex_narrow_stress_debug`,
  `percona_innodb/lru_flusher_debug`, `all_persisted_variables`.
- **Manual smoke**: start a debug server with `--innodb-sync-debug`, run a
  DML workload larger than the buffer pool (forces eviction, promotion,
  fill-group sealing, and old/young boundary movement), then run
  `SELECT * FROM information_schema.INNODB_BUFFER_PAGE_LRU` and
  `SHOW ENGINE INNODB STATUS` to sanity-check `OLD_LRU_LEN` / made-young
  counters, then clean shutdown to exercise `buf_validate`.
- **Adversarial review of the lock-ordering change** specifically, mirroring
  the pattern from `[[fix_lazy_mutex_own]]`-style prior tasks in this repo:
  after implementing, do a dedicated pass over every site that holds a group
  mutex, checking none of them can transitively wait on `LRU_list_mutex` or a
  frame rw-lock (extending the mutex-narrow branch's
  `rw_lock_assert_wait_allowed()` coverage to the group mutex).

## Implementation Plan

Given the size and correctness-criticality of this change, it proceeds in
phases; each phase must compile and (where applicable) pass its own focused
test before the next begins. **This task's committed scope for this session
is Phase P1** (additive foundation) — P2 onward is the coupled, high-risk
rewrite of `buf_pool_t::LRU`'s element type and is deliberately left as
follow-on work with its own future `REQUIREMENTS.md`, so that a change of
this blast radius is not rushed to fit a single sitting.

- **P0 (done):** Branch `PS-11141-8.4-lru-groups` created from
  `PS-11141-8.4-lru-mutex-narrow`; `PS-11141-8.4-lru-make-young-defer` merged
  in (commit `6ad5d6071a4`); a latent persisted-variables assert bug from the
  merge (491→492 bumped independently by both branches; true count 493) fixed
  and committed; branch pushed to `pawel` remote.
- **P1 Foundation (this session's target):**
  1. Add `BUF_LRU_GROUP_SIZE` constant and `buf_lru_group_t` struct to
     `buf0buf.h`, additive only (not yet used by `buf_pool_t::LRU`). Compile
     `ninja mysqld`.
  2. Add `SYNC_BUF_LRU_GROUP` to `sync0types.h`'s `latch_level_t` and update
     `sync0debug.cc` `LatchDebug` internals per its own comment. Compile.
  3. Add `buf_page_t` back-pointer fields (owning group pointer + slot
     index), unused so far. Compile.
  4. Add the ordered group-aware page iterator/visitor (operating on the
     *existing* flat `buf_pool_t::LRU`, since groups aren't wired in yet, so
     it is a no-op-shaped refactor validated against current behavior).
     Compile and run a quick manual smoke (start server, run
     `SHOW ENGINE INNODB STATUS`).
  5. Run `all_persisted_variables` and the four LRU-related MTR suites listed
     above to confirm no regression from the additive changes alone.
- **P2 Core ops (future task):** Wire `buf_pool_t::LRU` to
  `buf_lru_group_t`; rewrite `buf_LRU_add_block_low` (fill-group at old
  head), `buf_LRU_remove_block` (vacate slot, deferred empty-group unlink),
  `buf_LRU_make_block_young`/`_old`, `buf_LRU_old_adjust_len`/`_init` on group
  boundaries.
- **P3 Promotion drain (future task):** Rewrite
  `buf_LRU_drain_promote_queue`/`buf_LRU_promote_block_batched` to build and
  link one new group per drain instead of per-page reinsertion.
- **P4 Flush/eviction batch (future task):** `buf_flush_LRU_list_batch` /
  `buf_LRU_free_from_common_LRU_list` walk tail groups, best-effort partial
  eviction.
- **P5 Consumers (future task):** Migrate the ~10 flat-list walk sites onto
  the ordered iterator from P1; `buf_relocate`; `LRUHp`/`LRUItr` rebased on
  groups; `i_s.cc` / metrics.
- **P6 Validators + full MTR pass (future task):** Extend
  `buf_LRU_validate_instance` per Requirement 11; full regression pass.
