#ifndef DATA_POINT_H
#define DATA_POINT_H

#include <string>
#include "combined_data.h"
#include "date.h"
#include "date_interval.h"

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

  /// The time frame that is integrated in the data.
  DateInterval interval = DateInterval(Date(0,0), Date(0,0));
};

}  // namespace Output
}  // namespace Fauna

#endif  // DATA_POINT_H
