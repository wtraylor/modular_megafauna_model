#include "date_interval.h"
#include <algorithm>
#include <stdexcept>
#include "parameters.h"

using namespace Fauna;

DateInterval::DateInterval(const Date first, const Date last)
    : first_day(std::move(first)), last_day(std::move(last)) {
  if (last < first)
    throw std::invalid_argument(
        "Fauna::DateInterval::DateInterval() Last day is before first day.");
}

bool DateInterval::matches_output_interval(
    const OutputInterval& output_interval) const {
  switch (output_interval) {
    case OutputInterval::Daily:
      return get_first() == get_last();
    case OutputInterval::Monthly:
      // TODO
      throw std::logic_error(
          "Fauna::DateInterval::matches_output_interval() "
          "OutputInterval::Monthly not yet implemented.");
      return false;
    case OutputInterval::Annual:
      if (get_first().get_year() == get_last().get_year())
        return get_first().get_julian_day() == 0 &&
               get_last().get_julian_day() >= 364;
      else if (get_first().get_year() == get_last().get_year() + 1)
        return get_first().get_julian_day() == get_last().get_julian_day() + 1;
      else
        return false;
    case OutputInterval::Decadal: {
      // We use the existing Annual check by rewinding the last day by ten
      // years.
      const Date decade_earlier(get_last().get_julian_day(),
                                get_last().get_year() - 10);
      return DateInterval(get_first(), decade_earlier)
          .matches_output_interval(OutputInterval::Annual);
    }
    default:
      throw std::logic_error(
          "Fauna::DateInterval::matches_output_interval() "
          "OutputInterval member not implemented.");
  }
}
