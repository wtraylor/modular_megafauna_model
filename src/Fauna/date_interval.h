// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Two dates framing a time interval.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_DATE_INTERVAL_H
#define FAUNA_DATE_INTERVAL_H

#include "date.h"

namespace Fauna {
// Forward Declarations
enum class OutputInterval : int;

/// Two dates framing a time interval.
/**
 * The first date must not be after the last date, but they can be the same:
 * that is a one-day interval.
 */
class DateInterval {
 public:
  /// Constructor
  /**
   * \param first First day of the interval.
   * \param last Last day of the interval.
   * \throw std::invalid_argument If first day is after last day.
   */
  DateInterval(const Date first, const Date last);

  /// Expand the time interval just enough to include a new date.
  /**
   * If the new date is already covered by the interval, nothing is changed.
   * \param new_date The new day to be included in the interval.
   */
  void extend(const Date& new_date);

  /// First day of the interval.
  const Date& get_first() const { return first_day; }

  /// Last day of the interval.
  const Date& get_last() const { return last_day; }

  /// Check whether last and first day have the distance of given output
  /// interval.
  bool matches_output_interval(const OutputInterval&) const;

 private:
  Date first_day, last_day;
};
}  // namespace Fauna

#endif  // FAUNA_DATE_INTERVAL_H
