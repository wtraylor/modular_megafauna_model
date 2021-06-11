// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::Date.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "date.h"
#include "catch.hpp"

using namespace Fauna;

TEST_CASE("Fauna::Date", "") {
  CHECK_THROWS(Date(366, 0));
  CHECK_NOTHROW(Date(365, 0));
  CHECK_NOTHROW(Date(0, 0));

  SECTION("next day within one year") {
    for (int year = -3; year <= +3; year++)
      for (int day = 0; day < 365; day++) {
        const Date d1(day, year);
        // If next_day==365, it’s a leap year.
        for (int next_day = 0; next_day < 366; next_day++) {
          const Date d2(next_day, year);
          if (next_day == day + 1)
            CHECK(d1.is_successive(d2));
          else
            CHECK(!d1.is_successive(d2));
        }
      }
  }

  SECTION("next day at year boundary") {
    for (int year = -3; year <= +3; year++)
      for (int next_year = -3; next_year <= +3; next_year++) {
        if (year == next_year)
          continue;  // Within-year succession already checked.
        for (int day = 0; day < 366; day++) {
          const Date d1(day, year);
          for (int next_day = 0; next_day < 366; next_day++) {
            const Date d2(next_day, next_year);
            // Only at the last day, Jan 1st of the directly following
            // year is successive.
            const bool is_successive =
                (next_year == year + 1 && next_day == 0 &&
                 (day == 364 || day == 365));
            CHECK(d1.is_successive(d2) == is_successive);
          }
        }
      }
  }

  SECTION("equal dates") {
    CHECK(Date(0, 0) == Date(0, 0));
    CHECK(Date(1, 0) == Date(1, 0));
    CHECK(Date(1, -1) == Date(1, -1));
  }

  SECTION("unequal dates") {
    CHECK(Date(0, 0) != Date(0, 3));
    CHECK(Date(1, 0) != Date(4, 0));
    CHECK(Date(1, -1) != Date(4, -1));
  }

  SECTION("date A after date B") {
    CHECK(Date(0, 4) > Date(0, 3));
    CHECK(Date(1, 4) > Date(0, 4));
    CHECK(Date(0, 1) > Date(364, 0));
    CHECK(Date(0, 1) > Date(365, 0));
  }

  SECTION("date A before date B") {
    CHECK(Date(0, 2) < Date(0, 3));
    CHECK(Date(0, 4) < Date(1, 4));
    CHECK(Date(365, 0) < Date(0, 1));
    CHECK(Date(364, 0) < Date(0, 1));
  }

  SECTION("get_month()") {
    // January
    CHECK(Date(0, 0).get_month() == 0);
    CHECK(Date(30, 0).get_month() == 0);
    CHECK(Date(30, 0).get_month(true) == 0);

    // February
    CHECK(Date(31, 0).get_month() == 1);
    CHECK(Date(31 + 27, 0).get_month() == 1);
    CHECK(Date(31 + 28, 0).get_month(true) == 1);

    // March
    CHECK(Date(31 + 28, 0).get_month() == 2);
    CHECK(Date(31 + 28 + 31, 0).get_month(true) == 2);
    CHECK(Date(31 + 28 + 31 - 1, 0).get_month() == 2);

    // April
    CHECK(Date(31 + 28 + 31, 0).get_month() == 3);

    // December
    CHECK(Date(364, 0).get_month() == 11);
    CHECK(Date(364, 0).get_month(true) == 11);
    CHECK(Date(365, 0).get_month() == 11);
    CHECK(Date(365, 0).get_month(true) == 11);
  }

  SECTION("get_day_of_month()") {
    // Just check some arbitrary dates

    // Jan 1st
    CHECK(Date(0, 0).get_day_of_month() == 0);
    CHECK(Date(0, 0).get_day_of_month(true) == 0);

    // Feb 2nd
    CHECK(Date(32, 0).get_day_of_month() == 1);
    CHECK(Date(32, 0).get_day_of_month(true) == 1);

    // March 1st
    CHECK(Date(31 + 28, 0).get_day_of_month() == 0);
    CHECK(Date(31 + 28 + 1, 0).get_day_of_month(true) == 0);

    // Dec 31st
    CHECK(Date(364, 0).get_day_of_month() == 30);
    CHECK(Date(365, 0).get_day_of_month() == 30);  // assume leap year
    CHECK(Date(365, 0).get_day_of_month(true) == 30);
  }
}
