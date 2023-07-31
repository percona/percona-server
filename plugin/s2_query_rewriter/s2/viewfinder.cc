#include <ctime>
#include <chrono>

#include "s2.h"
#include "s2cap.h"
#include "s2cellid.h"
#include "s2latlng.h"
#include "s2regioncoverer.h"
#include "strings/strutil.h"

namespace {

const double kEarthCircumferenceMeters = 1000 * 40075.017;

// -- Header for function
int GetClosestLevel(double meters);

double DistanceBetweenLocations(double lat1, double lng1, double lat2, double lng2);

std::vector<std::string> IndexCells(double lat, double lng, int min_level, int max_level);
std::vector<std::string> SearchCells(double lat, double lng, double radius, int min_level, int max_level);
// -- Header for function


double EarthMetersToRadians(double meters) {
  return (2 * M_PI) * (meters / kEarthCircumferenceMeters);
}

double RadiansToEarthMeters(double radians) {
  return (radians * kEarthCircumferenceMeters) / (2 * M_PI);
}

string CellToString(const S2CellId& id) {
  return StringPrintf("%d:%s", id.level(), id.ToToken().c_str());
}

}  // namespace

// Returns the cell level with a side most closely matching the
// specified number of meters.
int GetClosestLevel(double meters) {
  return S2::kAvgEdge.GetClosestLevel(EarthMetersToRadians(meters));
}

// Returns the distance between two locations.
double DistanceBetweenLocations(double lat1, double lng1, double lat2, double lng2) {
  S2LatLng latlng1 = S2LatLng::FromDegrees(lat1, lng1);
  S2LatLng latlng2 = S2LatLng::FromDegrees(lat2, lng2);
  return RadiansToEarthMeters(latlng1.GetDistance(latlng2).radians());
}

// Generates a list of cells covering lat,lng between the target
// s2 cell levels (min_level, max_level).
vector<string> IndexCells(double lat, double lng, int min_level, int max_level) {
  S2CellId id(S2CellId::FromLatLng(S2LatLng::FromDegrees(lat, lng)));
  vector<string> v;
  do {
    if (id.level() >= min_level && id.level() <= max_level) {
      v.push_back(CellToString(id));
    }
    id = id.parent();
  } while (id.level() > 0);
  return v;
}

// Generates a list of cells at the target s2 cell levels which cover
// a cap of radius 'radius_meters' with center at lat & lng.
vector<string> SearchCells2(double lat, double lng, double radius_meters, int min_level, int max_level, int max_cells) {
  const double radius_radians = EarthMetersToRadians(radius_meters);
  S2Point const axis = S2LatLng::FromDegrees(lat, lng).Normalized().ToPoint();
  // const S2Cap region = S2Cap::FromAxisHeight( S2LatLng::FromDegrees(lat, lng).Normalized().ToPoint(), (radius_radians * radius_radians) / 2);
  const S2Cap region = S2Cap::FromAxisHeight( axis, (radius_radians * radius_radians) / 2);
  S2RegionCoverer coverer;
  coverer.set_min_level(min_level);
  coverer.set_max_level(max_level);
  coverer.set_max_cells(max_cells);

  vector<S2CellId> covering;
  coverer.GetCovering(region, &covering);
  vector<string> v(covering.size());
  for (size_t i = 0; i < covering.size(); ++i) {
    v[i] = CellToString(covering[i]);
  }

  return v;
}

// Generates a list of cells at the target s2 cell levels which cover
// a cap of radius 'radius_meters' with center at lat & lng.
vector<S2CellId> SearchCells3(double lat, double lng, double radius_meters, int min_level, int max_level, int max_cells) {
  const double radius_radians = EarthMetersToRadians(radius_meters);
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

/**
 *
 * ./viewfinder
 * >> Cell Id for []
 *     >> id 3854134522034812347
 *     >> lv 30
 * 
 * >> Radius Meters : 1000
 * 3854133913249120256 : 15:357ca25fc
 * 3854133918617829376 : 14:357ca261
 * 3854133922916990976 : 19:357ca262004
 * 3854134103301423104 : 13:357ca28c
 * 3854134124776259584 : 14:357ca291
 * 3854134133366194176 : 14:357ca293
 * 3854134141956128768 : 14:357ca295
 * 3854134334155915264 : 15:357ca2c1c
 * 3854134339524624384 : 14:357ca2c3
 * 3854134485553512448 : 14:357ca2e5
 * 3854134494143447040 : 14:357ca2e7
 * 3854134515618283520 : 13:357ca2ec
 * 3854134549978021888 : 13:357ca2f4
 * 3854134580042792960 : 14:357ca2fb
 * 
 * >> Radius Meters : 100
 * 3854134521519669248 : 19:357ca2ed5fc
 * 3854134521590972416 : 17:357ca2ed64
 * 3854134521775521792 : 18:357ca2ed6f
 * 3854134521859407872 : 17:357ca2ed74
 * 3854134521993625600 : 17:357ca2ed7c
 * 3854134522127843328 : 17:357ca2ed84
 * 3854134522211729408 : 18:357ca2ed89
 * 3854134522312392704 : 18:357ca2ed8f
 * 3854134522345947136 : 18:357ca2ed91
 * 3854134522452115456 : 21:357ca2ed9754
 * 3854134522530496512 : 17:357ca2ed9c
 * 3854134543044837376 : 19:357ca2f262c
 * 3854134543468462080 : 17:357ca2f27c
 * 3854134543804006400 : 16:357ca2f29
 * 3854134544089219072 : 18:357ca2f2a1
 * 3854134544122773504 : 18:357ca2f2a3
 * 3854134544563175424 : 19:357ca2f2bd4
 */
int main(int argc, char** argv){
  double lat = 37.566535;
  double lon = 126.977969;
  double radius_meters = 1000;
  int internalQueryS2GeoCoarsestLevel = 0;
  int internalQueryS2GeoFinestLevel = 23;
  int internalQueryS2GeoMaxCells = 20;

  std::cout << ">> Cell Id for []" << std::endl;
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  for(int idx=0; idx<10000; idx++){
    S2CellId id(S2CellId::FromLatLng(S2LatLng::FromDegrees(lat, lon)));
  }
  std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() <<std::endl;
  //std::cout << "    >> id " << id.id() << std::endl;
  //std::cout << "    >> lv " << id.level() << std::endl;

  std::cout << ">> Radius Meters : 1000 " << std::endl;
  std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
  for(int idx=0; idx<100000000; idx++){
    vector<S2CellId> res = SearchCells3(lat, lon, radius_meters, internalQueryS2GeoCoarsestLevel, internalQueryS2GeoFinestLevel, internalQueryS2GeoMaxCells);
  }
  std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1).count() <<std::endl;
  //for (size_t i = 0; i < res.size(); ++i) {
  //  std::cout << res[i].id() << " : " << CellToString(res[i]) << std::endl;
  //}
}
