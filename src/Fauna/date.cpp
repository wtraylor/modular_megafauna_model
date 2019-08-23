#include "date.h"

using namespace Fauna;

Date::Date(const unsigned int julian_day, const int year)
    : julian_day(julian_day), year(year) {
  // TODO
}

bool Date::is_successive(const Date& other_date) const {
  // TODO
  return true;
}
