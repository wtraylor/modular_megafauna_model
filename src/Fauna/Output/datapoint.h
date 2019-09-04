/**
 * \file
 * \brief Spatially and temporally aggregated output data container.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_OUTPUT_DATA_POINT_H
#define FAUNA_OUTPUT_DATA_POINT_H

#include <string>
#include "combined_data.h"
#include "date_interval.h"

namespace Fauna {
namespace Output {

/// Simple container for spatially and temporally aggregated output data.
/**
 * \ref CombinedData already holds and aggregates output data, but this class
 * additionally contains the relevant metadata for *what* the data represent.
 *
 * This struct holds the data from one or several \ref Fauna::SimulationUnit
 * objects that comprise one **aggregation unit**
 * (\ref Fauna::Habitat::get_aggregation_unit). The data are aggregated over a
 * time period given by the \ref interval property.
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
  /**
   * \see Fauna::OutputInterval
   */
  DateInterval interval = DateInterval(Date(0,0), Date(0,0));
};

}  // namespace Output
}  // namespace Fauna

#endif  // FAUNA_OUTPUT_DATA_POINT_H
