#include <stdint.h>
#include <iostream>
#include <sstream>

#include "s2.h"
#include "s2cap.h"
#include "s2cellid.h"
#include "s2latlng.h"
#include "s2regioncoverer.h"
#include "strings/strutil.h"

/**
 * S2 cell id is uint64 integer, and last bit is end-mark flag.
 * So cell id has difference of 2 even they are contiguous cell.
 */
#define S2CELLID_GAP               2

#define S2_DEFAULT_COARSEST_LEVEL  0
#define S2_DEFAULT_FINEST_LEVEL   23
#define S2_DEFAULT_MAX_CELLS      20

/**
 * S2 Default earth radius is 6378137(=40,075,017/M_PI/2) meters.
 * But MySQL 5.7 default earch radius is 6,370,986 meters
 * This code will use earth readius as MySQL default value. so kEarthCircumferenceMeters = 40,030,086 (=6,370,986*M_PI*2)
 */
const double kEarthCircumferenceMeters = 40075.017 * 1000;

double getEarthMetersToRadians(double meters) {
  return (2 * M_PI) * (meters / kEarthCircumferenceMeters);
}


/**
 * Generates a list of cells at the target s2 cell levels which cover
 * a cap of radius 'radius_meters' with center at lat & lng.
 */
vector<S2CellId> findCellToCover(double lat, double lng, double radius_meters, int min_level, int max_level, int max_cells) {
  const double radius_radians = getEarthMetersToRadians(radius_meters);
  S2Point const axis = S2LatLng::FromDegrees(lat, lng).Normalized().ToPoint();
  const S2Cap region = S2Cap::FromAxisHeight( axis, (radius_radians * radius_radians) / 2);

  S2RegionCoverer coverer;
  coverer.set_min_level(min_level);
  coverer.set_max_level(max_level);
  coverer.set_max_cells(max_cells);

  vector<S2CellId> covering;
  coverer.GetCovering(region, &covering);

  return covering;
}

bool isSorted(vector<S2CellId> const& cells) {
	for (size_t i = 0; i + 1 < cells.size(); ++i) {
		if (cells[i + 1] < cells[i]) {
			return false;
		}
	}
	return true;
}


/**
 * Replace S2WITHIN block to SQL predicate
 *
 * 1) Find s2 regions to cover give area
 *    >> 3854133912175378433 ~ 3854133914322862079 3854133914322862081
 *    >> 3854133914322862081 ~ 3854133922912796671 3854133922912796673
 *    >> 3854133922912796673 ~ 3854133922921185279 3854133922921185281
 *
 *    >> 3854134086121553921 ~ 3854134120481292287 3854134120481292289
 *    >> 3854134120481292289 ~ 3854134129071226879 3854134129071226881
 *    >> 3854134129071226881 ~ 3854134137661161471 3854134137661161473
 *    >> 3854134137661161473 ~ 3854134146251096063 3854134146251096065
 *
 *    >> 3854134333082173441 ~ 3854134335229657087 3854134335229657089
 *    >> 3854134335229657089 ~ 3854134343819591679 3854134343819591681
 *
 *    >> 3854134481258545153 ~ 3854134489848479743 3854134489848479745
 *    >> 3854134489848479745 ~ 3854134498438414335 3854134498438414337
 *    >> 3854134498438414337 ~ 3854134532798152703 3854134532798152705
 *    >> 3854134532798152705 ~ 3854134567157891071 3854134567157891073
 *
 *    >> 3854134575747825665 ~ 3854134584337760255 3854134584337760258
 *
 * 2) Merge s2 regions and convert them as SQL predicate (concate with OR)
 *     s2_location BETWEEN 3854133912175378433 AND 3854133922921185279
 *     OR s2_location BETWEEN 3854134086121553921 AND 3854134146251096063
 *     OR s2_location BETWEEN 3854134333082173441 AND 3854134343819591679
 *     OR s2_location BETWEEN 3854134481258545153 AND 3854134567157891071
 *     OR s2_location BETWEEN 3854134575747825665 AND 3854134584337760255
 */
void append_s2geometry_condition(std::string& rquery, std::string column_name, double lat, double lng, double radius_meters, int max_cells){
	bool is_first = true;
	uint64_t min, max;
	// I don't think S2 cell level is useful to end-user.
	// So max cells is only exposure to end-user.
	int revised_max_cells = (max_cells>=1 && max_cells<=200) ? max_cells : S2_DEFAULT_MAX_CELLS;

	vector<S2CellId> res = findCellToCover(lat, lng, radius_meters, S2_DEFAULT_COARSEST_LEVEL, S2_DEFAULT_FINEST_LEVEL, revised_max_cells);

	// TODO Should it be sorted, if not we have to sort it here
	//      But now just return error
	if(!isSorted(res)){
		std::cout << "ERROR :: cell ids are not sorted" << std::endl;
		return;
	}

	// DEBUG print s2 region min max id
	//for (size_t i = 0; i < res.size(); ++i) {
	//	std::cout << "    >> " << res[i].range_min().id() << " ~ " << res[i].range_max().id() << std::endl;
	//}

	for(size_t idx=0; idx<res.size(); ){
		min = res[idx].range_min().id();
		max = res[idx].range_max().id();

		while(++idx < res.size()){
			if(max+S2CELLID_GAP >= res[idx].range_min().id()){
				// This is contiguous cell id, so combine it
				max = res[idx].range_max().id();
			}else{
				break;
			}
		}

		if(is_first){
			is_first = false;
		}else{
			rquery.append(" OR ");
		}
		rquery.append(column_name).append(" BETWEEN ").append(std::to_string(min)).append(" AND ").append(std::to_string(max));
	}
}


/**
 * Test append_s2geometry_condition function
 */
int test_append_s2geometry_condition(int argc, char** argv){
	double lat = 37.566535;
	double lng = 126.977969;
	double radius_meters = 1000;

	int maxCells = -1;
	maxCells = (maxCells>=1 && maxCells<=200) ? maxCells : S2_DEFAULT_MAX_CELLS;

	std::string str;
	append_s2geometry_condition(str, "s2_location", lat, lng, radius_meters, maxCells);

	std::cout << str.c_str() << std::endl;
}
