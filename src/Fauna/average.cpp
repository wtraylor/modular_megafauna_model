/**
 * \file
 * \brief Helper functions/classes to aggregate data.
 * \copyright ...
 * \date 2019
 */
#include "average.h"
#include <cassert>
#include <cmath>
#include <numeric>
#include <stdexcept>

using namespace Fauna;

namespace Fauna {

//////////////////////////////////////////////////////////////////////
// Free Functions
//////////////////////////////////////////////////////////////////////

double average(const double a, const double b, const double weight_a,
               const double weight_b) {
  if (weight_a < 0.0 || weight_b < 0.0)
    throw std::invalid_argument(
        "Fauna::average() "
        "Weight must be >=0.0");
  if (weight_a != weight_a || weight_b != weight_b)
    throw std::invalid_argument(
        "Fauna::average() "
        "Weight is NAN");
  if (weight_a == INFINITY || weight_b == INFINITY)
    throw std::invalid_argument(
        "Fauna::average() "
        "Weight is INFINITY");
  if (weight_a + weight_b == 0.0)
    throw std::invalid_argument(
        "Fauna::average() "
        "Sum of weights is zero.");
  return (a * weight_a + b * weight_b) / (weight_a + weight_b);
}

}  // namespace Fauna

////////////////////////////////////////////////////////////////////////
// PeriodAverage
////////////////////////////////////////////////////////////////////////

PeriodAverage::PeriodAverage(const int count) : count(count) {
  if (count <= 0)
    throw std::invalid_argument(
        "Fauna:: PeriodAverage::PeriodAverage() "
        "Parameter `count` is zero or negative.");
  values.reserve(count);
}

void PeriodAverage::add_value(const double v) {
  assert(current_index < count);
  if (current_index < values.size())
    values[current_index] = v;  // Overwrite existing value.
  else
    values.push_back(v);  // Build up vector in the first round.
  current_index++;
  // Start counting from the beginning again if necessary.
  current_index = current_index % count;
}

double PeriodAverage::get_average() const {
  assert(values.size() <= count);
  if (values.empty())
    throw std::logic_error(
        "Fauna::PeriodAverage::get_average() "
        "No values have been added yet. Cannot build average.");
  const double sum = std::accumulate(values.begin(), values.end(), 0.0);
  return sum / (double)values.size();
}

double PeriodAverage::get_first() const {
  assert(values.size() <= count);
  if (values.empty())
    throw std::logic_error(
        "Fauna::PeriodAverage::get_first() "
        "No values have been added yet.");
  // When the record is filled completely, `current_index` will point to the
  // oldest value, which will be overwritten with the next call of
  // `add_value()`.
  // However, while the record is not filled yet, `current_index` is the array
  // position of the next value to be added, which is not in the array yet. So
  // in that case the first entry in the array is also the oldest one.
  if (current_index < values.size())
    return values[current_index];
  else
    return values[0];
}
