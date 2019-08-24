#ifndef DATE_INTERVAL_H
#define DATE_INTERVAL_H

#include "date.h"

namespace Fauna {
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

  /// First day of the interval.
  const Date& get_first() const { return first_day; }

  /// Last day of the interval.
  const Date& get_last() const { return last_day; }

 private:
  Date first_day, last_day;
};
}  // namespace Fauna

#endif  // DATE_INTERVAL_H
