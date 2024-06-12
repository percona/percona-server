/**
 * S2 Geometry with-in search (using mysql 5.7 query rewrite plug-in)
 *
 * If you are not familiar with S2 Geometry
 *   https://docs.google.com/presentation/d/1Hl4KapfAENAOf4gv-pSngKwvS_jwNVHRPZTTDzXXn6Q/
 *   http://blog.christianperone.com/2015/08/googles-s2-geometry-on-the-sphere-cells-and-hilbert-curve/
 *   https://code.google.com/archive/p/s2-geometry-library/source
 *
 *
 * ## ---------------------------------------------------------------
 * ## Install s2_query_rewriter
 * ## ---------------------------------------------------------------
 * mysql> INSTALL PLUGIN s2_geometry SONAME 's2_query_rewriter.so';
 *
 *
 * ## ---------------------------------------------------------------
 * ## CREATE sample table
 * ## ---------------------------------------------------------------
 * mysql> CREATE TABLE s2location (
 *          id BIGINT NOT NULL AUTO_INCREMENT,
 *          type ENUM('building', 'subway'),
 *          latitude DOUBLE,  -- // optional (can reverse lat/lon from s2cellid)
 *          longitude DOUBLE, -- // optional (can reverse lat/lon from s2cellid)
 *          s2cellid BIGINT UNSIGNED, -- // Need to be BIGINT UNSIGNED type
 *          name VARCHAR(30),
 *          PRIMARY KEY(id),
 *          INDEX ix_type_s2cellid(type, s2cellid) -- // INDEX for S2WITHIN search
 *        ) ENGINE=InnoDB;
 *
 * mysql> -- // Need to cast S2Cell() result as UNSIGNED BIGINT
 * mysql> INSERT INTO s2location VALUES (NULL, 'building', 37.542773, 127.039735, CAST(S2Cell(37.542773, 127.039735) AS UNSIGNED), 'Galleria foret');
 * mysql> INSERT INTO s2location VALUES (NULL, 'building', 37.538893, 127.044713, CAST(S2Cell(37.538893, 127.044713) AS UNSIGNED), 'Trimaje tower');
 * mysql> INSERT INTO s2location VALUES (NULL, 'subway',   37.547196, 127.047331, CAST(S2Cell(37.547196, 127.047331) AS UNSIGNED), 'Ttuksom subway station');
 * mysql> INSERT INTO s2location VALUES (NULL, 'subway',   37.394908, 127.111109, CAST(S2Cell(37.394908, 127.111109) AS UNSIGNED), 'Pangyo subway station');
 * mysql> INSERT INTO s2location VALUES (NULL, 'building', 37.566669, 126.978428, CAST(S2Cell(37.566669, 126.978428) AS UNSIGNED), 'Seoul city hall');
 *
 * mysql> SELECT * FROM s2location;
 * +----+----------+-----------+------------+---------------------+------------------------+
 * | id | type     | latitude  | longitude  | s2cellid            | name                   |
 * +----+----------+-----------+------------+---------------------+------------------------+
 * |  1 | building | 37.542773 | 127.039735 | 3854135140010626223 | Galleria foret         |
 * |  2 | building | 37.538893 | 127.044713 | 3854136266120187049 | Trimaje tower          |
 * |  3 | subway   | 37.547196 | 127.047331 | 3854136358462807037 | Ttuksom subway station |
 * |  4 | subway   | 37.394908 | 127.111109 | 3853770703657147925 | Pangyo subway station  |
 * |  5 | building | 37.566669 | 126.978428 | 3854134522099437321 | Seoul city hall        |
 * +----+----------+-----------+------------+---------------------+------------------------+
 *
 *
 * ## ---------------------------------------------------------------
 * ## S2WITHIN search example
 * ##   S2WITHIN(s2cellid_column, lat, lon, distance_meters)
 * ## ---------------------------------------------------------------
 * mysql> SELECT * FROM s2location WHERE type='building' and S2WITHIN(s2cellid, 37.547196, 127.047331, 2000);
 * +----+----------+-----------+------------+---------------------+----------------+
 * | id | type     | latitude  | longitude  | s2cellid            | name           |
 * +----+----------+-----------+------------+---------------------+----------------+
 * |  1 | building | 37.542773 | 127.039735 | 3854135140010626223 | Galleria foret |
 * |  2 | building | 37.538893 | 127.044713 | 3854136266120187049 | Trimaje tower  |
 * +----+----------+-----------+------------+---------------------+----------------+
 *
 * ## ---------------------------------------------------------------
 * ## S2WITHIN search example
 * ##   S2WITHIN(s2cellid_column, lat, lon, distance_meters, max_cells)
 * ## ---------------------------------------------------------------
 * mysql> SELECT * FROM s2location WHERE type='building' and S2WITHIN(s2cellid, 37.547196, 127.047331, 2000, 20);
 * +----+----------+-----------+------------+---------------------+----------------+
 * | id | type     | latitude  | longitude  | s2cellid            | name           |
 * +----+----------+-----------+------------+---------------------+----------------+
 * |  1 | building | 37.542773 | 127.039735 | 3854135140010626223 | Galleria foret |
 * |  2 | building | 37.538893 | 127.044713 | 3854136266120187049 | Trimaje tower  |
 * +----+----------+-----------+------------+---------------------+----------------+
 *
 * ## ---------------------------------------------------------------
 * ## S2WITHIN search example
 * ##   S2WITHIN(s2cellid_column, lat, lon, distance_meters, max_cells)
 * ##   and filtering condition to narrow down exact result with S2Distance func
 * ## ---------------------------------------------------------------
 * mysql> SELECT * FROM s2location WHERE type='building' AND S2WITHIN(s2cellid, 37.547196, 127.047331, 2000, 20) AND S2Distance(s2cellid, 37.547196, 127.047331)<=1000;
 * +----+----------+-----------+------------+---------------------+----------------+
 * | id | type     | latitude  | longitude  | s2cellid            | name           |
 * +----+----------+-----------+------------+---------------------+----------------+
 * |  1 | building | 37.542773 | 127.039735 | 3854135140010626223 | Galleria foret |
 * |  2 | building | 37.538893 | 127.044713 | 3854136266120187049 | Trimaje tower  |
 * +----+----------+-----------+------------+---------------------+----------------+
 *
 * ## ---------------------------------------------------------------
 * ## S2WITHIN search optimization
 * ##   S2WITHIN and S2distance will be processed as COVERING-INDEX
 * ## ---------------------------------------------------------------
 * mysql> explain SELECT COUNT(*) FROM s2location WHERE type='building' AND S2WITHIN(s2cellid, 37.547196, 127.047331, 2000, 20) AND S2Distance(s2cellid, 37.547196, 127.047331)<=2000;
 * +...+------+------------------+---------+-------+------+----------+--------------------------+
 * |...| type | key              | key_len | ref   | rows | filtered | Extra                    |
 * +...+------+------------------+---------+-------+------+----------+--------------------------+
 * |...| ref  | ix_type_s2cellid | 2       | const |    7 |    79.03 | Using where; Using index |
 * +...+------+------------------+---------+-------+------+----------+--------------------------+
 *
 *
 * ## ---------------------------------------------------------------
 * ## SHOW WANRINGS will print expanded search query
 * ## ---------------------------------------------------------------
 * mysql> SHOW WARNINGS\G
 *   Level: Note
 *    Code: 1105
 * Message: Query 'SELECT * FROM s2location WHERE type='building' AND S2WITHIN(s2cellid, 37.547196, 127.047331, 2000, 20) AND S2Distance(s2cellid, 37.547196, 127.047331)<=2000' rewritten to 'SELECT * FROM s2location WHERE type='building' AND  (s2cellid BETWEEN 3854134887669825537 AND 3854134888206696447 OR s2cellid BETWEEN 3854134970884816897 AND 3854135185633181695 OR s2cellid BETWEEN 3854135275827494913 AND 3854135277974978559 OR s2cellid BETWEEN 3854136124083535873 AND 3854136126231019519 OR s2cellid BETWEEN 385413 ...
 *
 *
 *
 * TODO
 *   1) Find a way to inform error to client with MySQL ERROR.
 *      (if there's error, current implementation will return different format of result-set)
 *   2) Does not parse if there's comment (both of single and multi line) in S2WITHIN() block.
 *
 * LIMITATIONS
 *   1) Prepared-statement is not supported (Because this plugin will rewrite query before-parse)
 *   2) Not tested with big area search which greater than hemisphere
 *
 */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <my_global.h>
#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <mysql/service_mysql_alloc.h>
#include <my_thread.h> // my_thread_handle needed by mysql_memory.h
#include <mysql/psi/mysql_memory.h>

#include "region_cover.h"


#define PROC_OK                    0 /* Processing OK, this is used both mysql plugin return and s2 query internal, so you don't change this value */

#define ERR_NO_S2CONDITION         1
#define ERR_S2CONDITION_TOO_LONG   2
#define ERR_INVALID_PARAMETERS     3
#define ERR_NOTCLOSED_QUOTMARK     4
#define ERR_NOTCLOSED_COMMENT      5
#define ERR_NOTCLOSED_S2CONDITION  6

#define ERR_INVALID_LATITUDE       7
#define ERR_INVALID_LONGITUDE      8
#define ERR_INVALID_RADIUS         9
#define ERR_INVALID_MAXCELLS       10

/**
 * MySQL Query Rewriter plugin will replace "S2WITHIN(...)" SQL Block of original query
 */
#define S2_QUERY_KEYWORD "S2WITHIN"

/**
 * Usually s2geometry condition will be expanded OR query and it will takes about 250bytes more, So estimate 500 bytes addition
 */
#define S2GEOMETRY_EXTRA_BUFFER_LEN  500




/* instrument the memory allocation */
#ifdef HAVE_PSI_INTERFACE
static PSI_memory_key key_memory_rewrite_s2geometry;

static PSI_memory_info all_rewrite_memory[]={
  { &key_memory_rewrite_s2geometry, "rewrite_s2geometry", 0 }
};

static int plugin_init(MYSQL_PLUGIN){
  const char* category= "sql";
  int count;

  count= array_elements(all_rewrite_memory);
  mysql_memory_register(category, all_rewrite_memory, count);
  return PROC_OK; /* success */
}
#else
#define plugin_init NULL
#define key_memory_rewrite_s2geometry PSI_NOT_INSTRUMENTED
#endif /* HAVE_PSI_INTERFACE */


/**
 * This plugin will
 *   1) find keyword for s2 geometry condition keyword "S2WITHIN(column_name STRING, latitude DOUBLE, longitude DOUBLE, radius_meters DOUBLE, max_cells INTEGER)" in SQL
 *   2) Replace this part with S2 geometry OR condition
 *   3) If some error, this plugin will generate totally different query for informing error to end-user
 *      SELECT 3 as errno, 'ERR_INVALID_PARAMETERS' as errmsg
 *
 *   Example
 *     >> Search all point within 700m radius distance from center(Latitude : 37.402054, Longitude : 127.108791), with max-cells=20
 *
 *     Original Query  :: SELECT .. FROM .. WHERE type='restaurant' AND S2WITHIN(s2location, 37.402054, 127.108791, 700, 20)
 *     Rewritten Query ::
 *       IF(ERROR-ON-PARAMETERS)
 *         SELECT 'error_message' as error;
 *       ELSE
 *         SELECT .. FROM .. WHERE type='restaurant' AND (s2location BETWEEN 3854133912175378433 AND 3854133922921185279
 *                                                        OR s2location BETWEEN 3854134086121553921 AND 3854134146251096063
 *                                                        OR s2location BETWEEN 3854134333082173441 AND 3854134343819591679
 *                                                        OR s2location BETWEEN 3854134481258545153 AND 3854134567157891071
 *                                                        OR s2location BETWEEN 3854134575747825665 AND 3854134584337760255)
 *
 * And you can append custom filter condition to narrow down result
 *     using S2DISTANCE UDF (this function only need s2 cellid column, so this condition can be processed as covering-index if there's index on s2 cellid column).
 *   Example
 *     Original Query  :: SELECT .. FROM .. WHERE type='restaurant' AND S2WITHIN(s2location, latitude, longitude, radius_meters, max_cells) AND S2DISTANCE(s2location, 37.402054, 127.108791)<=700
 *     Rewritten Query ::
 *       IF(ERROR-ON-PARAMETERS)
 *         SELECT 'error_message' as error;
 *       ELSE
 *         SELECT .. FROM .. WHERE type='restaurant' AND (s2location BETWEEN 3854133912175378433 AND 3854133922921185279
 *                                                        OR s2location BETWEEN 3854134086121553921 AND 3854134146251096063
 *                                                        OR s2location BETWEEN 3854134333082173441 AND 3854134343819591679
 *                                                        OR s2location BETWEEN 3854134481258545153 AND 3854134567157891071
 *                                                        OR s2location BETWEEN 3854134575747825665 AND 3854134584337760255)
 *                                                   AND S2DISTANCE(s2location, 37.402054, 127.108791)<=700
 *
 *
 */



/**
 * return char* will point first not space character. (No bounding check)
 *
 * SELECT .. FROM .. WHERE type='K' AND S2WITHIN   (column_name, latitude, longitude, radius_meters, max_cells)
 *                                             ^start                                                         ^end
 *                                                 ^return
 * return : char pointer which point first not space character
 */
const char* consume_space_characters(const char* pos, const char* end){
	const char* curr = pos;
	while(curr<=end && (*curr==' ' || *curr=='\t' || *curr=='\r' || *curr=='\n')){
		curr++;
	}

	if(curr>end) return NULL;
	return curr;
}

/**
 * return char* will point "\r" or "\n" character of end of line
 *
 * SELECT .. FROM .. -- one line comment \n WHERE type='K'  ...
 *                     ^start                               ... ^end
 *                                       ^return
 * return : char pointer which point end character of one line comment (i.e. NewLine like \r or \n)
 */
const char* consume_until_multi_line_comment(const char* pos, const char* end){
	const char* curr = pos;
	for(curr=pos; curr<end; curr++){ // Use "curr<end" becuase we look one character ahead
		if(*curr=='*' && *(curr+1)=='/'){
			return curr+1;
		}
	}

	return NULL;
}

/**
 * return char* will point "/" character of closing multi line comment
 *
 * SELECT .. FROM .. / * multi line comment * / WHERE type='K'  ...
 *                      ^start                                  ... ^end
 *                                            ^return
 * return : char pointer which point end character of multi line comment (i.e. slash character after '*')
 */
const char* consume_until_single_line_comment(const char* pos, const char* end){
	const char* curr = pos;
	for(curr=pos; curr<=end; curr++){ // Use "curr<=end" becuase we don't need to look one character ahead
		if(*curr=='\r' || *curr=='\n'){
			return curr;
		}
	}

	return NULL;
}

/**
 * return char* will point quotation mark (end quot mark, not begin)
 *
 * SELECT .., 'this is single quotation marked literal' as literal FROM ...
 *            ^start                                                    ... ^end
 *                                                    ^return
 * return : char pointer which point end character of multi line comment (i.e. slash character after '*')
 */
const char* consume_until_character(const char* pos, const char* end, char expected_character){
	const char* curr = pos;
	for(curr=pos; curr<=end; curr++){ // Use "curr<=end" becuase we don't need to look one character ahead
		if(*curr==expected_character){
			return curr;
		}
	}

	return NULL;
}

int find_s2geometry_parts(const char* query, const char* query_end, const char** part_start, const char** part_end){
	const char* curr = query;
	const char* start = NULL;
	for(curr=query; curr<=query_end; ){
		// TODO replace fixed char with S2_QUERY_KEYWORD in comparing
		if(start==NULL && (curr+7<query_end) && (*curr=='s' || *curr=='S') && *(curr+1)=='2'){
			if( (*(curr+2)=='w' || *(curr+2)=='W') &&
					(*(curr+3)=='i' || *(curr+3)=='I') &&
					(*(curr+4)=='t' || *(curr+4)=='T') &&
					(*(curr+5)=='h' || *(curr+5)=='H') &&
					(*(curr+6)=='i' || *(curr+6)=='I') &&
					(*(curr+7)=='n' || *(curr+7)=='N') ){
				start = curr;
				curr = consume_space_characters(curr+8, query_end);
				if(*curr != '('){
					// Not sql function format, so this is not s2 query rewriter symbol (ignore)
					start = NULL;
				}
			}
		}else if(start!=NULL && *curr==')'){ // End of s2 query rewriter
			*part_start = start;
			*part_end = curr;
			return PROC_OK;
		}



		else if(*curr=='\''){
			curr = consume_until_character(curr+1, query_end, '\'');
			if(curr==NULL){
				return ERR_NOTCLOSED_QUOTMARK; // Query Error :: Quotation mark is not closed
			}
		}else if(*curr=='"'){
			curr = consume_until_character(curr+1, query_end, '"');
			if(curr==NULL){
				return ERR_NOTCLOSED_QUOTMARK; // Query Error :: Quotation mark is not closed
			}
		}else if(*curr=='-'){
			curr++;
			if(curr<=query_end && *curr=='-'){
				curr = consume_until_single_line_comment(curr+1, query_end);
				if(curr==NULL){
					return ERR_NOTCLOSED_COMMENT; // Query Error :: Not ended single line comment
				}
			}
		}else if(*curr=='/'){
			curr++;
			if(curr<=query_end && *curr=='*'){
				curr = consume_until_multi_line_comment(curr+1, query_end);
				if(curr==NULL){
					return ERR_NOTCLOSED_COMMENT; // Query Error :: Not ended multi line comment
				}
			}
		}
		curr++;
	}

	return ERR_NO_S2CONDITION;
}


/**
 * Remove comments in S2WITHIN block
 * parse_s2geometry_arguments func does not recognize SQL comment
 *
 * TODO Remove all comments in s2 geometry condition part.
 */
void remove_comments(char* ptr){
	// Do nothing - MySQL server will remove pure-comment from query string
}


#define ARG_TOKEN_DELIMITERS ") \t\r\n'\","
/**
 * Parse and extract arguments of S2WITHIN block
 */
int parse_s2geometry_arguments(const char* part_start, const char* part_end, char* column_name, double* latitude, double* longitude, double* radius_meter, int* max_cells){
	char part[256];
	char* token = NULL;
	char* saveptr;
	const char* curr = consume_until_character(part_start, part_end, '(');

	if(part_end - curr >= 256){
		return ERR_S2CONDITION_TOO_LONG; // short of buffer, maybe 256 bytes is too short to buffering this part
	}

	// strtok will modifying original input string (marking NULL terminiation)
	//     So, we need to copy part to another buffer
	strncpy(part, curr+1, (part_end - curr /* including last character of part */));
	part[part_end - curr] = 0;

	// Remove single or multi line comments in s2geometry query part
	remove_comments(part);

	// 1st argument : column_name(required)
	token = strtok_r(part, ARG_TOKEN_DELIMITERS, &saveptr);
	if(token==NULL || strlen(token)<=0){
		// End of part
		return ERR_INVALID_PARAMETERS;
	}
	strncpy(column_name, token, strlen(token)); column_name[strlen(token)]=0;

	// 2nd argument : latitude(required)
	token = strtok_r(NULL, ARG_TOKEN_DELIMITERS, &saveptr);
	if(token==NULL || strlen(token)<=0){
		// End of part
		return ERR_INVALID_PARAMETERS;
	}
	char* next_ptr = NULL; // We don't use this
	*latitude = strtod(token, &next_ptr);
	// -90 <= Latitude <= 90
	if(*latitude>90 || *latitude<-90){
		return ERR_INVALID_LATITUDE;
	}

	// 3rd argument : longitude(required)
	token = strtok_r(NULL, ARG_TOKEN_DELIMITERS, &saveptr);
	if(token==NULL || strlen(token)<=0){
		// End of part
		return ERR_INVALID_PARAMETERS;
	}
	*longitude = strtod(token, &next_ptr);
	// -180 <= Longitude <= 180
	if(*longitude>180 || *longitude<-180){
		return ERR_INVALID_LONGITUDE;
	}

	// 4th argument : radius_meter(required)
	token = strtok_r(NULL, ARG_TOKEN_DELIMITERS, &saveptr);
	if(token==NULL || strlen(token)<=0){
		// End of part
		return ERR_INVALID_PARAMETERS;
	}
	*radius_meter = strtod(token, &next_ptr);
	// 0 < RadiusMeters <= 500 * 1000
	if(*radius_meter<=0 || *radius_meter>500*1000){
		return ERR_INVALID_RADIUS;
	}

	// 5th argument : max_cells(optional)
	token = strtok_r(NULL, ARG_TOKEN_DELIMITERS, &saveptr);
	if(token==NULL || strlen(token)<=0){
		// End of part
		*max_cells = 0; // This is optional, so we done
		return PROC_OK;
	}
	*max_cells = strtol(token, &next_ptr, 10);
	// 0 < max_cells < 200
	if(*max_cells<=0 || *max_cells>200){
		return ERR_INVALID_MAXCELLS;
	}

	return PROC_OK;
}

/**
 * Convert s2 geometry condition to normal mysql condition
 *
 * return
 *   0 : Ok
 *   1 : No s2geometry query
 *   2 : Error
 */
int convert_s2geometry_condition(std::string& rquery, const char* query, int length){
	bool has_s2geometry_cond = false;
	char column_name[64*4+1];
	double latitude, longitude, radius_meter;
	int max_cells;

	const char *part_start=NULL, *part_end=NULL;
	const char* query_start = query;
	const char* query_end = query + length -1;

	int ret;
	bool error_during_parse_arguments = false;

	while((ret=find_s2geometry_parts(query_start, query_end, &part_start, &part_end))==PROC_OK){
		if((ret=parse_s2geometry_arguments(part_start, part_end, column_name, &latitude, &longitude, &radius_meter, &max_cells))==PROC_OK){
			// Expand string capacity
			if(!has_s2geometry_cond){
				if(rquery.capacity()<10){
					rquery.reserve(length + S2GEOMETRY_EXTRA_BUFFER_LEN); // Expand string buffer size to final estimated length to avoid additional memory alloc and copy
				}
				has_s2geometry_cond = true;
			}
			
			rquery.append(query_start, ((const char*)part_start - (const char*)query_start));

			rquery.append(" (");
			append_s2geometry_condition(rquery, column_name, latitude, longitude, radius_meter, max_cells);
			rquery.append(") ");
			// Copy (query_start ~ part_start-1) to target_query
			// Build s2 geometry query to target_query
			query_start = part_end+1;
		}else{
			error_during_parse_arguments = true;
			// PARSE ERROR
			return ret;
		}
	}

	if(error_during_parse_arguments){
		return ret;
	}
	
	if(has_s2geometry_cond==false && ret!=PROC_OK){
		return ret;
	}

	// Copy (part_end+1 ~ query_end) to target_query
	if(part_end!=NULL){
		rquery.append(part_end+1, ((const char*)query_end - (const char*)part_end));
	}

	return PROC_OK;
}


void generate_error_query(int err, char* buffer, int len){
        if(err == ERR_NO_S2CONDITION){
                // This could not happen
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_NO_S2CONDITION' as errmsg", err);
        }else if(err == ERR_S2CONDITION_TOO_LONG){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_S2CONDITION_TOO_LONG' as errmsg", err);
        }else if(err == ERR_INVALID_PARAMETERS){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_INVALID_PARAMETERS' as errmsg", err);
        }else if(err == ERR_NOTCLOSED_QUOTMARK){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_NOTCLOSED_QUOTMARK' as errmsg", err);
        }else if(err == ERR_NOTCLOSED_COMMENT){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_NOTCLOSED_COMMENT' as errmsg", err);
        }else if(err == ERR_NOTCLOSED_S2CONDITION){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_NOTCLOSED_S2CONDITION' as errmsg", err);
        }else if(err == ERR_INVALID_LATITUDE){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_INVALID_LATITUDE' as errmsg", err);
        }else if(err == ERR_INVALID_LONGITUDE){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_INVALID_LONGITUDE' as errmsg", err);
        }else if(err == ERR_INVALID_RADIUS){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_INVALID_RADIUS' as errmsg", err);
        }else if(err == ERR_INVALID_MAXCELLS){
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_INVALID_MAXCELLS' as errmsg", err);
        }else{
                snprintf(buffer, len, "SELECT %d as errno, 'ERR_UNKNOWN' as errmsg", err);
        }
}

static int rewrite_s2geometry_query(MYSQL_THD thd, mysql_event_class_t event_class, const void *event){
	if(event_class == MYSQL_AUDIT_PARSE_CLASS){
		const struct mysql_event_parse *event_parse = static_cast<const struct mysql_event_parse *>(event);
		if (event_parse->event_subclass == MYSQL_AUDIT_PARSE_PREPARSE){
			size_t query_length = event_parse->query.length;
			const char* query = event_parse->query.str;

			std::string rquery;
			size_t estimated_query_len = query_length + S2GEOMETRY_EXTRA_BUFFER_LEN;

			int ret = convert_s2geometry_condition(rquery, query, query_length);

			if(ret==PROC_OK /* Has s2geometry condition and successfully converted */){
				char *rewritten_query = static_cast<char *>(my_malloc(key_memory_rewrite_s2geometry, rquery.length() + 1, MYF(0)));
				strncpy(rewritten_query, rquery.c_str(), rquery.length());
				event_parse->rewritten_query->str = rewritten_query;
				// event_parse->rewritten_query->str = strndup(rquery.c_str(), rquery.length());
				event_parse->rewritten_query->length = rquery.length();

				*((int *)event_parse->flags) |= (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
			}else if(ret==ERR_NO_S2CONDITION/* There's no s2geometry condition - no query changed & oss has nothing */){
				*((int *)event_parse->flags) |= (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_NONE;
			}else{ /* Error */
				char *rewritten_query = static_cast<char *>(my_malloc(key_memory_rewrite_s2geometry, 200 + 1, MYF(0)));
				generate_error_query(ret, rewritten_query, 200);
				event_parse->rewritten_query->str = rewritten_query;
				event_parse->rewritten_query->length = strlen(rewritten_query);

				*((int *)event_parse->flags) |= (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
			}
		}
	}

	return PROC_OK;
}




void parse_test(){
    const char* query = "SELECT a,b,c /* S2WITHIN(...) */, '/** S2WITHIN ( s2loc,1,1,1,1) **/' as b FROM tab WHERE type='K' AND    \t \n \n S2WITHIN  \t\r\n  (    s2location, 11.1111, 22.2222, 1000, 10) GROUP BY x;";
    //char* query = "SELECT a,b,c /* S2WITHIN(...) */, '/** S2WITHIN ( s2loc,1,1,1,1) **/' as b FROM tab WHERE type='K' GROUP BY x -- ;";
    //char* query = "SELECT a,b,c /* S2WITHIN(...) */, '/** S2WITHIN ( s2loc,1,1,1,1) **/' as b FROM tab WHERE type='K' GROUP BY x --";
    //char* query = "SELECT a,b,c /* S2WITHIN(...) */, '/** S2WITHIN ( s2loc,1,1,1,1) **/' as b FROM tab WHERE type='K' GROUP BY x /* .. */";
    //char* query = "SELECT a,b,c /* S2WITHIN(...) */, '/** S2WITHIN ( s2loc,1,1,1,1) **/' as b FROM tab WHERE type='K' GROUP BY x 'du\"mmy'";
    //char* query = "SELECT a,b,c /* S2WITHIN(...) */, '/** S2WITHIN ( s2loc,1,1,1,1) **/' as b FROM tab WHERE type='K' GROUP BY x \"du'mmy\"";
	const char* part_start;
	const char* part_end;
	char  buffer[1024];

	if(find_s2geometry_parts(query, query+strlen(query)-1, &part_start, &part_end)){

		fprintf(stdout, "query : %p\n", query);
		fprintf(stdout, "query_end : %p\n", query+strlen(query)-1);
		fprintf(stdout, "part_start: %p\n", part_start);
		fprintf(stdout, "part_end: %p\n", part_end);

		// *(part_end) = 0;
		strncpy(buffer, part_start, part_end - part_start+1);
		buffer[part_end - part_start+1] = 0;
		fprintf(stdout, "PART :: %s\n", buffer);
	}else{
		fprintf(stdout, "ERROR");
	}
}






/**
 * Audit plugin descriptor
 */
static struct st_mysql_audit rewrite_s2geometry_descriptor = {
  MYSQL_AUDIT_INTERFACE_VERSION,		    /* interface version */
  NULL,					                    /* release_thd()     */
  rewrite_s2geometry_query,			        /* event_notify()    */
  { 0,
    0,
    (unsigned long) MYSQL_AUDIT_PARSE_ALL, }/* class mask	     */
};

/**
 * Plugin descriptor
 */
mysql_declare_plugin(audit_log){
  MYSQL_AUDIT_PLUGIN,	              /* plugin type              */
  &rewrite_s2geometry_descriptor,     /* type specific descriptor */
  "s2_geometry",	                  /* plugin name              */
  "Seonguck Lee (Kakao corp.)",	      /* author                   */
  "Distance search using s2 geometry",/* description  */
  PLUGIN_LICENSE_GPL,	     /* license		          */
  plugin_init,		         /* plugin initializer	  */
  NULL,			             /* plugin deinitializer  */
  0x0002,			         /* version		          */
  NULL,			             /* status variables	  */
  NULL,			             /* system variables	  */
  NULL,			             /* reserverd		      */
  0			                 /* flags			      */
}
mysql_declare_plugin_end;

