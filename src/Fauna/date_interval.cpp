#include "date_interval.h"
#include <algorithm>
#include <stdexcept>

using namespace Fauna;

DateInterval::DateInterval(const Date first, const Date last)
    : first_day(std::move(first)), last_day(std::move(last)) {
  if (last < first)
    throw std::invalid_argument(
        "Fauna::DateInterval::DateInterval() Last day is before first day.");
}
