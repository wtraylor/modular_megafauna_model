// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::DateInterval.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "catch.hpp"
#include "date_interval.h"
#include "parameters.h"

using namespace Fauna;

TEST_CASE("Fauna::DateInterval", "") {
  static const int YEAR = 4;  // arbitrary

  SECTION("OutputInterval::extend") {
    static const Date D0(0,4);
    static const Date D1(10, 23); // a little ahead
    static const Date D2(17, 19); // in between
    static const Date D3(17, 0); // a little before
    DateInterval interval(D0,D0);

    interval.extend(D0);
    CHECK(interval.get_first() == interval.get_last());
    CHECK(interval.get_first() == D0);

    interval.extend(D1);
    CHECK(interval.get_first() == D0);
    CHECK(interval.get_last() == D1);

    // no changes
    interval.extend(D2);
    CHECK(interval.get_first() == D0);
    CHECK(interval.get_last() == D1);

    interval.extend(D3);
    CHECK(interval.get_first() == D3);
    CHECK(interval.get_last() == D1);

  }

  SECTION("OutputInterval::Annual") {
    for (int day = 0; day < 366; day++)
      for (int year = YEAR; year < YEAR + 3; year++) {
        const DateInterval interval(Date(0, YEAR), Date(day, year));
        const bool is_not_annual = ((day != 364 && day != 365) || year != YEAR);
        CHECK(interval.matches_output_interval(OutputInterval::Annual) ==
              !is_not_annual);
      }
  }

  // TODO Check for other output interval matches
}
