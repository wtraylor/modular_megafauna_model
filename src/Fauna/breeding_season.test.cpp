// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::BreedingSeason.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "breeding_season.h"
#include "catch.hpp"
using namespace Fauna;

TEST_CASE("Fauna::BreedingSeason", "") {
  CHECK_THROWS(BreedingSeason(-1, 1));
  CHECK_THROWS(BreedingSeason(365, 1));
  CHECK_THROWS(BreedingSeason(0, 0));
  CHECK_THROWS(BreedingSeason(0, -1));
  CHECK_THROWS(BreedingSeason(0, 366));

  const int START = 100;
  SECTION("check breeding season") {
    const int LENGTH = 90;
    const BreedingSeason b(START, LENGTH);
    // within season
    CHECK(b.is_in_season(START) > 0.0);
    CHECK(b.is_in_season(START + LENGTH) > 0.0);

    // before breeding season
    CHECK(b.is_in_season(START - 1) == 0.0);

    // after season
    CHECK(b.is_in_season(START + LENGTH + 1) == 0.0);
  }

  SECTION("check breeding season extending over year boundary") {
    const int LENGTH2 = 360;
    const BreedingSeason b(START, LENGTH2);
    const int END = (START + LENGTH2) % 365;

    // within season
    CHECK(b.is_in_season(START) > 0.0);
    CHECK(b.is_in_season(END) > 0.0);

    // before breeding season
    CHECK(b.is_in_season(START - 1) == 0.0);

    // after season
    CHECK(b.is_in_season(END + 1) == 0.0);
  }
}
