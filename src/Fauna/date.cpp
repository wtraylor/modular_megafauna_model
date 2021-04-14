// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Simple class to hold the date of a simulation day.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "date.h"
#include <cassert>
#include <stdexcept>

using namespace Fauna;

const std::array<int, 12> Date::MONTH_LENGTH = {
    31,  // Jan
    28,  // Feb
    31,  // Mar
    30,  // Apr
    31,  // May
    30,  // Jun
    31,  // Jul
    31,  // Aug
    30,  // Sep
    31,  // Oct
    30,  // Nov
    31   // Dec
};

const std::array<int, 12> Date::FIRST_OF_MONTH = {
    // Jan
    0,
    // Feb
    MONTH_LENGTH[0],
    // Mar
    MONTH_LENGTH[0] + MONTH_LENGTH[1],
    // Apr
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2],
    // May
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2] + MONTH_LENGTH[3],
    // Jun
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2] + MONTH_LENGTH[3] +
        MONTH_LENGTH[4],
    // Jul
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2] + MONTH_LENGTH[3] +
        MONTH_LENGTH[4] + MONTH_LENGTH[5],
    // Aug
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2] + MONTH_LENGTH[3] +
        MONTH_LENGTH[4] + MONTH_LENGTH[5] + MONTH_LENGTH[6],
    // Sep
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2] + MONTH_LENGTH[3] +
        MONTH_LENGTH[4] + MONTH_LENGTH[5] + MONTH_LENGTH[6] + MONTH_LENGTH[7],
    // Oct
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2] + MONTH_LENGTH[3] +
        MONTH_LENGTH[4] + MONTH_LENGTH[5] + MONTH_LENGTH[6] + MONTH_LENGTH[7] +
        MONTH_LENGTH[8],
    // Nov
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2] + MONTH_LENGTH[3] +
        MONTH_LENGTH[4] + MONTH_LENGTH[5] + MONTH_LENGTH[6] + MONTH_LENGTH[7] +
        MONTH_LENGTH[8] + MONTH_LENGTH[9],
    // Dec
    MONTH_LENGTH[0] + MONTH_LENGTH[1] + MONTH_LENGTH[2] + MONTH_LENGTH[3] +
        MONTH_LENGTH[4] + MONTH_LENGTH[5] + MONTH_LENGTH[6] + MONTH_LENGTH[7] +
        MONTH_LENGTH[8] + MONTH_LENGTH[9] + MONTH_LENGTH[10]};

const std::array<int, 12> Date::FIRST_OF_MONTH_LEAP = {
    FIRST_OF_MONTH[0],       // Jan
    FIRST_OF_MONTH[1],       // February has 29 days
    FIRST_OF_MONTH[2] + 1,   // Mar
    FIRST_OF_MONTH[3] + 1,   // Apr
    FIRST_OF_MONTH[4] + 1,   // May
    FIRST_OF_MONTH[5] + 1,   // Jun
    FIRST_OF_MONTH[6] + 1,   // Jul
    FIRST_OF_MONTH[7] + 1,   // Aug
    FIRST_OF_MONTH[8] + 1,   // Sep
    FIRST_OF_MONTH[9] + 1,   // Oct
    FIRST_OF_MONTH[10] + 1,  // Nov
    FIRST_OF_MONTH[11] + 1   // Dec
};

Date::Date(const unsigned int julian_day, const int year)
    : julian_day(julian_day), year(year) {
  if (julian_day < 0 || julian_day > 365)
    throw std::invalid_argument(
        "Fauna::Date::Date() The parameter `julian_day` is out of range.");
}

unsigned int Date::get_day_of_month(const bool leap_year) const {
  // 31st of December in a leap year.
  if (julian_day == 365) return 30;

  const auto& F = leap_year ? FIRST_OF_MONTH_LEAP : FIRST_OF_MONTH;
  return julian_day - F[get_month()];
}

unsigned int Date::get_month(const bool leap_year) const {
  const auto& F = leap_year ? FIRST_OF_MONTH_LEAP : FIRST_OF_MONTH;

  const unsigned int& d = julian_day;
  if (d < F[1])
    return 0;
  else if (d < F[2])
    return 1;
  else if (d < F[3])
    return 2;
  else if (d < F[4])
    return 3;
  else if (d < F[5])
    return 4;
  else if (d < F[6])
    return 5;
  else if (d < F[7])
    return 6;
  else if (d < F[8])
    return 7;
  else if (d < F[9])
    return 8;
  else if (d < F[10])
    return 9;
  else if (d < F[11])
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
