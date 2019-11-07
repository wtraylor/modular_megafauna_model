/**
 * \file
 * \brief Collect output of over space and time.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_OUTPUT_AGGREGATOR_H
#define FAUNA_OUTPUT_AGGREGATOR_H

#include "datapoint.h"

namespace Fauna {
// Forward Declarations
class Date;

namespace Output {
/// Aggregates output from different simulation units over time.
/**
 * Use this to aggregate output data for every day and from all simulation
 * units until one output interval is completed and the data can be sent to the
 * output writer.
 *
 * All the \ref Datapoint objects in this class have the same date interval
 * because the purpose of this class is to produce *one consistent set* of
 * aggregated data that is ready to be sent to output.
 */
class Aggregator {
 public:
  /// Add output data of one \ref SimulationUnit for completed simulation day.
  /**
   * \param today Date of the given output data.
   * \param aggregation_unit The identifier for spatial aggregation:
   * \ref Fauna::Habitat::get_aggregation_unit().
   * \param output The data from one simulation in given day:
   * \ref Fauna::SimulationUnit::get_output().
   */
  void add(const Date& today, const std::string& aggregation_unit,
           const CombinedData& output);

  /// The time span covered by the currently added data.
  /**
   * \throw std::logic_error If no data have been added yet.
   */
  const DateInterval& get_interval() const;

  /// Get the aggregated data and reset object state.
  /**
   * \return The aggregated data as one datapoint per aggregation unit. All
   * datapoints have the same date interval. If no data added yet, the
   * vector is empty.
   */
  std::vector<Datapoint> retrieve();

 private:
  /// Find the datapoint for a given aggregation unit (create it if missing).
  Datapoint& get_datapoint(const std::string& agg_unit);

  std::vector<Datapoint> datapoints;
  DateInterval interval = DateInterval(Date(0, 0), Date(0, 0));
};
}  // namespace Output
}  // namespace Fauna

#endif  // FAUNA_OUTPUT_AGGREGATOR_H
