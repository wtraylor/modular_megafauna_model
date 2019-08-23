#ifndef DATA_POINT_H
#define DATA_POINT_H

#include <string>
#include "combined_data.h"
#include "date.h"

namespace Fauna {
namespace Output {

/// Simple container for spatially and temporally aggregated output data.
/**
 * \ref CombinedData already holds and aggregates output data, but this class
 * additionally contains the relevant metadata for *what* the data represents.
 */
struct Datapoint {
  /// Identifier of the spatial aggregation unit.
  /**
   * \see Fauna::Habitat::get_aggregation_unit()
   */
  std::string aggregation_unit;

  /// The aggregated output data itself.
  CombinedData data;

  /// First day of the aggregated time interval.
  Date first_day = Date(0,0);

  /// Last day of the aggregated time interval.
  Date last_day = Date(0,0);
};

}  // namespace Output
}  // namespace Fauna

#endif  // DATA_POINT_H
