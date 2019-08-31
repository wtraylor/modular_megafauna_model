#ifndef DATE_H
#define DATE_H

namespace Fauna {
/// Helper class to hold an absolute simulation day.
class Date {
 public:
  /// Constructor
  /**
   * \param julian_day Day of the year. A value of zero equals January 1st. A
   * value of 365 is valid because it might be a leap year.
   * \param year An arbitrary year number. This could be a calendar year
   * or an abstract simulation year counter.
   * \throw std::invalid_argument If `julian_day` not in interval
   * [0,365].
   */
  Date(const unsigned int julian_day, const int year);

  /// Day of the year (counting from 0 == Jan 1st).
  unsigned int get_julian_day() const { return julian_day; }

  /// The year specified in the constructor.
  int get_year() const { return year; }

  /// Whether another Date object represents the following day.
  /**
   * This assumes a non-leap year: A Julian day of 364 (0==Jan 1st) can be
   * followed by a Julian day of 0. The last day of a leap year (365) will
   * *also* be validly followed by day 0.
   */
  bool is_successive(const Date& other_date) const;

  /// Whether another \ref Date object specifies the *same* day.
  bool operator==(const Date& rhs) const;

  /// Whether another \ref Date object specifies a *different* day.
  bool operator!=(const Date& rhs) const;

  /// Whether another date is *after* this date.
  bool operator<(const Date& rhs) const;

  /// Whether another date is *before* this date.
  bool operator>(const Date& rhs) const;

 private:
  unsigned int julian_day;
  int year;
};
}  // namespace Fauna

#endif  // DATE_H
