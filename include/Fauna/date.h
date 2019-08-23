#ifndef DATE_H
#define DATE_H

namespace Fauna {
/// Helper class to hold an absolute simulation day.
class Date {
 public:
  /// Constructor
  /**
   * \param julian_day Day of the year. A value of zero equals January 1st. A
   * value of 365 will only be valid in leap years.
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
  bool is_successive(const Date& other_date) const;

 private:
  const unsigned int julian_day;
  const int year;
};
}  // namespace Fauna

#endif  // DATE_H
