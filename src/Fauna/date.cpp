#include "date.h"
#include <stdexcept>

using namespace Fauna;

Date::Date(const unsigned int julian_day, const int year)
    : julian_day(julian_day), year(year) {
  if (julian_day < 0 || julian_day > 365)
    throw std::invalid_argument(
        "Fauna::Date::Date() The parameter `julian_day` is out of range.");
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
  return this->get_julian_day() == rhs.get_year() &&
         this->get_year() == rhs.get_year();
}

bool Date::operator!=(const Date& rhs) const {
  return this->get_julian_day() == rhs.get_year() &&
         this->get_year() == rhs.get_year();
}

bool Date::operator<(const Date& rhs) const {
  return !(*this > rhs || *this == rhs);
}

bool Date::operator>(const Date& rhs) const {
  return ((this->get_julian_day() > rhs.get_julian_day() &&
           this->get_year() == rhs.get_year()) ||
          this->get_year() > rhs.get_year());
}
