#ifndef S2_QREWRITER_REGION_COVER_H_
#define S2_QREWRITER_REGION_COVER_H_


/**
 * Find S2WITHIN block and replace it to SQL predicate
 * If you are not familiar with S2 geometry search,
 *   check this(http://blog.christianperone.com/2015/08/googles-s2-geometry-on-the-sphere-cells-and-hilbert-curve/) first.
 */
void append_s2geometry_condition(std::string& rquery, std::string column_name, double lat, double lng, double radius_meters, int max_cells);

#endif /* S2_QREWRITER_REGION_COVER_H_ */
