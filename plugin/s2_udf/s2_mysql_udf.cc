/**
 * S2 Geometry UDF
 *
 * If you are not familiar with S2 Geometry
 *   https://docs.google.com/presentation/d/1Hl4KapfAENAOf4gv-pSngKwvS_jwNVHRPZTTDzXXn6Q/
 *   http://blog.christianperone.com/2015/08/googles-s2-geometry-on-the-sphere-cells-and-hilbert-curve/
 *   https://code.google.com/archive/p/s2-geometry-library/source
 *
 *
 * ## ---------------------------------------------------------------
 * ## Install s2_udf
 * ## ---------------------------------------------------------------
 * mysql> CREATE FUNCTION S2Cell RETURNS INTEGER SONAME 's2_udf.so';
 * mysql> CREATE FUNCTION S2Distance RETURNS REAL SONAME 's2_udf.so';
 * mysql> CREATE FUNCTION S2Latitude RETURNS REAL SONAME 's2_udf.so';
 * mysql> CREATE FUNCTION S2Longitude RETURNS REAL SONAME 's2_udf.so';
 *
 * ## ---------------------------------------------------------------
 * ## Example s2_udf
 * ## (All udf func use parameters as (latitude, longitude) order)
 * ## ---------------------------------------------------------------
 * mysql> SELECT s2cell(35.198362, 129.053922) as busan;
 * +---------------------+
 * | busan               |
 * +---------------------+
 * | 3848489404846389721 |
 * +---------------------+
 *
 * mysql> SELECT s2cell(37.532600, 127.024612) as seoul;
 * +---------------------+
 * | seoul               |
 * +---------------------+
 * | 3854135263929674215 |
 * +---------------------+
 *
 * mysql> SELECT s2latitude(s2cell(37.532600, 127.024612)) as latitude, s2longitude(s2cell(37.532600, 127.024612)) as longitude;
 * +-------------------+-------------------+
 * | latitude          | longitude         |
 * +-------------------+-------------------+
 * | 37.53260002139435 | 127.0246119770569 |
 * +-------------------+-------------------+
 *
 * mysql> SELECT s2distance(s2cell(37.532600, 127.024612), 35.198362, 129.053922) as distance_meters_from_seoul_to_busan;
 * +-------------------------------------+
 * | distance_meters_from_seoul_to_busan |
 * +-------------------------------------+
 * |                       316815.053085 |
 * +-------------------------------------+
 *
 * ## ---------------------------------------------------------------
 * ## Drop udf
 * ## ---------------------------------------------------------------
 * mysql> DROP FUNCTION S2Cell;
 * mysql> DROP FUNCTION S2Distance;
 * mysql> DROP FUNCTION S2Latitude;
 * mysql> DROP FUNCTION S2Longitude;
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <iostream>
#include <string>
#include <cmath>

#include <mysql.h>
// #include <private/spatial.h>

#include "s2.h"
#include "s2cap.h"
#include "s2cellid.h"
#include "s2latlng.h"
#include "s2regioncoverer.h"
#include "strings/strutil.h"

using namespace std;

#define MYSQL_OK    0
#define MYSQL_ERROR 1

#ifdef __cplusplus
  extern "C" {
#endif

  /**
   * Definition for ::
   *   S2CELLID(latitude DOUBLE, longitude DOUBLE) RETURN BIGINT UNSIGNED
   */
  my_bool S2Cell_init(UDF_INIT* initid, UDF_ARGS* args, char* message);
  void S2Cell_deinit(UDF_INIT* initid);
  my_ulonglong S2Cell(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

  /**
   * Definition for ::
   *   S2DISTANCE(s2cellid BIGINT UNSIGNED, latitude DOUBLE, longitude DOUBLE) RETURN DOUBLE
   */
  my_bool S2Distance_init(UDF_INIT* initid, UDF_ARGS* args, char* message);
  void S2Distance_deinit(UDF_INIT* initid);
  double S2Distance(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

  /**
   * Definition for ::
   *   S2LATITUDE(s2cellid BIGINT UNSIGNED) RETURN DOUBLE
   */
  my_bool S2Latitude_init(UDF_INIT* initid, UDF_ARGS* args, char* message);
  void S2Latitude_deinit(UDF_INIT* initid);
  double S2Latitude(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

  /**
   * Definition for ::
   *   S2LONGITUDE(s2cellid BIGINT UNSIGNED) RETURN DOUBLE
   */
  my_bool S2Longitude_init(UDF_INIT* initid, UDF_ARGS* args, char* message);
  void S2Longitude_deinit(UDF_INIT* initid);
  double S2Longitude(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

#ifdef __cplusplus
  };
#endif

/** --------------------------------------------------------------------------------- */
#define MIN_LAT  -90.0
#define MAX_LAT   90.0
#define MIN_LON -180.0
#define MAX_LON  180.0


/**
 * S2 Default earth radius is 6378137(=40,075,017/M_PI/2) meters.
 * But MySQL 5.7 default earch radius is 6,370,986 meters
 * This code will use earth readius as MySQL default value. so kEarthCircumferenceMeters = 40,030,086 (=6,370,986*M_PI*2)
 */
#define EARTH_RADIUS_METERS 6370986.884258304 // Meters


/**
 * -- S2CELLID ----------------------------------------------------------------------
 */
my_bool S2Cell_init(UDF_INIT* initid, UDF_ARGS* args, char* message){
	if (args->arg_count != 2){
		strncpy(message,"Wrong number of arguments: S2CELL(latitude DOUBLE, longitude DOUBLE) requires two arguments", MYSQL_ERRMSG_SIZE);
		return MYSQL_ERROR;
	}

	if(args->arg_type[0]==INT_RESULT || args->arg_type[0]==DECIMAL_RESULT) args->arg_type[0]=REAL_RESULT;
	if(args->arg_type[1]==INT_RESULT || args->arg_type[1]==DECIMAL_RESULT) args->arg_type[1]=REAL_RESULT;

	if( args->arg_type[0]!=REAL_RESULT || args->arg_type[1]!=REAL_RESULT ){
	    strcpy(message, "Wrong argument type: S2CELL() requires args (DOUBLE, DOUBLE)");
	    return MYSQL_ERROR;
	}

	initid->maybe_null=1;	    /* The result may be null */

	return MYSQL_OK;
}

void S2Cell_deinit(UDF_INIT* initid){
//	if (initid->ptr){
//		free(initid->ptr);
//	}
}

my_ulonglong S2Cell(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error){
	double *longitude, *latitude;

	if(!args->args[0] || !args->args[1]){
		*is_null = 1;
		*error = MYSQL_OK;
		return 0;
	}

	latitude = (double*)args->args[0];
	longitude = (double*)args->args[1];

	if(*latitude <= MAX_LAT && *latitude >= MIN_LAT && *longitude <= MAX_LON && *longitude >= MIN_LON){
		S2CellId _id(S2CellId::FromLatLng(S2LatLng::FromDegrees(*latitude, *longitude)));
		return _id.id();
	}else{
		*is_null = 1;
		*error = MYSQL_ERROR;
		return 0;
	}
}



/**
 * -- S2DISTANCE ----------------------------------------------------------------------
 */
/**
 * Return distance as METERs
 */
double haversine_distance(double fromLatitude, double fromLongitude, double toLatitude, double toLongitude){
	double long1 = -2 * (fromLongitude / 360.0) * M_PI;
	double lat1 = 2 * (fromLatitude / 360.0) * M_PI;
	double long2 = -2 * (toLongitude / 360.0) * M_PI;
	double lat2 = 2 * (toLatitude / 360.0) * M_PI;

	/* compute difference in longitudes - want < 180 degrees */
	double longdiff = fabs(long1 - long2);
	if (longdiff > M_PI) {
		longdiff = (2 * M_PI) - longdiff;
	}

	double sino = sqrt(sin(fabs(lat1 - lat2) / 2.0)
			* sin(fabs(lat1 - lat2) / 2.0)
			+ cos(lat1) * cos(lat2)
			* sin(longdiff / 2.0) * sin(longdiff / 2.0));
	if (sino > 1.){
		sino = 1.0;
	}

	double distance = EARTH_RADIUS_METERS * 2.0 * asin(sino);
	return distance;
}

my_bool S2Distance_init(UDF_INIT* initid, UDF_ARGS* args, char* message){
	if (args->arg_count != 3){
		strncpy(message,"Wrong number of arguments: S2DISTANCE(s2cellid BIGINT UNSIGNED, latitude DOUBLE, longitude DOUBLE) requires three arguments", MYSQL_ERRMSG_SIZE);
		return MYSQL_ERROR;
	}

	if(args->arg_type[1]==INT_RESULT || args->arg_type[1]==DECIMAL_RESULT) args->arg_type[1]=REAL_RESULT;
	if(args->arg_type[2]==INT_RESULT || args->arg_type[2]==DECIMAL_RESULT) args->arg_type[2]=REAL_RESULT;

	if( args->arg_type[0]!=INT_RESULT || args->arg_type[1]!=REAL_RESULT || args->arg_type[2]!=REAL_RESULT ){
	    strcpy(message, "Wrong argument type: S2DISTANCE() requires args (BIGINT, DOUBLE, DOUBLE)");
	    return MYSQL_ERROR;
	}

	initid->maybe_null=1;	    /* The result may be null */

	return MYSQL_OK;
}

void S2Distance_deinit(UDF_INIT* initid){
//	if (initid->ptr){
//		free(initid->ptr);
//	}
}

double S2Distance(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error){
	double *longitude, *latitude;
	unsigned long long s2cellid;

	if(!args->args[0] || !args->args[1] || !args->args[2]){
		*is_null = 1;
		*error = MYSQL_OK;
		return 0;
	}

	s2cellid = *((unsigned long long*)args->args[0]);
	latitude = (double*)args->args[1];
	longitude = (double*)args->args[2];


	if(*latitude <= MAX_LAT && *latitude >= MIN_LAT && *longitude <= MAX_LON && *longitude >= MIN_LON){
		S2CellId origin(s2cellid);
		S2LatLng originLatLng = origin.ToLatLng();
		return haversine_distance(originLatLng.lat().degrees(), originLatLng.lng().degrees(), *latitude, *longitude);
	}else{
		*is_null = 1;
		*error = MYSQL_ERROR;
		return 0.0;
	}
}


/**
 * -- S2LATITUDE ----------------------------------------------------------------------
 */
my_bool S2Latitude_init(UDF_INIT* initid, UDF_ARGS* args, char* message){
	if (args->arg_count != 1){
		strncpy(message,"Wrong number of arguments: S2LATITUDE(s2cellid BIGINT UNSIGNED) requires three arguments", MYSQL_ERRMSG_SIZE);
		return MYSQL_ERROR;
	}

	if( args->arg_type[0]!=INT_RESULT ){
	    strcpy(message, "Wrong argument type: S2LATITUDE() requires args (BIGINT)");
	    return MYSQL_ERROR;
	}

	initid->maybe_null=1;	    /* The result may be null */

	return MYSQL_OK;
}

void S2DLatitude_deinit(UDF_INIT* initid){
//	if (initid->ptr){
//		free(initid->ptr);
//	}
}

double S2Latitude(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error){
	unsigned long long s2cellid;

	if(!args->args[0]){
		*is_null = 1;
		*error = MYSQL_OK;
		return 0;
	}

	s2cellid = *((unsigned long long*)args->args[0]);

	S2CellId origin(s2cellid);
	S2LatLng originLatLon = origin.ToLatLng();
	return originLatLon.lat().degrees();
}


/**
 * -- S2LONGITUDE ----------------------------------------------------------------------
 */
my_bool S2Longitude_init(UDF_INIT* initid, UDF_ARGS* args, char* message){
	if (args->arg_count != 1){
		strncpy(message,"Wrong number of arguments: S2LONGITUDE(s2cellid BIGINT UNSIGNED) requires three arguments", MYSQL_ERRMSG_SIZE);
		return MYSQL_ERROR;
	}

	if( args->arg_type[0]!=INT_RESULT ){
	    strcpy(message, "Wrong argument type: S2LONGITUDE() requires args (BIGINT)");
	    return MYSQL_ERROR;
	}

	initid->maybe_null=1;	    /* The result may be null */

	return MYSQL_OK;
}

void S2DLongitude_deinit(UDF_INIT* initid){
//	if (initid->ptr){
//		free(initid->ptr);
//	}
}

double S2Longitude(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error){
	unsigned long long s2cellid;

	if(!args->args[0]){
		*is_null = 1;
		*error = MYSQL_OK;
		return 0;
	}

	s2cellid = *((unsigned long long*)args->args[0]);

	S2CellId origin(s2cellid);
	S2LatLng originLatLon = origin.ToLatLng();
	return originLatLon.lng().degrees();
}









/**
int main(int argc, char** argv){
  char hash[13];
  double longitude, latitude;
  calculateStringGeoHashFromLongitudeLatitude(126.8900442, 37.5099451, 12, hash);
  calculateLongitudeLatitudeFromStringGeoHash(hash, 12, &longitude, &latitude);
  cout << "test : " << hash << endl;
  cout << "test : " << longitude << ", " << latitude << endl;
  return 1;
}
*/

