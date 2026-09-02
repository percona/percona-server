# Persisting the HNSW Vector Index

*Percona Server · InnoDB · `KEY (v) TYPE hnsw`*

---

# Part I — What this is

## 1. The problem, and the shape of the answer

An HNSW index is a graph, and a graph is a memory structure. Every node holds a vector and a
list of pointers to its neighbours, and a search walks those pointers from an entry point down
to the closest node it can find. Nothing about that shape wants to live on disk.

So the question this work answers is not "how do we implement HNSW" — that is done, and not by
us. It is: **what is the graph's durable form, and how does a graph shared by every session obey
the transaction rules of a database that assumes per-session snapshots?**

Three bodies of work meet here.

### What already exists

The vector *type* and its surface syntax are in place. A user can already write:

```sql
CREATE TABLE t (id BIGINT UNSIGNED PRIMARY KEY,
                v  VECTOR(4) NOT NULL,
                KEY vk (v) TYPE hnsw WITH (M = 16));
```

and the parser, the data dictionary and the distance kernels all understand it.

| Component | What it provides |
|---|---|
| **Vector data type** | `VECTOR(n)` columns, `STRING_TO_VECTOR()`, `DISTANCE()` |
| **Distance functions** | L2 / inner-product kernels, with AVX-512 paths |
| **Index syntax** (PS-11203) | `KEY (v) TYPE hnsw WITH (M = 16)`, its validation and error codes |
| **Data dictionary** (PS-11264) | the index `TYPE` and its `WITH (…)` parameters are stored in, and restored from, the DD |

There is no `VECTOR KEY` keyword: a key becomes `KEYTYPE_VECTOR` because it named a `TYPE` that
is not BTREE, RTREE or HASH. And until this work, that index did nothing — the parameters were
parsed, validated and discarded, and every query fell back to a full scan with a sort.

### What Dmitry's work gives us

`vector-common/hnsw.h` is the algorithm, and it is deliberately storage-agnostic. It gives us
four things:

- **The graph itself** — `insert()`, `k_nn_search()` returning `SearchHit {id, base_pk}`, and a
  streaming `nn_search_start` / `nn_search_next`.
- **A persistence seam.** The class takes a `Persistor` as a *template parameter* and calls it
  back after it has mutated the graph. It never decides what a row looks like or when anything
  commits.
- **Lazy loading.** `init_from_entry_point()` loads exactly one node; everything else faults in
  through `load_node_cb` as a traversal reaches it. A cold index does not have to be read whole.
- **Thread safety.** Concurrent `insert()` and search on one instance, and concurrent faulting
  of the same node, are the class's own problem and it solves them — `m_global_lock`,
  `m_entry_point_lock`, and striped per-node locks.

What it explicitly does *not* do is store anything, including its own parameters:

> *Index metadata is not persisted by HNSW itself. The class users must store it alongside the
> graph (at minimum: vector dimensions, M, distance kind).*

One thing this design still needs from the class and does not have: `insert()` cannot report
failure — it returns `void` (§27).

### What this work adds

Everything between that class and a working index. Four problems, in the order they bite:

**Persistence.** Each vector index gets an InnoDB table of its own — one row per node, holding
the vector, the neighbour list, the level, and the primary key of the base row the node
describes. That table is the graph's only durable form; the graph is rebuilt from it, lazily,
whenever an index is opened. This is what makes an index survive a restart.

**MVCC.** The graph is one shared structure, but every session reads through its own snapshot.
A node cannot be told to "look old", so the graph never decides visibility — it only ever
*proposes candidates*. Each candidate is then resolved against the reader's own view by reading
the base row, and a hidden label column ties a row version to the node that describes it.
A stale node fails that comparison and is skipped.

**Concurrency and durability.** The aux writes ride a sub-transaction that commits *before* the
user's, so a graph that memory already holds is never missing from disk. Committing per callback
rather than per statement is what keeps concurrent inserts from deadlocking on each other's
neighbour rows, and the redo flush that would otherwise cost is unnecessary because the user's
own commit flushes past our LSN.

**DDL.** Create, drop, rename, truncate, rebuild and `ALTER` all have to leave the aux and the
base table agreeing, and the operations that cannot be made to agree are refused rather than
half-supported.

### How it fits together

```
        SQL           CREATE TABLE … KEY (v) TYPE hnsw       SELECT … ORDER BY DISTANCE(…) LIMIT k
         │                        │                                        │
         ▼                        ▼                                        ▼
   ┌───────────────────────────────────────────────────────────────────────────────┐
   │  InnoDB handler        ha_innobase::vec_init / vec_read_first / vec_read_next  │
   └───────────────────────────────────────────────────────────────────────────────┘
         │                                                    │
         │ base row, by primary key,                          │ candidates
         │ under the SESSION'S read view  ◄── MVCC ───────────┤ (node id + base_pk)
         ▼                                                    ▼
   ┌──────────────────────┐                     ┌──────────────────────────────────┐
   │  base table          │                     │  HNSW graph          IN MEMORY   │
   │  id │ v │ …          │                     │  shared by all sessions          │
   │  + percona_vec_aux_id│  ◄── the label ───► │  nodes, neighbours, entry point  │
   └──────────────────────┘                     └──────────────────────────────────┘
                                                          │  Persistor callbacks
                                                          ▼
                                                ┌──────────────────────────────────┐
                                                │  percona_vec_hnsw_<tid>_<iid>    │
                                                │  one row per node      ON DISK   │
                                                └──────────────────────────────────┘
```

The graph is a **cache**; the aux table is the truth. The hidden `percona_vec_aux_id` column is
what lets a base row and a graph node recognise each other, and it is the whole of the MVCC
story.

---

## 2. Terminology

| Term | Meaning |
|---|---|
| **node** | One vector in the graph. The unit HNSW inserts, links and searches. |
| **label** | A node's identity — the `id` the HNSW class takes. A `BIGINT UNSIGNED` from a per-index counter. Never reused, never zero (0 is the class's empty-slot sentinel). |
| **base_pk** | The PRIMARY KEY of the base-table row a node describes. |
| **layer / level** | How many HNSW layers a node participates in. Assigned randomly at insert; geometrically distributed, so high layers are rare. |
| **neighbours** | A node's outgoing edges, one list per layer. The graph *is* these lists. |
| **entry point** | The node searches start from — the first node inserted on the topmost layer. The analogue of a B-tree root. |
| **aux table** | The InnoDB table holding one row per node. |
| **fresh label** | Changing a row's vector mints a *new* label rather than editing the existing node. |
| **orphan** | A node that no visible base row claims. |

---

# Part II — User interface, and what lands on disk

## 3. What you can write, and what is refused

The surface is deliberately small. A vector index is declared like any other key, and the only
statement that reads it is `ORDER BY DISTANCE(...) LIMIT k`.

```sql
CREATE TABLE t (id BIGINT UNSIGNED PRIMARY KEY,
                v  VECTOR(4) NOT NULL,
                KEY vk (v) TYPE hnsw WITH (M = 16));

SELECT id FROM t ORDER BY DISTANCE(v, STRING_TO_VECTOR('[1,0,0,0]'), 'EUCLIDEAN') LIMIT 5;
```

### DDL

| Operation | Effect on the index |
|---|---|
| `CREATE TABLE … KEY (v) TYPE hnsw` | adds the hidden label column, creates the aux table, registers it in the DD |
| `ALTER TABLE … ADD KEY (v) TYPE hnsw` | COPY only; builds the graph from a clustered scan (§11) |
| `DROP INDEX` | drops that index's aux table; the hidden column is **retained** |
| `DROP TABLE` | drops the aux table with the parent |
| `TRUNCATE TABLE` | drop and recreate — the aux comes back empty and the label counter restarts |
| `RENAME TABLE` | same schema: nothing to do. Cross-schema: the aux moves with the parent |
| `OPTIMIZE` / rebuilding `ALTER` | the base table is rebuilt and the graph rebuilt with it; labels are carried forward, not reissued |

### What is refused, and why

Each of these is refused because the alternative is an index that is silently wrong.

| Refused | Reason |
|---|---|
| `IMPORT` / `DISCARD TABLESPACE` | the imported rows have no aux rows describing them; the index would silently omit every one |
| `ALTER … ALGORITHM=INSTANT` (ADD/DROP COLUMN) | an instant change does not rewrite rows, so the hidden label column cannot be maintained |
| `ALGORITHM=INPLACE` for the *first* `ADD KEY` | adding the hidden column is a table rebuild by definition |
| `LOCK=NONE` on `ADD KEY` | concurrent DML during the build has nowhere to record itself (§11) |
| Changing a vector index's `TYPE` in place | the stored aux rows belong to the old implementation; drop and re-add instead |
| `ON UPDATE CASCADE` / `ON DELETE CASCADE` into a vector-indexed table | a cascade bypasses the row path that maintains the aux (§22) |
| A second vector index on one table | one hidden label column cannot serve two graphs (§3) |
| A vector index on a virtual column, a partitioned table, or a temporary table | refused above InnoDB, by the server |
| A nullable vector column | rejected at DDL, so an indexed vector always has a value |

Nothing about the aux table's name is reserved from users: a table that merely begins with
`percona_vec_` is not mistaken for one, because the name is recognised by parsing its whole
shape — the prefix, a known index-type token, then two object ids.

## 4. What appears on disk

Creating that table produces **two** tablespaces and one column the user never sees.

```
   t.ibd                                the base table
      id │ v │ percona_vec_aux_id       ← hidden: invisible to SELECT *, SHOW COLUMNS,
                                          and to any application that did not ask for it

   percona_vec_hnsw_<table_id>_<index_id>.ibd      one per vector index
      one row per graph node, plus record 0 for the entry point
```

The aux table is a real InnoDB table registered in the data dictionary, so it takes part in
crash recovery, DDL logging and tablespace management like any other. It is hidden from
`SHOW TABLES` and `INFORMATION_SCHEMA.TABLES`, and visible in `INFORMATION_SCHEMA.INNODB_TABLES`
— the same treatment FTS auxiliary tables get.

The hidden column is the `FTS_DOC_ID` device: an ordinary `BIGINT UNSIGNED` column, versioned by
undo like any other, which is precisely what makes it usable for visibility decisions. It
carries **no unique index** — nothing looks a row up by label, only ever the reverse — and it is
retained when the vector index is dropped, so re-adding an index does not have to rebuild the
table.

---

# Part III — High level design

## 5. Where the objects live

Three layers, each owning one thing.

```
   dict_table_t  (the user's table)
        │
        └── dict_index_t  ── the vector index: its id, its TYPE
                 │
                 └── Vec_runtime *vec ──┐   per-index, created on first use
                                        │
                              ┌─────────┴─────────┐
                              │  the HNSW graph   │   in memory
                              │  + arena          │
                              │  + Persistor      │
                              └───────────────────┘
                                        │  callbacks
                                        ▼
                              percona_vec_hnsw_<tid>_<iid>    on disk
```

**`dict_index_t::vec`** is a pointer to the runtime. It is `nullptr` until something first opens
the index, and it owns the `HNSW` object, the arena its nodes are allocated from, the persistor,
and the index parameters read back from the DD.

### Why the runtime hangs off the index, not the table

Everything it holds is a property of one index, and a table will eventually carry more than one
vector index (§18).

Two consequences follow:

**The pointer must be raw.** `dict_index_t` is never constructed or destructed — the memory is
zeroed and `dict_mem_fill_index_struct()` stands in for a constructor — so `Vec_runtime *vec` is
zero-initialised by that memset and released by hand in `dict_mem_index_free()`, exactly as
`destroy_fields_array()` already is. No smart pointer, no non-POD member.

**A vector index still carries its key part.** `DICT_VECTOR` is a bit in `dict_index_t::type`,
beside `DICT_FTS` and `DICT_SPATIAL` — the family with no real B-tree — and `dict0dd.cc`
asserts:

```c
ut_ad(!!(type & (DICT_FTS | DICT_VECTOR)) == (n_uniq == 0));
```

That constrains `n_uniq`, not `n_fields`. `dict_index_add_col()` runs for a vector key on both
the CREATE and DD-open paths, so `index->get_field(0)->col` **is** the indexed column. Nothing
may search for it instead: `VECTOR`, `BLOB`, `TEXT` and `JSON` all collapse to `DATA_BLOB`, so no
type test can distinguish them. Its `prefix_len` is 1 and means nothing.


**The aux table** is named `percona_vec_hnsw_<table_id>_<index_id>`, hidden from `SHOW TABLES` and
`INFORMATION_SCHEMA.TABLES`, visible in `INNODB_TABLES`. Nothing about the name is reserved from
users. `vec_aux_parse_table_name()` recognises an aux table by parsing the whole shape — the
`percona_vec_` prefix, a known index-type token, then two object ids — so a user table that
merely begins with those characters is never mistaken for one.

**A hidden column on the base table** carries the label: `percona_vec_aux_id BIGINT UNSIGNED`,
invisible to `SELECT *` and `SHOW COLUMNS`. This is the same device FTS uses with
`FTS_DOC_ID`. It is what lets a row and its node find each other.

---

## 6. The aux table

**One row per node, updated in place.**

```sql
CREATE TABLE percona_vec_hnsw_<tid>_<iid> (
  id        BIGINT UNSIGNED PRIMARY KEY,  -- the label
  vec       BLOB NOT NULL,                -- dims * 4 bytes
  base_pk   BIGINT UNSIGNED NOT NULL,     -- the base row this node describes
  level     TINYINT NOT NULL,             -- top layer for this node
  neighbors BLOB NOT NULL                 -- neighbour ids, 0 = empty slot
)
```

`PRIMARY KEY(id)` is load-bearing. Nodes are fetched one at a time, by label (§11), so the
label must lead the key.

**The system columns are InnoDB's, not ours.** Nothing above adds them, and nothing needs to: the
aux is created through `row_create_table_for_mysql`, which calls `dict_table_add_system_columns()`
for it exactly as for a user table. Because the clustered index is `UNIQUE` on `id`, `DB_ROW_ID`
is not among them — that one is added only to an index that is *not* unique — so the record is:

```
id │ DB_TRX_ID │ DB_ROLL_PTR │ vec │ base_pk │ level │ neighbors
```

Two things follow, and both are load-bearing elsewhere. The aux is redo- and undo-logged and
MVCC-capable like any other table, which is what lets §14 recover it with no vector-specific
machinery. And **no aux field may be addressed by its user-column ordinal**, because two system
columns sit between the key and the payload (§21).

**Record 0 holds the entry point.** Since 0 is never a real label, that row is free to use as
index metadata:

```
id = 0    base_pk = <entry point node id>      ← the payload
          vec = ''    level = 0    neighbors = ''
```

Empty blobs are not NULL, so this needs no schema change, and reading it is a lookup of the
leftmost record of the clustered index.

Two things follow. **No record 0 means the index is empty**, not broken — a freshly created
index has no nodes and no entry point until the first insert. And **record 0 must be written
after the node it names**, since it is a reference like any neighbour list (§17).

---

## 7. INSERT

A user runs:

```sql
INSERT INTO t (id, v) VALUES (7, STRING_TO_VECTOR('[1,0,0,0]'));
```

**1. The server assigns a label.** Before the base row is written, the counter hands out the
next label — say 10 — and it is stamped into the row's hidden `percona_vec_aux_id` column. The row is
inserted by the ordinary InnoDB path, on the user's transaction.

**2. We start a sub-transaction and call the graph.**

```c
Context ctx{ trx_allocate_for_background(), aux, thd, DB_SUCCESS };
hnsw.insert(/*id=*/10, /*base_pk=*/7, vector_bytes, &ctx);
```

**3. The class does its work, then calls us back.** It picks a layer for the new node, searches
for its nearest neighbours, links them mutually, and prunes each affected neighbour's edge list
back to `get_Mmax(layer)` — which is `2 * M` on layer 0, not `M`. Only when all of that is
done — the graph fully mutated — does it fire the callbacks of §20, in this order:

```
insert_cb(ctx, 10, 7, [1,0,0,0], layer, nbrs)   →  vec_aux_insert      row 10
update_neighbors_cb(ctx, 5,  nbrs_of_5)         →  vec_aux_update_row  row 5,  neighbors
update_neighbors_cb(ctx, 12, nbrs_of_12)        →  vec_aux_update_row  row 12, neighbors
      … one per neighbour whose edges changed …
update_entry_point_cb(ctx, 10)                  →  record 0, upsert    (only if layer is new)
```

So one INSERT produces **one aux insert plus roughly M aux updates**, each of them a row
operation on `ctx->trx` as shown in §21. Note the callbacks fire *after* all graph mutations, not
at each one, so the neighbour list we serialise is that node's final state for this insert.

**4. Each callback commits the sub-transaction, and starts it again.**

```c
static void vec_ctx_step_commit(Vec_ctx *ctx) {
  trx_commit_for_mysql(ctx->trx);
  trx_start_internal(ctx->trx, UT_LOCATION_HERE);
}
```

Not one commit per INSERT — **one per aux row**. That is a deliberate reversal of the obvious
design, and the reason is deadlock.

The locks an insert takes are on the aux rows of the neighbours it rewires, and *the graph picks
those neighbours, not the user*. Two concurrent inserts that land near each other in vector space
contend on the same rows, in an order neither controls. With one transaction spanning all the
callbacks, each insert holds every lock it has taken so far while waiting for the next — the
textbook deadlock shape, and under load not a theoretical one: 8 connections inserting 20000 rows
committed **5074** of them, the rest dying as deadlock victims.

Committing per callback removes the shape instead of mitigating it. A row lock now lives only as
long as the callback that took it, so a sub-transaction holds **at most one lock at a time and
never waits while holding one**. A cycle needs two waiters each holding what the other wants, and
there is no longer a state in which that can be true. All 20000 rows land.

**The sub-transaction never flushes the redo log**, which is what makes those extra commits
affordable: `flush_log_later` is set on it once, and durability comes from LSN ordering instead.
That is not a shortcut, and §14 has both the argument and the measurement — the entire cost of
committing per callback turned out to be the fsync.

**The closing commit is a tidy-up.** By the time `insert()` returns, every aux row is already
committed; `trx_commit_for_mysql` just ends whatever the last `trx_start_internal` opened, which
is often empty.

On failure the first error is in `ctx.err`, later callbacks short-circuit, and the statement
fails — taking the base row with it. The rollback is `trx_rollback_to_savepoint(aux_trx,
nullptr)`, not `trx_rollback_for_mysql`: this is a *background* transaction and is not in the
MySQL trx list the latter asserts membership of, which is why `fts_sql_rollback` makes the same
call. It can only undo the callback that failed — everything before it is committed and stands.
Those rows are the orphan §13 accepts, and it is the right direction to diverge in, because the
in-memory rewire cannot be undone either: rolling the whole insert back would leave memory
holding a node the aux had thrown away.

There is no NULL case to handle anywhere in this design: `sql_table.cc` rejects a vector
index on a nullable column, so an indexed vector always has a value.

---

## 8. UPDATE

**Changing the vector** does not edit the node. Nodes are immutable: HNSW cannot safely move a
point once its neighbours are linked to it. So a new label is minted and inserted, and the old
node is left alone.

```sql
UPDATE t SET v = STRING_TO_VECTOR('[0,1,0,0]') WHERE id = 7;
```

```
base row 7:  percona_vec_aux_id  10 → 20
graph:       node 10 stays (vector [1,0,0,0])
             node 20 added (vector [0,1,0,0])
aux:         row 10 untouched; row 20 inserted; ~M neighbour rows updated
```

Both nodes now carry `base_pk = 7`. §12 explains how a query tells them apart.

**Changing only the primary key** does not touch the graph at all — the vector has not moved.
It does need the row's *current* node to be re-pointed, which is one more `vec_aux_update_row`
on the sub-transaction: the same self-positioned `upd_node` + `row_upd_step` path §21 describes,
addressing the aux row by label and writing only `base_pk`. No SQL is parsed and no cursor is
searched for; we already know the key.

Older nodes for the same row keep the stale `base_pk`; §12 shows why that is harmless.

---

## 9. DELETE

```sql
DELETE FROM t WHERE id = 7;
```

**Nothing happens to the graph, and nothing is written to the aux table.**

That is not an omission. A transaction that started before this delete is still entitled to see
row 7, and the only way it can reach the row through the index is via node 10 — so the node
must stay. Removing it would break isolation, not tidy up.

Deleted nodes are filtered at read time (§12), so they can never be *returned*; they simply
remain as part of the graph's structure, still usable as routers during traversal.

The cost is that dead nodes accumulate; §18 covers what that means and how it is eventually
reclaimed.

---

## 10. SELECT

```sql
SELECT id FROM t ORDER BY DISTANCE(v, STRING_TO_VECTOR('[1,0,0,0]'), 'EUCLIDEAN') LIMIT 5;
```

```
id  select_type  table  type         key  Extra
1   SIMPLE       t      vector_knn   vk   NULL      <- index used, no filesort
```

Two things have to be true for that plan: the query has to be one the index can honestly answer,
and there has to be a handler API capable of expressing the question. This section covers both,
then how a candidate becomes a row.

### Which SELECTs use the index, and which must not

**This is a contract, not an optimisation.** An approximate index returns *near* neighbours, not
*all* qualifying rows. Used to satisfy a predicate, it would silently drop rows the user asked
for — a wrong answer with nothing to signal it. So the index is used only where the user has
explicitly accepted approximation, and that shape is `ORDER BY <distance> LIMIT k`:

> a single-table query block, one **ascending** `ORDER BY` expression that is a distance call over
> the indexed vector column, with a **constant** query vector, a **finite `LIMIT`**, and a metric
> the graph can serve.

| query | plan | why |
|---|---|---|
| `ORDER BY DISTANCE(v, <const>, 'EUCLIDEAN') LIMIT 3` | **`vector_knn`** | the canonical shape |
| `... 'EUCLIDEAN_SQUARED' LIMIT 3` | **`vector_knn`** | monotonic transform of the graph's own metric — same ordering |
| `ORDER BY DISTANCE(<const>, v, ...) LIMIT 3` | **`vector_knn`** | argument order does not matter |
| `WHERE k = 1 ORDER BY DISTANCE(...) LIMIT 3` | **`vector_knn`** | the filter sits *above* the scan, which simply continues |
| `ORDER BY DISTANCE(...)` — no `LIMIT` | `ALL` + filesort | asks for a total ordering; an approximate index cannot give one |
| `... LIMIT 3` but `DESC` | `ALL` + filesort | farthest-first is not a question HNSW answers |
| `... 'MANHATTAN' LIMIT 3` | `ALL` + filesort | the graph was built under L2; another metric would rank by a distance it does not hold |
| `WHERE DISTANCE(...) <= 2` | `ALL` + filesort | **the important one**: a predicate needs every qualifying row, and approximation would drop some silently |
| `ORDER BY DISTANCE(v, v, ...) LIMIT 3` | `ALL` + filesort | query vector is not constant — nothing to search the graph *for* |
| `IGNORE INDEX (vk)` | `ALL` + filesort | the user asked for the exact path |

Every row of that table is asserted in `vector_search.test`. The exact path is always available
and always correct; the index path is the opt-in.

Recognition itself is mechanical: `Item_func_vector_distance` registers with the query block at
itemization into a `vector_func_list` — the `ftfunc_list` analog, carried across derived-table and
subquery merges the same way — and `JOIN::optimize` calls `optimize_vector_query`, which switches
the plan to `JT_VECTOR` when the shape matches. Cost plays no part: `dict_stats_should_ignore_index()`
skips vector indexes, so there is no cardinality to weigh against a scan. Once matched, `LIMIT` is
pushed into the access path and the now-redundant sort is elided.

### The handler API, and why it has to be a new one

`VectorSearchIterator` drives three methods — the vector analog of the `ft_init` family:

| handler method | what it does |
|---|---|
| `vec_init()` | `rnd_init()`: sets up the clustered-index positioning the row fetch needs |
| `vec_read_first(item, buf, limit)` | evaluates the query vector, runs the first search with `k = LIMIT`, returns the first row |
| `vec_read_next(buf)` | returns the next row, taking the next candidate from the open scan |

They exist because the ordinary index API **cannot express the question**. `index_read` takes a
key and a comparison operator; `index_next` walks in key order; `records_in_range` estimates a
range. "The k nearest to q" has no key to seek, no order to walk and no range to estimate.
FULLTEXT hit the same wall, which is why `ft_init` / `ft_read` exist as a separate family beside
`index_*` and `rnd_*` rather than as a mode of them.

This is also why `index_flags()` returns 0 for a vector index: it can serve no ordinary read, and
saying otherwise is what let the optimizer pick it for `COUNT(*)` and scan a B-tree that is not
there.

### How a candidate becomes a row

`vec_read_next` is where the graph meets MVCC:

```
1. take the next candidate {node id, base_pk} from the scan
2. build a one-field clustered search tuple from base_pk
3. row_search_for_mysql(..., ROW_SEL_EXACT) under the SESSION'S OWN read view
       not found / invisible?              -> SKIP, take the next candidate
       visible, but its label != node id?  -> SKIP, take the next candidate
       otherwise                           -> return the row
4. no more candidates? the scan's frontier is empty -> end of file
```

Step 3 **is** check ② of §12, and it needs no vector-specific machinery: an ordinary primary-key
read under the reader's own view already answers "may I see this row?". Uncommitted points from
other transactions, rows deleted after the graph saw them, and orphans from rolled-back inserts
all simply miss.

The `SKIP` matters more than it looks. Returning end-of-data on the first miss would truncate
results under concurrency — one invisible candidate would end the scan while visible rows were
still to come.

Check ① runs at the same point, on the record step 3 just read. The scan returns a node id per
candidate (§26); the visible row version carries the label it was stamped with. Equal means
the node still describes this version.

Reading the label belongs to the engine. `percona_vec_aux_id` exists only inside InnoDB and is
deliberately absent from the MySQL row template, so `row_sel_store_mysql_rec` takes it straight
off the clustered record into `prebuilt->vec_aux_id` — the same thing it already does for
`fts_doc_id`, for the same reason — and the handler compares. A mismatch is skipped exactly like
a check ② miss.

The label is always the returned row's: every `DB_SUCCESS` on this path goes through that one
function, whether the row came from the direct store, the adaptive-hash shortcut, or an index
condition, and `direction == 0` clears the fetch cache on entry.

### Why `LIMIT` is only the first batch

`LIMIT` does not bound the work, so the read path never asks the graph for "the k nearest" and
stops. `WHERE k = 1 ORDER BY DISTANCE(...) LIMIT 3` needs however many candidates it takes to find
three rows that match, because the filter sits above the iterator and may consume any number of
them — and MVCC consumes them too, since a candidate failing either check of §12 is skipped. Even
an unfiltered query can need more candidates than rows, because an updated row's stale node is one
of the things check ① drops.

So the scan is **streaming**, always: `nn_search_start` once, then `nn_search_next` per candidate.
The class keeps the traversal's visited set and its unexplored frontier in the scan, so continuing
costs only the nodes not yet seen, a candidate is never offered twice, and the scan ends when the
frontier empties rather than at some guessed bound.

`LIMIT` sizes the first batch and nothing more.

Streaming is also what keeps the ordering promise. `EXPLAIN` shows no sort, because the access
path guarantees ascending distance, and the scan enforces that across batch refills: a candidate
that would go backwards is dropped rather than emitted out of order.

`ef_search` comes from the session variable `innodb_hnsw_ef_search` (default 40); the effective
width is `max(ef_search, batch)`. It is per-query rather than per-index because the same index
serves a dashboard wanting speed and a batch job wanting recall.

The one caveat is the class's own: streaming "is not intended to scan the entire or large part of
the index", because an approximate search omits nodes. A `LIMIT` approaching the row count is
exactly that shape, and there is no cost model to refuse it (§31).

The graph walk is pure memory (plus node faults, §11). The only disk access per candidate is that
primary-key read — the same lookup any secondary index performs to return a row.

---

## 11. Lazy loading

**The graph is never loaded in one go.** Without this, the first query on an index after a
restart would read every node before returning a row.

Cold start does the minimum:

```
1. read dims, M, ef_construction from the DD
2. construct an empty HNSW with exactly those parameters
3. read record 0 -> the entry point label
       absent?  -> the index is empty; stop
4. init_from_entry_point(entry_label, &ctx)      loads exactly ONE node
```

The entry point is the graph's root, so that one node is enough to start traversing. Everything
else arrives through `load_node_cb`, which reads a single aux row by primary key and hands its
four fields back:

```c
h.load_set_layer(handle, level);     // must be first: it sizes the neighbour array
h.load_set_vec(handle, vec);
h.load_set_base_pk(handle, base_pk);
h.load_node_neighbors(handle, ids);  // must be last: allocates, and creates stubs
```

The order is part of the class's contract, not a style choice.

Loading a node creates **stubs** for each of its neighbours — nodes that exist by id and hold no
data, filled the first time a traversal reaches them. A search therefore descends one path from
the entry point to layer 0, faulting roughly one node per layer plus the neighbourhood it
examines at the bottom: for ten million rows at `M = 16`, on the order of six nodes.

Three properties decide how this behaves:

- **No read view is taken.** The graph is shared by every transaction rather than being
  per-transaction state, so there is no snapshot it could belong to; it always loads the latest
  row. A node whose statement later rolls back becomes an orphan, and the read path already
  filters orphans (§12).
- **This bounds latency, not memory.** A stub already allocates its node block including vector
  space; only the neighbour array is deferred. Nothing shrinks either — a node never returns to
  the unloaded state, and the arena has no per-block free (§22).
- **The parameters must match** what the index was built with. The class states that a mismatch
  in dimensions or `M` corrupts the graph or yields wrong results, which is why they come from
  the DD and never from a default.

The read behind that callback addresses every field through `dict_col_get_clust_pos()` rather
than by user-column ordinal — §6 shows why the two differ, and §21 what happens when they are
confused.

---

## 12. How MVCC works

There is no vector-specific visibility machinery. The graph is a **candidate generator** and
the base table is the authority: the graph proposes, the row decides.

This works because of four properties that each exist for their own reason:

| Property | Why it exists | What it also gives |
|---|---|---|
| a vector change mints a fresh label | nodes are immutable | every historical vector is a distinct node — in effect, a version |
| nodes are never removed | HNSW cannot delete safely | that version history is retained |
| `percona_vec_aux_id` is an ordinary column | it is the `FTS_DOC_ID` device | it is **versioned by undo**, so the row version a reader sees names the node that represents it |
| edges are navigation, not data | HNSW rewires freely | the path taken to a candidate never affects what may be returned |

So one shared graph serves every transaction. It is a *superset* of what any reader should see,
and each reader narrows it with its own read view. Two checks do that.

**Check ① — `row.percona_vec_aux_id == node_id`.** Consider the update from §8, and a query for the
*old* vector:

```
graph:   node 10 → [1,0,0,0], base_pk 7      (stale — that vector is gone)
         node 20 → [0,1,0,0], base_pk 7      (current)
base:    row 7 now carries percona_vec_aux_id = 20

SELECT … ORDER BY DISTANCE(v, STRING_TO_VECTOR('[1,0,0,0]'), 'EUCLIDEAN') LIMIT 1
    node 10 is an exact match → distance 0 → base_pk 7
    but row 7's vector today is [0,1,0,0], which is far from the query
```

Returning row 7 here would rank it at a distance belonging to a vector it no longer has —
placing it ahead of rows that genuinely are near the query, with nothing to signal it. Check ①
rejects it, because `20 != 10`, and node 20 later returns row 7 at its true distance.

A wider `LIMIT` turns the same fault into a second one. Both nodes name row 7, so both resolve to
it, and the row is returned once per node it has ever owned — a row updated three times comes
back four times. Check ① is what stops it.

The same check handles a recycled primary key: if row 7 is deleted and a different row is
inserted with `id = 7`, the stale node still points at PK 7 — but the new row's `percona_vec_aux_id` is
not 10.

**Check ② — a row deleted after the reader's snapshot.** This one needs no code. The node is
still in the graph, its `base_pk` was never touched by the delete, and the fetch under an older
read view returns the pre-delete version of the row from undo — whose `percona_vec_aux_id` still reads
10, so check ① passes as well.

Every case a concurrent workload produces:

| Situation | base-row fetch | check ① | Result |
|---|---|---|---|
| another transaction's uncommitted insert | not visible | — | skipped |
| deleted, and the delete is visible to me | not visible | — | skipped |
| deleted **after** my snapshot | visible, from undo | passes | **returned** |
| vector updated; this is the stale node | visible | `20 != 10` | skipped |
| primary key reused by a different row | visible | fails | skipped |
| the inserting statement rolled back | not found | — | skipped |

**Isolation levels need nothing from us.** REPEATABLE READ asks one read view for the whole
transaction; READ COMMITTED asks a fresh one per statement. Both simply filter the same shared
graph differently.

SERIALIZABLE is not reachable on the index path: phantom prevention would need predicate locks
over "the k nearest neighbours of q", and there is no key order in ℝᵈ for gap locks to attach
to. Such sessions fall back to the exact path.

---

## 13. Rollback, and why orphans are acceptable

If the statement in §7 rolls back, the base row disappears — but **the aux rows stay**, because
they were committed on their own transaction. Node 10 remains in the graph and in the aux, now
describing a row that does not exist. That is an *orphan*.

This is deliberate. The HNSW class has no delete operation, so on rollback we **cannot** remove
the node from memory. If the aux rolled back while memory kept the node, disk and memory would
disagree, and the graph would silently change shape at the next restart. Leaving both in place
keeps them consistent.

Orphans are harmless because the read path already filters them: a candidate whose `base_pk`
resolves to no visible row is skipped (§10). The same filter handles deleted rows and
superseded vectors, so orphans need no special case.

The direction of the failure is what matters. An **extra** node costs a wasted candidate slot.
A **missing** node would mean a committed row that the index never returns — a wrong answer
with nothing to report it. §17 states the rules that keep the error always on the harmless side.

### When a persistor callback fails

`HNSW::insert()` runs in two phases: it rewires the in-memory graph first, then hands the result
to the persistor. `select_neighbors()` overwrites a neighbour's whole slot array in place, so by
the time a callback can fail the old shape is gone. The class offers no undo, and adding one
would mean either an undo log for every edge or deferring all mutation until persistence
succeeds — both a large change to a class we do not own.

**We accept the divergence.** The statement fails and the base row goes with it, but memory keeps
the rewire, and disk keeps whatever already committed.

What reaches disk is worth being exact about, because committing per callback changed it. The
rollback is **not** a unit: `vec_ctx_step_commit` has already committed every callback before the
failing one, and the rollback undoes only the current one. So disk is left holding a *partially*
updated graph — not the self-consistent older graph a single per-insert transaction would have
left.

Lock contention arrives in three shapes, and only one of them is expected:

| | what happens |
|---|---|
| `DB_LOCK_WAIT` | **the normal case, and not a failure.** `row_mysql_handle_errors` suspends the thread; the step-retry loop retries when it wakes. Two inserts rewiring the same neighbour meet here |
| `DB_LOCK_WAIT_TIMEOUT` | the wait outlived `innodb_lock_wait_timeout`. Rolls back to the savepoint taken before this callback's write, undoing this row |
| `DB_DEADLOCK` | **not reachable between aux writers** — a sub-transaction holds one row lock at a time and never waits while holding one (§7), and the user's transaction never locks an aux row (§17), so there is no cycle to detect. Handled anyway: it rolls back "the whole transaction", which here is just this one row |

Both rollbacks go through `trx_rollback_to_savepoint`, never `trx_rollback_for_mysql`. That
matters: the aux transaction is a *background* transaction and is not in the MySQL transaction
list the latter asserts membership of, so the assertion would fire on the first deadlock that
ever did arrive.

**In memory, nothing is undone, and the node is published anyway.** The class calls
`new_node->set_complete()` immediately after `insert_cb`, unconditionally — `insert_cb` returns
`void`, so the class cannot know it failed (§27). A node whose row was rolled back is therefore
searchable until the next restart. It cannot return a row: the base row went with the statement,
so check ② skips it. And a neighbour whose committed row points at it will fail to load that node
on the next cold start, which marks it `NODE_LOST` and search skips it — the class documents that
scenario as exactly this one.

That is tolerable because of what the divergence can and cannot cost:

- It **cannot** cost correctness. Every candidate the search returns is resolved through
  `base_pk` under the reader's own view (§10), so a drifted graph can only propose the wrong
  *candidates*; it can never produce a row the reader is not entitled to, or a row that does not
  exist. The two skip conditions do not depend on the graph being accurate.
- It **can** cost recall. Edges present in one copy and not the other change which neighbourhood
  a search walks, so a query may miss a row it would otherwise have ranked. This is an
  approximate index; recall is already not guaranteed.
- It is **self-correcting**. Later inserts and updates rewire the same neighbourhoods and
  persist them, and a restart discards the memory side entirely and reloads from the aux.
  Neither copy accumulates error.

So the failure mode is a temporary, bounded loss of search quality on an index that is
approximate by construction — not a wrong answer. What the engine still owes the user is a
*report*: the statement must fail loudly rather than silently succeed against a graph that was
not persisted.

### What committing per callback adds to this list

Each callback commits its own write, so the aux no longer moves one whole insert at a time. That
changes the *granularity* of divergence, not its kind — but it makes two states reachable that a
single per-insert transaction made impossible.

**The entry point can lag by one layer.** `insert_cb` commits the new node's row before
`update_entry_point_cb` commits record 0, so a crash between them leaves record 0 naming the
previous entry point while a higher node exists. `HNSW::validate()` calls that inconsistent —
"entry point must sit on the highest layer" (`hnsw.h`) — so a **debug** build can report
it. Three things make it harmless:

- The lag is **bounded at one layer**. `random_layer()` caps a draw at `current_max_layer + 1`
  (`hnsw.h`), so no node is ever created more than one layer above the entry point of the
  moment, which is what record 0 still names.
- It is **usable**: starting one layer down is a valid HNSW search, identical to the state a
  marginally younger index is in.
- It **self-heals through the ordinary growth path**. Because the cap is `max_layer + 1`, the
  hierarchy always climbs one layer at a time; the next insert drawing above the current entry
  point takes the spot and rewrites record 0. There is no repair path to write.

**Record 0 can be missing while node rows exist**, on the very first insert only. `vec_runtime_load`
then reads `DB_RECORD_NOT_FOUND` and treats the index as empty, leaving those rows unreferenced.

**Both are confined to orphans**, which is what makes them acceptable rather than merely
tolerable. §14 shows why: LSN ordering means any base row that actually became visible has all of
its aux writes durable, so a lagging or missing record 0 belongs to a node whose row never
committed — which checks ① and ② refuse to return anyway.

The ordering that matters — **a node's row must exist before anything references it** (§17) — is
strengthened rather than weakened: with one transaction the two writes were merely simultaneous;
now `insert_cb` is durably ordered before `update_entry_point_cb`.

---

## 14. Durability and crash recovery

**The aux table is an ordinary InnoDB table.** Its writes are redo-logged and undo-logged like
any other, so recovery restores it with no vector-specific machinery. There is no separate log,
no checkpoint of the graph, and nothing to replay by hand.

**The graph itself is never persisted as such** — only the rows it can be rebuilt from. After a
crash there is no graph in memory; the first statement to touch the index rebuilds it from the
entry point (§11).

**The label counter** is persisted as a watermark that runs ahead of the labels actually handed
out, so a crash can never cause a label to be reissued. A reissued label would give two rows the
same identity in one graph.

**The aux sub-transaction never flushes the redo log.** `flush_log_later` is set on it once, so
`trx_commit_low` records `must_flush_log_later` instead of calling `trx_flush_log_if_needed`
(`trx0trx.cc`) — and nothing ever consumes that flag for a background trx, since only
`trx_commit_complete_for_mysql` does and that is reached solely from the user-transaction
handler path (`ha_innodb.cc`).

Durability comes from **LSN ordering** instead, and it is exactly as strong. Every aux write
happens during the statement, so it sits below the LSN of the user's commit; that commit calls
`log_write_up_to()` for its own higher LSN, which makes everything beneath it durable, ours
included. So rule 1 — `aux ⊇ committed base rows` — holds for free: any base row that became
visible has its node row, neighbour updates and record 0 all durable. If the user's transaction
never commits, the aux rows are an orphan at worst, which §13 accepts.

This is not a micro-optimisation. It is what makes committing per callback affordable: measured
on an idle 128-core box, 40000 single-connection inserts took 8.5s with one commit per insert,
17.7s with one per callback, and 8.6-9.3s per callback with the flag. The entire cost of the
extra commits was the fsync.

**A crash between the two commits** — the aux sub-transaction and the user's transaction — is
the interesting window, and the ordering rule in §17 makes it safe: the sub-transaction commits
first, so a crash in between leaves an orphan node, never a committed row without one.

---

## 15. `ALTER TABLE … ADD KEY (v) TYPE hnsw`

There are two routes, and which one runs depends on a single question: **does the table already
have the hidden label column?**

### The first vector index: table copy

```sql
ALTER TABLE t ADD KEY vk (v) TYPE hnsw WITH (M = 16);   -- t has no vector index yet
```

The hidden column has to be added, and adding a column to every row is a table rebuild by
definition. So this is a COPY, and `ALGORITHM=INPLACE` is **refused** rather than silently
downgraded — a native in-place rebuild would stamp a label on every row and leave the graph
empty, which is a silently incomplete index.

The copy path rewrites every row into the new table, and each of those rows travels the ordinary
INSERT path: a label is assigned, `hnsw.insert()` runs, the callbacks populate the aux. The
graph is built as a side effect of the copy, with no separate build phase.

### A later vector index: in place, by clustered scan

```sql
ALTER TABLE t DROP KEY vk;                                        -- column is retained
ALTER TABLE t ADD KEY vk2 (v) TYPE hnsw WITH (M = 4),
              ALGORITHM=INPLACE, LOCK=SHARED;                     -- supported
```

Once the column exists there is nothing to rebuild, so `vec_build_index` does the work directly:
one clustered scan of the base table, feeding each row into a private graph, **reusing the label
already stamped on that row** rather than issuing a new one. That is what makes the operation
repeatable — dropping and re-adding an index does not renumber anything, and rows keep the
identity the rest of the design depends on.

Two things about that build are deliberately unlike DML:

- **The aux writes ride the ALTER's own transaction**, not a sub-transaction. The aux does not
  exist outside this ALTER yet, so if the ALTER fails its rows must disappear with it. There is
  no committed base row for them to be a superset of until the ALTER commits.
- **The private graph is discarded on return.** Nothing is handed over; the index's runtime is
  built lazily from the committed aux on first access, like any other cold index (§9).

### Why `LOCK=NONE` is not supported

`ADD VECTOR INDEX` requires at least `LOCK=SHARED`. This is the *online* axis, independent of
INPLACE-vs-COPY, and the reason is that there is nowhere for concurrent DML to record itself.
An online build logs concurrent changes to a row log and replays them against the finished
index; a vector index never enters `ONLINE_INDEX_CREATION` and the modification-log loop exempts
it, so a concurrent INSERT during the build would simply be missing from the graph. FTS makes
the same choice, and supporting `LOCK=NONE` here would be a deviation beyond FTS that would need
justifying as one.

---

## 16. Foreign keys, and why CASCADE is refused

Every rule in §7–§10 assumes DML arrives through the handler. Cascaded DML does not. When a
parent row is deleted or its key updated, InnoDB runs the child-side action *inside the engine*,
through the FK cascade nodes in `row_ins_foreign_check_on_constraint`. Those never reach
`ha_innobase::write_row` / `delete_row`, which is where every maintenance rule here hangs.

It is worth being exact about which half of that is a problem, because the two cascades differ.

**`ON DELETE CASCADE` is harmless.** A DELETE writes nothing — §9 — because the node has to
stay for read views that are still entitled to the row. A cascaded delete therefore lands in
precisely the state an ordinary delete lands in: base row gone, aux row and node retained, read
path filtering them by base-row lookup. Nothing is owed, so nothing is missed.

**`ON UPDATE CASCADE` is a real gap**, and only in one shape. §8 says a primary-key change
re-points the row's current node — one `vec_aux_update_row` writing the new `base_pk` at the
row's current label. A cascade bypasses that write, so the aux row keeps the old key. The consequence is a *false
negative*, never a false positive: the node resolves to a key that is gone, so the row silently
drops out of kNN results — and if some later row takes over that key, check ① rejects it,
because its label is different. Missing, never wrong.

Narrower still: §18 restricts us to a single-column `BIGINT UNSIGNED` primary key, so a cascade
can only reach the primary key when the foreign key *is* that column — the shared-primary-key
1:1 pattern. A cascade on any ordinary foreign-key column changes neither `base_pk` nor the
vector, and needs no maintenance at all.

`ON DELETE SET NULL` is a non-issue from both directions: it cannot target the vector column,
which must be `NOT NULL`, and nulling an ordinary column touches nothing tracked here.

### What is refused today

Both cascades, on any vector-indexed table, in `mysql_prepare_create_table`. That is broader
than the defect and deliberately so for MVP — the accurate rule is *`ON UPDATE CASCADE` where
the foreign key overlaps the primary key*, which is worth adopting when cascades are supported
for real.

### What supporting them takes

FTS solves the same problem engine-side, and its shape is the one to copy. It calls
`fts_trx_add_op()` from inside the cascade handling (`row0ins.cc`), gated on a per-foreign
-key predicate `foreign->is_fts_col_affected()`, and — the important part — it **queues** the
operation on the transaction rather than doing I/O inline.

Four pieces:

| | |
|---|---|
| **A per-FK predicate** | Not "does this FK touch the vector column" — a foreign key cannot be on one. Ours is "do this FK's child columns overlap the child's primary key", since that is what invalidates `base_pk`. |
| **A hook in the `DICT_FOREIGN_ON_UPDATE_CASCADE` branch** | After `row_ins_cascade_calc_update_vec` has computed the new values. The `ON DELETE CASCADE` branch needs nothing. |
| **A deferred queue, drained at commit** | This is what makes it engine work rather than a handler tweak. Our aux writes ride a sub-transaction owned at handler level; `row0ins` cannot see it, and opening aux I/O inside cascade processing would nest transactions mid-statement. Queue "re-point label L to primary key P" and drain on the sub-transaction where §13 already drains. |
| **Recursion safety** | Cascades chain through multi-level foreign-key graphs, so the queue is per affected child row, not per statement. |

It reuses the sub-transaction drain, so it belongs after the write path exists rather than
alongside it.

---

## 17. The rules

Three invariants. Everything above is arranged so that they hold.

**1. The aux is a superset of committed base rows.**
Extra nodes are filtered at read time and cost only a wasted candidate. A missing node is a
committed row the index never returns — a wrong answer with nothing to report it. Every choice
in this design puts the error on the harmless side.

**2. Every aux write commits before the user's transaction.**
This is what keeps rule 1 true across a crash. One `trx_t` is allocated per `insert()`, but
**each callback commits it and starts it again** (`vec_ctx_step_commit`), so the node's own row,
each neighbour update and record 0 are separate commits rather than one.

That still satisfies the ordering requirement that **a node's row must exist before anything
references it** — and satisfies it more strongly than a single transaction did. With one
transaction the writes were merely simultaneous; now `insert_cb` is *durably ordered* before the
neighbour updates and record 0, so a dangling reference is impossible at every intermediate
point, not just at the end.

What is given up is per-insert atomicity: a callback failing midway leaves the earlier ones
committed. §13 records why that is the harmless direction — the resulting orphan is filtered at
read time, whereas rolling the whole insert back left memory holding a node the aux had
discarded, and the in-memory rewire could not be undone either.

Why per callback: one transaction spanning the whole insert holds an X lock on every row it
touches until `insert()` returns, and the rows nearest the entry point are rewired by almost
every insert. §7 step 4 has the argument and the measurement.

**3. Aux writes never touch the parent table.**
The sub-transaction reads and writes the aux table only. The user's transaction never locks an
aux row, and the sub-transaction never locks a base row, so the two can never deadlock against
each other.

---

## 18. Limitations

**Dead nodes accumulate.** A deleted row, a superseded vector and a rolled-back insert all leave
a node behind, and MVP never removes them. The index therefore grows with the number of
*mutations* rather than the number of rows. They are reclaimed today only by dropping the index
or rebuilding it.

Reclaiming them properly is a later phase. The rule is known — a label may be removed once no
active read view can see any row version carrying it, which is exactly the negation of the
condition check ② depends on — and it needs a record of retirement events, since one of them (a
rolled-back insert) leaves no trace anywhere else in the engine.

**Rejected at DDL by the layer above**, independently of anything here: a vector index on a
virtual generated column, on a partitioned table, or on a temporary table. `sql_table.cc` raises
`ER_INDEX_MUST_HAVE_COMPATIBLE_COLUMN` for the first and `ER_NOT_SUPPORTED_YET` for the other
two, so none of them reach the aux code.

**`max_elements` is unread and unsettable.** `HnswParam` carries a default of 10000, the parser
never assigns it (§19), and nothing in the tree looks at it — not the HNSW class, not the arena. Whether it is a hard cap or a sizing hint is
open, and it matters here: the arena has no per-block free (§22), so a cap that is actually
enforced would need an answer for the insert that exceeds it.

**The aux is not transactional with the base table.** By design (§13). A count of aux rows will
not equal a count of base rows, and that is correct behaviour rather than corruption.

**One vector index per table**, and a single-column integer primary key.

The restriction is about the hidden `percona_vec_aux_id` column, not about the runtime — §5 already puts
the graph on `dict_index_t`, so a second index needs no structural change there. What a second
index needs is a second *label*, because check ① compares a row's label against a node's id and
each graph numbers its nodes independently. That leaves two ways forward, and they are not
equally good:

| | how it works | cost |
|---|---|---|
| a column per vector index | `percona_vec_aux_id_1`, `percona_vec_aux_id_2`, … each moving independently | one more hidden column per index; the first `ADD` is already COPY-only (§15), so adding one is free |
| one shared column, FTS-style | a single label meaning "this version of this row", every aux keying on it | an UPDATE to *one* index's vector bumps the shared label, invalidating this row's nodes in **every** vector index — each must then insert a fresh node or the row silently vanishes from its results |

FTS chose the shared column and pays that cost: a new `FTS_DOC_ID` on UPDATE re-indexes the row
in every FULLTEXT index. For vectors the cost is worse, because re-inserting a node means
re-running graph insertion — a search plus neighbour rewiring — for an index whose vector did
not change. So a column per index is the direction, and MVP simply rejects the second index
rather than half-building it.

---

# Part IV — Internals

Everything below is implementation detail. A reader who
wants the design has it already; a reader who has to change this code needs this.


---

## 19. The type seam

HNSW will not be the only vector index type, so the engine keeps a seam for a second one. The
seam is deliberately thin: a type token that survives on disk, and a type-erased runtime pointer.

```c
enum class Vec_index_type : uint8_t { HNSW = 0 };   // vec0aux.h

struct Vec_runtime {                                // vec0index.h
  virtual ~Vec_runtime() = default;                 // ...the whole base
};
```

`dict_index_t::vec` is a `Vec_runtime *`; `vec_t` derives from it and the downcast is file-local,
so the compiler — not a convention — stops any other translation unit interpreting the subtype.
There are no virtuals to dispatch on because there is nothing yet to dispatch between.

The token is carried where it must be durable: in the aux table name,
`percona_vec_<type>_<table_id>_<index_id>`, parsed back by `vec_aux_parse_table_name()`. A second
type therefore costs an enumerator and a token, and existing aux tables keep naming themselves
unambiguously.

What a second type would add is behaviour dispatch, which does not exist: `vec_insert_row()`,
`vec_update_row()` and `vec_runtime_open()` call HNSW directly today. Turning those three call
sites into a vtable on `Vec_runtime`, or a registry keyed by `Vec_index_type`, is a contained
change precisely because the runtime already hangs off `dict_index_t` (§5) and the type already
survives on disk.

The parameter side is upstream's, and is a variant rather than a vtable:

```c
struct HnswParam { int M{25}; int max_elements{10000};
                   int ef_construction{200}; std::string_view metric{"euclidean"}; };
using VectorIndexParam = std::variant<std::monostate, HnswParam>;
```

Adding a type there means adding an alternative. `parse_options()` is split into a shared
implementation with two overloads — `Key_spec` for DDL, `KEY` for table open — so the same parse
both validates at DDL time and carries the parameters into the runtime at open.

Only `M` and `metric` are actually settable. `ef_construction` and `max_elements` are fields of
`HnswParam` that the parser does not accept, so both are unreachable defaults — 200 and 10000
(§29, §32).

A `WITH (...)` value is a bare identifier or a number, never a quoted string — the grammar rule is
`ident EQ ident | ident EQ NUM`. So it is `metric = euclidean`, and `metric = 'euclidean'` is a
parse error.

One rule keeps the seam honest: **implementations are stateless.** All per-index state lives in
the runtime, so there is no object lifetime to manage and no per-index singleton. The HNSW class
imposes the same rule on the persistor (§20).

---

## 20. Wiring the persistor

**There is no registration call.** The class takes the persistor as a *template parameter* and
holds one as a member:

```c
template <typename ArenaAllocator, typename Persistor,
          typename RandomEngine = std::default_random_engine>
class HNSW {
  using PersistorContext = typename Persistor::Context;
  ...
  ArenaAllocator m_allocator;
  Persistor      m_persistor;      // default-constructed
};
```

So "registering the callbacks" is simply instantiating the template:

```c
using Vec_hnsw = HNSW<Vec_arena, Vec_persistor, Vec_random_engine>;
```

The third parameter defaults to `std::default_random_engine`, which is not thread-safe; §23 says
what we pass instead.

Three consequences follow from that being compile-time rather than runtime:

- **No function pointers and no virtual dispatch.** The calls inline.
- **The persistor must be stateless.** It is default-constructed and shared by every call, so it
  can hold nothing per-call, per-transaction or per-thread. Everything of that kind travels in
  `Context`, which the class passes through untouched.
- **`Context` is ours to define** — the class only knows it as `typename Persistor::Context`.

### How a call reaches our function

Nothing is looked up at run time. The chain is entirely compile-time:

```
using Vec_hnsw = HNSW<Vec_arena, Vec_persistor, Vec_random_engine>;  ← the registration

  1. the compiler substitutes Vec_persistor for the Persistor parameter
  2. the member `Persistor m_persistor;` becomes a real Vec_persistor object
  3. inside insert(), the class writes
         m_persistor.update_neighbors_cb(ctx, neighbor->id(), neighbor_ids(neighbor));
     which is an ordinary member call on that object, resolved by name
```

If `Vec_persistor` has no method of that name with compatible parameters, **the code does not
compile**. That failure is the only "registration check" there is — no vtable, no function
pointer, no registry, and nothing to forget to wire up at run time.

### What the callbacks must look like

One wrinkle shapes the signatures. The neighbour range the class passes is
`HNSW<A,P>::NeighborIdRange` — a type nested inside the very instantiation that needs our
persistor, so naming it would be circular. The class resolves this by making the callbacks
**member templates**, and the persistor never names the HNSW type at all.

`Context` is ours to define; the class only knows it as `typename Persistor::Context`. It carries
everything the persistor is forbidden to hold:

```c
struct Vec_ctx {
  trx_t        *trx;        // the transaction the aux writes ride
  dict_table_t *aux;        // opened once per statement, MDL held
  THD          *thd;
  uint32_t      m, vec_bytes;
  dberr_t       err;        // callbacks return void; the first failure lands here
};
```

Two consequences of `void` returns. Each callback opens by short-circuiting when `ctx->err` is
already set, so the first failure sticks and the rest become no-ops. And the caller — not the
class — decides what a failure means: `insert()` runs to completion regardless (§13).

### The graph object lives in the runtime

```c
struct vec_t : Vec_runtime {       // hangs off dict_index_t::vec
  Vec_hnsw          *hnsw;         // owns its arena and its persistor by value
  std::mutex         load_mutex;   // cold path only; hot paths lock nothing (§23)
  std::atomic<bool>  loaded;       // false -> true exactly once, then never read under a lock
  // plus index_id, table, dims, m, ef_construction — from the DD at open
};
```

Order matters on teardown: the graph is destroyed before the arena its nodes live in (§22).

---

## 21. How the callbacks write to the aux table

Each callback ends in exactly one row operation on the aux table, built with InnoDB's query-graph
C API — `ins_node_create` / `row_ins_step` for the insert, `row_create_update_node_for_mysql` /
`row_upd_step` for the updates, driven through `pars_complete_graph_for_exec`.

| callback | operation |
|---|---|
| `insert_cb` | insert one row: `(id, vec, base_pk, level, neighbors)` |
| `update_neighbors_cb` | update the `neighbors` column of one row, by id |
| `update_entry_point_cb` | upsert record 0, whose `base_pk` holds the entry point |

Four things about that path are not obvious, and each is load-bearing.

**The internal SQL parser is deliberately not used.** `pars_sql` / `que_eval_sql` would be far
less code, but they serialise every statement on the global `pars_mutex` — and this path runs
once per rewired neighbour on every insert.

**The neighbour update must take its own locks.** An UPDATE normally runs a search first, which
both positions the cursor and acquires the row lock. We already know the key and skip the search,
so the lock has to be taken explicitly — `lock_clust_rec_read_check_and_lock` with `LOCK_X` and
`LOCK_REC_NOT_GAP`, not the modify variant — and the cursor position stored *after* the lock, not
before.

**Field positions are record positions.** A clustered-index record is the primary key, then
`DB_TRX_ID` and `DB_ROLL_PTR`, then the rest, so every column is addressed through
`dict_col_get_clust_pos()`. The user-column ordinal reads `DB_ROLL_PTR` instead and fails
silently downstream.

**Errors go back through `row_mysql_handle_errors`.** Each operation runs in a step-retry loop:
on a lock wait the loop blocks and retries the step, on anything else it propagates. Aux writes
therefore behave like ordinary DML under contention, because they are driven by the same
machinery the server drives rather than a reimplementation of it. §13 says which of those
outcomes can actually reach a vector insert, and what each one leaves behind.

An empty blob is not a NULL. Record 0 carries a zero-length `vec` and `neighbors`, and both
columns are `NOT NULL`; writing them as SQL NULL trips the record-size assertions.

---

## 22. Dictionary cache eviction

A `dict_table_t` can be evicted when nothing references it. For a vector-indexed table that
means the runtime, the `HNSW` object and its arena are all destroyed together, and every node's
memory is reclaimed in a single step.

Nothing needs to be saved first — everything in the graph is already in the aux table. The next
statement that touches the table opens the runtime again and starts from the entry point, as in
§11. Because recovery is one node load rather than a full scan, eviction is cheap enough that
vector-indexed tables need no special protection from it.

The one constraint is teardown order. The class states that it *"does not destroy Nodes and must
not outlive the allocator"*, so the graph must be destroyed before the arena its nodes live in.

---

## 23. Concurrency

**We take no latch on either hot path.** Neither INSERT nor kNN search locks anything of ours.
The only synchronisation the runtime owns is `vec_t::load_mutex`, a plain `std::mutex` held on
the cold path alone, and it is not touched again once the graph is built.

That is possible because the HNSW class is thread-safe for everything we do after the build:

- concurrent `insert()` and search on one instance (`hnsw.h`);
- concurrent faulting of the *same* lazily loaded node — `load_node()` takes a striped lock and
  **re-checks the node state under it**, so the second thread finds `NODE_COMPLETE` or
  `NODE_LOST` and returns without loading;
- its own internal structures, under `m_global_lock`, `lock_node` and `m_entry_point_lock`.

**The one thing the class declines** is `init_from_entry_point()` running alongside insert or
search (`hnsw.h`) — it mutates `m_nodes` and the arena *without* taking `m_global_lock`, which
`insert()` does take. Rather than lock the hot paths against that, we make it **unreachable**:
`loaded` goes false to true exactly once, and only after the graph is fully built, so no thread
can reach `insert()` or a search until init has finished. There is nobody left to exclude.

`loaded` is `std::atomic<bool>` with release/acquire ordering, which also publishes the `hnsw`
pointer to every thread that observes it true — that is what makes the unlocked hot paths sound.

**Why a mutex and not `std::call_once`.** `vec_runtime_load` reports failure by *returning*
`DB_OUT_OF_MEMORY`, not by throwing, and `call_once` consumes its flag on a normal return. That
would leave an index permanently unloaded after one transient failure. With the mutex, a failure
simply leaves `loaded` false and the next statement retries.

Two things had to change in the class contract to get here. `Vec_persistor::load_node_cb` returns
`bool`, so a failed load marks the node `NODE_LOST` rather than leaving a half-filled `COMPLETE`
one. And the graph needs a thread-safe `RandomEngine` for its layer draw, which
`std::default_random_engine` is not. `Vec_random_engine` is an adapter over `ut::random_64()`,
whose seed is an `extern thread_local` — thread-safe by construction, so the layer draw needs no
mutex of its own. It accepts a seed and ignores it, because there is no shared state to seed.

### Why the aux needs no version column

The callbacks do not interleave with the graph mutations. `insert()` rewires the whole
neighbourhood first, collecting the touched nodes, and only then walks that set calling
`update_neighbors_cb` with each node's *final* state — so a node rewired at three layers is
persisted once, correctly. There is no older-overwrites-newer race, and therefore nothing for a
per-node mutation-order stamp to protect against.

What that ordering does create is the failure gap in §13: by the time a callback can fail, the
in-memory rewire is already done and cannot be undone. That is a divergence question rather than
an ordering one, and §13 records the decision.

The consequence this section used to flag as something to watch for is no longer hypothetical.
Two concurrent inserts that pick overlapping neighbours update the same aux rows from two
different background sub-transactions, so they contend for a row lock — which is what makes each
callback commit its own write. §7 step 4 has the measurement and the argument.

---

## 24. Memory limits

`innodb_hnsw_max_memory` bounds, in bytes, the memory held by HNSW graphs **across all tables and
all indexes**. Dynamic, so a workload blocked by it recovers without a restart; `0` means no
limit.

Every graph byte passes through `Vec_arena::allocate()`, so one atomic counter there covers
exactly the scope the variable promises, and each arena subtracts its total when destroyed.

**The refusal deliberately does not live in `allocate()`.** Returning `nullptr` there is what
`hnsw.h` turns into a `throw std::bad_alloc` — four sites, `Node::create` and `alloc_neighbors`
among them — partway through a rewire, with neighbours already relinked and no per-block free to
unwind with (§22). So the check sits at the entry to the insert, before the graph has been
touched, where failing is just a failed statement.

**It is a charge check, not a prediction.** It asks whether the budget is already spent, not
whether this insert would fit. Sizing an insert from outside the class is not possible:
`sizeof(Node)` is private, and one insert also allocates stubs for lazily loaded neighbours and a
copy of the query vector. The bound can therefore be overshot by at most what a single insert
allocates — the price of refusing before mutating rather than during.

Refusal reaches the user as `ER_OUT_OF_RESOURCES`. That required adding `DB_OUT_OF_MEMORY` to
`row_mysql_handle_errors()`, beside `DB_OUT_OF_FILE_SPACE`: it had no case there, so it fell to
the default branch and called `ib::fatal`, taking the server down instead of failing the
statement. A resource ceiling is not a corrupt engine.

---

# Part V — The commits


---

## 25. The commits

What is on this branch, in order, and what each one is for. This document is the last commit on the branch, so every hash below is accurate as written;
only this commit's own cannot appear. They still **change whenever the branch is rebased** — the
subjects are the stable identifier.

| commit | what it achieves |
|---|---|
| `2f511b4601f` | **TEMPORARY.** Dmitry's HNSW phase-2 class, part 1 — the persistor concept, lazy node loading, cold start from a persisted entry point. Carried here so this work could be built before PS-11267 landed. **Drop all four TEMPORARY commits once PS-11267 is in `vector-mvp`.** |
| `861c5a89360` | **TEMPORARY.** Part 2 — thread safety: concurrent `insert()` and search on one instance, concurrent faulting of the same node, `load_node_cb` returning `bool`, a `RandomEngine` template parameter. Because this is in the base, the graph was never ours to serialise; the commits below only had to stop obstructing it. |
| `655a11e97cb` | **TEMPORARY.** Phase-1 follow-up. Reworks the search result: both search APIs return `SearchHit {id, base_pk}` instead of bare `base_pk`s. This is what closed §26 and made check ① of §12 runnable. |
| `ecb908769a5` | **TEMPORARY.** Phase-2 post-push fix, carried from `dlenev/vector-mvp-11267`. |
| `64e15e3e0e9` | The aux module and the table lifecycle that does not involve ALTER: one aux table per vector index named `percona_vec_hnsw_<table_id>_<index_id>`, plus CREATE, RENAME (a no-op within a schema) and TRUNCATE. |
| `8aff4d15266` | ALTER manages the aux per index — created inside the ALTER's transaction on ADD, dropped on DROP INDEX, with both error paths covered by injection. DISCARD/IMPORT refused. |
| `baccd1f249b` | The hidden `percona_vec_aux_id` column: stamped on INSERT, carried across a rebuild, retained on DROP INDEX. Plus the DDL it restricts — INSTANT ADD/DROP COLUMN, BULK LOAD, native rebuild, CASCADE foreign keys, and COPY-only for the first ADD. |
| `d431cf11d16` | Split `parse_options` so the same parse serves DDL (`Key_spec`) and table open (`KEY`) — previously the parameters were validated and thrown away, so the engine never saw `M`. |
| `40201328b89` | The runtime, and everything needed to be a correct *user* of the HNSW class: the per-index anchor on `dict_index_t`, the arena, parser-free aux DML, the persisted label counter, the persistor — plus `Vec_random_engine` over `ut::random_64()`, `load_node_cb` returning `bool`, and `QUE_FORK_ACTIVE` on the aux DML forks. Each of those is a defect from the moment the code is written, so they belong where the class is first instantiated. |
| `112b9e552f8` | The write path. INSERT builds the graph on a sub-transaction; the load path rebuilds it from the aux one node at a time, lazily and exactly once behind `load_mutex`; UPDATE mints a fresh label and re-points the row, because a node is immutable. Also the two things the aux writes must get right: rolling the background trx back with `trx_rollback_to_savepoint`, and treating a not-yet-committed neighbour's `DB_RECORD_NOT_FOUND` as success. |
| `d9ef8e8bcce` | When the sub-transaction commits, and why it need not flush. Each callback commits it and starts it again, so a row lock cannot outlive the callback that took it — which makes deadlock impossible rather than merely rare. `flush_log_later` then removes the fsync those extra commits would otherwise pay, because the user's commit already flushes past our LSN. 20000 of 20000 rows under 8 connections, against 5074 before. |
| `a58ac9f3253` | The `vec_aux_verify` interpreter command, `vector_concurrent_insert` and `vector_insert_contention`. All assert the result rather than the serialisation — one node per committed row, no id issued twice, no two nodes naming the same row — which is the property that holds however the inserts interleave. |
| `9e6f4d5b814` | ADD VECTOR INDEX populates the graph: a clustered scan builds it in place on the INPLACE path, reusing each row's stamped label. Plus `vec_knn` so the graph can be queried in tests. |
| `85899e38df8` | `innodb_hnsw_max_memory`: a server-wide byte budget, refused at the entry to an insert and at each step of a build rather than inside the arena. |
| `18b4efd99ae` | Regression test for `SELECT COUNT(*)` returning 0 when the optimizer picked the vector index. The fix itself is upstream's; this keeps it from coming back. |
| `ec0e164a9d5` | `ORDER BY DISTANCE(...) LIMIT k` served from the graph — optimizer recognition, the `vec_init` / `vec_read_first` / `vec_read_next` handler family, a streaming scan of the graph, and `innodb_hnsw_ef_search`. Both MVCC checks of §12: ② is the primary-key read under the session's own view, ① compares the node id against the label `row_sel_store_mysql_rec` lifts off the visible record into `prebuilt->vec_aux_id`, the way it already lifts `fts_doc_id`. |
| *(this commit)* | This design document. Last on the branch so the table above can name every commit accurately; only its own hash cannot appear. |
---

# Part VI — Open items

Everything known to be missing, wrong, or temporary. One section each, with the code, an example
you can run, and who owns it.

There is no GA blocker left: §26, which was one, is resolved.

| § | item | owner |
|---|---|---|
| 26 | the search APIs return the node id — **resolved** | HNSW class |
| 27 | `insert()` cannot report failure | HNSW class |
| 28 | `insert()` is not exception-safe mid-rewire | HNSW class |
| 29 | `max_elements` should be deleted; `ef_construction` should be accepted | HNSW class |
| 30 | No way to size an insert before it runs | HNSW class |
| 31 | Read path is in; hypergraph, cost model, approximation-visibility remain | server |
| 32 | `ef_search` settable; `ef_construction` still is not | HNSW class |
| 33 | The four cherry-picked HNSW class commits must be dropped | us |
| 34 | Two `innodb` suite tests fail on the base branch | not ours |
| 35 | The upstream autoinc crash window | not ours |
| 36 | Dead nodes are never reclaimed | post-MVP |
| 37 | CASCADE foreign keys are refused | post-MVP |
| 38 | One vector index per table | post-MVP |
| 39 | The read path streams — **resolved** | us |
| 40 | Layer draws are not reproducible, so no test may record topology | us |
| 41 | A replica's graph differs from the source's — approximate answers can differ | server |

---


## 26. The search APIs return the node id — RESOLVED

```c
// hnsw.h
struct SearchHit { uint64_t id; uint64_t base_pk; };
std::pair<bool, SearchHit> nn_search_next(NNSearchContext *ctx);
```

It used to return `base_pk` alone, which made check ① of §12 unrunnable: the check is
`row.percona_vec_aux_id == node_id`, and there was no node id to compare against. Check ② alone
is not enough. Dmitry's follow-up commits added `SearchHit`, and this branch now carries the
check.

**What it was.** A single row updated once:

```sql
INSERT INTO t1 VALUES (7, STRING_TO_VECTOR('[1,0,0,0]'));   -- label 1, node 1
UPDATE t1 SET v = STRING_TO_VECTOR('[0,0,1,0]') WHERE id = 7;
```

```
count=4 max_id=3
id=1 level=0 base_pk=7 nb=2,3      <- superseded node, still holds [1,0,0,0]
id=3 level=1 base_pk=7 nb=...      <- current node, holds [0,0,1,0]
```

Two nodes name `base_pk = 7` on purpose (§8 — a node is immutable, so a changed vector becomes a
new node), and row 7 now carries `percona_vec_aux_id = 3`. A search for `[1,0,0,0]` walked into
node 1, check ② found row 7 live and committed, and the row came back ranked as if it still held
a vector it had given up. Not an error, a warning or a crash: a silently wrong ranking, appearing
the moment anyone updated an indexed vector.

Two symptoms, not one. Without a node id there was nothing to deduplicate on but `base_pk`, so a
row that had been updated *n* times could also be returned *n* times — the wrong ranking came with
duplicates.

**What it is now.** The scan hands the handler `vec_hit_t {id, base_pk}`; the read path compares
the id against the label on the visible record (§10) and skips a mismatch. Duplicates cannot arise
at all any more, because the streaming scan never offers a candidate twice (§39).

`vector_stale_node` covers it end to end, in SQL: the wrong ranking, the duplicates, and the
reverse case under an older read view — where the stale node is the live one for that reader and
the *new* node is what must be dropped. Removing the comparison turns every one of those
assertions red, which is what makes the test worth having.

---

## 27. `insert()` cannot report failure

```c
void insert(uint64_t id, uint64_t base_pk, const char *q, ...);   // returns void
```

`insert_cb`, `update_neighbors_cb` and `update_entry_point_cb` all return `void` as well. A
failed aux write can set `ctx->err` so that later callbacks short-circuit, but it cannot stop
`insert()` and cannot tell the caller.

**Example.** The aux tablespace goes read-only between an INSERT starting and its persistor
running. `insert()` rewires the graph in memory, every aux write fails, `ctx->err` is set, and
`insert()` returns normally. We roll the sub-transaction back and fail the statement — so the
user is told — but the in-memory graph keeps the rewire that was never persisted. §13 explains
why that divergence is acceptable and self-correcting; it would be better not to have it.

`load_node_cb` is the one callback that *can* report failure: it returns `bool`, and a false
return marks the node `NODE_LOST` rather than leaving a half-filled `NODE_COMPLETE`. Asking for
the same from the write callbacks, and for `insert()` to propagate it, is what would close this.

---

## 28. `insert()` is not exception-safe mid-rewire

The class allocates through the arena and throws when it cannot. Three `throw std::bad_alloc`
sites exist in `hnsw.h`, two of them inside the rewire:

| function | what it was allocating |
|---|---|
| `Node::create` | `ALIGN_SIZE(sizeof(Node)) + ALIGN_SIZE(dims * sizeof(float))` |
| `alloc_neighbors` | `(layer + 2) * M * sizeof(Node *)` |
| the streaming search's `init` | a `malloc` copy of the query vector — search path, not insert |

**Example.** `alloc_neighbors` throws for a node at layer 3 with `M = 16`, after the rewire loop
has already relinked two earlier layers. The exception unwinds out of `insert()`, through
`vec_add_node()`, into InnoDB code that is not exception-safe — leaving the graph half-rewired,
and leaving it visible: the hot paths hold no lock of ours (§23), so another thread reads the
half-rewired graph as though it were finished.

§24 keeps us off these paths for the *budget* case by refusing before `insert()` starts, but a
genuine allocator failure still reaches them.

**Question for the class owner:** is `insert()` intended to be exception-safe, and what is the
graph's state after a throw? The contract says `allocate()` may return `nullptr` and that this is
"asserted", which does not match code that throws.

---

## 29. `max_elements` should be deleted, and `ef_construction` should be accepted

Two fields of `HnswParam` that a user cannot set. They look alike and are opposite cases.

```c
struct HnswParam {
  int M{25};                  // settable
  int max_elements{10000};    // not settable, and nothing reads it
  int ef_construction{200};   // not settable, but the graph is built with it
  std::string_view metric{"euclidean"};   // settable
};
```

The `WITH (...)` parser accepts `M` and `metric` only; its `else` branch rejects everything else
with `ER_ILLEGAL_INDEX_CONSTRUCTION_PARAMETER`.

### `ef_construction` — a wired parameter the parser forgot

It is not dead. It reaches the graph and changes what the graph is:

```
parse -> vec->ef_construction -> Vec_hnsw ctor -> m_ef_construction = max(ef_construction, M)
      -> search_layer(q, nearest, m_ef_construction, l)   inside insert()
```

It is the width of the candidate search *at insert time* — the build-time counterpart of
`ef_search`. Wider means better neighbour selection and a better-connected graph, so higher
recall later, at the cost of slower inserts.

What makes this more than a missing knob is that graph quality is **baked in at insert time**. A
graph built with too small an `ef_construction` cannot be recovered by raising `ef_search`
afterwards: the edges were never created. The only remedy is DROP and re-ADD. So every index is
permanently built at 200, and the user cannot trade insert time for recall even once.

**It should be accepted by the parser** — one `else if` branch mirroring `M`. Nothing else is
needed, because the plumbing behind it is already complete.

### `max_elements` — an hnswlib concept that does not apply here

It should be deleted. Nothing sets it and nothing reads it, but the reason to remove rather than
wire it up is that the parameter does not belong in a database index.

`max_elements` is how **hnswlib** works: that library keeps nodes in a flat contiguous array
indexed by internal id, so it must preallocate and grow through `resizeIndex()`. This class has
neither a capacity nor a resize — nodes are arena-allocated one block at a time (§5). The field
has nothing to size.

It also cannot serve as a memory bound, which is the only reason one might keep it. Per-node
memory is `sizeof(Node) + dims * 4 + (layer + 2) * M * 8`, so `max_elements = 10000` is roughly
3 MB at `VECTOR(4)` and 65 MB at `VECTOR(1536)` — a twentyfold range for the same "limit".
Capping the element *count* does not cap bytes.

`innodb_hnsw_max_memory` (§24) bounds the resource that is actually scarce, server-wide rather
than per index, and dynamically rather than fixed at `CREATE INDEX` time — when there is no data
yet to size a cap from. Having both would give an INSERT two unrelated reasons to fail, where
raising one limit does not help if the other was hit, and would leave a secondary index refusing
rows because it is "full": a behaviour no other index type in the server has. A database index is
unbounded by row count and bounded by memory.

---

## 30. No way to size an insert before it runs

`Node::create` allocates `ALIGN_SIZE(sizeof(Node)) + ALIGN_SIZE(dims * sizeof(float))` and
`alloc_neighbors` allocates `(layer + 2) * M * sizeof(Node *)`, where `layer` is
`random_layer(max_layer)` capped at `max_layer + 1` (`hnsw.h`).

That cannot be computed from outside the class: `sizeof(Node)` is private, and one insert may
also allocate stubs for lazily loaded neighbours plus a copy of the query vector.

**Consequence.** §24 is a *charge* check — "is the budget already spent?" — rather than a
prediction — "would this insert fit?". So `innodb_hnsw_max_memory` can be overshot by whatever a
single insert allocates.

**Fix.** A static `HNSW::worst_case_insert_bytes(dims, M, max_layer)` would make the check exact
and remove the overshoot.

---

## 31. The read path is in; three things it does not yet do

`ORDER BY DISTANCE(...) LIMIT k` is served from the graph (§10): optimizer recognition into
`JT_VECTOR`, a `VectorSearchIterator`, the `vec_init` / `vec_read_first` / `vec_read_next` handler
family, both MVCC checks of §12, and a streaming scan that continues when a filter above
consumes candidates. `innodb_hnsw_ef_search` is the per-session recall knob.

What is still missing is bounded, and worth naming precisely.

**The hypergraph optimizer plans the exact path.** Recognition lives in the old optimizer only, so
a session using the hypergraph join optimizer falls back to scan-plus-sort. Correct results, no
index.

**No cost model, so no choice.** The rule is a shape match, because
`dict_stats_should_ignore_index()` leaves nothing to cost against. A query that would be cheaper
as a scan — a tiny table, or a `LIMIT` approaching the row count — still takes the index; and
there is no way for the optimizer to decline the index when recall would be poor.

**Nothing surfaces the approximation.** A short result is a legitimate answer (the graph was
exhausted), and so is a result that omits a row the exact path would have returned. Neither is
visible: no warning, no `EXPLAIN` annotation, nothing separating "these are the 5 nearest" from
"these are 5 near ones". For a feature whose whole contract is approximate, that deserves a
decision rather than a silence.

---

## 32. `ef_search` is settable; `ef_construction` still is not

`ef_search` has a home: the session variable `innodb_hnsw_ef_search`, default 40, effective width
`max(ef_search, k)` (§10). Per-session is the right scope — it is a per-query recall/latency
trade, not a property of the index.

Its build-time counterpart is not settable at all (§29): the `WITH (...)` parser accepts `M` and
`metric` only, so every graph is built at `ef_construction = 200`. That asymmetry is what is worth
fixing. A user can widen the *search* to chase recall but cannot widen the *build*, and no amount
of `ef_search` recovers edges that were never created.

---

## 33. The cherry-picked HNSW class commits must be dropped

Four commits are carried on this branch from `vector-mvp-11267` and `dlenev/vector-mvp-11267`, so
that this work could be built before they landed: phase 2 parts 1 and 2, the phase-1 follow-up
that made the search APIs return `SearchHit` (§26), and the phase-2 post-push fix. Drop all four
once PS-11267 is in `vector-mvp`, or the branch will carry duplicates. They are marked
`TEMPORARY:` in their subjects and are listed in §25.

---

## 34. Two `innodb` suite tests fail on the base branch

`innodb.innodb_stats` and `innodb.use_latest_stats`. PR #5987 added an `INDEX_OPTIONS` column to
`INFORMATION_SCHEMA.STATISTICS` (`sql/dd/impl/system_views/statistics.cc`) without updating the
two result files.

**Example** — every diff line is one of these two shapes:

```
-... IS_VISIBLE  EXPRESSION
+... IS_VISIBLE  EXPRESSION  INDEX_OPTIONS
-def test t1 1 test a 1 a A 1 NULL NULL YES BTREE   YES NULL
+def test t1 1 test a 1 a A 1 NULL NULL YES BTREE   YES NULL  flags=0;
```

Nothing on this branch touches `sql/dd/` or those results, and the failure diff hashes identically
before and after our changes. Belongs in Martin's PR.

---

## 35. The upstream autoinc crash window

Written up in `PS-autoinc-persist-crash-window.md`. Upstream advances the persisted autoinc
watermark *before* the redo record enters the mtr, so a racing thread sees the watermark already
advanced, skips logging, and a committed value can be reissued after a crash.

Our counter deliberately does not copy that: `dict_table_vec_next_id_persisted_advance()` is
called *after* `mtr.commit()`. The deviation is intentional and documented; the upstream autoinc
bug it avoids is still unfiled.

---

## 36. Dead nodes are never reclaimed

Deleted rows (§9), superseded vectors (§8) and rolled-back inserts (§13) all leave nodes
behind, so the index grows with the number of **mutations** rather than the number of rows.

**Example.** Two rows, one of them updated three times:

```sql
INSERT INTO t1 VALUES (7,...),(8,...);
UPDATE t1 SET v = ... WHERE id = 7;   -- three times, different vectors
SELECT COUNT(*) FROM t1 FORCE INDEX (PRIMARY);   -- 2
```

```
nodes=5  distinct_ids=5  distinct_base_pks=2  dup_base_pks=3
```

Five nodes for two live rows. Three of them are dead and will never be removed; the only
reclamation today is dropping or rebuilding the index.

The retirement rule is known — a label may be removed once no active read view can see any row
version carrying it, the exact negation of what check ② depends on. It needs a durable record of
retirement events, because a rolled-back insert leaves no trace anywhere else in the engine.

---

## 37. CASCADE foreign keys are refused

**Example.**

```sql
mysql> CREATE TABLE ch (id BIGINT UNSIGNED PRIMARY KEY, p BIGINT UNSIGNED,
    ->                  v VECTOR(4) NOT NULL, KEY (v) TYPE hnsw WITH (M=4),
    ->                  FOREIGN KEY (p) REFERENCES par(id) ON UPDATE CASCADE);
ERROR 1235 (42000): This version of MySQL doesn't yet support
                    'vector indexes on tables with a CASCADE foreign key'
```

§16 gives the reasoning and what support would take. The refusal is deliberately broad: `ON
DELETE CASCADE` is in fact harmless, because DELETE writes nothing to the aux. Narrowing it to
`ON UPDATE CASCADE` where the foreign key overlaps the primary key — the only case that actually
moves a `base_pk` — is the follow-up.

---

## 38. One vector index per table

**Example.**

```sql
mysql> ALTER TABLE t1 ADD KEY k2 (v) TYPE hnsw WITH (M = 4);
ERROR 1235 (42000): This version of MySQL doesn't yet support
                    'multiple vector indexes on a single table'
```

The restriction is about the hidden label column, not the runtime — §5 already puts the graph on
`dict_index_t`, so a second index needs no structural change there. What a second index needs is a
second *label*, because check ① compares a row's label against a node id and each graph numbers
its nodes independently. §18 sets out the two ways forward and why a hidden column per index beats
the shared FTS-style column for vectors.

---

## 39. The read path streams — RESOLVED

The read path once widened by re-running a bounded search with a larger `k` and filtering out the
node ids it had already returned. It now uses the class's streaming search:

```c
vec_knn_open(index, q, batch_size, ef_search, thd, &scan);   // nn_search_start
while (vec_knn_next(scan, &hit)) { ... }                     // nn_search_next
vec_knn_close(scan);
```

One descent instead of one per widening, no exclude set, no guessed bound to stop at — the scan
ends when its frontier empties. It also fixed a correctness bug rather than only cost: each
re-search explored wider than the last, so it could surface a candidate *closer* than one already
returned and emit it, breaking the ascending-distance contract that lets the optimizer elide the
sort. The scan cannot, because it never offers a candidate twice and drops anything that would go
backwards.

**What it cost.** The aux table and its MDL ticket are held for the whole scan rather than
re-taken per widening, because `nn_search_next` faults nodes in through `load_node_cb` and that
reads `ctx->aux`. Fewer acquisitions overall, but a longer-lived one — the same shape the base
table's own MDL already has for the statement.

Getting that lifetime wrong crashed the server, and the failure was not where it looked.
Releasing the scan in `index_end()` is not enough: `ha_reset()` clears `inited` at statement end
*before* the iterator's destructor runs, so its `ha_index_or_rnd_end()` does nothing and the scan
leaks. The next `DROP` of that table then fails `table->get_ref_count() == 0` and takes the server
down. The release belongs in `end_stmt()`, which every statement reaches, with `close()` covering
a handler closed mid-scan. `vector_scan_lifetime` asserts it across nine shapes — scan abandoned
early, scan run to exhaustion, DROP INDEX, rebuild, TRUNCATE, back-to-back scans, a failed
statement, a rollback and a restart.

The class's own caveat still applies and is not ours to fix: the streaming search "is not intended
to scan the entire or large part of the index", and nothing refuses a `LIMIT` that does (§31).

---

## 40. Layer draws are not reproducible

`Vec_random_engine` is an adapter over `ut::random_64()`, whose state is
`extern thread_local uint64_t random_seed` (`ut0rnd.h`), seeded per thread. It is therefore
thread-safe without a lock, which the HNSW class requires of its `RandomEngine` once inserts may
run concurrently (`hnsw.h`) and which its default, `std::default_random_engine`, is not.

The consequence is that **the graph is not reproducible across runs**, even for identical
sequential single-threaded inserts. `random_layer()` picks each node's top layer, and that decides
the descent path, which node becomes the entry point, and — through `select_neighbors` pruning —
the neighbour lists of nodes inserted earlier.

So no test may record per-node `level` or `nb`, the entry point (record 0's `base_pk`), or the
order of equidistant kNN hits. Three tests mask those fields. The property they stood in for —
that neighbour lists round-trip through the aux unchanged — is asserted deterministically by
`vector_aux_dml_debug`, which passes explicit levels and never invokes the graph.

Reproducibility was in any case already absent under concurrent insert.

---

## 41. A replica's graph is not the source's, so approximate answers can differ

The aux table is engine-internal (`DICT_TF2_VEC_AUX`), like an FTS aux table, and every write to
it goes through parser-free InnoDB DML on a background transaction. Nothing reaches the SQL
layer, so **the aux is never binlogged**. A replica applies the user's INSERT and builds its own
graph, from its own RNG draws, in its own apply order.

So source and replica hold physically different graphs. For an ordinary index that would be
unremarkable — a B-tree's page layout already differs between the two, and what has to converge
is the *answers*, not the bytes. But an HNSW index is **approximate**, and there the distinction
leaks: the same `ORDER BY DISTANCE(...) LIMIT k` can return a different set of rows on a replica
than on the source.

**Exact access still converges.** The base table replicates normally, so a table scan or a
primary-key lookup is identical on both. Only index-accelerated approximate queries diverge.

**This is not caused by the RNG change (§40).** Two older reasons already guaranteed it:
concurrent inserts produce non-deterministic graphs whatever the RNG does, and a replica applies
transactions with different parallelism and interleaving than the source. What §40 removed was
the last case that *would* have converged — identical, sequential, single-threaded inserts on
both sides.

**The question this raises is a product one, not an engine one.** What do we promise a user who
routes reads to replicas? The option space is small and each choice costs something:

| option | cost |
|---|---|
| replicate the aux table | it is engine-internal and large; and the graph would then have to be applied in lockstep with the base rows |
| make the build deterministic on both sides | needs a seeded RNG **and** identical apply order — which parallel replication defeats anyway |
| accept divergence and document it | a user comparing source and replica sees different rows for the same query |

The third is the only one that survives parallel replication, and it is what the design does
today by default — but it is currently undocumented, which is the actual defect. An ANN index
returning "approximately the k nearest" is entitled to return a different approximation on a
different server; a user who has not been told that will file it as a bug.

**Not investigated:** how MariaDB's MHNSW and upstream MySQL's vector indexes handle this — whether
they replicate the graph, force determinism, or accept divergence. Worth checking before deciding,
since matching an existing convention is worth more than inventing one. Raised for the server
layer (Martin) because the decision is about user-visible semantics, not storage.

