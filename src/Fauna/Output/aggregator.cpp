#include "aggregator.h"
#include "date.h"
#include "habitat.h"
#include "herbivore.h"
#include "population_list.h"
#include "simulation_unit.h"
using namespace Fauna;
using namespace Fauna::Output;

void Aggregator::add(const Date& today, const std::string& aggregation_unit,
                     const CombinedData& output) {
  if (datapoints.empty())
    interval = DateInterval(today, today);
  else
    interval.extend(today);
  get_datapoint(aggregation_unit).data.merge(output);
}

Datapoint& Aggregator::get_datapoint(const std::string& agg_unit) {
  int i = 0;
  while (i < datapoints.size() && datapoints[i].aggregation_unit != agg_unit)
    i++;

  if (i < datapoints.size())
    return datapoints[i];
  else {
    Datapoint new_datapoint;
    new_datapoint.aggregation_unit = agg_unit;
    datapoints.push_back(std::move(new_datapoint));
    return datapoints.back();
  }
  assert(false);  // Don’t ever reach this point. We should have returned.
}

const DateInterval& Aggregator::get_interval() const {
  if (datapoints.empty())
    throw std::logic_error(
        "Fauna::Output::Aggregator::get_interval() "
        "No output data has been added yet.");
  return interval;
}

std::vector<Datapoint> Aggregator::retrieve() {
  for (auto& i : datapoints) i.interval = interval;
  std::vector<Datapoint> result;  // Create empty vector.
  std::swap(result, datapoints);  // Use efficient move semantics.
  return result;
}
