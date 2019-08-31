#include "date.h"
#include <stdexcept>

using namespace Fauna;

Date::Date(const unsigned int julian_day, const int year)
    : julian_day(julian_day), year(year) {
  if (julian_day < 0 || julian_day > 365)
    throw std::invalid_argument(
        "Fauna::Date::Date() The parameter `julian_day` is out of range.");
}

unsigned int Date::get_month(const bool leap_year) const {

  static const auto& M = MONTH_LENGTH;

  // Cumulative month lengths.
  static const std::array<int, 11> CL = {
      M[0],
      M[0] + M[1],
      M[0] + M[1] + M[2],
      M[0] + M[1] + M[2] + M[3],
      M[0] + M[1] + M[2] + M[3] + M[4],
      M[0] + M[1] + M[2] + M[3] + M[4] + M[5],
      M[0] + M[1] + M[2] + M[3] + M[4] + M[5] + M[6],
      M[0] + M[1] + M[2] + M[3] + M[4] + M[5] + M[6] + M[7],
      M[0] + M[1] + M[2] + M[3] + M[4] + M[5] + M[6] + M[7] + M[8],
      M[0] + M[1] + M[2] + M[3] + M[4] + M[5] + M[6] + M[7] + M[8] + M[9],
      M[0] + M[1] + M[2] + M[3] + M[4] + M[5] + M[6] + M[7] + M[8] + M[9] +
          M[10]};

  const unsigned int& d = julian_day;
  if (d < CL[0])
    return 0;
  else if (d < CL[1] + leap_year)
    return 1;
  else if (d < CL[2] + leap_year)
    return 2;
  else if (d < CL[3] + leap_year)
    return 3;
  else if (d < CL[4] + leap_year)
    return 4;
  else if (d < CL[5] + leap_year)
    return 5;
  else if (d < CL[6] + leap_year)
    return 6;
  else if (d < CL[7] + leap_year)
    return 7;
  else if (d < CL[8] + leap_year)
    return 8;
  else if (d < CL[9] + leap_year)
    return 9;
  else if (d < CL[10] + leap_year)
    return 10;
  else
    return 11;
}

bool Date::is_successive(const Date& other_date) const {
  // One day follows within one year.
  if ((other_date.get_year() == this->get_year()) &&
      (other_date.get_julian_day() == this->get_julian_day() + 1))
    return true;
  // We step from December 31st to January 1st.
  if ((other_date.get_year() == this->get_year() + 1) &&
      (other_date.get_julian_day() == 0) &&
      (this->get_julian_day() == 364 || this->get_julian_day() == 365))
    return true;
  return false;
}

bool Date::operator==(const Date& rhs) const {
  return this->get_julian_day() == rhs.get_julian_day() &&
         this->get_year() == rhs.get_year();
}

bool Date::operator!=(const Date& rhs) const {
  return this->get_julian_day() != rhs.get_julian_day() ||
         this->get_year() != rhs.get_year();
}

bool Date::operator<(const Date& rhs) const {
  return !(*this > rhs || *this == rhs);
}

bool Date::operator>(const Date& rhs) const {
  return ((this->get_julian_day() > rhs.get_julian_day() &&
           this->get_year() == rhs.get_year()) ||
          this->get_year() > rhs.get_year());
}
