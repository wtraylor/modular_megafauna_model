// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Two dates framing a time interval.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
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

void DateInterval::extend(const Date& new_date) {
  if (new_date < first_day)
    first_day = new_date;
  else if (new_date > last_day)
    last_day = new_date;
}

bool DateInterval::matches_output_interval(
    const OutputInterval& output_interval) const {
  switch (output_interval) {
    case OutputInterval::Daily:
      return get_first() == get_last();
    case OutputInterval::Monthly:
      // TODO: Implement check for monthly output
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
      // We use the existing Annual check by rewinding the last day by nine
      // years. If the original first and last are separated by a decade, then
      // the first and last-9 are separated by one year.
      const Date decade_earlier(get_last().get_julian_day(),
                                get_last().get_year() - 9);
      if (decade_earlier < get_first())
        return false;
      else
        return DateInterval(get_first(), decade_earlier)
            .matches_output_interval(OutputInterval::Annual);
    }
    default:
      throw std::logic_error(
          "Fauna::DateInterval::matches_output_interval() "
          "OutputInterval member not implemented.");
  }
}
