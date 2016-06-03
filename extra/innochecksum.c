/*
   Copyright (c) 2005, 2011, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/*
  InnoDB offline file checksum utility.  85% of the code in this file
  was taken wholesale fron the InnoDB codebase.

  The final 15% was originally written by Mark Smith of Danga
  Interactive, Inc. <junior@danga.com>

  Published with a permission.
*/

#include <my_global.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

/* all of these ripped from InnoDB code from MySQL 4.0.22 */
#define UT_HASH_RANDOM_MASK     1463735687
#define UT_HASH_RANDOM_MASK2    1653893711
#define FIL_PAGE_LSN          16 
#define FIL_PAGE_FILE_FLUSH_LSN 26
#define FIL_PAGE_OFFSET     4
#define FIL_PAGE_DATA       38
#define FIL_PAGE_END_LSN_OLD_CHKSUM 8
#define FIL_PAGE_SPACE_OR_CHKSUM 0
#define FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID  34

#define FIL_PAGE_TYPE           24
#define FIL_PAGE_TYPE_FSP_HDR   8       /*!< File space header */
/** Offset of the space header within a file page */
#define FSP_HEADER_OFFSET       38
#define FSP_SPACE_FLAGS         16      /* fsp_space_t.flags, similar to
                                        dict_table_t::flags */
/** Smallest compressed page size */
#define PAGE_ZIP_MIN_SIZE       (1 << 10)

#define DICT_TF_BITS                    6       /*!< number of flag bits */
#define DICT_TF_FORMAT_SHIFT            5       /* file format */
#define DICT_TF_FORMAT_MASK             \
((~(~0 << (DICT_TF_BITS - DICT_TF_FORMAT_SHIFT))) << DICT_TF_FORMAT_SHIFT)
#define DICT_TF_FORMAT_51               0       /*!< InnoDB/MySQL up to 5.1 */
#define DICT_TF_FORMAT_ZIP              1       /*!< InnoDB plugin for 5.1:
                                                compressed tables,
                                                new BLOB treatment */
#define DICT_TF_ZSSIZE_SHIFT            1
#define DICT_TF_ZSSIZE_MASK             (15 << DICT_TF_ZSSIZE_SHIFT)

#define UNIV_PAGE_SIZE_SHIFT_MAX	14
#define UNIV_PAGE_SIZE_MAX	(1 << UNIV_PAGE_SIZE_SHIFT_MAX)

/* command line argument to do page checks (that's it) */
/* another argument to specify page ranges... seek to right spot and go from there */

typedef unsigned long int ulint;

/* innodb function in name; modified slightly to not have the ASM version (lots of #ifs that didn't apply) */
ulint mach_read_from_4(uchar *b)
{
  return( ((ulint)(b[0]) << 24)
          + ((ulint)(b[1]) << 16)
          + ((ulint)(b[2]) << 8)
          + (ulint)(b[3])
          );
}

ulint mach_read_from_2(uchar *b)
{
  return(((ulint)(b[0]) << 8) | (ulint)(b[1]));
}

ulint
ut_fold_ulint_pair(
/*===============*/
            /* out: folded value */
    ulint   n1, /* in: ulint */
    ulint   n2) /* in: ulint */
{
    return(((((n1 ^ n2 ^ UT_HASH_RANDOM_MASK2) << 8) + n1)
                        ^ UT_HASH_RANDOM_MASK) + n2);
}

ulint
ut_fold_binary(
/*===========*/
            /* out: folded value */
    uchar*   str,    /* in: string of bytes */
    ulint   len)    /* in: length */
{
    ulint   i;
    ulint   fold= 0;

    for (i= 0; i < len; i++)
    {
      fold= ut_fold_ulint_pair(fold, (ulint)(*str));

      str++;
    }

    return(fold);
}

static
ulint
buf_calc_page_new_checksum(
/*=======================*/
               /* out: checksum */
    uchar*    page, /* in: buffer page */
    ulint     page_size)
{
    ulint checksum;

    /* Since the fields FIL_PAGE_FILE_FLUSH_LSN and ..._ARCH_LOG_NO
    are written outside the buffer pool to the first pages of data
    files, we have to skip them in the page checksum calculation.
    We must also skip the field FIL_PAGE_SPACE_OR_CHKSUM where the
    checksum is stored, and also the last 8 bytes of page because
    there we store the old formula checksum. */

    checksum= ut_fold_binary(page + FIL_PAGE_OFFSET,
                             FIL_PAGE_FILE_FLUSH_LSN - FIL_PAGE_OFFSET)
            + ut_fold_binary(page + FIL_PAGE_DATA,
                             page_size - FIL_PAGE_DATA
                             - FIL_PAGE_END_LSN_OLD_CHKSUM);
    checksum= checksum & 0xFFFFFFFF;

    return(checksum);
}

ulint
buf_calc_page_old_checksum(
/*=======================*/
               /* out: checksum */
    uchar*    page) /* in: buffer page */
{
    ulint checksum;

    checksum= ut_fold_binary(page, FIL_PAGE_FILE_FLUSH_LSN);

    checksum= checksum & 0xFFFFFFFF;

    return(checksum);
}

/**********************************************************************//**
Calculate the compressed page checksum.
@return	page checksum */
static
ulint
page_zip_calc_checksum(
/*===================*/
  const void*	data,	/*!< in: compressed page */
  ulint		size)	/*!< in: size of compressed page */
{
  /* Exclude FIL_PAGE_SPACE_OR_CHKSUM, FIL_PAGE_LSN,
  and FIL_PAGE_FILE_FLUSH_LSN from the checksum. */

  const Bytef* s= data;
  uLong adler;

  adler= adler32(0L, s + FIL_PAGE_OFFSET,
		 FIL_PAGE_LSN - FIL_PAGE_OFFSET);
  adler= adler32(adler, s + FIL_PAGE_TYPE, 2);
  adler= adler32(adler, s + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID,
		 size - FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);

  return((ulint) adler);
}

static
void
display_format_info(uchar *page)
{
  ulint page_type;
  ulint flags;

  /* Read page type. Pre-5.1.7 InnoDB always have zero in FIL_PAGE_TYPE for the
  first page, later versions initialize it to FIL_PAGE_TYPE_FSP_HDR. */
  page_type= mach_read_from_2(page + FIL_PAGE_TYPE);

  /* Read FSP flags from the page header. */
  flags = mach_read_from_4(page + FSP_HEADER_OFFSET + FSP_SPACE_FLAGS);

  if (!page_type)
  {
    printf("Detected file format: Antelope (pre-5.1.7).\n");
    if (flags != 0) {
      printf("But FSP_SPACE_FLAGS is non-zero: %lu. Corrupted tablespace?\n",
              flags);
    }
  }
  else if (page_type == FIL_PAGE_TYPE_FSP_HDR)
  {
    ulint format = flags & DICT_TF_FORMAT_MASK >> DICT_TF_FORMAT_SHIFT;
    ulint zip_size = ((PAGE_ZIP_MIN_SIZE >> 1)
                      << ((flags & DICT_TF_ZSSIZE_MASK)
                          >> DICT_TF_ZSSIZE_SHIFT));

    if (!flags)
    {
      printf("Detected file format: Antelope (5.1.7 or newer).\n");
    }
    else if (format == DICT_TF_FORMAT_ZIP)
    {
      printf("Detected file format: Barracuda ");
      if (!zip_size)
        printf("(not compressed).\n");
      else
        printf("(compressed with KEY_BLOCK_SIZE=%lu).\n", zip_size);
    }
    else
      printf("Unknown file format: %lu\n", format);
  }
  else
  {
    printf("Bogus FIL_PAGE_TYPE value: %lu. Cannot detect the file format.\n",
           page_type);
  }
}

static
int lsn_match(uchar* p, ulint page_no, ulint page_size, int compressed,
	      int debug) {
  ulint logseq, logseqfield;
  logseq= mach_read_from_4(p + FIL_PAGE_LSN + 4);
  logseqfield= mach_read_from_4(p + page_size - FIL_PAGE_END_LSN_OLD_CHKSUM
				+ 4);
  if (debug) {
    printf("page %lu: log sequence number: first = %lu; second = %lu\n",
	   page_no, logseq, logseqfield);
    if (compressed && (logseq == logseqfield))
      printf("WARNING: lsns should not always match for compressed pages!\n");
  }
  return logseq == logseqfield;
}

static
int checksum_match(uchar* p, ulint page_no, ulint page_size, int compressed,
		   int debug) {
  ulint csum, csumfield, oldcsumfield, oldcsum;
  if (compressed) {
    csum = page_zip_calc_checksum(p, page_size);
    csumfield= mach_read_from_4(p + FIL_PAGE_SPACE_OR_CHKSUM);
    if (debug)
      printf("page %lu: calculated = %lu; recorded = %lu\n", page_no, csum,
	     csumfield);
    return csum == csumfield;
  } else {
    /* check the "stored log sequence numbers" */
    if (!lsn_match(p, page_no, page_size, 0, debug))
    {
      return 0;
    }
    /* check old method of checksumming */
    oldcsum= buf_calc_page_old_checksum(p);
    oldcsumfield= mach_read_from_4(p + page_size
				   - FIL_PAGE_END_LSN_OLD_CHKSUM);
    if (debug)
      printf("page %lu: old style: calculated = %lu; recorded = %lu\n",
	     page_no, oldcsum, oldcsumfield);
    if (oldcsumfield != mach_read_from_4(p + FIL_PAGE_LSN) &&
	oldcsumfield != oldcsum)
    {
      return 0;
    }
    /* now check the new method */
    csum= buf_calc_page_new_checksum(p, page_size);
    csumfield= mach_read_from_4(p + FIL_PAGE_SPACE_OR_CHKSUM);
    if (debug)
      printf("page %lu: new style: calculated = %lu; recorded = %lu\n",
	     page_no, csum, csumfield);
    if (csumfield != 0 && csum != csumfield)
    {
      return 0;
    }
    return 1;
  }
}

static
int find_page_size(FILE *f, ulint *page_size, int *compressed, int debug)
{
  uchar p[UNIV_PAGE_SIZE_MAX];
  size_t bytes;
  ulint flags;
  ulint zip_ssize;
  bytes= fread(p, 1, PAGE_ZIP_MIN_SIZE, f);
  rewind(f);
  if (bytes != PAGE_ZIP_MIN_SIZE) {
    fprintf(stderr, "Error in reading the first %d bytes of the ibd file.\n",
	    PAGE_ZIP_MIN_SIZE);
    return 0;
  }

  flags= mach_read_from_4(p + FIL_PAGE_DATA + FSP_SPACE_FLAGS);
  zip_ssize= (flags & DICT_TF_ZSSIZE_MASK) >> DICT_TF_ZSSIZE_SHIFT;
  if (zip_ssize) { /* table is compressed */
    *compressed= 1;
    *page_size= ((ulint)PAGE_ZIP_MIN_SIZE >> 1) << zip_ssize;
    return 1;
  }

  *compressed= 0;
  *page_size= 16384;

  if (debug)
    printf("checking if page_size is %lu\n", *page_size);
  bytes= fread(p, 1, *page_size, f);
  rewind(f);

  if (bytes != *page_size) {
    fprintf(stderr, "Error in reading the first %lu bytes of the ibd file.\n",
	    *page_size);
    return 0;
  }

  if (checksum_match(p, 0, *page_size, 0, debug)) {
    if (debug)
      printf("table has page size %lu and is uncompressed\n", *page_size);
    return 1;
  }

  fprintf(stderr, "Page size can not be determined for the table\n");
  return 0;
}

int main(int argc, char **argv)
{
  FILE *f;                     /* our input file */
  uchar *p;                     /* storage of pages read */
  size_t bytes;                /* bytes read count */
  ulint ct;                    /* current page number (0 based) */
  int now;                     /* current time */
  int lastt;                   /* last time */
  struct stat st;              /* for stat, if you couldn't guess */
  unsigned long long int size; /* size of file (has to be 64 bits) */
  ulint pages;                 /* number of pages in file */
  ulint start_page= 0, end_page= 0, use_end_page= 0; /* for starting and ending at certain pages */
  off_t offset= 0;
  int just_count= 0;          /* if true, just print page count */
  int display_format= 0;      /* if true, just print format information */
  int verbose= 0;
  int debug= 0;
  int c;
  int fd;
  int compressed= 0;
  ulint page_size= 0;

  /* remove arguments */
  while ((c= getopt(argc, argv, "fcvds:e:p:")) != -1)
  {
    switch (c)
    {
    case 'v':
      verbose= 1;
      break;
    case 'c':
      just_count= 1;
      break;
    case 's':
      start_page= atoi(optarg);
      break;
    case 'e':
      end_page= atoi(optarg);
      use_end_page= 1;
      break;
    case 'p':
      start_page= atoi(optarg);
      end_page= atoi(optarg);
      use_end_page= 1;
      break;
    case 'd':
      debug= 1;
      break;
    case ':':
      fprintf(stderr, "option -%c requires an argument\n", optopt);
      return 1;
      break;
    case 'f':
      display_format= 1;
      break;
    case '?':
      fprintf(stderr, "unrecognized option: -%c\n", optopt);
      return 1;
      break;
    }
  }

  /* debug implies verbose... */
  if (debug) verbose= 1;

  /* make sure we have the right arguments */
  if (optind >= argc)
  {
    printf("InnoDB offline file checksum utility.\n");
    printf("usage: %s [-c] [-s <start page>] [-e <end page>] [-p <page>] [-v] [-d] <filename>\n", argv[0]);
    printf("\t-f\tdisplay information about the file format and exit\n");
    printf("\t-c\tprint the count of pages in the file\n");
    printf("\t-s n\tstart on this page number (0 based)\n");
    printf("\t-e n\tend at this page number (0 based)\n");
    printf("\t-p n\tcheck only this page (0 based)\n");
    printf("\t-v\tverbose (prints progress every 5 seconds)\n");
    printf("\t-d\tdebug mode (prints checksums for each page)\n");
    return 1;
  }

  /* stat the file to get size and page count */
  if (stat(argv[optind], &st))
  {
    perror("error statting file");
    return 1;
  }

  /* open the file for reading */
  f= fopen(argv[optind], "r");
  if (!f)
  {
    perror("error opening file");
    return 1;
  }

  if (!find_page_size(f, &page_size, &compressed, debug)) {
    fprintf(stderr, "error in determining the page size and/or"
	    " whether the table is in compressed format\n");
    return 1;
  }

  if (!display_format)
  {
    printf("Table is %s\n", (compressed ? "compressed" : "not compressed"));
    printf("%s size is %luK\n", (compressed ? "Key block" : "Page"),
	   page_size >> 10);
  }

  size= st.st_size;
  pages= size / page_size;
  if (just_count)
  {
    printf("%lu\n", pages);
    return 0;
  }
  else if (verbose)
  {
    printf("file %s = %llu bytes (%lu pages)...\n", argv[optind], size, pages);
    printf("checking pages in range %lu to %lu\n", start_page, use_end_page ? end_page : (pages - 1));
  }

  /* seek to the necessary position, ignore with -f as we only need to read the
  first page */
  if (start_page && !display_format)
  {
    fd= fileno(f);
    if (!fd)
    {
      perror("unable to obtain file descriptor number");
      return 1;
    }

    offset= (off_t)start_page * (off_t)page_size;

    if (lseek(fd, offset, SEEK_SET) != offset)
    {
      perror("unable to seek to necessary offset");
      return 1;
    }
  }

  /* allocate buffer for reading (so we don't realloc every time) */
  p= (uchar *)malloc(page_size);

  /* main checksumming loop */
  ct= start_page;
  lastt= 0;
  while (!feof(f))
  {
    bytes= fread(p, 1, page_size, f);
    if (!bytes && feof(f))
    {
      free(p);
      return 0;
    }
    if (bytes != page_size)
    {
      fprintf(stderr,
	      "bytes read (%lu) doesn't match universal page size (%lu)\n",
	      (unsigned long)bytes, page_size);
      free(p);
      return 1;
    }

    if (display_format)
    {
      /* for -f, analyze only the first page and exit */
      display_format_info(p);
      free(p);
      return 0;
    }

    if (!checksum_match(p, ct, page_size, compressed, debug))
    {
      fprintf(stderr, "page %lu invalid (fails new style checksum)\n", ct);
      free(p);
      return 1;
    }

    /* end if this was the last page we were supposed to check */
    if (use_end_page && (ct >= end_page))
    {
      free(p);
      return 0;
    }

    /* do counter increase and progress printing */
    ct++;
    if (verbose)
    {
      if (ct % 64 == 0)
      {
        now= time(0);
        if (!lastt) lastt= now;
        if (now - lastt >= 1)
        {
          printf("page %lu okay: %.3f%% done\n", (ct - 1), (float) ct / pages * 100);
          lastt= now;
        }
      }
    }
  }
  free(p);
  return 0;
}

