// Copyright 2014 Google

#ifndef btr0scrub_h
#define btr0scrub_h

#include "univ.i"

#include "dict0dict.h"
#include "data0data.h"
#include "page0cur.h"
#include "mtr0mtr.h"
#include "btr0types.h"

/**
 * enum describing page allocation status
 */
enum btr_scrub_page_allocation_status_t {
	BTR_SCRUB_PAGE_FREE,
	BTR_SCRUB_PAGE_ALLOCATED,
	BTR_SCRUB_PAGE_ALLOCATION_UNKNOWN
};

/**
* constants returned by btr_page_needs_scrubbing & btr_scrub_recheck_page
*/
#define BTR_SCRUB_PAGE 1		      /* page should be scrubbed */
#define BTR_SCRUB_SKIP_PAGE 2		      /* no scrub & no action */
#define BTR_SCRUB_SKIP_PAGE_AND_CLOSE_TABLE 3 /* no scrub & close table */
#define BTR_SCRUB_SKIP_PAGE_AND_COMPLETE_SPACE \
	4 /* no scrub & complete space */
#define BTR_SCRUB_TURNED_OFF \
	5 /* we detected that scrubbing
						 was disabled by global
						 variable */

/** struct for keeping scrub statistics. */
struct btr_scrub_stat_t {
	/* page reorganizations */
	ulint page_reorganizations;
	/* page splits */
	ulint page_splits;
	/* scrub failures */
	ulint page_split_failures_underflow;
	ulint page_split_failures_out_of_filespace;
	ulint page_split_failures_missing_index;
	ulint page_split_failures_unknown;
};

/** struct for thread local scrub state. */
struct btr_scrub_t {
	/* current space */
	ulint space;

	/* is scrubbing enabled for this space */
	bool scrubbing;

	/* is current space compressed */
	bool compressed;

	dict_table_t* current_table;
	dict_index_t* current_index;
	/* savepoint for X_LATCH of block */
	ulint savepoint;

	/* statistic counters */
	btr_scrub_stat_t scrub_stat;
};

/** Init scrub global variables */
void
btr_scrub_init();

/** Cleanup scrub globals */
void
btr_scrub_cleanup();

/** Return crypt statistics
@param[out] stat stats to update */
void
btr_scrub_total_stat(btr_scrub_stat_t* stat);

/** Check if a page needs scrubbing
@param[in] 	scrub_data
@param[in]	block
@param[in]	allocated	is block allocated, free or unknown
@return		BTR_SCRUB_PAGE if page should be scrubbed
		else BTR_SCRUB_SKIP_PAGE should be called
		with this return value (and without any latches held) */
int
btr_page_needs_scrubbing(btr_scrub_t* scrub_data, buf_block_t* block,
			 btr_scrub_page_allocation_status_t allocated);

/** Recheck if a page needs scrubbing, and if it does load appropriate
table and index
@param[in] 	scrub_data
@param[in]	block
@param[in]	allocated	is block allocated or free
@param[in]	mtr
@return		BTR_SCRUB_PAGE if page should be scrubbed
		else BTR_SCRUB_SKIP_PAGE should be called
		with this return value (and without any latches held) */
int
btr_scrub_recheck_page(btr_scrub_t* scrub_data, buf_block_t* block,
		       btr_scrub_page_allocation_status_t allocated,
		       mtr_t* mtr);

/** Perform actual scrubbing of page
@param[in] 	scrub_data
@param[in]	block
@param[in]	allocated	is block allocated
@param[in]	mtr
*/
int
btr_scrub_page(btr_scrub_t* scrub_data, buf_block_t* block,
	       btr_scrub_page_allocation_status_t allocated, mtr_t* mtr);

/** Perform cleanup needed for a page not needing scrubbing
@param[in] 	scrub_data
@param[in]	needs_scrubbing */
void
btr_scrub_skip_page(btr_scrub_t* scrub_data, int needs_scrubbing);

/** Start iterating a space
@param[in]	scrub_data
@param[in]	compressed
  @return true if scrubbing is turned on
*/
bool
btr_scrub_start_space(ulint space, btr_scrub_t* scrub_data, bool compressed);

/** Complete iterating a space.
@param[in,out]	scrub_data	 scrub data */
void
btr_scrub_complete_space(btr_scrub_t* scrub_data);

#endif
